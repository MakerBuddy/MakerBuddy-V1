# MakerBuddy PCB Design

**Professional PCB design files for the MakerBuddy V1.0 IoT Kit**

## 📋 Contents

This folder contains the PCB design files for MakerBuddy V1.0:

- **`MakerBuddy V1-0.fzz`** - Fritzing project file containing the full schematic and PCB layout

### Design Software
- **Fritzing** (Free, Open Source) - Required to open and edit the `.fzz` design file. Download from [fritzing.org](https://fritzing.org)


## 🎯 Planned Features

### PCB Specifications
- **Layers**: 2-layer PCB design
- **Size**: Compact form factor (TBD)
- **Components**: All sensors and actuators integrated
- **Connectors**: Easy plug-and-play connections
- **Power**: USB-C or DC barrel jack input
- **Mounting**: Standard mounting holes for enclosure

### Integrated Components
- ESP32 module socket
- Sensor terminals (DHT11, DS18B20, HC-SR04, PIR, etc.)
- Actuator connections (LED, RGB, Relay, Servo, Buzzer)
- LCD I2C connector
- Power regulation circuit (3.3V and 5V rails)
- Protection circuits (reverse polarity, overcurrent)
- Status LEDs
- Reset and Boot buttons

### Optional Features
- Expansion headers for additional sensors
- OLED display option
- Battery backup circuit
- External antenna connector
- Programming headers

## 🛠️ Current Development Status

- ✅ Circuit schematic design complete
- ✅ PCB layout complete
- ✅ Design file released (`MakerBuddy V1-0.fzz`)
- ⏳ Prototype testing
- ⏳ Manufacturing file generation (Gerber export)
- ⏳ Documentation preparation

## 📐 Design Goals

When designing the PCB, we're focusing on:

### Reliability
- Proper power distribution
- Decoupling capacitors for all ICs
- ESD protection on input pins
- Quality connectors

### User-Friendly
- Clear silkscreen labels
- Color-coded indicators
- Easy troubleshooting access points
- Modular sensor connections

### Manufacturing
- Standard component sizes
- Single-sided assembly option
- Cost-effective production
- Compatible with JLCPCB, PCBWay, etc.

### Expandability
- Breakout pins for GPIO
- I2C bus expansion
- SPI header (future use)
- UART debugging header

## 🤝 Community Contributions

Have you designed your own PCB for MakerBuddy? We'd love to see it!

- Share your design on GitHub Issues
- Submit a pull request with your files
- Post on our social media with #MakerBuddy
- Help others in the community

## 📱 Connect With Us

Follow our progress and join the community:

- 🎥 **YouTube**: [@makerbuddycc](https://www.youtube.com/@makerbuddycc) - Tutorials and project updates
- 💻 **GitHub**: [MakerBuddy](https://github.com/MakerBuddy) - Code and design files
- 📘 **Facebook**: [makerbuddycc](https://www.facebook.com/makerbuddycc) - Community discussions
- 💼 **LinkedIn**: [MakerBuddy Company](https://www.linkedin.com/company/makerbuddy/) - Professional updates
- 🔧 **Hackster.io**: [makerbuddy](https://www.hackster.io/makerbuddy) - Project showcases
- ❤️ **Patreon**: [Support Us](https://www.patreon.com/cw/MakerBuddy) - Early access to designs

## 📚 Resources

### Learn PCB Design
Looking to improve your PCB design skills?
- Free KiCad tutorials on YouTube
- Online PCB design courses
- Community forums (r/PrintedCircuitBoard, EEVblog)
- Practice with simple projects

### Recommended Tools
- **KiCad** (Free, Open Source)
- **EasyEDA** (Free, Web-based)
- **Fusion 360** (Free for hobbyists)
- **LTSpice** (Circuit simulation)

### Manufacturing Services
When ready to manufacture:
- PCBWay (Quality service)
- JLCPCB (Budget-friendly)
- OSH Park (USA, purple boards)
- Seeed Studio (Good for prototypes)

## 💰 Estimated Costs

Once released, approximate costs:
- **PCB Manufacturing**: $5-20 (5-10 pieces)
- **Components**: $30-50 (full BOM)
- **Shipping**: Varies by location
- **Total Project**: ~$50-100

*Prices are estimates and may vary based on location and suppliers.*

## ⚡ Technical Specifications (Planned)

| Specification | Details |
|---------------|---------|
| PCB Layers | 2-layer FR4 |
| Board Thickness | 1.6mm |
| Copper Weight | 1oz (35µm) |
| Min Track/Space | 6/6 mil |
| Min Drill Size | 0.3mm |
| Surface Finish | HASL / ENIG |
| Solder Mask | Green (standard) |
| Silkscreen | White |

## 🔐 License

**Creative Commons Attribution-ShareAlike 4.0 International (CC BY-SA 4.0)**

Copyright (C) 2024 Muhammad Afzal
Website: [https://makerbuddy.cc](https://makerbuddy.cc)
Email: info@makerbuddy.cc

All PCB design files are licensed under CC BY-SA 4.0, which means you are free to:
- **Share**: Copy and redistribute the material in any medium or format
- **Adapt**: Remix, transform, and build upon the material for any purpose, even commercially

Under the following terms:
- **Attribution**: You must give appropriate credit, provide a link to the license, and indicate if changes were made
- **ShareAlike**: If you remix, transform, or build upon the material, you must distribute your contributions under the same license as the original

See [LICENSE-CC-BY-SA](./LICENSE-CC-BY-SA) for the full license text or visit https://creativecommons.org/licenses/by-sa/4.0/



## 📞 Questions?

Have questions about the PCB design?

- Open an issue on [GitHub](https://github.com/MakerBuddy/MakerBuddy-V1.0/issues)
- Email us (check our website)
- Ask on our social media pages
- Join our community Discord (coming soon)

## 🎯 Support the Project

Help us continue improving MakerBuddy:

- ❤️ Support on [Patreon](https://www.patreon.com/cw/MakerBuddy) for early access
- ⭐ Star this repository to show interest
- 📢 Share with fellow makers
- 💬 Provide feedback on design priorities

## 📝 Revision History

### v1.0
- Initial PCB design release
- Fritzing project file (`MakerBuddy V1-0.fzz`) with complete schematic and layout

---

**Made with ❤️ by MakerBuddy Team**

*Building the future, one circuit at a time.* ⚡
