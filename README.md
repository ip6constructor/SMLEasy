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


<img width="1625" height="547" alt="image" src="https://github.com/user-attachments/assets/6a848a1f-c715-4604-bfb2-7afa7fa27f3a" />


<img width="1631" height="1218" alt="image" src="https://github.com/user-attachments/assets/3739f59f-1785-4d61-b76f-5b932d90eb8a" />

## Hardware

- ESP32-C3 Mini
- WiFi IR Smart Meter Interface / compatible optical SML head
- UART: TX GPIO 1, RX GPIO 3; serial format 9600 baud, 8N1

Use a compatible optical head and observe its voltage, wiring, and meter-specific instructions. The firmware does not enable the meter's extended optical dataset or alter its PIN settings; configure those on the meter itself where applicable.

z.b.: https://www.christians-shop.de/WiFi_IR_SMI_V32_DE_1 (Need Flash Hardware and a connector to overwrite the Default Safe-Boot Image from 0000000)

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

The current published firmware is **v3.6.0**, with the OTA asset [`SMLEasy-3.6.0.bin`](https://github.com/ip6constructor/SMLEasy/releases/download/v3.6.0/SMLEasy-3.6.0.bin). In the device configuration, open **OTA-Update**, load GitHub versions, select **v3.6.0**, and install it. Older GitHub release entries have been removed; the corresponding source tags remain in the repository.

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

## Android app (SMLEasy Board)

Two variants are maintained: `SMLEasy-Board-<version>-legacy-debug.apk` (minSdk 22, Android 5.1+, e.g. Fire HD 10 7th gen) and `SMLEasy-Board-<version>-standard-debug.apk` (minSdk 24, Android 7+). The app shows the energy board full screen. It is **not available in the Google Play Store**; install it manually (sideload, allow "unknown sources"; Play Protect may warn because the APK is debug-signed). A Play Store release would need a release signature, a developer account and a privacy declaration, and is not planned yet. The app is untested on real devices.

Pairing (IDF-6 development firmware): open `/pair` locally and start the five-minute pairing mode. In the app choose **App koppeln > Geraet suchen** (or enter the IP). Discovery uses `_smleasy-pair._tcp`; pairing uses TLS/TCP port **7384**, not HTTP, QR codes, public DNS or a public CA. Compare the six-digit number in the app and web page before approving that request in the web page. Only then does the app receive its own token and the SHA-256 certificate fingerprint. `/pair` also lists the four app slots, supports renaming and revoking individual apps, and shows the creation time when available. No secrets are displayed.

Regular app access remains HTTPS on port 443, including Internet access via IPv6/FQDN; only pairing is temporary/local. The app tries the hostname, then IPv6, then IPv4, preserves IPv6 interface scopes, and never sends a paired token over HTTP or follows API redirects. Direct-IP HTTPS verifies the approved certificate fingerprint, including self-signed/staging certificates. A changed certificate (including ACME renewal) currently requires local re-pairing; automatic trusted pin rotation is not implemented. Do not expose port 7384 to the Internet. Existing tokens remain valid; the old QR claim endpoint explicitly returns HTTP 410. Manual password-only access remains available.

The energy board's bottom bar shows self-consumption, today's PV generation, net grid costs and savings, in both web and Android views. It uses the status snapshot independently of external power tiles; unknown values are shown as a dash rather than zero.

## Security

First login: username `admin`, password `P@assword26`. Change the password immediately after flashing the firmware. OTA requires the same web-interface login. The dashboard uses unencrypted HTTP; keep it on a trusted local network and do not expose it directly to the Internet.

## Project layout

- `src/app/`: ESP-IDF firmware and embedded dashboard
- `components/sml/`: SML reader and parser
- `custom_components/smleasy/`: Home Assistant HACS integration
- `partitions_4mb_ota.csv`: dual-slot OTA partition layout

## Development target design

The next development phases, acceptance gates, and AI/KI working instructions
are documented in [AGENTS.md](AGENTS.md) and the
[target design](docs/ZIEL-DESIGN.md). The supporting documents cover
[IPv4/IPv6 investigation and hardware tests](docs/IPV6-PRUEFPLAN.md) and
[eFuse provisioning, device keys, and pairing](docs/SCHLUESSEL-UND-PAIRING.md).
These are planned capabilities, not features enabled in the current firmware.

## License

This project is licensed under the GNU Affero General Public License v3.0 or later. See [LICENSE](LICENSE). The dashboard links to the corresponding source for the running release and to the license text.
