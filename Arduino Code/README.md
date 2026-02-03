# MakerBuddy Arduino Code

ESP32 firmware for the MakerBuddy V1.0 IoT kit with integrated web server, sensor management, and automation engine.

## 📋 Overview

This Arduino sketch provides a complete IoT solution for the MakerBuddy V1.0 kit, featuring:
- Multi-sensor data acquisition
- Actuator control via REST API
- WiFi connectivity with AP mode fallback
- Automation rules engine
- Real-time web dashboard serving
- Persistent configuration storage

## 🔧 Hardware Requirements

### Microcontroller
- **ESP32 Development Board** (30-pin or compatible)

### Sensors
| Sensor | Pin | Description |
|--------|-----|-------------|
| DHT11 | GPIO 25 | Temperature & Humidity |
| DS18B20 | GPIO 26 | Waterproof Temperature Probe |
| LDR | GPIO 35 (ADC) | Light Level Detection |
| MQ-2 | GPIO 33 (ADC) | Gas/Smoke Sensor |
| Soil Moisture | GPIO 32 (ADC) | Soil Moisture Level |
| HC-SR04 Echo | GPIO 2 | Ultrasonic Distance Sensor |
| HC-SR04 Trig | GPIO 15 | Ultrasonic Distance Sensor |
| HC-SR501 | GPIO 18 | PIR Motion Sensor |
| Push Button | GPIO 5 | Digital Input |
| Potentiometer | GPIO 34 (ADC) | Analog Input |

### Actuators
| Actuator | Pin | Description |
|----------|-----|-------------|
| LED | GPIO 12 | Single LED with PWM |
| RGB LED (R) | GPIO 13 | Red Channel |
| RGB LED (G) | GPIO 14 | Green Channel |
| RGB LED (B) | GPIO 27 | Blue Channel |
| Relay | GPIO 23 | High Power Switch |
| Servo | GPIO 19 | Position Control |
| Buzzer | GPIO 4 | Audio Alerts |
| LCD (I2C) | SDA/SCL | 16x2 Display (0x27) |

## 📚 Required Libraries

Install these libraries via Arduino Library Manager or manually:

### Core Libraries
```
WiFi (Built-in)
Wire (Built-in)
```

### External Libraries
```
ESPAsyncWebServer by me-no-dev
AsyncTCP by me-no-dev
ArduinoJson by Benoit Blanchon (v6.x)
OneWire by Paul Stoffregen
DallasTemperature by Miles Burton
Adafruit Unified Sensor by Adafruit
DHT sensor library by Adafruit
LiquidCrystal I2C by Frank de Brabander
ESP32Servo by Kevin Harrington
Preferences (Built-in ESP32)
DNSServer (Built-in ESP32)
Ticker (Built-in ESP32)
```

### Installation Commands (PlatformIO)
```ini
[env:esp32dev]
platform = espressif32
board = esp32dev
framework = arduino
lib_deps =
    me-no-dev/ESPAsyncWebServer
    me-no-dev/AsyncTCP
    bblanchon/ArduinoJson@^6.21.3
    paulstoffregen/OneWire
    milesburton/DallasTemperature
    adafruit/DHT sensor library
    adafruit/Adafruit Unified Sensor
    marcoschwartz/LiquidCrystal_I2C
    madhephaestus/ESP32Servo
```

## 🚀 Setup & Configuration

### 1. Arduino IDE Setup

1. **Install ESP32 Board Support**
   - Open Arduino IDE
   - Go to `File > Preferences`
   - Add to Additional Board Manager URLs:
     ```
     https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
     ```
   - Go to `Tools > Board > Boards Manager`
   - Search "ESP32" and install "esp32 by Espressif Systems"

2. **Install Required Libraries**
   - Go to `Sketch > Include Library > Manage Libraries`
   - Search and install each library listed above

### 2. WiFi Configuration

**Method 1: Direct Code Modification** (Lines 18-19)
```cpp
const char* ssid = "YourWiFiSSID";
const char* password = "YourWiFiPassword";
```

**Method 2: Configuration Portal** (Recommended which is applicable after the upload of Firmaware)
- On first boot, ESP32 creates a hotspot named `MakerBuddy-XXXXXX`
- Connect to this hotspot (password: `12345678`)
- Open browser to `192.168.4.1`
- Enter your WiFi credentials
- Device will save and restart in Station mode

### 3. Upload Firmware

1. Connect ESP32 to computer via USB
2. Select board: `Tools > Board > ESP32 Dev Module`
3. Select port: `Tools > Port > COMx` (Windows) or `/dev/ttyUSBx` (Linux/Mac)
4. Set upload speed: `Tools > Upload Speed > 115200`
5. Click Upload button
6. Monitor Serial output at 115200 baud to see IP address

### 4. Verify Installation

1. Open Serial Monitor (115200 baud)
2. Look for:
   ```
   Connected to WiFi
   IP Address: 192.168.x.x
   HTTP server started
   ```
3. Note the IP address for dashboard connection

## 🌐 API Endpoints

### Device Information
```
GET /api/device
Response: {deviceName, mac, uptime}
```

### Sensor Data
```
GET /api/sensors
Response: {
  temperature, humidity, ds18b20Temp, lightLevel, gasLevel,
  soilMoisture, potValue, distance, motionDetected, buttonPressed,
  ledState, ledPWM, rgbR, rgbG, rgbB, relayState, servoPosition
}
```

### Control Endpoints

**LED Control**
```
POST /api/led
Parameters: state (bool), pwm (0-255), blink (bool)
```

**RGB LED Control**
```
POST /api/rgb
Parameters: r (0-255), g (0-255), b (0-255)
```

**Relay Control**
```
POST /api/relay
Parameters: state (bool), onDelay (ms), offDelay (ms)
```

**Servo Control**
```
POST /api/servo
Parameters: position (0-180)
```

**Buzzer Control**
```
POST /api/buzzer
Parameters: state (bool)
```

**LCD Control**
```
POST /api/lcd
Parameters: mode (default/custom), text1 (16 chars), text2 (16 chars)
```

### Automation Rules
```
GET /api/rules
Response: {rules: [...]}

POST /api/rules
Parameters:
  action: add/toggle/delete
  name, condition, operator, threshold, actionType, actionValue, enabled
```

### WiFi Management
```
GET /api/wifi/scan
Response: {networks: [...]}

GET /api/wifi/status
Response: {ssid, ip, rssi, connected}

POST /api/wifi/connect
Parameters: ssid, password

GET /api/wifi/reset
Action: Reset to AP mode
```

## 🤖 Automation Rules Engine

### Rule Structure
- **Condition**: Sensor to monitor (e.g., temperature, humidity)
- **Operator**: Comparison operator (>, <, >=, <=, ==)
- **Threshold**: Trigger value
- **Action**: What to do (LED, Buzzer, Servo, Relay, RGB)
- **Value**: Action parameter (e.g., LED PWM, Servo angle)

### Example Rules
```cpp
// If temperature > 30°C, turn on LED
condition: "temperature"
operator: ">"
threshold: 30
actionType: "led_on"
actionValue: 255

// If motion detected, turn on relay
condition: "motionDetected"
operator: "=="
threshold: 1
actionType: "relay_on"

// If distance < 10cm, set RGB to red
condition: "distance"
operator: "<"
threshold: 10
actionType: "rgb_red"
```

### Rule Priorities
- Rules are checked every sensor update cycle
- Multiple rules can trigger simultaneously
- Rule actions override manual controls
- Manual control resumes when rule condition is false

## 🔍 Pin Configuration Details

### PWM Channels
- LED uses PWM channel for brightness control
- RGB LED uses 3 separate PWM channels
- Servo uses built-in ESP32Servo PWM

### I2C Configuration
- SDA: GPIO 21 (default)
- SCL: GPIO 22 (default)
- LCD Address: 0x27 (adjustable in code)

### Analog Inputs (ADC1)
- LDR: GPIO 35
- Potentiometer: GPIO 34
- MQ-2: GPIO 33
- Soil Moisture: GPIO 32

**Note**: ESP32 ADC2 pins cannot be used when WiFi is active.

## 💾 Data Persistence

Configuration stored in ESP32 NVS (Non-Volatile Storage):
- WiFi credentials (SSID, Password)
- Automation rules (up to 10 rules)
- Device preferences

## 🐛 Troubleshooting

### WiFi Connection Issues
**Problem**: ESP32 not connecting to WiFi
- Check credentials in code or via config portal
- Verify WiFi is 2.4GHz (ESP32 doesn't support 5GHz)
- Check serial monitor for error messages
- Press reset button on ESP32

### Sensor Reading Errors
**Problem**: Sensors showing incorrect values
- Verify wiring and pin assignments
- Check sensor power supply (3.3V or 5V)
- Test sensors individually with simple sketches
- Ensure pull-up resistors where needed (DHT, DS18B20)

### Upload Errors
**Problem**: Code won't upload
- Hold BOOT button while uploading
- Try lower upload speed (115200)
- Check USB cable (must support data transfer)
- Try different USB port
- Ensure no serial monitor is open

### Library Errors
**Problem**: Compilation errors about missing libraries
- Install all required libraries
- Check library versions (especially ArduinoJson v6.x)
- Restart Arduino IDE after installing libraries

### Web Server Not Responding
**Problem**: Cannot access web dashboard
- Verify ESP32 IP address from serial monitor
- Ensure devices are on same network
- Check firewall settings
- Try accessing `/api/device` endpoint directly

### LCD Not Displaying
**Problem**: LCD backlight on but no text
- Check I2C address (use I2C scanner sketch)
- Adjust LCD contrast potentiometer
- Verify I2C connections (SDA, SCL)
- Common addresses: 0x27, 0x3F

## 📊 Memory Usage

Typical flash/RAM usage:
- Sketch size: ~1.2MB of 1.9MB (63%)
- Global variables: ~65KB of 320KB (20%)
- Heap available: ~250KB for dynamic allocation

## ⚡ Performance

- Sensor update rate: ~2 seconds
- Web server response time: <100ms
- Maximum simultaneous connections: 5
- Rule evaluation time: <1ms

## 🔒 Security Considerations

⚠️ **This code is designed for local network use:**
- No authentication on API endpoints
- No HTTPS encryption
- WiFi password stored in plain text
- Suitable for trusted home/lab networks

For production use, consider adding:
- API authentication (token-based)
- HTTPS with certificates
- Input validation and sanitization
- Rate limiting

## 📱 Social Links

Connect with MakerBuddy:

- 🎥 **YouTube**: [@makerbuddycc](https://www.youtube.com/@makerbuddycc)
- 💻 **GitHub**: [MakerBuddy](https://github.com/MakerBuddy)
- 📘 **Facebook**: [makerbuddycc](https://www.facebook.com/makerbuddycc)
- 💼 **LinkedIn**: [MakerBuddy Company](https://www.linkedin.com/company/makerbuddy/)
- 🔧 **Hackster.io**: [makerbuddy](https://www.hackster.io/makerbuddy)
- ❤️ **Patreon**: [Support Us](https://www.patreon.com/cw/MakerBuddy)

## 🎓 Learning Resources

### Tutorials
- Watch our YouTube channel for setup guides
- Check Hackster.io for project ideas
- Join our community on social media

### Code Examples
```cpp
// Read sensor value
float temp = sensors.temperature;

// Control actuator via API
POST /api/led?state=true&pwm=128

// Create automation rule
{
  "name": "Night Light",
  "condition": "lightLevel",
  "operator": "<",
  "threshold": 30,
  "actionType": "led_on",
  "actionValue": 255
}
```

## 🔄 Firmware Updates

To update firmware:
1. Download latest `.ino` file from GitHub
2. Open in Arduino IDE
3. Verify no compilation errors
4. Upload to ESP32
5. Existing WiFi settings and rules are preserved

## 📝 Version History

### v1.0 (Current)
- Initial release
- All sensor and actuator support
- REST API implementation
- Automation rules engine
- WiFi configuration portal
- Persistent storage

## 🤝 Contributing

Improve the firmware:
- Report bugs on GitHub Issues
- Submit pull requests with enhancements
- Share your custom modifications
- Suggest new features

## 📄 License

**GNU General Public License v3.0 (GPL-3.0)**

Copyright (C) 2024 Muhammad Afzal
Website: [https://makerbuddy.cc](https://makerbuddy.cc)
Email: info@makerbuddy.cc

This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with this program. If not, see <https://www.gnu.org/licenses/>.

See [LICENSE-GPL](../LICENSE-GPL) in the root directory for the full license text.

---

**Made with ❤️ by MakerBuddy Team**

*Happy Making! 🛠️*
