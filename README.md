# MakerBuddy V1.0

**A comprehensive ESP32-based IoT development kit for makers, hobbyists, and learners**

MakerBuddy V1.0 is an all-in-one IoT learning platform that combines multiple sensors, actuators, and a beautiful web dashboard for real-time monitoring and control. Perfect for electronics enthusiasts, students, and anyone looking to dive into IoT development.

## 📋 Repository Contents

- **[Web Dashboard](./Web%20Dashboard/)** - Modern, responsive web interface for real-time sensor monitoring and device control
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
- **LCD Display (16x2)** - Real-time information display

### 🤖 Smart Features
- Real-time sensor data visualization
- Automation rules engine (IF-THEN logic)
- WiFi connectivity
- REST API endpoints
- Mobile-responsive interface
- Device information display

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
   - Open `Arduino Code/MakerBuddy V1-0.ino` in Arduino IDE
   - Configure WiFi credentials
   - Upload to your ESP32 board

3. **Setup Web Dashboard**
   - Copy the `Web Dashboard` folder to your web server
   - Open `index.html` in your browser
   - Enter your MakerBuddy IoT Kit ESP32's IP address to connect

## 📖 Documentation

Detailed documentation for each component can be found in their respective folders:
- [Web Dashboard Documentation](./Web%20Dashboard/README.md)
- [Arduino Code Documentation](./Arduino%20Code/README.md)
- [PCB Design Documentation](./PCB%20Design/README.md)

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
- 5MM RGB LED (Common Cathode)
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

## 📄 License

This project uses dual licensing:

### Software (Firmware & Web Dashboard)
- **License:** GNU General Public License v3.0 (GPL-3.0)
- **Applies to:** Arduino Code, Web Dashboard
- You are free to use, modify, and distribute the software under the terms of GPL-3.0
- See [LICENSE-GPL](./LICENSE-GPL) for full license text

### Hardware (PCB Design)
- **License:** Creative Commons Attribution-ShareAlike 4.0 International (CC BY-SA 4.0)
- **Applies to:** PCB Design files
- You are free to share and adapt the hardware design, as long as you:
  - Give appropriate credit
  - Indicate if changes were made
  - Distribute under the same license
- See [PCB Design/LICENSE-CC-BY-SA](./PCB%20Design/LICENSE-CC-BY-SA) for full license text

**Copyright (C) 2024 Muhammad Afzal**
Website: [https://makerbuddy.cc](https://makerbuddy.cc)
Email: info@makerbuddy.cc

## 💖 Support the Project

If you find this project helpful, consider supporting us on [Patreon](https://www.patreon.com/cw/MakerBuddy) to help us create more awesome IoT projects!

---

**Made with ❤️ by MakerBuddy Team**

*Empowering makers, one project at a time.*
