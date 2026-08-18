# MakerBuddy

**A comprehensive ESP32-based IoT development kit for makers, hobbyists, and learners**

MakerBuddy is an all-in-one IoT learning platform that combines multiple
sensors and actuators with a hosted WebApp dashboard. The firmware and WebApp
use real-time WebSocket communication for monitoring and control.
The sequence-based automation engine supports four loop modes: None, Forever,
Timer (interval-only), and Condition.

Current releases (17 August 2026): **firmware 2.1** and **WebApp 1.9**.

This public repository contains the open-source Arduino firmware, WebApp, and
PCB design files. Other MakerBuddy clients are maintained separately and are
intentionally not included here.

## 📋 Repository Contents

- **[WebApp](./WebApp/)** - Hosted/static responsive dashboard for modern browsers
- **[Arduino Code](./Arduino%20Code/)** - ESP32 firmware with sensor integration and web server
- **[PCB Design](./PCB%20Design/)** - PCB design files (Fritzing `.fzz`)

## ✨ Features

### 🔬 Sensors
- **DHT11** - Temperature and Humidity sensor
- **DS18B20** - Waterproof temperature probe
- **LDR** - Light level detection
- **MQ-2** - Gas/smoke sensor
- **Soil Moisture Sensor** - For plant monitoring
- **HC-SR04** - Ultrasonic distance sensor
- **HC-SR501** - Motion detection sensor
- **Push Button** - Digital input

### 🎮 Actuators
- **LED** - With PWM brightness control
- **RGB LED** - Full color control
- **Relay** - For switching high-power devices
- **Servo Motor** - Position control (0-180°)
- **Buzzer** - Audio alerts
- **LCD Display (16x2)** - Real-time information display with live sensor wildcards

### 🤖 Smart Features
- Real-time sensor data via WebSocket (push-based, no polling)
- Sequence automation engine with 4 loop modes and up to 10 multi-step rules
- Priority-based device locking (Condition > Timer > Forever > None)
- Potentiometer mapping to LED PWM, RGB color wheel, or Servo
- Soil moisture calibration with NVS persistence
- OTA (over-the-air) firmware updates with LCD progress
- WiFi connectivity with mDNS support (`devicename.local`)
- REST API + WebSocket endpoints for all actuators
- Mobile-responsive interface with dark/light theme
- Device information display (configured name, IP, firmware, MAC, uptime, and free memory)
- Wildcard-aware LCD validation based on the rendered 16-column output rather than token-name length

## 🚀 Getting Started

### Prerequisites
- ESP32 Development Board
- Arduino IDE (v1.8.x or later)
- Required Arduino Libraries (see Arduino Code folder)
- Web server (Apache/XAMPP) for hosting the dashboard

### Quick Setup

1. **Clone the repository**
   ```bash
   git clone https://github.com/MakerBuddy/MakerBuddy-V1.0.git
   cd MakerBuddy-V1.0
   ```

2. **Upload Arduino Code**
   - Open `Arduino Code/MakerBuddy/MakerBuddy.ino` in Arduino IDE
   - Upload to your ESP32 board
   - On first boot, use the built-in `MakerBuddy_XXXX` provisioning access point

3. **Setup Web Dashboard**
   - Copy the `WebApp` folder to your web server
   - Open `index.html` in Firefox, Safari, or Edge
   - Enter your MakerBuddy IoT Kit ESP32's device name or IP address to connect

## 📖 Documentation

Detailed documentation for each component can be found in their respective folders:
- [Web Dashboard Documentation](./WebApp/README.md)
- [Arduino Code Documentation](./Arduino%20Code/README.md)

## 🛠️ Hardware/components Requirements

- ESP32 Development Board
- MakerBuddy PCB V1.0
- 2x 15-Pin Female Headers
- 2x 15-Pin Male headers (Optional)
- 2x 5-Pin Male headers (Optional)
- 1x Capactior 10v(680uF)
- 7x 3Pin 2.54MM JST Connectors (Male/Female Both)
- 5x 4Pin 2.54MM JST Connectors (Male/Female Both)
- 1x Resistor 10K for LDR
- 1x Resistor 4.7K for DS18B260 Temperature Probe
- Desoldering Pump to remove pins from sensors
- Soldering Station
- 3 core and 4 core cables for sensors to connectors
- Potentiometer
- DHT11 Temperature & Humidity Sensor
- DS18B20 Temperature Probe
- LDR (Light Dependent Resistor)
- MQ-2 Gas Sensor
- Soil Moisture Sensor
- HC-SR04 Ultrasonic Sensor
- HC-SR501 PIR Motion Sensor
- Push Button
- 5MM LDR
- 5MM LED
- 5MM RGB LED (firmware output is inverted for common-anode wiring)
- 5V Relay Module
- SG90 Servo Motor
- Passive Buzzer
- 16x2 LCD Display with I2C Module

## 📱 Connect With Us

Stay updated with our latest projects and tutorials:

[![YouTube](https://img.shields.io/badge/YouTube-@makerbuddycc-red?style=for-the-badge&logo=youtube)](https://www.youtube.com/@makerbuddycc)
[![GitHub](https://img.shields.io/badge/GitHub-MakerBuddy-black?style=for-the-badge&logo=github)](https://github.com/MakerBuddy)
[![Facebook](https://img.shields.io/badge/Facebook-makerbuddycc-blue?style=for-the-badge&logo=facebook)](https://www.facebook.com/makerbuddycc)
[![LinkedIn](https://img.shields.io/badge/LinkedIn-MakerBuddy-0077B5?style=for-the-badge&logo=linkedin)](https://www.linkedin.com/company/makerbuddy/)
[![Hackster.io](https://img.shields.io/badge/Hackster.io-MakerBuddy-00979D?style=for-the-badge&logo=hackster)](https://www.hackster.io/makerbuddy)
[![Patreon](https://img.shields.io/badge/Patreon-Support%20Us-FF424D?style=for-the-badge&logo=patreon)](https://www.patreon.com/cw/MakerBuddy)

## 🤝 Contributing

We welcome contributions! Feel free to:
- Report bugs
- Suggest new features
- Submit pull requests
- Share your projects built with MakerBuddy

## 📝 Release Notes

### Current firmware and WebApp releases — 17 August 2026

**Firmware v2.1**

- A finishing condition step retains ownership of its actuator until its configured delay expires.
- Opposing condition rules retry without consuming their delay, enforcing actuator dwell times.

**Firmware v2.0 features**

- Uses the configured device name consistently across LCD, mDNS, REST, and WebSocket identity responses.
- Returns an empty device name when none is configured and reports the loaded identity through Serial diagnostics.
- Shows connected WiFi/IP and device-name screens for five seconds at boot and in the default LCD rotation.
- Validates custom LCD templates against 16 rendered columns and compacts recognized wildcard tokens inside sequence storage without changing the persisted struct layout.

**WebApp v1.9**

- Display the ESP32-reported device IP address in Settings.
- Count LCD wildcards by their maximum displayed width and show live 16-column counters in manual controls and rule steps.
- Allow longer source templates such as `{humidity}{distance}` when their rendered output fits the physical LCD.
- Preserve the unified device-identity UI, rule-list-first layout, interval-only timer controls, and synchronized rule-step values.

### Earlier v1.9 release

**Firmware v1.9**

- Timer mode simplified to interval-only; all timer rules repeat after their configured interval.
- Sequence device locking narrowed to the current step so parked sequences release their devices.
- Sequence type declarations aligned at version 1.9.

**WebApp v1.7**

- Timer UI simplified to an Interval field.
- Rule step values remain intact when adding or removing steps.
- LED On defaults to 255, and Servo values are restricted to 0-180.

### v1.8

**Firmware (Arduino Code)**
- Optimized NVS storage with dirty bitmask tracking — only changed sequence slots are written
- Compact sequence save format (header + used steps, saves ~336-432 bytes per rule)
- Orphaned NVS entry cleanup (`seq_N`/`seqms_N` keys for unused slots removed on boot)
- ADC noise reduction: 4-read averaging per analog pin (LDR, Pot, MQ-2, Soil Moisture)
- WiFi power saving: falls back to WIFI_OFF on connection failure
- Relay delay protocol fields (`od`/`fd`) added to WebSocket for future timed relay support
- Legacy pot mapping string-to-UChar migration

**Web Dashboard (v1.7)**
- Push button sensor card with live Pressed/Released state display
- Relay messages include reserved `od`/`fd` fields, but firmware currently switches immediately and ignores them
- Sidebar state persistence across sessions (localStorage), smart show/hide on mobile
- Debounced resize handler for desktop/mobile sidebar transitions
- Backdrop dismiss — tap overlay to close sidebar on mobile

### v1.2

**Firmware (Arduino Code v1.2)**
- WebSocket communication replaces HTTP polling for real-time sensor data push
- Sequence engine rewrite with 4 loop modes: None, Forever, Timer (duration/interval), Condition (sensor-based)
- Up to 10 sequences with 10 steps each, priority-based device locking, debounced condition evaluation
- Potentiometer mapping: map pot to LED PWM, RGB color wheel, or Servo position
- Soil moisture calibration with dry/wet values persisted to NVS
- OTA firmware updates over-the-air with LCD progress display
- mDNS support for device discovery via `devicename.local`
- Direct LEDC servo control eliminates timer conflicts with analogWrite (ESP32Servo no longer required)
- LCD wildcard support: `{temp}`, `{humidity}`, `{ds18b20}`, `{light}`, `{gas}`, `{soil}`, `{distance}`, `{pot}`, `{motion}`, `{button}`
- Compact JSON WebSocket protocol for bandwidth efficiency
- WiFi network scan caching, PMF disable for AP compatibility
- Ticker-based non-blocking architecture

**Web Dashboard (v1.2)**
- Real-time WebSocket communication (no more polling)
- New Rule Engine tab with create/edit/manage workflow and 4 loop modes
- Multi-step rule editor with up to 10 steps per rule
- Settings tab for sensor/actuator card visibility and color customization
- Potentiometer mapping control card (None/LED/RGB/Servo)
- Soil moisture calibration modal with live ADC capture
- OTA firmware update UI (check, compare versions, install)
- Automation indicators: actuator cards show warning banner and disable when controlled by active rules
- Tab navigation with animated transitions (Dashboard, Rule Engine, Settings)
- mDNS support with `.local` auto-append in connection input
- Servo slider throttle (50ms debounce) to prevent flooding
- Dark/light theme persistence via localStorage

### v1.0
- Initial release with all sensors and actuators
- REST API and basic IF-THEN rules
- WiFi configuration portal
- Dark/light theme with mobile-responsive design

## 📄 License

The software in this checkout is GPL-3.0 licensed:

### Software (Firmware and WebApp)
- **License:** GNU General Public License v3.0 (GPL-3.0)
- **Applies to:** Arduino Code and WebApp
- You are free to use, modify, and distribute the software under the terms of GPL-3.0
- See [LICENSE-GPL](./LICENSE-GPL) for full license text

PCB design files are not included in this checkout. Consult the distribution
that supplies those hardware files for its hardware-design license.

**Copyright (C) 2024 Muhammad Afzal**
Website: [https://makerbuddy.cc](https://makerbuddy.cc)
Email: info@makerbuddy.cc

## 💖 Support the Project

If you find this project helpful, consider supporting us on [Patreon](https://www.patreon.com/cw/MakerBuddy) to help us create more awesome IoT projects!

---

**Made with ❤️ by MakerBuddy Team**

*Empowering makers, one project at a time.*
