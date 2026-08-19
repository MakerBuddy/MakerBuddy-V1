# MakerBuddy Web Dashboard

A modern, responsive WebSocket dashboard for MakerBuddy. Current WebApp version: **2.3**.

## What's new in v2.2

- Automatically checks the official firmware feed after connecting and shows a release notice when newer firmware is available.
- Dismissing a release notice suppresses that same version for 24 hours.

## What's new in v2.1

- Rule Engine locks now disable the matching LED, RGB LED, and Servo options in Potentiometer Mapping, with a visible explanation for each unavailable target.

## What's new in v2.0

- Applied the MakerBuddy violet-and-amber brand palette to light and dark themes.
- Amber now carries primary actions and active states with accessible near-black labels.
- Warning states use yellow, and interactive controls have visible amber focus rings.

## 🆕 What's New in v1.9

- **Device IP Address** — The Settings device-information card now displays the MakerBuddy IoT Kit IP address reported by the ESP32 identity message.
- **Wildcard-Aware LCD Length** — LCD fields count each wildcard by its maximum rendered width, show a live 16-column counter, and allow useful templates whose source token names exceed 16 characters.
- **Version** — WebApp bumped to 1.9 on 8 August 2026.

### v1.8 Features (also included)

- **Unified Top Bar** — Chrome-extension-style social links, status, theme, device identity, and disconnect controls replace the legacy sidebar.
- **Connected Device Name** — The configured name from the ESP32 identity response remains visible even when connecting by IP address.
- **Legacy Firmware Warning** — The old hard-coded `MakerBuddy IoT Kit` identity is identified as legacy firmware instead of being presented as the configured name.
- **Device Information in Settings** — Firmware, MAC, uptime, and memory details moved into a responsive Settings card.
- **Rule List First** — Existing rules appear before the create/edit form in both dashboard clients.
- **Version** — WebApp bumped to 1.8 on 8 August 2026.

### v1.7 Features (also included)

- **Timer UI Simplified** — "Interval" label replaces "⏱️ Timer Mode" + radio buttons. Duration option removed; all timer rules use interval repeat.
- **Step Values Fix** — Editing a step's Value or Delay no longer resets when adding/removing steps. DOM inputs now sync to memory before re-rendering.
- **LED On Default Value** — Selecting LED On action auto-sets the value to 255 (max brightness).
- **Servo Value Limits** — Servo value input restricted to 0-180 with proper min/max attributes.
- **Chrome Extension Parity** — Same fixes and UI improvements applied to the Chrome Extension dashboard.

### v1.6 Features (also included)

- **Push Button Sensor Card** — Live Pressed/Released state display for the onboard push button (also used for WiFi reset).
- **Relay Delay Protocol Reservation** — WebSocket relay messages include compact `od`/`fd` fields, but current firmware ignores them and switches immediately.

### v1.2 Features (also included)
- **WebSocket Communication** — Real-time bidirectional data replaces HTTP polling. Instant sensor updates and actuator control.
- **Rule Engine Tab** — Complete redesign with create/edit/manage workflow, 4 loop modes, up to 10 steps per rule, timer and condition settings.
- **Settings Tab** — Customize sensor/actuator card visibility, background colors, and accent colors. All persisted to localStorage.
- **Potentiometer Mapping Card** — Map the potentiometer to LED PWM, RGB color wheel, or Servo position with radio buttons.
- **Soil Moisture Calibration Modal** — Live raw ADC capture with dry/wet calibration save buttons via WebSocket.
- **OTA Firmware Update UI** — Check for updates, view current/latest versions, install with progress tracking.
- **Automation Indicators** — Actuator cards show warning banners and disable when controlled by active rule engine.
- **Tab Navigation** — Dashboard, Rule Engine, and Settings tabs with animated transitions.
- **mDNS Support** — Auto-append `.local` for easy device discovery.
- **Servo Throttle** — 50ms debounce on slider to prevent flooding the ESP32 WebSocket.

## 🎨 Features

### 📊 Real-Time Monitoring
- **Temperature Sensors**: DHT11 and DS18B20 readings
- **Humidity**: Ambient humidity levels
- **Light Level**: LDR-based light detection
- **Gas Level**: MQ-2 gas sensor readings
- **Soil Moisture**: Plant watering indicator (with calibration modal)
- **Distance**: HC-SR04 ultrasonic measurements
- **Motion Detection**: HC-SR501 PIR sensor status
- **Button State**: Push button input status
- **Potentiometer**: Analog input value

### 🎮 Device Control
- **LED Control**: On/Off toggle with PWM brightness slider (0-255)
- **RGB LED**: Three-channel sliders + number inputs with live color preview
- **Relay**: Power switching toggle
- **Servo Motor**: Position slider (0-180°) with 50ms throttle
- **Buzzer**: Audio alert button
- **LCD Display**: 16x2 display with default/custom mode, wildcard support

### 🤖 Rule Engine (v2)
- **Create Rules** with name, loop mode, and multiple steps
- **4 Loop Modes**:
  - ⏹️ None — Run once and stop
  - ♾️ Forever — Repeat continuously
  - ⏱️ Timer — Repeat a completed sequence after a fixed interval (hours:minutes:seconds)
  - 🎯 Condition — Auto-start/stop based on sensor thresholds
- **Multi-Step Rules** — Up to 10 steps per rule with individual actions, values, and delays
- **Condition Options** — Single threshold or range (AND/OR) with 10 sensor types
- **Rule Management Table** — View all rules with status, loop details, and inline actions
- **Edit Rules** — Click to re-populate the form and modify existing rules
- **Live Status Badges** — Running, Monitoring, Stopped, Manually Stopped
- **Automation Lock** — Actuator cards show warning banner and disable when controlled by active rules

### 🎯 User Interface
- **Three Tabs**: Dashboard, Rule Engine, Settings — with animated switching
- **Dark/Light Theme**: Toggle with localStorage persistence
- **Top Connection Bar**: Social links, connection status, configured device name, theme, and disconnect controls
- **Device Info Panel**: Name, IP address, firmware version, MAC, uptime, and memory details in the Settings tab
- **Connection Status**: Real-time indicator with pulse animation, last update time
- **Responsive Design**: Desktop, tablet, and mobile layouts with breakpoints

### ⚙️ Settings & Customization
- **Sensor Card Settings**: Enable/disable individual sensors, set background and accent colors
- **Actuator Card Settings**: Show/hide control cards, customize colors
- **Firmware Update**: Check for updates from `app.makerbuddy.cc`, one-click install with progress

## 📁 Project Structure

```
WebApp/
├── index.html          # Main dashboard (single-page app with inline JS)
├── style.css           # Styling with CSS custom properties and dark/light themes
├── images/
│   └── logo.png        # MakerBuddy logo
└── README.md           # This file
```

The release version is maintained in the `application-version` metadata and
source header in `index.html`, the header in `style.css`, and this README.

## 🚀 Setup Instructions

### Prerequisites
- Web server (Apache, XAMPP, or any HTTP server)
- Modern web browser (Firefox, Safari, Edge recommended)
- ESP32 device running MakerBuddy firmware
- Both devices on the same network

### Installation

1. **Copy Files to Web Server**
   ```bash
   # For XAMPP (Windows/Mac/Linux)
   cp -r WebApp /path/to/xampp/htdocs/makerbuddy

   # For Apache (Linux)
   cp -r WebApp /var/www/html/makerbuddy
   ```

2. **Access the Dashboard**
   - Open your browser
   - Navigate to: `http://localhost/makerbuddy/index.html`
   - Or use your server IP: `http://your-server-ip/makerbuddy/index.html`

3. **Connect to MakerBuddy ESP32**
   - Enter the device name (e.g., `makerbuddy-01` — `.local` auto-appended) or IP address in the centered connection panel
   - Click "Connect"
   - Dashboard establishes WebSocket connection and loads with live data

### Standalone Usage (No Server Required)

You can also open the HTML file directly, but using a local web server is recommended for CORS compatibility:

```bash
open index.html  # Mac
start index.html # Windows
xdg-open index.html # Linux
```

## 🔌 Communication Protocol

The dashboard communicates with the ESP32 exclusively via **WebSocket** at `ws://<device>/ws` using a compact JSON protocol.

### Data Flow
1. On connect, the ESP32 sends device info (`m:"i"`), firmware info (`m:"f"`), and sequence state (`m:"q"`)
2. Sensor data is pushed automatically every 2 seconds (`m:"s"`)
3. Actuator commands are sent with single-letter message types:
   - `L` — LED, `R` — RGB, `Y` — Relay, `V` — Servo, `Z` — Buzzer, `D` — LCD
   - `P` — Potentiometer mapping, `M` — Soil calibration, `O` — OTA update
   - `X` — Sequence management (add/delete/start/stop/toggle). Editing is implemented in the client as delete followed by add.

### Compact JSON Keys
| Key | Meaning | Key | Meaning |
|---|---|---|---|
| `te` | Temperature | `md` | Motion Detected |
| `hu` | Humidity | `bp` | Button Pressed |
| `d8` | DS18B20 Temp | `ls` | LED State |
| `li` | Light Level | `lp` | LED PWM |
| `ga` | Gas Level | `lb` | LED Blinking |
| `sm` | Soil Moisture | `rs` | Relay State |
| `sr` | Soil Raw ADC | `sp` | Servo Position |
| `di` | Distance | `lm` | LCD Mode |
| `po` | Potentiometer | `fh` | Free Heap |
| `pm` | Pot Mapping | `pv` | Pot Mapped Value |

## 🎨 Theming & Customization

### Theme System

The dashboard uses CSS custom properties with `:root` (light) and `[data-theme="dark"]` selectors. Theme preference is persisted to `localStorage`.

```css
:root {
    --primary-color: #2563eb;
    --bg-primary: #f0f4f8;
    --bg-secondary: #ffffff;
    --text-primary: #0f172a;
    /* ... 30+ tokens */
}

[data-theme="dark"] {
    --bg-primary: #0f172a;
    --bg-secondary: #1e293b;
    --text-primary: #f1f5f9;
    /* ... same tokens, dark values */
}
```

### Dashboard Settings (Settings Tab)

All settings are stored in `localStorage` under key `dashboardSettings`:

```json
{
  "sensors": {
    "temp_dht": {"enabled": true, "bgColor": "#ffffff", "accentColor": "#2563eb"},
    "humidity": {"enabled": true, "bgColor": "#ffffff", "accentColor": "#2563eb"}
    // ... all 10 sensors
  },
  "actuators": {
    "led": {"enabled": true, "bgColor": "#ffffff", "accentColor": "#2563eb"}
    // ... all 7 actuators
  }
}
```

### Responsive Grid Layout

| Class | Default | 1400px | 1200px | 768px |
|---|---|---|---|---|
| `.grid-5` | 5 cols | 4 cols | 3 cols | 1 col |
| `.grid-4` | 4 cols | — | — | 1 col |
| `.grid-3` | 3 cols | — | — | 1 col |

## 📱 Mobile Support

- Touch-friendly controls with larger tap targets
- Responsive top-bar social icons and connection controls
- Optimized spacing and font sizes at 480px and 768px breakpoints

## 🔧 Troubleshooting

### Cannot Connect to ESP32
- Verify ESP32 is powered on and connected to WiFi
- Check IP address or device name is correct
- Ensure both devices are on the same network (WebSocket requires direct LAN access)
- Try using the IP address directly instead of mDNS name

### Sensor Data Not Updating
- Check browser console for WebSocket errors (F12)
- Verify the status indicator shows green (connected)
- Check ESP32 serial output for errors
- Refresh the page and reconnect

### Dashboard Not Loading
- Clear browser cache (Ctrl+F5 or Cmd+Shift+R)
- Check browser console for errors
- Verify all files are in correct locations on the web server
- Try a different browser

### Rule Engine Not Working
- Ensure device is connected (green status indicator)
- Verify the rule is enabled (toggle in the rules table)
- Check that steps have valid actions selected
- For condition-based rules, verify the sensor condition is actually met

### Theme Not Persisting
- Check browser localStorage is enabled
- Clear site data and try again
- Ensure cookies are not blocked

## 🌐 Browser Compatibility

Tested and working on:
- ✅ Firefox (v88+)
- ✅ Safari (v14+)
- ✅ Edge (v90+)
- ✅ Opera (v76+)
- ⚠️ Chrome — requires [MakerBuddy Chrome Extension](https://chromewebstore.google.com/detail/makerbuddy-iot-controller/jdhfgbalmimccengljapbgfoaldmleah) for full functionality

## 📊 Performance

| Metric | Value |
|---|---|
| Data refresh | Real-time via WebSocket push (2s interval) |
| Sequence capacity | 10 rules, 10 steps per rule |
| Servo input throttle | 50ms in the WebApp |

Browser memory, frame rate, payload size, and network latency vary by device and
are not fixed guarantees of the source.

## 🔒 Security Notes

⚠️ **Important**: This dashboard is designed for local network use only.

- No authentication implemented (add if exposing to internet)
- WebSocket and HTTP connections are unencrypted
- Firmware performs bounds checks for many protocol fields, but there is no authentication or authorization
- Suitable for home/lab environments

## 📱 Social Links

Stay connected with MakerBuddy:

- 🎥 **YouTube**: [@makerbuddycc](https://www.youtube.com/@makerbuddycc)
- 💻 **GitHub**: [MakerBuddy](https://github.com/MakerBuddy)
- 📘 **Facebook**: [makerbuddycc](https://www.facebook.com/makerbuddycc)
- 💼 **LinkedIn**: [MakerBuddy Company](https://www.linkedin.com/company/makerbuddy/)
- 🔧 **Hackster.io**: [makerbuddy](https://www.hackster.io/makerbuddy)
- ❤️ **Patreon**: [Support Us](https://www.patreon.com/cw/MakerBuddy)

## 📝 Version History

### v1.9 (Current)

- MakerBuddy IoT Kit IP address added to the Settings device-information card.
- LCD inputs validate wildcard-expanded display width and show live column counts.

### v1.8
- Sidebar removed and social links moved into the responsive top bar
- Configured device name displayed for both hostname and IP connections
- Legacy hard-coded firmware identity detected and reported clearly
- Device information moved into the Settings tab
- Rule table displayed before the create/edit form

### v1.7
- Timer UI simplified (Interval label, Duration removed)
- Step values persist when adding/removing steps (DOM sync fix)
- LED On defaults to value 255; Servo value restricted to 0-180
- Chrome Extension parity

### v1.6
- Push button sensor card with live Pressed/Released state display
- Reserved relay fields (`od`/`fd`) are sent by the UI; current firmware ignores them

### v1.2
- WebSocket communication replaces HTTP polling
- Rule Engine tab with create/edit/manage workflow and 4 loop modes
- Multi-step rules (up to 10 steps) with priority-based device locking
- Settings tab with sensor/actuator card visibility and color customization
- Potentiometer mapping card (None/LED/RGB/Servo)
- Soil moisture calibration modal with live ADC capture
- OTA firmware update check and install UI
- Automation indicators on actuator cards (warning banner + disable)
- Tab navigation (Dashboard, Rule Engine, Settings)
- mDNS support with `.local` auto-append
- Servo slider throttle (50ms debounce)
- Compact WebSocket JSON protocol
- Dark/light theme persistence

### v1.0
- Initial release
- All sensor and actuator support
- HTTP polling-based updates
- Simple IF-THEN rules
- Dark/Light theme
- Mobile responsive design

## 🤝 Contributing

Found a bug or have a feature request?
- Open an issue on [GitHub](https://github.com/MakerBuddy/MakerBuddy-V1/issues)
- Submit a pull request
- Share your customizations!

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
