# M5Stick-KeySender

ESP32-based Bluetooth LE keyboard automation controlled over Wi-Fi.

**Project status: Documentation and repository setup in progress. Firmware source has not been published yet.**

## Overview

M5Stick-KeySender is a small project intended to let an M5StickC/ESP32 act as a Bluetooth LE keyboard, sending configured keystrokes to a paired computer. The original project was built for remote and recurring keystrokes, including automation scenarios with Home Assistant.

## Intended features

- Bluetooth Low Energy (BLE) HID keyboard emulation
- Wi-Fi remote control using an HTTP API
- Start/stop control from the M5Stick button
- Periodic key presses with a configurable interval
- Keyboard shortcuts, including a double Scroll Lock action
- Home Assistant integration and status feedback

These features describe the previous working project. The actual source code and its precise API behavior still need to be added and verified before publishing setup instructions.

## Hardware and software

- M5StickC / compatible ESP32 device (exact tested model to be confirmed)
- A computer supporting Bluetooth LE keyboards
- Wi-Fi network for remote control
- Arduino/ESP32 development environment (exact dependency versions to be documented)

## Planned repository structure

```text
M5Stick-KeySender/
├── README.md
├── LICENSE
├── CONTRIBUTING.md
├── .gitignore
├── firmware/              # Original Arduino/ESP32 source (pending)
└── docs/                  # Wiring, setup, API and HA examples (pending)
```

## Getting started

The tested firmware has **not yet been uploaded**. Please do not flash unverified code or rely on the described API until the original implementation is added.

Once firmware is available, this section will describe installation, BLE pairing, Wi-Fi configuration and configuration of key sequences.

## Security notes

The device is designed to generate keystrokes on a paired computer. Only pair it with computers you own or are authorized to control. When the HTTP interface is implemented, keep it on a trusted network and add authentication or other access restrictions before exposing it beyond that network. Do not commit Wi-Fi passwords, API tokens or other secrets.

## Contributing

Issues, documentation improvements and pull requests are welcome. See [CONTRIBUTING.md](CONTRIBUTING.md).

## License

MIT License — Copyright (c) 2026 Nolaris Solutions AB. See [LICENSE](LICENSE).
