# 🔥 Smart IoT Fire & Smoke Detection System

A real-time fire and smoke detection system built with **ESP32**, featuring a live web dashboard, multi-sensor monitoring, automated fan control, and instant visual/audio alerts.

---

## 📋 Table of Contents

- [Overview](#overview)
- [Features](#features)
- [Hardware Components](#hardware-components)
- [Circuit Connections](#circuit-connections)
- [Software & Libraries](#software--libraries)
- [How It Works](#how-it-works)
- [Web Dashboard](#web-dashboard)
- [Setup & Installation](#setup--installation)
- [Project Files](#project-files)

---

## Overview

This system continuously monitors the environment using **5 flame sensors**, an **MQ-2 gas/smoke sensor**, and a **DS18B20 temperature sensor**. When danger is detected, it triggers a buzzer alarm, changes an RGB LED to red, and spins a fan at full speed — all while broadcasting live data to a web page hosted directly on the ESP32.

---

## ✨ Features

- 🔥 **5-Zone Flame Detection** — Independent flame sensors covering multiple directions
- 💨 **Smoke & Gas Monitoring** — MQ-2 sensor with configurable threshold levels
- 🌡️ **Temperature Sensing** — Real-time readings via DS18B20 (Dallas OneWire)
- 🌐 **Built-in Web Server** — Live dashboard accessible from any device on the same WiFi
- 🔊 **Buzzer Alarm** — Immediate audio alert on fire or heavy smoke
- 🌈 **RGB LED Indicator** — Green = Safe, Red = Danger
- 🌀 **Automatic Fan Control** — Variable speed (OFF / LOW / MAX) based on threat level
- 📟 **LCD Display** — Scrolls through sensor data in real time
- 📡 **JSON API Endpoint** — `/data` route returns sensor readings for integration

---

## 🛠️ Hardware Components

| Component | Quantity | Purpose |
|---|---|---|
| ESP32 Dev Board | 1 | Main microcontroller + WiFi |
| Flame Sensor (IR) | 5 | Fire detection (5 zones) |
| MQ-2 Gas Sensor | 1 | Smoke & gas detection |
| DS18B20 Temperature Sensor | 1 | Ambient temperature |
| I2C LCD Display (16×2) | 1 | Local status display |
| RGB LED | 1 | Visual status indicator |
| Buzzer | 1 | Audio alarm |
| DC Fan + L298N Motor Driver | 1 | Ventilation/exhaust |
| Resistors, wires, breadboard | — | Wiring |

---

## 🔌 Circuit Connections

| Pin | Component |
|---|---|
| GPIO 35 | Flame Sensor 1 |
| GPIO 32 | Flame Sensor 2 |
| GPIO 33 | Flame Sensor 3 |
| GPIO 25 | Flame Sensor 4 |
| GPIO 23 | Flame Sensor 5 |
| GPIO 34 | MQ-2 Gas Sensor (Analog) |
| GPIO 4 | DS18B20 Temperature Sensor |
| GPIO 15 | Buzzer |
| GPIO 18 | RGB LED — Red |
| GPIO 19 | RGB LED — Green |
| GPIO 27 | Motor Driver IN1 |
| GPIO 26 | Motor Driver IN2 |
| GPIO 14 | Motor Driver ENA (PWM) |
| SDA / SCL | I2C LCD (address 0x27) |

---

## 📚 Software & Libraries

Install these libraries in the **Arduino IDE Library Manager**:

- `WiFi.h` *(built-in with ESP32 board package)*
- `WebServer.h` *(built-in with ESP32 board package)*
- `LiquidCrystal_I2C` by Frank de Brabander
- `OneWire` by Paul Stoffregen
- `DallasTemperature` by Miles Burton

**Board:** ESP32 Dev Module  
**Upload Speed:** 115200 baud

---

## ⚙️ How It Works

### Detection Logic

| Condition | Buzzer | LED | Fan Speed |
|---|---|---|---|
| All clear | OFF | 🟢 Green | OFF |
| Gas detected (>1400) | OFF | 🔴 Red | LOW (200/255) |
| Heavy smoke (>2000) | ON | 🔴 Red | MAX (255/255) |
| Flame detected | ON | 🔴 Red | MAX (255/255) |

### Sensor Thresholds (MQ-2)

```
SMOKE_NONE  = 1400   → Gas present, no immediate danger
SMOKE_LOW   = 2000   → Heavy smoke, full alert
```

---

## 🌐 Web Dashboard

The ESP32 hosts a web server on port **80**. After connecting to WiFi, open the IP address shown on the LCD in any browser on the same network.

**Available Routes:**

| Route | Description |
|---|---|
| `/` | Live HTML dashboard with auto-refresh every 2 seconds |
| `/data` | JSON endpoint with all current sensor readings |

**Example JSON response from `/data`:**
```json
{
  "f1": 0, "f2": 0, "f3": 1, "f4": 0, "f5": 0,
  "gasVal": 1850,
  "temp": 27,
  "motor": 255,
  "fire": true,
  "gas": true,
  "danger": true
}
```

---

## 🚀 Setup & Installation

1. **Install Arduino IDE** and add ESP32 board support:  
   Go to *File → Preferences* and add this URL to Additional Board Manager URLs:
   ```
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
   ```

2. **Install required libraries** via *Tools → Manage Libraries*

3. **Open `fire_test1.ino`** in Arduino IDE

4. **Update your WiFi credentials** in the code:
   ```cpp
   const char* ssid     = "YOUR_WIFI_NAME";
   const char* password = "YOUR_WIFI_PASSWORD";
   ```
   > ⚠️ **Important:** The ESP32 only connects to **2.4 GHz** networks, not 5 GHz.

5. **Select the correct board and port:**  
   *Tools → Board → ESP32 Dev Module*  
   *Tools → Port → COMx (your ESP32 port)*

6. **Upload the sketch** and open the Serial Monitor at **115200 baud**

7. **Find the IP address** printed on the Serial Monitor or LCD, then open it in your browser

---

## 📁 Project Files

```
Iot_Project/
├── fire_test1.ino                          # Main Arduino sketch
├── architecture.png                        # System architecture diagram
├── Block Diagram.jpg                       # Hardware block diagram
├── logic_flow.png                          # Program logic flowchart
├── Demo video.mp4                          # Live demo recording
├── Smart_IoT_Fire_&_Smoke_Detection_System.pptx   # Presentation slides
└── Technical_Documentation.pdf            # Full technical report
```

---

> **Note:** Always test fire/smoke detection systems in a safe, controlled environment. This project is intended for educational purposes.
