#include <WiFi.h>
#include <WebServer.h>
#include <LiquidCrystal_I2C.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// ================= WiFi CREDENTIALS =================
const char* ssid = "Sarah's A16";        // Change this
const char* password = "sarah124"; // Change this

WebServer server(80);
LiquidCrystal_I2C lcd(0x27, 16, 2);  

#define FLAME1 35
#define FLAME2 32
#define FLAME3 33
#define FLAME4 25
#define FLAME5 23
#define MQ2_PIN 34
#define BUZZER 15
#define IN1 27
#define IN2 26
#define ENA 14
#define RGB_R 18
#define RGB_G 19
#define ONE_WIRE_BUS 4

// ================= SMOKE THRESHOLDS =================
#define SMOKE_NONE 1400
#define SMOKE_LOW  2000

// ================= MOTOR SPEEDS =================
#define MOTOR_OFF  0
#define MOTOR_LOW  200
#define MOTOR_MAX  255

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature tempSensor(&oneWire);

// Global variables for sensor readings
int s1, s2, s3, s4, s5;
int gasValue;
int temperature;
int motorSpeed;
bool fireDetected;
bool gasDetected;
bool Smoke;
bool danger;
unsigned long lastLcdUpdate = 0;
unsigned long lastSerialUpdate = 0;
int lcdPage = 0;

void setup() {
  Serial.begin(115200);
  
  lcd.init();
  lcd.backlight();
  tempSensor.begin();
  
  pinMode(FLAME1, INPUT);
  pinMode(FLAME2, INPUT);
  pinMode(FLAME3, INPUT);
  pinMode(FLAME4, INPUT);
  pinMode(FLAME5, INPUT);
  pinMode(BUZZER, OUTPUT);
  digitalWrite(BUZZER, LOW);
  
  pinMode(RGB_R, OUTPUT);
  pinMode(RGB_G, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(ENA, OUTPUT);
  
  // Motor direction (fixed forward)
  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);
  
  // Connect to WiFi
  Serial.println("=========================");
  Serial.print("Connecting to: ");
  Serial.println(ssid);
  Serial.println("=========================");
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  
  lcd.setCursor(0, 0);
  lcd.print("Connecting WiFi");
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 40) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  Serial.println();
  
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("=========================");
    Serial.println("WiFi Connection FAILED!");
    Serial.println("=========================");
    Serial.println("Check:");
    Serial.println("1. WiFi name is correct");
    Serial.println("2. Password is correct");
    Serial.println("3. WiFi is 2.4GHz (not 5GHz)");
    Serial.println("4. Router is working");
    Serial.println("=========================");
    
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("WiFi Failed!");
    lcd.setCursor(0, 1);
    lcd.print("Check Serial");
    
    while(1) { delay(1000); } // Stop here
  }
  
  Serial.println();
  Serial.println("=========================");
  Serial.println("WiFi Connected!");
  Serial.println("=========================");
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("WiFi Connected");
  lcd.setCursor(0, 1);
  lcd.print(WiFi.localIP());
  delay(3000);
  
  // Setup web server routes
  server.on("/", handleRoot);
  server.on("/data", handleData);
  server.begin();
  
  Serial.println("Web server started");
  Serial.println("=========================");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  Serial.println("=========================");
  Serial.println("Open this in your browser!");
  Serial.println();
}

void loop() {
  server.handleClient();
  
  // Read all sensors
  s1 = digitalRead(FLAME1);
  s2 = digitalRead(FLAME2);
  s3 = digitalRead(FLAME3);
  s4 = digitalRead(FLAME4);
  s5 = digitalRead(FLAME5);
  gasValue = analogRead(MQ2_PIN);
  
  fireDetected = (s1 == 1 || s2 == 1 || s3 == 1 || s4 == 1 || s5 == 1);
  Smoke = gasValue > SMOKE_LOW;
  gasDetected = gasValue > SMOKE_NONE;
  danger = fireDetected || gasDetected;
  
  tempSensor.requestTemperatures();
  temperature = tempSensor.getTempCByIndex(0);
  
  motorSpeed = MOTOR_OFF;
  
  if (fireDetected) {
    digitalWrite(BUZZER, HIGH);
    digitalWrite(RGB_R, HIGH);
    digitalWrite(RGB_G, LOW);
    motorSpeed = MOTOR_MAX;
  } else if (Smoke) {
    digitalWrite(BUZZER, HIGH);
    digitalWrite(RGB_R, HIGH);
    digitalWrite(RGB_G, LOW); 
    motorSpeed = MOTOR_MAX; 
    } else if (gasDetected) {
    digitalWrite(BUZZER, LOW);
    digitalWrite(RGB_R, HIGH);
    digitalWrite(RGB_G, LOW); 
    motorSpeed = MOTOR_LOW;
  } else {
    digitalWrite(BUZZER, LOW);
    digitalWrite(RGB_R, LOW);
    digitalWrite(RGB_G, HIGH);
    motorSpeed = MOTOR_OFF;
  }
  
  analogWrite(ENA, motorSpeed);
  
  // Print to Serial Monitor every 1 second
  if (millis() - lastSerialUpdate >= 1000) {
    lastSerialUpdate = millis();
    
    Serial.println("=========================");
    Serial.print("IP Address: "); Serial.println(WiFi.localIP());
    Serial.println("=========================");
    Serial.print("D1: "); Serial.print(s1);
    Serial.print(" | D2: "); Serial.print(s2);
    Serial.print(" | D3: "); Serial.print(s3);
    Serial.print(" | D4: "); Serial.print(s4);
    Serial.print(" | D5: "); Serial.println(s5);
    
    Serial.print("MQ-2 Gas Value: "); Serial.println(gasValue);
    
    Serial.print("Flame: ");
    Serial.print(fireDetected ? "🔥 YES" : "No");
    Serial.print(" | Gas: ");
    Serial.print(gasDetected ? "💨 YES" : "No");
    Serial.print(" | Danger: ");
    Serial.println(danger ? "⚠️ ACTIVE" : "Safe");
    
    Serial.print("Temperature: "); Serial.print(temperature); Serial.println("°C");
    Serial.print("Fan Speed: ");
    if (motorSpeed == MOTOR_OFF) Serial.println("OFF (0)");
    else if (motorSpeed == MOTOR_LOW) Serial.println("LOW (200)");
    else Serial.println("MAX (255)");
    Serial.println("=========================");
    Serial.println();
  }
  
  // Update LCD every 1500ms instead of blocking with delay
  if (millis() - lastLcdUpdate >= 1500) {
    lastLcdUpdate = millis();
    
    if (lcdPage == 0) {
      // ================= LCD PAGE 1 =================
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("F:");
      lcd.print(s1); lcd.print(" ");
      lcd.print(s2); lcd.print(" ");
      lcd.print(s3); lcd.print(" ");
      lcd.print(s4); lcd.print(" ");
      lcd.print(s5);
      lcd.setCursor(0, 1);
      lcd.print("Gas:");
      lcd.print(gasValue);
      lcd.print(" Fan:");
      if (motorSpeed == MOTOR_OFF) lcd.print("OFF");
      else if (motorSpeed == MOTOR_LOW) lcd.print("LOW");
      else lcd.print("MAX");
      
      lcdPage = 1;
    } else {
      // ================= LCD PAGE 2 =================
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Flame:");
      lcd.print(fireDetected ? "YES" : "NO");
      lcd.print(" Gas:");
      lcd.print(gasDetected ? "YES" : "NO");
      
      lcd.setCursor(0, 1);
      lcd.print("T:"); lcd.print(temperature);
      lcd.print(" STAT:");
      lcd.print(danger ? "DANGER" : "SAFE");
      
      lcdPage = 0;
    }
  }
  
  // Small delay to prevent overwhelming the system
  delay(50);
}

void handleRoot() {
  String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>Fire Detection System</title>
  <style>
    * {
      margin: 0;
      padding: 0;
      box-sizing: border-box;
    }
    body {
      font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
      background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
      min-height: 100vh;
      padding: 20px;
    }
    .container {
      max-width: 600px;
      margin: 0 auto;
    }
    h1 {
      color: white;
      text-align: center;
      margin-bottom: 30px;
      font-size: 28px;
      text-shadow: 2px 2px 4px rgba(0,0,0,0.3);
    }
    .status-card {
      background: white;
      border-radius: 15px;
      padding: 25px;
      margin-bottom: 20px;
      box-shadow: 0 10px 30px rgba(0,0,0,0.3);
    }
    .alert-banner {
      padding: 20px;
      border-radius: 10px;
      text-align: center;
      font-size: 24px;
      font-weight: bold;
      margin-bottom: 20px;
      animation: pulse 1.5s ease-in-out infinite;
    }
    .alert-danger {
      background: #ff4444;
      color: white;
    }
    .alert-safe {
      background: #00C851;
      color: white;
    }
    @keyframes pulse {
      0%, 100% { transform: scale(1); }
      50% { transform: scale(1.05); }
    }
    .sensor-grid {
      display: grid;
      grid-template-columns: repeat(2, 1fr);
      gap: 15px;
      margin-top: 20px;
    }
    .sensor-box {
      background: #f8f9fa;
      padding: 15px;
      border-radius: 10px;
      text-align: center;
      border: 2px solid #e0e0e0;
    }
    .sensor-label {
      font-size: 14px;
      color: #666;
      margin-bottom: 8px;
    }
    .sensor-value {
      font-size: 24px;
      font-weight: bold;
      color: #333;
    }
    .flame-indicators {
      display: flex;
      justify-content: space-around;
      margin: 20px 0;
    }
    .flame-dot {
      width: 50px;
      height: 50px;
      border-radius: 50%;
      display: flex;
      align-items: center;
      justify-content: center;
      font-weight: bold;
      color: white;
      font-size: 18px;
    }
    .flame-active {
      background: #ff4444;
      box-shadow: 0 0 20px #ff4444;
    }
    .flame-inactive {
      background: #28a745;
    }
    .loading {
      text-align: center;
      color: white;
      font-size: 18px;
    }
  </style>
</head>
<body>
  <div class="container">
    <h1>🔥 Fire Detection System</h1>
    
    <div id="alert" class="alert-banner alert-safe">SYSTEM SAFE</div>
    
    <div class="status-card">
      <h2 style="margin-bottom: 15px; color: #333;">Flame Sensors</h2>
      <div class="flame-indicators" id="flameIndicators">
        <div class="flame-dot flame-inactive">1</div>
        <div class="flame-dot flame-inactive">2</div>
        <div class="flame-dot flame-inactive">3</div>
        <div class="flame-dot flame-inactive">4</div>
        <div class="flame-dot flame-inactive">5</div>
      </div>
    </div>
    
    <div class="status-card">
      <div class="sensor-grid">
        <div class="sensor-box">
          <div class="sensor-label">🔥 Fire Detected</div>
          <div class="sensor-value" id="fireStatus">NO</div>
        </div>
        <div class="sensor-box">
          <div class="sensor-label">💨 Gas Detected</div>
          <div class="sensor-value" id="gasStatus">NO</div>
        </div>
        <div class="sensor-box">
          <div class="sensor-label">🌡️ Temperature</div>
          <div class="sensor-value" id="temperature">--°C</div>
        </div>
        <div class="sensor-box">
          <div class="sensor-label">💨 Gas Value</div>
          <div class="sensor-value" id="gasValue">----</div>
        </div>
        <div class="sensor-box">
          <div class="sensor-label">🌀 Fan Speed</div>
          <div class="sensor-value" id="fanSpeed">OFF</div>
        </div>
        <div class="sensor-box">
          <div class="sensor-label">⚡ Motor PWM</div>
          <div class="sensor-value" id="motorPWM">0</div>
        </div>
      </div>
    </div>
  </div>

  <script>
    function updateData() {
      fetch('/data')
        .then(response => response.json())
        .then(data => {
          // Update alert banner
          const alert = document.getElementById('alert');
          if (data.danger) {
            alert.className = 'alert-banner alert-danger';
            alert.textContent = '⚠️ DANGER DETECTED ⚠️';
          } else {
            alert.className = 'alert-banner alert-safe';
            alert.textContent = '✓ SYSTEM SAFE';
          }
          
          // Update flame indicators
          const flames = [data.f1, data.f2, data.f3, data.f4, data.f5];
          const container = document.getElementById('flameIndicators');
          container.innerHTML = flames.map((val, i) => 
            `<div class="flame-dot ${val == 1 ? 'flame-active' : 'flame-inactive'}">${i+1}</div>`
          ).join('');
          
          // Update sensor values
          document.getElementById('fireStatus').textContent = data.fire ? 'YES' : 'NO';
          document.getElementById('fireStatus').style.color = data.fire ? '#ff4444' : '#28a745';
          
          document.getElementById('gasStatus').textContent = data.gas ? 'YES' : 'NO';
          document.getElementById('gasStatus').style.color = data.gas ? '#ff4444' : '#28a745';
          
          document.getElementById('temperature').textContent = data.temp + '°C';
          document.getElementById('gasValue').textContent = data.gasVal;
          
          let fanText = 'OFF';
          if (data.motor == 255) fanText = 'MAX';
          else if (data.motor == 200) fanText = 'LOW';
          document.getElementById('fanSpeed').textContent = fanText;
          document.getElementById('motorPWM').textContent = data.motor;
        })
        .catch(err => console.error('Error:', err));
    }
    
    // Update every 2 seconds
    setInterval(updateData, 2000);
    updateData(); // Initial update
  </script>
</body>
</html>
)rawliteral";
  
  server.send(200, "text/html", html);
}

void handleData() {
  String json = "{";
  json += "\"f1\":" + String(s1) + ",";
  json += "\"f2\":" + String(s2) + ",";
  json += "\"f3\":" + String(s3) + ",";
  json += "\"f4\":" + String(s4) + ",";
  json += "\"f5\":" + String(s5) + ",";
  json += "\"gasVal\":" + String(gasValue) + ",";
  json += "\"temp\":" + String(temperature) + ",";
  json += "\"motor\":" + String(motorSpeed) + ",";
  json += "\"fire\":" + String(fireDetected ? "true" : "false") + ",";
  json += "\"gas\":" + String(gasDetected ? "true" : "false") + ",";
  json += "\"danger\":" + String(danger ? "true" : "false");
  json += "}";
  
  server.send(200, "application/json", json);
}
