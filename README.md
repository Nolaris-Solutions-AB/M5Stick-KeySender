# M5Stick-KeySender

Bluetooth LE keyboard automation for M5StickC, controlled using Wi-Fi, a built-in web interface and an HTTP API.

**Status:** Original firmware source is published. The published sanitization and the exact toolchain/dependency versions have not yet been hardware-tested. This is not yet a tagged release.

## Features

- BLE HID keyboard emulation using ESP32-BLE-Keyboard
- Built-in Swedish-language responsive web interface
- Manual start, stop, toggle and send-once actions
- Recurring key combinations with configurable minutes and seconds (default: every 4 minutes)
- Default key sequence: `SCROLLLOCKX2`
- Button A toggles sending; starting sends the first sequence immediately
- BLE, Wi-Fi and sending status, with a count of sent sequences
- HTTP endpoints for automation / Home Assistant

## Hardware

Designed for **M5StickC (original)**. Other M5Stick models may require changes to the library and display/button code.

## Build and installation

1. Install Arduino IDE and a compatible ESP32 boards package.
2. Install the **M5StickC** and **ESP32 BLE Keyboard** libraries (the sketch includes `M5StickC.h` and `BleKeyboard.h`).
3. Open [firmware/KEY_SENDER/KEY_SENDER.ino](firmware/KEY_SENDER/KEY_SENDER.ino) in Arduino IDE.
4. In the same directory, copy `secrets.example.h` to `secrets.h`.
5. Edit `secrets.h` with your Wi-Fi SSID and password. **Do not commit this file.**
6. Select the M5StickC target and upload the sketch.
7. Pair the device named `M5StickC KEY SENDER` as a Bluetooth keyboard with the computer.
8. Find the device IP address on its display or serial monitor (115200 baud) and visit `http://DEVICE_IP/`.

The firmware waits for Wi-Fi before starting BLE and the web server; if Wi-Fi credentials are wrong or the access point cannot be reached, setup will wait indefinitely.

**Compatibility note:** Exact ESP32 core/library versions have not yet been established. If the current versions fail to compile, please open an issue with your environment information.

## HTTP API

| Method | Endpoint | Description |
|---|---|---|
| GET | `/` | Web interface |
| GET | `/api/status` | JSON status |
| GET | `/api/running` | Plain-text `1` (active) or `0` (stopped) |
| POST | `/api/start` | Send immediately and start periodic sending |
| POST | `/api/stop` | Stop periodic sending |
| POST | `/api/toggle` | Toggle active state |
| POST | `/api/send` | Send current combination once |
| POST | `/api/settings` | Set `combo`, `minutes` and `seconds` form parameters |

The command endpoints return JSON status when successful. Start/toggle/send may return HTTP 503 when BLE is not connected. Settings rejects malformed parameters with HTTP 400.

Example:

```bash
curl http://DEVICE_IP/api/status
curl -X POST http://DEVICE_IP/api/toggle
curl -X POST http://DEVICE_IP/api/settings \
  -d 'combo=SCROLLLOCKX2' -d 'minutes=4' -d 'seconds=0'
```

Example status fields: `sending`, `bluetooth`, `wifi`, `minutes`, `seconds`, `combo`, and `count`.

## Keyboard combinations

Examples: `SCROLLLOCKX2`, `SCROLLLOCK`, `CTRL+ALT+3`, `WIN+L`, `CTRL+SHIFT+ESC`, `ALT+F4`.

## Automation / Home Assistant

You can use REST commands in your own automation system:

```yaml
rest_command:
  keysender_toggle:
    url: "http://DEVICE_IP/api/toggle"
    method: POST
```

For status, GET `/api/running` returns `1` or `0`; `/api/status` provides more fields. Replace `DEVICE_IP` with the local IP address.

## Security and limitations

**Important:** The current HTTP server has **no authentication and uses unencrypted HTTP**. Anybody who can reach the device on port 80 can change the configured keystrokes, send keystrokes and start/stop automation. Restrict the device to a trusted, isolated network and never expose it directly to the internet. Bluetooth pairing also gives the device the ability to enter keys on the paired host; only pair with authorized systems.

The settings are kept in RAM and are reset to defaults on reboot. The firmware does not currently handle Wi-Fi reconnection automatically.

## License

MIT License. Copyright (c) 2026 Nolaris Solutions AB. See [LICENSE](LICENSE).

Contributions welcome: [CONTRIBUTING.md](CONTRIBUTING.md).
