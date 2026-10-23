# MakerBuddy Arduino Code

MakerBuddy ESP32 firmware v2.10 with WebSocket server, sensor management, and sequence-based automation engine.

## Rule status updates — v2.10 (23 October 2026)

- Push the existing `q` rule list to every connected client whenever a rule starts, finishes or stops on its own (No Loop completion, Condition start/stop) and after REST rule changes, at most once per second. Previously `q` was sent only on connect and after WebSocket `X` commands, so dashboards kept showing “▶️ Running” and kept manual controls locked. No new message type: older clients already apply `q` at any time.
- Start on a Condition rule (WebSocket `X a=2` or REST `action=start`) now clears the manual stop and resumes monitoring with a fresh condition debounce. The first cycle begins at step 0 through the priority engine once the condition is true; while it is false nothing runs. Previously the rule was marked running mid-cycle, so step 0 was skipped (or steps 1+ ran once when the condition was false). Non-condition starts are unchanged.
- Timer intervals are parsed as unsigned 32-bit values over WebSocket and REST, so the dashboards' 999 h maximum is stored as sent instead of becoming 0 ms (WebSocket) or saturating near 596 h (REST).
- GPIO assignments, NVS layouts and protocol keys are unchanged.

## Servo mapping rule locks — v2.9 (30 September 2026)

- Disable Servo Mapping sensor/inversion controls and the potentiometer Servo option while an enabled, running rule contains a servo step, including timer waits and non-servo steps. Enabled condition rules also reserve mapping while monitoring.
- Firmware 2.9 pauses existing sensor/pot servo mapping throughout that reservation, holds the last angle, and rejects mapping enables through REST/WebSocket. Stop, disable, delete or one-shot completion releases it; an existing mapping resumes smoothly.
- None and unrelated potentiometer targets remain available subject to their own rule locks. Saved mapping settings and rule-to-rule step priority are preserved.

## Servo PWM isolation — v2.8 (30 September 2026)

- Reserve ESP32 LEDC channel 8 in a separate timer bank for the servo on GPIO 19. RGB and single-LED automatic channels remain in the other bank.
- Prevent the timer sharing possible with automatic channel allocation on earlier Arduino-ESP32 3.x cores; a separate pin/channel alone does not isolate PWM timers.
- Keep the 50 Hz, 16-bit, 500–2500 µs signal holding the last commanded angle after a rule ends or enters its timer interval. Relative +10° from 73° remains 83° with mappings/other servo commands absent.
- Report a PWM attachment failure on Serial. GPIO assignments, protocol, rule priority, movement modes, and stored rule formats are unchanged.

## Servo rule movements — v2.7 (30 September 2026)

- Fix servo movement dropdown and duration controls with matching themed fields, visible labels, touch-sized inputs, and a stacked narrow layout.
- Add one-way Start → destination and Start → destination → start movements with bounded 0–180° angles, duration per direction, and hold after the complete movement. Choose Forever to repeat continuously.
- Firmware 2.7 is required for sweeps. Each cycle positions at the configured start angle first; one-way repeats reposition at start. A 0 ms return settles at start. These are positional servo movements within 0–180°, not unrestricted motor rotation.
- Preserve rule priority and non-blocking motion. Version-2 `seqv_N` companion records load version-1 records without changing legacy `Sequence` or `SequenceStep` layouts.

## 📋 Overview

This Arduino sketch provides a complete IoT solution for the MakerBuddy kit, featuring:
- Multi-sensor data acquisition
- Real-time WebSocket communication (replaces HTTP polling)
- Actuator control via REST API and WebSocket
- Advanced sequence engine with 4 loop modes and multi-step automation
- WiFi connectivity with AP provisioning, station mode, mDNS, and standalone operation after a saved-network failure
- OTA (over-the-air) firmware updates
- LCD wildcard support for live sensor values
- Persistent configuration storage with Preferences/NVS

The canonical runtime version is `FIRMWARE_VERSION` in `MakerBuddy.ino`; its
source header, `MakerBuddyTypes.h`, this README, and `ARCHITECTURE.md` must stay
aligned for every firmware release.

## Servo direction fix — v2.6 (30 September 2026)

- Fix downward servo interpolation: unsigned elapsed time no longer wraps a negative angle change and drives the servo to 180°.
- LDR and other sensor mappings keep responding as readings rise and fall, including with inversion. The shared fix also covers potentiometer mapping and smooth absolute/relative rule steps.
- No protocol, GPIO, mapping defaults, or persisted record changes. Existing browser and mobile clients use the corrected firmware behavior.

## Simple Servo Mapping — v2.5 (30 September 2026)

- A separate Servo Mapping card contains a compact sensor dropdown (None, soil moisture, gas, light, DHT11 temperature, DS18B20 temperature, humidity) and Invert direction. Changes apply immediately; None releases sensor mapping for manual control.
- Sensor ranges automatically translate to 0–180°: percentages use 0–100, DHT11 uses 0–50°C, and DS18B20 uses -55–125°C. Potentiometer, motion, push button, and distance are excluded from this dropdown.
- Smoothing and deadband use built-in defaults. Rules retain priority, invalid or stale readings hold position, and mapping remains disabled after reboot.
- Browser and mobile demos continuously translate their displayed sensor readings. Card visibility is independently configurable.

## Servo automation — v2.4 (30 September 2026)

- Map soil moisture, gas, light, DHT11 temperature, DS18B20 temperature, or humidity to a bounded servo range, with inversion, EMA smoothing, and deadband.
- Create absolute or relative servo rule steps with non-blocking movement duration and a separate hold time; targets clamp to 0–180°.
- Firmware arbitrates servo ownership across rules, sensor mapping, potentiometer mapping, and manual commands. Invalid/stale mapping readings hold position.
- Mapping settings persist but mapping remains disabled after restart. Existing rule records retain their byte layout; versioned companion records store new servo options.
- New mapping and movement options require firmware 2.4. No additional robotics features are included.

DS18B20 conversions now run asynchronously to avoid blocking servo interpolation.
Smooth movement uses an approximately 20 ms loop update opportunity; legacy buzzer
pulses, ultrasonic timeout, provisioning, and OTA can still delay loop work.

## 🆕 What's New in v2.1

- **Condition-Step Ownership** — When a condition turns false mid-step, that
  step retains ownership of its current actuator until its configured delay
  expires. Competing condition rules retry instead of consuming their delay
  while blocked.
- **Minimum Dwell Guarantee** — Opposing rules such as servo `90°` for 10000 ms
  and servo `0°` for 1000 ms can no longer overwrite the active 10-second step
  early.
- **Version** — Firmware bumped to 2.1 on 16 August 2026.

### v2.0 Features (also included)

- **Reliable Device Identity** — The configured WiFi-provisioning name is normalized, loaded once from NVS, and used consistently by LCD, mDNS, REST, and WebSocket identity responses.
- **No Misleading Name Fallback** — REST and WebSocket now return an empty `dn` when no name is configured instead of reporting `MakerBuddy IoT Kit` as if it were the saved name.
- **Identity Diagnostics** — Boot output at 115200 baud reports the configured device name for deployment verification.
- **LCD Network Identity Timing** — Connected SSID/IP and the saved device name are each shown for 5 seconds at boot and remain available in the default LCD rotation.
- **Wildcard-Aware LCD Width** — Custom LCD lines are validated against their 16 displayed columns. Wildcard templates in sequence steps use compact internal markers, preserving the existing NVS struct while allowing source text such as `{humidity}{distance}`.
- **Version** — Firmware bumped to 2.0 on 8 August 2026.

### v1.9 Features (also included)

- **Timer Mode: Interval Only** — Duration option removed from `SeqTimerType` enum. `seqTimerTypeFromString()` and `seqTimerTypeToString()` always return "interval". `processSequences()` timer branch simplified — always parks at end for interval wait. REST and WebSocket handlers default to "interval".
- **Buzzer Conflict Fix** — `markSequenceDevices()` now locks only the currently executing step's device (not all steps). When a sequence is parked at end (`currentStep >= stepCount`), no devices are locked — releasing the buzzer and other actuators for concurrent rules.
- **Types Header v1.9** — Enum cleanup with `STT_DURATION` removed.
- **Version** — Firmware bumped to 1.9.

### v1.8 Features (also included)

- **Optimized NVS Storage** — Dirty bitmask tracking writes only changed sequence slots. Compact format saves ~336-432 bytes per rule by storing only the header and used steps. Orphaned NVS entries (`seq_N`, `seqms_N`) are automatically cleaned up on boot.
- **ADC Noise Reduction** — 4-read averaging per analog pin (LDR, Potentiometer, MQ-2, Soil Moisture) for cleaner sensor readings.
- **WiFi Power Saving** — Falls back to `WIFI_OFF` on connection failure instead of staying in AP mode, reducing power consumption.
- **Relay Delay Protocol Reservation** — Dashboards send `od` and `fd` fields, but firmware currently ignores them and switches the relay immediately.
- **Sequence Save Performance** — Per-slot dirty bitmask prevents unnecessary NVS writes, avoiding flash wear and reducing save latency.

### v1.2 Features (also included)
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
ArduinoJson by Benoit Blanchon (v6.x API; compiles with the installed 7.4.2)
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

**Configuration Portal**

The empty `ssid`/`password` constants near the top of the sketch are not used by
`connectToWiFi()`; provisioning through Preferences is the supported path.

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
- `dn` contains the normalized name saved during WiFi provisioning and is empty when no name is configured. The firmware also prints the loaded name to Serial Monitor during boot.
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
           step0_action, step0_value, step0_delay, step0_relative?, step0_movementMs?, step0_sweep?, step0_startAngle?, step0_lcdMode?, step0_lcdText1?, step0_lcdText2?, ...
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
| `j` | `{m:"j", sc, so, si, ss, tg, sv}` | Servo ownership and mapping settings on connect; these fields also appear in `s` |
| `e` | `{m:"e", error}` | Mapping validation or manual servo ownership rejection |
| `m` | `{m:"m", d, w}` | Soil calibration confirmation |

### Client → Server Commands

| `m` | Command | Payload |
|---|---|---|
| `L` | LED | `{m:"L", s:0\|1, p:0-255, b:0\|1}` |
| `R` | RGB | `{m:"R", r:0-255, g:0-255, b:0-255}` |
| `Y` | Relay | `{m:"Y", s:0\|1, od:0, fd:0}` |
| `V` | Servo | `{m:"V", p:0-180}` |
| `Z` | Buzzer | `{m:"Z", s:1}` |
| `D` | LCD | `{m:"D", md:0\|1, t1, t2}` |
| `P` | Pot Mapping | `{m:"P", md:0\|1\|2\|3}` |
| `J` | Sensor Servo Mapping | `{m:"J", en, sn, il, ih, al, ah, iv, db, sm}` |
| `M` | Soil Calibration | `{m:"M", d?, w?}` |
| `O` | OTA Update | `{m:"O", u:"firmware_url"}` |
| `X` | Sequences | `{m:"X", a:0-4, i?, s?}` |

Incoming WebSocket text frames are limited to 4096 bytes. Servo steps may add
`rv:1` for a relative angle and `mv` for movement duration (0–3600000 ms).
Firmware 2.7 advertises `sc:1` for start/destination sweeps: `sw:1` means one
way, `sw:2` means return, `sa` is start (0–180°), and `v` is destination.
`mv` applies to each direction and `d` holds after the entire movement. Each
cycle positions at start first. Forever repeats within 0–180°; one-way cycles
reposition to start. Zero-duration return settles at start. Sweeps force `rv:0`.
REST form equivalents are `stepN_sweep` and `stepN_startAngle`.
`d` remains the delay for other actions and is the hold after arrival for servo
steps. Missing `rv`/`mv` preserves legacy absolute instant behavior.

`POST /api/servomapping` accepts the same numeric mapping fields as form body
parameters. A simple `{m:"J", en:1, sn:5, iv:0}` command uses automatic input
limits and a 0–180° output; optional legacy ranges remain supported.
`sn` is 0 DHT temperature, 1 humidity, 2 DS18B20, 3 light, 4 gas,
or 5 soil. `il/ih` are input limits, `al/ah` angle limits, `iv` inversion,
`db` deadband (1–10°), and `sm` new-sample weight (1–100%). `en` enables the
mapping. REST `/api/servo` returns 409 while a rule or mapping owns the servo.

## 🤖 Sequence Engine (v2.0)

The sequence engine replaces the old rules engine with a powerful multi-mode automation system.

### Loop Modes

| Mode | Description | Priority |
|---|---|---|
| **None** | Run sequence once and stop | 4th (lowest) |
| **Forever** | Repeat continuously | 3rd |
| **Timer** | Run a cycle, then repeat after a fixed interval | 2nd |
| **Condition** | Run while a sensor condition is met (auto start/stop) | 1st (highest) |

### Timer Behavior
- **Interval only**: Run a complete cycle, park at the end, wait the configured interval, then repeat. The wait starts after the cycle completes.

### Condition Sub-Modes
- **Single Condition**: e.g., `temperature > 30`
- **Range Condition**: e.g., `temperature > 20 AND temperature < 30` or `distance < 5 OR distance > 100`
- Operators: `>`, `<`, `>=`, `<=`, `==`
- Debounce: 300ms to prevent rapid toggling

### Priority System
Higher-priority sequences lock devices, preventing lower-priority ones from conflicting:
1. **Condition** — Runs first and locks its current step's device
2. **Timer** — Only runs on unlocked devices
3. **Forever** — Only runs on still-unlocked devices
4. **None** — Runs last

### Device Locking
Each sequence step targets a device (LED, RGB, Buzzer, Servo, Relay, LCD). A running higher-priority sequence locks only its current step's device against lower-priority tiers. A timer parked at the end of its cycle locks nothing.

### Autostart
Sequences marked for autostart begin running on boot (except condition-based ones, which wait for their condition to become true).

### LCD Wildcards
Custom LCD text supports wildcards replaced with live sensor values:
`{temp}`, `{humidity}`, `{ds18b20}`, `{light}`, `{gas}`, `{soil}`, `{distance}`, `{pot}`, `{motion}`, `{button}`

The 16-column limit is based on the maximum rendered width, not the wildcard
name. `{temp}`, `{humidity}`, `{ds18b20}`, and `{distance}` count as 5 columns;
the other supported wildcards count as 3. Sequence templates compact each
recognized wildcard to a one-byte internal marker for NVS storage and decode it
again in REST/WebSocket responses.

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
- Preferences namespace: `iot-kit`
- Up to 10 sequences stored in individual NVS keys (`seq_0` through `seq_9`)
- **Dirty bitmask tracking** — only modified slots are written to NVS, avoiding unnecessary flash wear
- **Compact format** — stores only the header and used steps instead of the full struct, saving ~336-432 bytes per rule
- **Orphan cleanup** — stale `seq_N` and `seqms_N` keys for unused slots are automatically removed on boot
- Legacy blob format (`seqBlob`) automatically migrated to compact format on first boot
- Runtime state (running/stopped, current step, timers) is sanitized on load
- Version-2 `seqv_N` companion records preserve relative/movement/sweep/start settings and read version-1 records without resizing legacy structs. Missing companions load as absolute/instant; unused companions are removed.
- `servoMap` preserves mapping ranges and filtering options. Sensor mapping and pot mapping always start disabled after boot.

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

**⚠️ Warning:** The complete `iot-kit` Preferences namespace is cleared before
the firmware download is attempted. WiFi credentials, device name, sequences,
LCD state, pot mapping, and soil calibration are lost even if OTA later fails.

## 🔍 Pin Configuration Details

### PWM/LEDC Channels
- LED uses `analogWrite()` (internal LEDC channel)
- RGB LED uses 3 separate `analogWrite()` channels
- Servo uses the pin-based LEDC API at 50Hz / 16-bit resolution with a 500-2500 µs pulse mapping

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
| `seq_0` … `seq_9` | Bytes | Compact per-sequence header + used steps + step count |
| `seqms_0` … `seqms_9` | Bool | Separately persisted manually-stopped state |
| `lcdDefault` | Bool | LCD display mode |
| `customText1/2` | String | Custom LCD text |
| `potMappingId` | UChar | Potentiometer mapping mode |
| `soilDry/soilWet` | Int | Soil moisture calibration |

## 📊 Memory Visibility

Build size depends on the installed ESP32 core and library versions. Runtime free
heap is exposed as `fh` by REST and WebSocket telemetry and logged where useful;
do not rely on fixed RAM/flash estimates in documentation.

## ⚡ Performance

| Metric | Value |
|---|---|
| Sensor update rate | ~1 second |
| WebSocket push interval | 2 seconds |
| Maximum sequences | 10 |
| Maximum steps per sequence | 10 |
| Condition debounce | 300ms |
| Servo PWM | 50Hz, 16-bit LEDC |

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

### v2.6 (Current)
- Fix downward sensor/pot servo mapping and smooth rule movement getting clamped to 180°
- Regression tests cover repeated sensor reversals, default LDR smoothing/inversion, and descending absolute/relative rules

### v2.1
- Condition steps retain current-actuator ownership while finishing their dwell
- Blocked condition rules retry without starting or consuming their step delay
- Opposing actuator rules therefore respect the active step's minimum duration

### v2.0
- Configured device name used consistently by LCD, mDNS, REST, and WebSocket
- Device names normalized to 32-character mDNS-safe values
- Empty identity returned when no device name is configured
- Five-second WiFi/IP and device-name LCD screens
- Boot-time Serial diagnostic for the loaded device name
- Wildcard-aware 16-column LCD validation without changing persisted struct sizes

### v1.9
- Timer mode: Duration removed, Interval only. Simplified processSequences() timer branch.
- Buzzer conflict fix: markSequenceDevices() locks only current step, releases parked devices.
- Types header v1.9 (STT_DURATION removed from SeqTimerType enum).
- REST/WS defaults changed to "interval".

### v1.8
- Optimized NVS storage with dirty bitmask tracking — only changed slots written
- Compact sequence save format (header + used steps, saves ~336-432 bytes per rule)
- Orphaned NVS entry cleanup (stale `seq_N`/`seqms_N` keys removed on boot)
- ADC noise reduction: 4-read averaging per analog pin for cleaner readings
- Relay delay protocol fields (`od`/`fd`) added to WebSocket for future timed relay support
- WiFi power saving: falls back to `WIFI_OFF` on connection failure
- Legacy pot mapping string-to-UChar migration

### v1.2
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
