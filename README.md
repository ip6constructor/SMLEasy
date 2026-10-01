# SMLEasy

Open-source firmware and Home Assistant integration for the ESP32-C3 WiFi IR Smart Meter Interface. The firmware reads SML telegrams from an optical meter head; the Home Assistant custom integration polls the device locally over HTTP.

[![Install with HACS](https://my.home-assistant.io/badges/hacs_repository.svg)](https://my.home-assistant.io/redirect/hacs_repository/?owner=ip6constructor&repository=SMLEasy&category=integration)

## Features

- SML readout over the optical UART, including the Iskraemeco MT631/MS2020 profile
- Repeating reads at a configurable interval, with single-read and stop controls
- Local dashboard, MQTT Home Assistant discovery, and a separate HACS custom integration
- OTA firmware updates and configurable OBIS mappings

The MT631 currently reports total import/export energy and net active power through this optical interface. Other values, such as phase currents, are shown only when the meter actually includes them in its SML telegram.

## Hardware

- ESP32-C3 Mini
- WiFi IR Smart Meter Interface / compatible optical SML head
- UART: TX GPIO 1, RX GPIO 3; serial format 9600 baud, 8N1

Use a compatible optical head and observe its voltage, wiring, and meter-specific instructions. The firmware does not enable the meter's extended optical dataset or alter its PIN settings; configure those on the meter itself where applicable.

## Firmware

The device dashboard is available at `http://<device-ip>/`. The configuration page is `/config`.

The default meter profile is Iskraemeco MT631/MS2020. A successful read validates the SML frame CRC. Continuous reading starts by default after boot; the interval defaults to 30 seconds. Stop disables it until the next reboot.

### Build

With PlatformIO and the ESP-IDF toolchain installed:

```powershell
platformio run --environment esp32c3_optical_meter
```

The OTA image is produced at `.pio/build/esp32c3_optical_meter/firmware.bin`. Upload it from the configuration page. OTA uploads require the web-interface credentials.

## Home Assistant

SMLEasy offers two separate HA paths. Choose one to avoid duplicate entities:

### HACS custom integration

The integration under `custom_components/smartmeter_v32` polls `/api/status` locally and provides sensors and control buttons. Install using the HACS button above, restart Home Assistant, then add **SMLEasy** under **Settings > Devices & services > Add integration**. Enter the device IP and port (normally 80).

Manual installation: copy `custom_components/smartmeter_v32` into `<config>/custom_components/`, restart Home Assistant, and add the integration.

### MQTT discovery from firmware

Alternatively, enable **Home Assistant Integration** in the device configuration and set the broker URI and credentials. Home Assistant's MQTT integration and a reachable broker are required. The firmware publishes retained MQTT discovery messages.

## Security

First login: username `admin`, password `P@assword26`. Change the password immediately after flashing the firmware. OTA uploads require the same web-interface login. The device currently serves HTTP without TLS; use it only on a trusted local network and do not forward its web port to the Internet.

## Project layout

- `src/app/`: ESP-IDF firmware and embedded dashboard
- `components/sml/`: SML reader and parser
- `custom_components/smartmeter_v32/`: Home Assistant HACS integration
- `partitions_4mb_ota.csv`: dual-slot OTA partition layout

## License

This project is licensed under the GNU General Public License v3.0. See [LICENSE](LICENSE).
