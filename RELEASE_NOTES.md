# Release notes — v1.0.0

**M5Stick-KeySender — initial public release**

First public version of the M5StickC BLE keyboard sender by Nolaris Solutions AB.

## Features

- Emulate a Bluetooth LE keyboard using the original M5StickC.
- Send individual or repeated key combinations, including double Scroll Lock (`SCROLLLOCKX2`).
- Configure a repeating interval and start, stop, toggle or trigger a single send.
- Control the device from the English-language web interface over local Wi-Fi.
- Integrate with Home Assistant or other tools using the HTTP API.
- View Bluetooth, Wi-Fi, sending status and transmission count.
- Use a local `secrets.h` file for Wi-Fi credentials (not tracked in Git).

## Hardware

- Target: **original M5StickC**.
- The latest code has been compiled and verified by the project maintainer using Arduino IDE on 2026-10-23.
- Full runtime testing of BLE pairing, web controls and keystroke delivery has not yet been separately confirmed.

## Getting started

See [README.md](README.md) for prerequisites, installation, API endpoints and examples. Before building, copy `firmware/KEY_SENDER/secrets.example.h` to `firmware/KEY_SENDER/secrets.h` and enter your own Wi-Fi credentials.

## Security

The current web/API interface provides **no authentication** and uses HTTP. Keep the device on a trusted or isolated local network, never forward the HTTP port to the internet, and only pair with computers you are authorized to control.

## License

MIT — © 2026 Nolaris Solutions AB.
