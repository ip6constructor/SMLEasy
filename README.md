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

## Hardware

- ESP32-C3 Mini
- WiFi IR Smart Meter Interface / compatible optical SML head
- UART: TX GPIO 1, RX GPIO 3; serial format 9600 baud, 8N1

Use a compatible optical head and observe its voltage, wiring, and meter-specific instructions. The firmware does not enable the meter's extended optical dataset or alter its PIN settings; configure those on the meter itself where applicable.

## Firmware

When no valid certificate is enabled, the dashboard is available in HTTP recovery mode at `http://<device-ip>/`. After importing a certificate or enabling the self-signed fallback, use `https://<device-ip>/`; the dashboard and API listen on port 443.

The default meter profile is Iskraemeco MT631/MS2020. A successful read validates the SML frame CRC. Continuous reading starts by default after boot; the interval defaults to 30 seconds. Stop disables it until the next reboot.

### Build

With PlatformIO and the ESP-IDF toolchain installed:

```powershell
platformio run --environment esp32c3_optical_meter
```

The OTA image is produced at `.pio/build/esp32c3_optical_meter/firmware.bin`. Upload it from the configuration page or choose a published GitHub Release. Release assets are named `SMLEasy-<version>.bin` and require a matching `v<version>` tag. OTA uploads require the web-interface credentials.

### GitHub Releases

The GitHub Actions workflow builds the ESP32-C3 PlatformIO environment when a `v*` tag is pushed. The tag must match `build_version.txt`; the workflow publishes `SMLEasy-<version>.bin` to the release. The dashboard OTA picker only accepts assets from this repository with that exact naming scheme.

### HTTPS and Let's Encrypt

- Until a valid certificate is active, HTTP on port 80 remains available for recovery. Dashboard/API HTTPS uses port 443 after a valid certificate is installed.
- The TLS form accepts a certificate chain (`fullchain.pem`) and matching private key in PEM format. The private key is stored in NVS; protect physical access to the device and enable flash encryption for deployments that require at-rest protection.
- A self-signed certificate is generated only when the administrator explicitly enables that fallback. Browsers and Home Assistant will warn/reject it unless they trust the issuer; it is intended for encryption/testing, not public trust.
- Let's Encrypt uses the HTTP-01 challenge. The FQDN must resolve publicly through A and/or AAAA to the ESP's current public address. The router must forward IPv4 TCP port 80 directly to ESP port 80 and allow inbound IPv6 TCP port 80 to the ESP's global IPv6 address. Port 80 serves only the challenge while ACME is active; the dashboard remains on 443.
- Select the Let's Encrypt staging environment for initial testing. Staging certificates are not trusted by normal browsers. Switch to production after confirming DNS and router reachability.
- Let's Encrypt certificates are normally valid for 90 days; the default renewal interval is 60 days with randomized hourly checking. Renewal needs the FQDN and HTTP-01 route to remain reachable.

### Network Access and Language

The dashboard defaults to allowing RFC1918 IPv4, IPv6 ULA, and link-local clients. Additional IPv4/IPv6 CIDR networks can be added in the configuration page. The public ACME challenge listener is separate and is not restricted by this dashboard allowlist. The browser language is selected automatically with English as fallback; an explicit device setting can choose EN, DE, NL, FR, or PL.

## Home Assistant

SMLEasy offers two separate HA paths. Choose one to avoid duplicate entities:

### HACS custom integration

The integration under `custom_components/smleasy` polls `/api/status` locally and provides sensors and control buttons. Install using the HACS button above, restart Home Assistant, then add **SMLEasy** under **Settings > Devices & services > Add integration**. Use port 80 with HTTP recovery, or port 443 and enable **HTTPS** when a browser-trusted certificate is active. The self-signed fallback is not trusted by Home Assistant's default TLS verification.

Manual installation: copy `custom_components/smleasy` into `<config>/custom_components/`, restart Home Assistant, and add the integration. After upgrading from the old `smartmeter_v32` domain, remove the old integration entry and add SMLEasy again.

### MQTT discovery from firmware

Alternatively, enable **Home Assistant Integration** in the device configuration and set the broker URI and credentials. Home Assistant's MQTT integration and a reachable broker are required. The firmware publishes retained MQTT discovery messages.

## Security

First login: username `admin`, password `P@assword26`. Change the password immediately after flashing the firmware. OTA and TLS settings require the same web-interface login. Do not expose the dashboard/API directly to the Internet; only the isolated HTTP-01 challenge path is intended to be publicly reachable during ACME issuance and renewal.

## Project layout

- `src/app/`: ESP-IDF firmware and embedded dashboard
- `components/sml/`: SML reader and parser
- `custom_components/smleasy/`: Home Assistant HACS integration
- `partitions_4mb_ota.csv`: dual-slot OTA partition layout

## License

This project is licensed under the GNU General Public License v3.0. See [LICENSE](LICENSE).
