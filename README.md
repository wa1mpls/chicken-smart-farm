# 🐔 Chicken Smart Farm (ESP32 + Wokwi)

This project simulates a **Smart Chicken Farm** using the ESP32 microcontroller, designed in the [Wokwi](https://wokwi.com) online simulator.

It automates tasks such as lighting, ventilation, and monitoring conditions for poultry using virtual components.

---

## 📂 Project Structure

```
smart-farm/
├── sketch.ino            # Main code (written for ESP32)
├── diagram.json          # Virtual circuit configuration for Wokwi
├── wokwi-project.txt     # Link to open the simulation on Wokwi
└── libraries.txt         # External libraries required (if any)
```

---

## 🔧 Features

- 🧠 Control system for lighting and fans (simulated with LEDs and relays)
- 🌡️ Sensor simulation possible (e.g., DHT22, light sensor, soil moisture, etc.)
- 🕓 Time-based or condition-based automation logic
- ⚙️ Ready to deploy to real ESP32 boards

---

## 🚀 How to Run on Wokwi

1. Click this link to open simulation:  
   👉 **[Live Simulation](https://wokwi.com/projects/407033391730481153)**

2. Press **"Start Simulation"**.

3. Observe the system behavior: LEDs, relays, fan states, etc.

---

## 🧰 Hardware (Simulated)

Configured via `diagram.json`, including:
- ESP32 DevKit v1 board
- Relay module (for fan/light control)
- LEDs as actuators
- Optional: sensors (can be added in Wokwi)

---

## 📦 Libraries

Check `libraries.txt` for any external dependencies used in `sketch.ino`.

Use **Arduino IDE** or **PlatformIO** with ESP32 board support installed.

---

## 🧠 Educational Purpose

This project is built for learning embedded systems and IoT with ESP32.
It can be extended with real sensors like:
- DHT11/DHT22 (temperature/humidity)
- Soil moisture sensor
- LDR (light sensor)
- HC-SR04 (ultrasonic)

---

## 📜 License

MIT License – free to use and modify.  
Created as part of a student project on Smart Agriculture & IoT.
