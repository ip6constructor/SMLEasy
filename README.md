# SMLEasy

Open-source firmware and Home Assistant integration for the ESP32-C3 WiFi IR Smart Meter Interface. The firmware reads SML telegrams from an optical meter head and provides a local dashboard, HTTPS/ACME support, GitHub OTA, and Home Assistant connectivity.

[![Install with HACS](https://my.home-assistant.io/badges/hacs_repository.svg)](https://my.home-assistant.io/redirect/hacs_repository/?owner=ip6constructor&repository=SMLEasy&category=integration)

## Features

- SML readout over the optical UART, including the Iskraemeco MT631/MS2020 profile
- Repeating reads at a configurable interval, with single-read and stop controls
- Local multilingual dashboard (EN default, DE, NL, FR, PL) and configurable CIDR access allowlist
- MQTT Home Assistant discovery and a separate HACS custom integration
- GitHub Release OTA and manual firmware upload
- Optional HTTPS with imported PEM certificates, Let's Encrypt HTTP-01, and an explicitly enabled self-signed fallback
- Persistent previous-day import/export energy totals

The MT631 currently reports total import/export energy and net active power through this optical interface. Other values, such as phase currents, are shown only when the meter actually includes them in its SML telegram.

<img width="1586" height="1271" alt="image" src="https://github.com/user-attachments/assets/250e6126-2f37-481e-8f2c-2bc8f16167d9" />


## Hardware

- ESP32-C3 Mini
- WiFi IR Smart Meter Interface / compatible optical SML head
- UART: TX GPIO 1, RX GPIO 3; serial format 9600 baud, 8N1

Use a compatible optical head and observe its voltage, wiring, and meter-specific instructions. The firmware does not enable the meter's extended optical dataset or alter its PIN settings; configure those on the meter itself where applicable.

## Firmware

The current recovery firmware is temporarily IPv4-only and serves the dashboard and API over HTTP at `http://<device-ip>/` on port 80. HTTPS, IPv6, and ACME certificate issuance are disabled while the firmware is stabilized.

The default meter profile is Iskraemeco MT631/MS2020. A successful read validates the SML frame CRC. Continuous reading starts by default after boot; the interval defaults to 30 seconds. Stop disables it until the next reboot.

### Build

With PlatformIO and the ESP-IDF toolchain installed:

```powershell
platformio run --environment esp32c3_optical_meter
```

The OTA image is produced at `.pio/build/esp32c3_optical_meter/firmware.bin`. Upload it from the configuration page or choose a published GitHub Release. Release assets are named `SMLEasy-<version>.bin` and require a matching `v<version>` tag. OTA uploads require the web-interface credentials.

### GitHub Releases

The GitHub Actions workflow builds the ESP32-C3 PlatformIO environment when a `v*` tag is pushed. The tag must match `build_version.txt`; the workflow publishes `SMLEasy-<version>.bin` to the release. The dashboard OTA picker only accepts assets from this repository with that exact naming scheme.

The current published firmware is **v2.5.6**, with the OTA asset [`SMLEasy-2.5.6.bin`](https://github.com/ip6constructor/SMLEasy/releases/download/v2.5.6/SMLEasy-2.5.6.bin). In the device configuration, open **OTA-Update**, load GitHub versions, select **v2.5.6**, and install it. Older GitHub release entries have been removed; the corresponding source tags remain in the repository.

### Meter Profiles

The editable catalog is [`profiles.json`](profiles.json) and is loaded from GitHub at `https://raw.githubusercontent.com/ip6constructor/SMLEasy/main/profiles.json`. Embedded profiles remain available offline; matching catalog IDs can update them, and new IDs are added to the selector. The catalog stays in the browser cache; the device stores the active meter settings and one previous profile snapshot in NVS, not the full catalog. To add a profile, append an entry with a unique `id`, `name`, `manufacturer`, `model`, `login_cmd`, `login_wait_ms`, and the OBIS fields used by the meter.

### Temporary Network Mode

- The dashboard and API use IPv4 HTTP on port 80. Do not expose the device directly to the Internet; HTTP traffic is not encrypted.
- IPv6, the HTTPS server, and ACME/Let's Encrypt issuance are temporarily disabled. The standalone ACME component is not linked into the firmware.
- GitHub OTA downloads still use outbound HTTPS.

### Network Access and Language

The dashboard defaults to allowing RFC1918 IPv4 clients. Additional IPv4 CIDR networks can be added in the configuration page. The browser language is selected automatically with English as fallback; an explicit device setting can choose EN, DE, NL, FR, or PL.

## Home Assistant

SMLEasy offers two separate HA paths. Choose one to avoid duplicate entities:

### HACS custom integration

The integration under `custom_components/smleasy` polls `/api/status` locally and provides sensors and control buttons. Install using the HACS button above, restart Home Assistant, then add **SMLEasy** under **Settings > Devices & services > Add integration**. Use the device's IPv4 address, port 80, and HTTP.

Manual installation: copy `custom_components/smleasy` into `<config>/custom_components/`, restart Home Assistant, and add the integration. After upgrading from the old `smartmeter_v32` domain, remove the old integration entry and add SMLEasy again.

### MQTT discovery from firmware

Alternatively, enable **Home Assistant Integration** in the device configuration and set the broker URI and credentials. Home Assistant's MQTT integration and a reachable broker are required. The firmware publishes retained MQTT discovery messages.

## Security

First login: username `admin`, password `P@assword26`. Change the password immediately after flashing the firmware. OTA requires the same web-interface login. The dashboard uses unencrypted HTTP; keep it on a trusted local network and do not expose it directly to the Internet.

## Project layout

- `src/app/`: ESP-IDF firmware and embedded dashboard
- `components/sml/`: SML reader and parser
- `custom_components/smleasy/`: Home Assistant HACS integration
- `partitions_4mb_ota.csv`: dual-slot OTA partition layout

## License

This project is licensed under the GNU Affero General Public License v3.0 or later. See [LICENSE](LICENSE). The dashboard links to the corresponding source for the running release and to the license text.
