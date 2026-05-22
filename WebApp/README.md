# MakerBuddy Web Dashboard

A modern, responsive web interface for real-time monitoring and control of the MakerBuddy V1.2 IoT kit via WebSocket.

## 🆕 What's New in v1.2

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
  - ⏱️ Timer — Run for a duration or at an interval (hours:minutes:seconds)
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
- **Collapsible Sidebar**: Connection panel, device info, theme toggle, social links
- **Device Info Panel**: Name, firmware version, MAC, uptime, free/total memory
- **Connection Status**: Real-time indicator with pulse animation, last update time
- **Responsive Design**: Desktop, tablet, and mobile layouts with breakpoints

### ⚙️ Settings & Customization
- **Sensor Card Settings**: Enable/disable individual sensors, set background and accent colors
- **Actuator Card Settings**: Show/hide control cards, customize colors
- **Firmware Update**: Check for updates from `app.makerbuddy.cc`, one-click install with progress

## 📁 Project Structure

```
Web Dashboard/
├── index.html          # Main dashboard (single-page app with inline JS)
├── style.css           # Styling with CSS custom properties and dark/light themes
├── images/
│   └── logo.png        # MakerBuddy logo
└── README.md           # This file
```

## 🚀 Setup Instructions

### Prerequisites
- Web server (Apache, XAMPP, or any HTTP server)
- Modern web browser (Firefox, Safari, Edge recommended)
- ESP32 device running MakerBuddy v1.2 firmware
- Both devices on the same network

### Installation

1. **Copy Files to Web Server**
   ```bash
   # For XAMPP (Windows/Mac/Linux)
   cp -r "Web Dashboard" /path/to/xampp/htdocs/makerbuddy

   # For Apache (Linux)
   cp -r "Web Dashboard" /var/www/html/makerbuddy
   ```

2. **Access the Dashboard**
   - Open your browser
   - Navigate to: `http://localhost/makerbuddy/index.html`
   - Or use your server IP: `http://your-server-ip/makerbuddy/index.html`

3. **Connect to MakerBuddy ESP32**
   - Enter the device name (e.g., `makerbuddy-01` — `.local` auto-appended) or IP address in the sidebar
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
   - `X` — Sequence management (add/delete/start/stop/toggle/edit)

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
- Collapsible sidebar with backdrop overlay
- Sidebar auto-collapses on mobile when connected to maximize screen space
- Optimized spacing and font sizes at 480px and 768px breakpoints
- Sidebar re-shows on mobile after disconnection

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
| Typical message size | ~200-300 bytes per sensor update |
| Control latency | <20ms (WebSocket round-trip) |
| Memory usage | ~15MB (browser dependent) |
| Mobile performance | Optimized for 60fps |

## 🔒 Security Notes

⚠️ **Important**: This dashboard is designed for local network use only.

- No authentication implemented (add if exposing to internet)
- WebSocket and HTTP connections are unencrypted
- Input validation is client-side only
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

### v1.2 (Current)
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
