# MakerBuddy Arduino and WebApp Change Log

This file records release changes for the open-source Arduino firmware and
WebApp mirrored in this repository.

## 19 August 2026

### Firmware 2.3 / WebApp 2.3

- Added optional two-sensor condition rules with independent comparisons,
  `AND`/`OR` logic, and range support for each sensor.
- Preserved existing saved rules through separate versioned `seqc_N` condition
  records in ESP32 NVS.

### Firmware 2.2

- LCD wildcard validation now measures each wildcard using its current
  formatted sensor value, matching the physical 16-column display.
- Sequence LCD template validation now agrees with the WebApp and allows
  templates such as `Temperature={temp}` when the rendered value fits.

### WebApp 2.1

- LCD manual-control and rule-editor counters now calculate wildcard space from
  the current telemetry values instead of fixed worst-case token widths.
- Counters refresh as sensor values change, and source templates may exceed 16
  characters when their rendered LCD output fits.

### WebApp 2.2

- Checks the official firmware feed after connection and shows information about
  newer releases.
- Dismissed release notices are suppressed for 24 hours per firmware version.

## 17 August 2026

### Firmware 2.1

- A finishing condition step retains ownership of its actuator until its
  configured delay expires.
- Opposing condition rules retry without consuming their delay, enforcing
  actuator dwell times.

### Firmware 2.0

- Uses the configured device name consistently across LCD, mDNS, REST, and
  WebSocket identity responses.
- Shows connected WiFi/IP and device-name screens for five seconds at boot and
  in the default LCD rotation.
- Validates custom LCD templates against 16 rendered columns and compacts
  recognized wildcard tokens inside sequence storage.

### WebApp 1.9

- Added the ESP32-reported device IP address to Settings.
- Added live 16-column LCD counters for manual controls and rule steps.
- Allowed longer source templates when their rendered output fits the LCD.
