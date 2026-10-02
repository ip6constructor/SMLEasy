# Home Assistant Integration (MQTT Discovery)

Dieses Dokument beschreibt den MQTT-Discovery-Pfad der Firmware. Die separate HACS-Integration ist in `custom_components/smleasy/README.md` dokumentiert.

Diese Firmware integriert Home Assistant als einen einzigen Integrationsmodus.
Ein separater "generischer MQTT-Modus" ist nicht vorgesehen.
MQTT wird nur als technischer Transport fuer die HA-Discovery genutzt.

## Voraussetzungen

- In Home Assistant ist die MQTT-Integration eingerichtet.
- Ein laufender MQTT-Broker ist erreichbar (z. B. Mosquitto).
- ESP32 und Home Assistant sind im selben Netzwerk.

## HTTPS und HACS

Die HACS-Integration fragt die lokale REST-API ab. Im HTTP-Recovery-Modus verwendet sie Port 80. Sobald ein gültiges TLS-Zertifikat aktiviert ist, in Home Assistant Port 443 und `HTTPS` auswählen.

Das selbstsignierte Fallback verschlüsselt die Verbindung, wird aber von Home Assistant standardmäßig nicht als vertrauenswürdig akzeptiert. Für HACS sollte ein öffentlich vertrauenswürdiges Let's-Encrypt-Zertifikat verwendet werden.

## Geraet konfigurieren

1. Oeffne `http://<geraete-ip>/config`.
2. Unter `Home Assistant Integration`:
- `Aktiviert`: einschalten
- `MQTT Broker URI`: z. B. `mqtt://192.168.1.10:1883`
- `Benutzername` / `Passwort`: falls noetig
- `Geraetename`: frei waehlbar (wird in HA angezeigt)
- `HA Discovery-Praefix`: standard `homeassistant`
3. Speichern.

## Wie die Integration arbeitet

- State Topic:
  - `ha_meter/<device_id>/state`
- Availability Topic:
  - `ha_meter/<device_id>/availability`
- Discovery Topics:
  - `homeassistant/sensor/<device_id>/<sensor_id>/config`

Die Discovery-Nachrichten werden retained veroeffentlicht und beim Reconnect erneut gesendet.

## Erwartete Sensoren in Home Assistant

- Vorwaerts/Rueckwaerts Wirkenergie
- Import/Export Blindenergie
- Wirkleistung (+/-)
- Spannung L1/L2/L3
- Strom L1/L2/L3
- Frequenz
- Leistungsfaktor L1
- Systemstatus (Komponente im HA-Geraet)
- System-Uptime (Komponente im HA-Geraet)

## Fehlersuche

- MQTT Status bleibt "Getrennt": Broker URI und Zugangsdaten pruefen.
- Keine Sensoren in HA: MQTT-Integration in HA pruefen, Discovery muss aktiv sein.
- Keine Werte: zuerst eine Ablesung am Dashboard starten.
