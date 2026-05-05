# MakerBuddy Arduino Code

ESP32 firmware for the MakerBuddy V1.2 IoT kit with WebSocket server, sensor management, and sequence-based automation engine.

## 📋 Overview

This Arduino sketch provides a complete IoT solution for the MakerBuddy V1.2 kit, featuring:
- Multi-sensor data acquisition
- Real-time WebSocket communication (replaces HTTP polling)
- Actuator control via REST API and WebSocket
- Advanced sequence engine with 4 loop modes and multi-step automation
- WiFi connectivity with AP mode fallback and mDNS
- OTA (over-the-air) firmware updates
- LCD wildcard support for live sensor values
- Persistent configuration storage with Preferences/NVS

## 🆕 What's New in v1.2

- **WebSocket Communication** — Real-time bidirectional data push replaces polling. Sensor data streams every 2s, device info on connect, and all actuator commands use compact single-letter JSON.
- **Sequence Engine (Rule Engine v2)** — Complete rewrite with 10 sequences, each supporting up to 10 steps, 4 loop modes, priority-based device locking, and debounced condition evaluation.
- **Potentiometer Mapping** — Map the potentiometer to LED PWM, RGB color wheel, or Servo position. Configurable via WebSocket and persisted to NVS.
- **Soil Moisture Calibration** — Calibrate soil sensor with dry/wet values via WebSocket. Values persist in NVS.
- **OTA Firmware Updates** — Download and install firmware binaries over-the-air. Triggered via REST API or WebSocket with LCD progress display.
- **mDNS Support** — Devices accessible via `devicename.local` for easy discovery.
- **Direct LEDC Servo** — Servo driven via ESP32 LEDC API, eliminating timer conflicts with `analogWrite()`.

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
| Push Button | GPIO 5 | Digital Input (also used for WiFi reset) |
| Potentiometer | GPIO 34 (ADC) | Analog Input |

### Actuators
| Actuator | Pin | Description |
|----------|-----|-------------|
| LED | GPIO 12 | Single LED with PWM |
| RGB LED (R) | GPIO 13 | Red Channel (common anode — inverted) |
| RGB LED (G) | GPIO 14 | Green Channel |
| RGB LED (B) | GPIO 27 | Blue Channel |
| Relay | GPIO 23 | High Power Switch |
| Servo | GPIO 19 | Position Control (LEDC @ 50Hz) |
| Buzzer | GPIO 4 | Audio Alerts |
| LCD (I2C) | SDA/SCL | 16x2 Display (address 0x27) |

## 📚 Required Libraries

Install these libraries via Arduino Library Manager or manually:

### Built-in Libraries
```
WiFi, Wire, Preferences, DNSServer, Ticker, HTTPClient, Update, ESPmDNS
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
```

> **Note:** `ESP32Servo` is no longer required. The servo is now driven directly via the ESP32 LEDC API to avoid timer conflicts with `analogWrite()`.

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

**Method 1: Direct Code Modification** (Lines 48-50)
```cpp
const char* ssid = "YourWiFiSSID";
const char* password = "YourWiFiPassword";
```

**Method 2: Configuration Portal** (Recommended — applicable after firmware upload)
- On first boot, ESP32 creates a hotspot named `MakerBuddy_XXXX` (last 4 hex digits of MAC)
- Connect to this hotspot (password: `12345678`)
- Open browser to `192.168.4.1`
- Set a Device Name (e.g., `makerbuddy-01`) and select/enter WiFi credentials
- Device will save settings, restart, and connect in Station mode
- Access via `http://devicename.local` (mDNS) or the assigned IP

**WiFi Reset:** Hold the push button for 10 seconds to clear saved credentials and restart in AP mode.

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
   WiFi connected successfully!
   IP address: 192.168.x.x
   Web server started
   ```
3. Note the IP address for dashboard connection

## 🌐 API Endpoints

### Device Information
```
GET /api/device
Response (compact JSON): {dn, fv, ip, mc, up, fh, sd, sw}
```
- `dn`: Device name, `fv`: Firmware version, `ip`: IP address, `mc`: MAC address
- `up`: Uptime in seconds, `fh`: Free heap, `sd/sw`: Soil calibration dry/wet

### Sensor Data
```
GET /api/sensors
Response (compact JSON): {
  te, hu, d8, li, po, ga, sm, sr, di, md, bp,
  ls, lp, lb, r, rs, sp, lm, fh, pm, pv
}
```
Includes all sensor readings, actuator states, and potentiometer mapping info.

### Firmware Info
```
GET /api/firmware
Response: {fv, oi, op}
```
- `fv`: Firmware version, `oi`: OTA in progress, `op`: OTA progress %

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
Parameters: state (bool)
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

**Potentiometer Mapping**
```
POST /api/potmapping
Parameters: mode (none/led/rgb/servo)
```

**OTA Update**
```
POST /api/ota/update
Parameters: url (firmware binary URL)
```

### Sequence Engine
```
GET /api/sequences
Response: {sequences: [{id, enabled, name, loop, loopMode, autostart,
                         isRunning, timerType?, loopDuration?,
                         loopSensor?, loopOperator?, loopThreshold?,
                         loopUseRange?, loopRangeOperator?, loopOperator2?,
                         loopThreshold2?, steps: [{action, actionValue, delay, lcdMode?, lcdText1?, lcdText2?}]}]}

POST /api/sequences
Parameters:
  action: add/delete/start/stop/toggle
  For add: name, enabled, loopMode, autostart, stepCount, timerType?, loopDuration?,
           loopSensor?, loopOperator?, loopThreshold?, loopUseRange?, ...,
           step0_action, step0_value, step0_delay, step0_lcdMode?, step0_lcdText1?, step0_lcdText2?, ...
  For others: index
```

## 🔌 WebSocket Protocol

The WebSocket server at `/ws` uses compact single-letter JSON keys for efficiency on the ESP32.

### Server → Client Messages

| Message `m` | Payload | Description |
|---|---|---|
| `i` | `{m:"i", dn, fv, ip, mc, up, fh, sd, sw}` | Device info on connect |
| `f` | `{m:"f", fv, oi, op}` | Firmware info on connect |
| `q` | `{m:"q", sq:[{i, e, n, lm, tt, ld, lr, lo, lt, rg, ro, o2, t2, ms, rn, as, st:[{a, v, d, lm?, t1?, t2?}]}]}` | Sequence state sync |
| `s` | `{m:"s", te, hu, d8, li, po, ga, sm, sr, di, md, bp, ls, lp, lb, r:[r,g,b], rs, sp, lm, fh, pm, pv}` | Sensor data push (every 2s) |
| `m` | `{m:"m", d, w}` | Soil calibration confirmation |

### Client → Server Commands

| `m` | Command | Payload |
|---|---|---|
| `L` | LED | `{m:"L", s:0\|1, p:0-255, b:0\|1}` |
| `R` | RGB | `{m:"R", r:0-255, g:0-255, b:0-255}` |
| `Y` | Relay | `{m:"Y", s:0\|1}` |
| `V` | Servo | `{m:"V", p:0-180}` |
| `Z` | Buzzer | `{m:"Z", s:1}` |
| `D` | LCD | `{m:"D", md:0\|1, t1, t2}` |
| `P` | Pot Mapping | `{m:"P", md:0\|1\|2\|3}` |
| `M` | Soil Calibration | `{m:"M", d?, w?}` |
| `O` | OTA Update | `{m:"O", u:"firmware_url"}` |
| `X` | Sequences | `{m:"X", a:0-4, i?, s?}` |

## 🤖 Sequence Engine (v1.2)

The sequence engine replaces the old rules engine with a powerful multi-mode automation system.

### Loop Modes

| Mode | Description | Priority |
|---|---|---|
| **None** | Run sequence once and stop | 4th (lowest) |
| **Forever** | Repeat continuously | 3rd |
| **Timer** | Run for a duration or at an interval | 2nd |
| **Condition** | Run while a sensor condition is met (auto start/stop) | 1st (highest) |

### Timer Sub-Modes
- **Duration**: Loop for a specified time (hours:minutes:seconds), then stop
- **Interval**: Repeat the sequence at a fixed interval

### Condition Sub-Modes
- **Single Condition**: e.g., `temperature > 30`
- **Range Condition**: e.g., `temperature > 20 AND temperature < 30` or `distance < 5 OR distance > 100`
- Operators: `>`, `<`, `>=`, `<=`, `==`
- Debounce: 300ms to prevent rapid toggling

### Priority System
Higher-priority sequences lock devices, preventing lower-priority ones from conflicting:
1. **Condition** — Runs first, locks all its devices
2. **Timer** — Only runs on unlocked devices
3. **Forever** — Only runs on still-unlocked devices
4. **None** — Runs last

### Device Locking
Each sequence step targets a device (LED, RGB, Buzzer, Servo, Relay, LCD). When a higher-priority sequence is running, its devices are locked and cannot be overridden by lower-priority ones.

### Autostart
Sequences marked for autostart begin running on boot (except condition-based ones, which wait for their condition to become true).

### LCD Wildcards
Custom LCD text supports wildcards replaced with live sensor values:
`{temp}`, `{humidity}`, `{ds18b20}`, `{light}`, `{gas}`, `{soil}`, `{distance}`, `{pot}`, `{motion}`, `{button}`

### Example Sequences

**Night Light (Condition)**
```json
{
  "name": "Night Light",
  "loopMode": "condition",
  "loopSensor": "lightLevel",
  "loopOperator": "<",
  "loopThreshold": 20,
  "steps": [
    {"action": "led_on", "actionValue": 128, "delay": 0}
  ]
}
```

**Traffic Signal (Forever)**
```json
{
  "name": "Traffic Signal",
  "loopMode": "forever",
  "steps": [
    {"action": "rgb_red",    "delay": 5000},
    {"action": "rgb_yellow", "delay": 2000},
    {"action": "rgb_green",  "delay": 5000}
  ]
}
```

**Periodic Alert (Timer Interval)**
```json
{
  "name": "Periodic Alert",
  "loopMode": "timer",
  "timerType": "interval",
  "loopDuration": 60000,
  "steps": [
    {"action": "buzzer_on",  "delay": 500},
    {"action": "buzzer_off", "delay": 500}
  ]
}
```

### Persistence
- Up to 10 sequences stored in individual NVS keys (`seq_0` through `seq_9`)
- Legacy blob format (`seqBlob`) automatically migrated on first boot
- Runtime state (running/stopped, current step, timers) is reset on load

## 🔆 Potentiometer Mapping

Map the potentiometer to control any actuator:

| Mode | Description |
|---|---|
| **None** | Potentiometer disabled (default on startup) |
| **LED** | Controls LED PWM brightness (0-255) |
| **RGB** | Controls RGB LED through a color wheel (Red → Yellow → Green → Cyan → Blue → Magenta) |
| **Servo** | Controls servo position (0-180°) |

Set via API: `POST /api/potmapping` with `mode=none|led|rgb|servo`
Or via WebSocket: `{m:"P", md:0|1|2|3}`

## 🌱 Soil Moisture Calibration

Calibrate the soil moisture sensor for accurate readings:

- **Dry value**: Raw ADC reading in dry air/soil (default: 3000)
- **Wet value**: Raw ADC reading in water/wet soil (default: 1200)
- The firmware maps raw ADC between these values to 0-100% moisture
- Calibrate via WebSocket: `{m:"M", d:dryValue}` or `{m:"M", w:wetValue}`
- Values persist in NVS

## 🔄 OTA Firmware Updates

Update firmware over-the-air without USB connection:

1. Trigger via REST: `POST /api/ota/update` with `url=..."
2. Or via WebSocket: `{m:"O", u:"firmware_url"}`
3. ESP32 downloads the binary and writes it to flash
4. LCD shows progress in 10% increments
5. Device restarts automatically on success

**⚠️ Warning:** All NVS preferences are cleared before the update to ensure a clean state.

## 🔍 Pin Configuration Details

### PWM/LEDC Channels
- LED uses `analogWrite()` (internal LEDC channel)
- RGB LED uses 3 separate `analogWrite()` channels
- Servo uses dedicated LEDC at 50Hz / 16-bit resolution (channels 0-7 reserved for analogWrite)

### I2C Configuration
- SDA: GPIO 21 (default)
- SCL: GPIO 22 (default)
- LCD Address: 0x27

### Analog Inputs (ADC1)
- LDR: GPIO 35
- Potentiometer: GPIO 34
- MQ-2: GPIO 33
- Soil Moisture: GPIO 32

**Note**: ESP32 ADC2 pins cannot be used when WiFi is active.

## 💾 Data Persistence

Configuration stored in ESP32 NVS (Non-Volatile Storage) under namespace `iot-kit`:

| Key | Type | Description |
|---|---|---|
| `wifi_ssid` | String | Saved WiFi SSID |
| `wifi_password` | String | Saved WiFi password |
| `deviceName` | String | mDNS hostname |
| `seqCount` | Int | Number of active sequences |
| `seq_0` … `seq_9` | Bytes | Individual sequence data (sizeof Sequence) |
| `lcdDefault` | Bool | LCD display mode |
| `customText1/2` | String | Custom LCD text |
| `potMappingId` | UChar | Potentiometer mapping mode |
| `soilDry/soilWet` | Int | Soil moisture calibration |

## 📊 Memory Usage

Typical flash/RAM usage:
- Sketch size: ~1.2MB of available flash
- Global variables: ~65KB of ~320KB (20%)
- Heap available: ~250KB for dynamic allocation

## ⚡ Performance

| Metric | Value |
|---|---|
| Sensor update rate | ~1 second |
| WebSocket push interval | 2 seconds |
| Web server response time | <100ms |
| Maximum sequences | 10 |
| Maximum steps per sequence | 10 |
| Condition debounce | 300ms |
| PWM frequency | 1000Hz (LED/RGB), 50Hz (Servo) |

## 🐛 Troubleshooting

### WiFi Connection Issues
- Check credentials in code or via config portal
- Verify WiFi is 2.4GHz (ESP32 doesn't support 5GHz)
- Hold push button for 10 seconds to reset WiFi and re-enter config mode
- Check serial monitor for error messages

### mDNS Not Working
- Ensure device name was set during WiFi configuration
- Some networks block mDNS — use IP address directly as fallback
- Windows requires Bonjour/Apple mDNS service installed

### Sensor Reading Errors
- Verify wiring and pin assignments
- Check sensor power supply (3.3V or 5V)
- Test sensors individually with simple sketches
- Ensure pull-up resistors where needed (DHT, DS18B20)

### Upload Errors
- Hold BOOT button while uploading
- Try lower upload speed (115200)
- Check USB cable (must support data transfer)
- Ensure no serial monitor is open

### Library Errors
- Install all required libraries
- Check library versions (especially ArduinoJson v6.x)
- Restart Arduino IDE after installing libraries

### Web Server Not Responding
- Verify ESP32 IP address from serial monitor
- Ensure devices are on same network
- Check firewall settings
- Try accessing `/api/device` endpoint directly

### LCD Not Displaying
- Check I2C address (use I2C scanner sketch)
- Adjust LCD contrast potentiometer
- Verify I2C connections (SDA, SCL)
- Common addresses: 0x27, 0x3F

### Sequence Not Running
- Verify sequence is enabled (toggle in dashboard)
- Check `manuallyStopped` state — condition sequences won't auto-start if manually stopped
- Verify step count > 0
- Check serial output for `[SEQ]` debug messages

## 🔒 Security Considerations

⚠️ **This code is designed for local network use:**
- No authentication on API or WebSocket endpoints
- No HTTPS encryption
- WiFi password stored in plain text in NVS
- CORS set to allow all origins for dashboard access
- Suitable for trusted home/lab networks

For production use, consider adding:
- API authentication (token-based)
- HTTPS with certificates
- Input validation and sanitization
- Rate limiting on WebSocket

## 📱 Social Links

Connect with MakerBuddy:

- 🎥 **YouTube**: [@makerbuddycc](https://www.youtube.com/@makerbuddycc)
- 💻 **GitHub**: [MakerBuddy](https://github.com/MakerBuddy)
- 📘 **Facebook**: [makerbuddycc](https://www.facebook.com/makerbuddycc)
- 💼 **LinkedIn**: [MakerBuddy Company](https://www.linkedin.com/company/makerbuddy/)
- 🔧 **Hackster.io**: [makerbuddy](https://www.hackster.io/makerbuddy)
- ❤️ **Patreon**: [Support Us](https://www.patreon.com/cw/MakerBuddy)

## 📝 Version History

### v1.2 (Current)
- WebSocket communication replaces HTTP polling for real-time data
- Complete sequence engine rewrite with 4 loop modes and multi-step automation
- Potentiometer mapping to LED/RGB/Servo
- Soil moisture calibration with NVS persistence
- OTA firmware updates with LCD progress
- mDNS support for device discovery
- Direct LEDC servo control (ESP32Servo no longer required)
- Compact JSON protocol for bandwidth efficiency
- WiFi network scan caching and PMF disable for AP compatibility
- Ticker-based non-blocking architecture
- LCD wildcard support

### v1.0
- Initial release
- All sensor and actuator support
- REST API implementation
- Basic IF-THEN rules engine
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
