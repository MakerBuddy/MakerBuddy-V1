# MakerBuddy

**A comprehensive ESP32-based IoT development kit for makers, hobbyists, and learners**

MakerBuddy is an all-in-one IoT learning platform that combines multiple
sensors and actuators with a hosted WebApp dashboard. The firmware and WebApp
use real-time WebSocket communication for monitoring and control. The
sequence-based automation engine supports four loop modes: None, Forever,
Timer (interval-only), and Condition.

Current releases (20 August 2026): **firmware 2.3** and **WebApp 2.4**.

This public repository contains the open-source Arduino firmware, WebApp, and
PCB design files. Other MakerBuddy clients are maintained separately and are
intentionally not included here.

## 📋 Repository Contents

- **[WebApp](./WebApp/)** - Hosted/static responsive dashboard for modern browsers
- **[Arduino Code](./Arduino%20Code/)** - ESP32 firmware with sensor integration and web server
- **[PCB Design](./PCB%20Design/)** - PCB design files (Fritzing `.fzz`)

## ✨ Features

### 🔬 Sensors

- **DHT11** - Temperature and humidity sensor
- **DS18B20** - Waterproof temperature probe
- **LDR** - Light level detection
- **MQ-2** - Gas/smoke sensor
- **Soil Moisture Sensor** - For plant monitoring
- **HC-SR04** - Ultrasonic distance sensor
- **HC-SR501** - Motion detection sensor
- **Push Button** - Digital input

### 🎮 Actuators

- **LED** - PWM brightness control
- **RGB LED** - Full-color control
- **Relay** - Switching high-power devices
- **Servo Motor** - Position control from 0–180°
- **Buzzer** - Audio alerts
- **LCD Display (16x2)** - Real-time information with sensor wildcards

### 🤖 Smart Features

- Real-time sensor data via WebSocket (push-based, no polling)
- Sequence automation with four loop modes and up to 10 multi-step rules
- Priority-based device locking (Condition > Timer > Forever > None)
- Optional two-sensor conditions with independent comparisons and AND/OR logic
- Potentiometer mapping to LED PWM, RGB color wheel, or Servo
- Soil moisture calibration with NVS persistence
- OTA firmware updates with LCD progress
- WiFi connectivity with mDNS support (`devicename.local`)
- REST API and WebSocket endpoints for all actuators
- Mobile-responsive dashboard with dark/light theme
- Device information display (name, IP, firmware, MAC, uptime, and free memory)
- LCD validation based on rendered 16-column output

## 🚀 Getting Started

### Prerequisites

- ESP32 Development Board
- Arduino IDE (v1.8.x or later)
- Required Arduino libraries listed in [Arduino Code](./Arduino%20Code/)
- Web server such as Apache/XAMPP for hosting the dashboard

### Quick Setup

1. **Clone the repository**

   ```bash
   git clone https://github.com/MakerBuddy/MakerBuddy-V1.0.git
   cd MakerBuddy-V1.0
   ```

2. **Upload the Arduino code**

   - Open `Arduino Code/MakerBuddy/MakerBuddy.ino` in Arduino IDE.
   - Upload it to the ESP32 board.
   - On first boot, connect through the built-in `MakerBuddy_XXXX` provisioning access point.

3. **Set up the WebApp**

   - Copy the `WebApp` folder to your web server.
   - Open `index.html` in Firefox, Safari, or Edge.
   - Enter the MakerBuddy device name or IP address to connect.

## 📖 Documentation

- [WebApp Documentation](./WebApp/README.md)
- [Arduino Code Documentation](./Arduino%20Code/README.md)
- [PCB Design](./PCB%20Design/)

## 🛠️ Hardware Requirements

- ESP32 Development Board and MakerBuddy PCB V1.0
- 2x 15-pin female headers, 2x 15-pin male headers (optional), and 2x 5-pin male headers (optional)
- 680uF 10V capacitor and 2.54mm JST connectors
- 10K resistor for LDR and 4.7K resistor for DS18B20
- Potentiometer, soldering station, desoldering pump, and sensor cables
- DHT11, DS18B20, LDR, MQ-2, soil moisture, HC-SR04, HC-SR501, and push button
- 5mm LDR, 5mm LED, and 5mm RGB LED (common-anode wiring)
- 5V relay module, SG90 servo motor, passive buzzer, and 16x2 I2C LCD

## 📱 Connect With Us

[![YouTube](https://img.shields.io/badge/YouTube-@makerbuddycc-red?style=for-the-badge&logo=youtube)](https://www.youtube.com/@makerbuddycc)
[![GitHub](https://img.shields.io/badge/GitHub-MakerBuddy-black?style=for-the-badge&logo=github)](https://github.com/MakerBuddy)
[![Facebook](https://img.shields.io/badge/Facebook-makerbuddycc-blue?style=for-the-badge&logo=facebook)](https://www.facebook.com/makerbuddycc)
[![LinkedIn](https://img.shields.io/badge/LinkedIn-MakerBuddy-0077B5?style=for-the-badge&logo=linkedin)](https://www.linkedin.com/company/makerbuddy/)
[![Hackster.io](https://img.shields.io/badge/Hackster.io-MakerBuddy-00979D?style=for-the-badge&logo=hackster)](https://www.hackster.io/makerbuddy)
[![Patreon](https://img.shields.io/badge/Patreon-Support%20Us-FF424D?style=for-the-badge&logo=patreon)](https://www.patreon.com/cw/MakerBuddy)

## 🤝 Contributing

We welcome contributions. Feel free to report bugs, suggest features, submit
pull requests, and share projects built with MakerBuddy.

## 📝 Release Notes

### 20 August 2026

**WebApp 2.4**

- Keeps the optional second sensor editor hidden until its checkbox is enabled.
- Uses explanatory AND/OR radio choices for two-condition and second-range logic.
- Aligns both sensor condition rows and their optional ranges across desktop and mobile layouts.

### 19 August 2026

**Firmware 2.3 / WebApp 2.3**

- Added optional two-sensor condition rules with independent comparisons,
  `AND`/`OR` logic, and range support for each sensor.
- Preserved existing saved rules through separate versioned `seqc_N` condition
  records in ESP32 NVS.

**Firmware 2.2**

- LCD wildcard validation now measures each wildcard using its current
  formatted sensor value, matching the physical 16-column display.
- Sequence LCD validation now agrees with the WebApp and allows templates such
  as `Temperature={temp}` when the rendered value fits.

**WebApp 2.1 / 2.2**

- LCD counters use current telemetry values and refresh as sensor values change.
- Longer source templates are allowed when their rendered output fits the LCD.
- Checks the official firmware feed and suppresses dismissed release notices
  for 24 hours per firmware version.

### 17 August 2026

**Firmware 2.1**

- A finishing condition step retains ownership of its actuator until its
  configured delay expires.
- Opposing condition rules retry without consuming their delay, enforcing
  actuator dwell times.

**Firmware 2.0 / WebApp 1.9**

- Uses the configured device name consistently across LCD, mDNS, REST, and
  WebSocket identity responses.
- Shows connected WiFi/IP and device-name screens at boot and in the LCD rotation.
- Validates custom LCD templates against 16 rendered columns.
- Adds the ESP32-reported IP to Settings and live 16-column LCD counters.

### Earlier releases

**Firmware 1.9 / WebApp 1.7**

- Timer mode became interval-only and parked sequences release their devices.
- Added push-button telemetry, sidebar persistence, and safer LED/Servo controls.

**v1.8**

- Added optimized NVS sequence storage, ADC averaging, WiFi power saving,
  relay protocol fields, and legacy potentiometer migration.

**v1.2**

- Added WebSocket telemetry, the multi-step rule engine, potentiometer mapping,
  soil calibration, OTA updates, mDNS discovery, LCD wildcards, and dark/light
  theme persistence.

**v1.0**

- Initial release with sensors, actuators, REST API, basic IF-THEN rules,
  provisioning, and responsive dashboard.

## 📄 License

### Software (Firmware and WebApp)

- **License:** GNU General Public License v3.0 (GPL-3.0)
- **Applies to:** Arduino Code and WebApp
- See [LICENSE-GPL](./LICENSE-GPL) for the full license text.

### Hardware (PCB Design)

- PCB design files are distributed under their applicable hardware-design
  license. See the files in [PCB Design](./PCB%20Design/).

**Copyright (C) 2024 Muhammad Afzal**  
Website: [makerbuddy.cc](https://makerbuddy.cc)  
Email: info@makerbuddy.cc

## 💖 Support the Project

If MakerBuddy helps you, consider supporting the project on
[Patreon](https://www.patreon.com/cw/MakerBuddy).

---

**Made with ❤️ by MakerBuddy Team**

*Empowering makers, one project at a time.*
