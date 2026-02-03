# MakerBuddy Web Dashboard

A modern, responsive web interface for real-time monitoring and control of the MakerBuddy V1.0 IoT kit.

## 🎨 Features

### 📊 Real-Time Monitoring
- **Temperature Sensors**: DHT11 and DS18B20 readings
- **Humidity**: Ambient humidity levels
- **Light Level**: LDR-based light detection
- **Gas Level**: MQ-2 gas sensor readings
- **Soil Moisture**: Plant watering indicator
- **Distance**: HC-SR04 ultrasonic measurements
- **Motion Detection**: HC-SR501 PIR sensor status
- **Button State**: Push button input status
- **Potentiometer**: Analog input value

### 🎮 Device Control
- **LED Control**: On/Off with PWM brightness adjustment (0-255)
- **RGB LED**: Full RGB color control with live preview
- **Relay**: High-power device switching
- **Servo Motor**: Position control (0-180°)
- **Buzzer**: Activate audio alerts
- **LCD Display**: Custom text display (16x2) with default/custom modes

### 🤖 Automation Engine
- Create IF-THEN automation rules
- Sensor-based triggers with customizable thresholds
- Multiple action types (LED, Buzzer, Servo, Relay, RGB)
- Enable/disable rules on the fly
- Visual status indicators

### 🎯 User Interface
- **Responsive Design**: Works seamlessly on desktop, tablet, and mobile
- **Dark/Light Theme**: Toggle between themes with localStorage persistence
- **Real-Time Updates**: Auto-refresh sensor data every 2 seconds
- **Device Information**: MAC address, uptime, and connection status
- **Collapsible Sidebar**: Optimized for different screen sizes
- **Connection Management**: Easy connect/disconnect with IP address storage

## 📁 Project Structure

```
Web Dashboard/
├── index.html          # Main dashboard interface
├── style.css           # Styling and theme definitions
├── images/
│   └── logo.png        # MakerBuddy logo
└── README.md           # This file
```

## 🚀 Setup Instructions

### Prerequisites
- Web server (Apache, XAMPP, or any HTTP server)
- Modern web browser (Chrome, Firefox, Safari, Edge)
- ESP32 device running MakerBuddy firmware
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

3. **Connect to ESP32**
   - Enter your ESP32's IP address in the sidebar
   - Click "Connect"
   - Dashboard will load with live sensor data

### Standalone Usage (No Server Required)

You can also run the dashboard directly from your file system:

```bash
# Simply open the HTML file in your browser
open index.html  # Mac
start index.html # Windows
xdg-open index.html # Linux
```

**Note**: CORS restrictions may apply when accessing ESP32 from `file://` protocol. Using a local web server is recommended.

## 🔌 API Endpoints

The dashboard communicates with ESP32 via REST API:

### GET Endpoints
- `GET /api/device` - Device information (name, MAC, uptime)
- `GET /api/sensors` - All sensor readings and actuator states
- `GET /api/rules` - List of automation rules

### POST Endpoints
- `POST /api/led` - Control LED (state, pwm, blink)
- `POST /api/rgb` - Control RGB LED (r, g, b values)
- `POST /api/relay` - Control Relay (state, delays)
- `POST /api/servo` - Control Servo (position)
- `POST /api/buzzer` - Activate Buzzer (state)
- `POST /api/lcd` - Update LCD Display (mode, text1, text2)
- `POST /api/rules` - Manage automation rules (add, toggle, delete)

## 🎨 Customization

### Theming

The dashboard supports dark and light themes. Modify `style.css` to customize colors:

```css
:root {
    /* Light Theme Variables */
    --bg-primary: #f5f5f5;
    --bg-secondary: #ffffff;
    --text-primary: #2c3e50;
    /* ... */
}

[data-theme="dark"] {
    /* Dark Theme Variables */
    --bg-primary: #1a1a1a;
    --bg-secondary: #2d2d2d;
    --text-primary: #ffffff;
    /* ... */
}
```

### Adding Custom Sensors

To add new sensors to the dashboard:

1. Add a sensor card in `index.html`:
```html
<div class="sensor-card">
    <div class="sensor-icon">🌡️</div>
    <div class="sensor-label">New Sensor</div>
    <div class="sensor-value" id="newSensor">--</div>
    <div class="sensor-unit">unit</div>
</div>
```

2. Update the sensor reading in JavaScript:
```javascript
document.getElementById('newSensor').textContent = data.newSensor;
```

### Grid Layout

The dashboard uses responsive CSS Grid:
- `.grid-5`: 5 columns for sensor cards
- `.grid-3`: 3 columns for control cards

Adjust breakpoints in `style.css` for different screen sizes.

## 📱 Mobile Support

The dashboard is fully responsive with mobile-optimized features:
- Touch-friendly controls
- Collapsible sidebar with backdrop
- Optimized spacing and font sizes
- Auto-hide sidebar when connected (mobile only)

## 🔧 Troubleshooting

### Cannot Connect to ESP32
- Verify ESP32 is powered on and connected to WiFi
- Check IP address is correct (use serial monitor to confirm)
- Ensure both devices are on the same network
- Check ESP32 serial output for connection errors

### Sensor Data Not Updating
- Check browser console for errors (F12)
- Verify ESP32 is responding to `/api/sensors` endpoint
- Check network connectivity
- Ensure sensors are properly connected to ESP32

### Dashboard Not Loading
- Clear browser cache (Ctrl+F5 or Cmd+Shift+R)
- Check browser console for errors
- Verify all files are in correct locations
- Try a different browser

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

## 📊 Performance

- Dashboard refresh rate: 2 seconds
- Typical data transfer: ~2KB per update
- Memory usage: ~15MB (browser dependent)
- Mobile performance: Optimized for 60fps

## 🔒 Security Notes

⚠️ **Important**: This dashboard is designed for local network use only.

- No authentication implemented (add if exposing to internet)
- API endpoints are unencrypted (consider HTTPS)
- Input validation on client-side only
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

### v1.0 (Current)
- Initial release
- All sensor and actuator support
- Automation rules engine
- Dark/Light theme
- Mobile responsive design

## 🤝 Contributing

Found a bug or have a feature request?
- Open an issue on [GitHub](https://github.com/MakerBuddy/MakerBuddy-V1.0/issues)
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
