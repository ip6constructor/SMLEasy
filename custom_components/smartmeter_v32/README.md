# SMLEasy (Home Assistant Custom Integration)

![SMLEasy](icon.svg)

Diese Integration liest den Status deines Smartmeter-Geraets ueber HTTP (`/api/status`) und stellt Sensoren in Home Assistant bereit.

## Features

- Config Flow (UI-Einrichtung in Home Assistant)
- Polling ueber DataUpdateCoordinator
- Vollstaendige Sensorabdeckung aller Felder aus `/api/status`
- Alle Sensoren mit passenden Icons
- Energy Dashboard bereit: Import/Export Energie mit `device_class: energy` und `state_class: total_increasing`
- Sensoren fuer Energie, Leistung, Spannung, Strom, Frequenz sowie Diagnose-/Systemwerte
- Button-Entitaeten: Ablesung starten, stoppen, Zaehler reset
- Diagnostics-Unterstuetzung fuer Support/Fehleranalyse

## Installation

### Option A: Manuell

1. Kopiere den Ordner `smartmeter_v32` nach:
   - `<config>/custom_components/smartmeter_v32`
2. Starte Home Assistant neu.
3. Gehe zu Einstellungen -> Geraete & Dienste -> Integration hinzufuegen.
4. Suche nach `SMLEasy`.

### Option B: HACS (Custom Repository)

1. In HACS -> Integrationen -> Benutzerdefiniertes Repository.
2. Repository-URL eintragen.
3. Kategorie `Integration` waehlen.
4. `SMLEasy` installieren und Home Assistant neu starten.

## Konfiguration

- Host/IP: IP deines Smartmeters
- Port: Standard 80
- HTTPS: nur aktivieren, wenn dein Geraet TLS anbietet
- Scan-Intervall: mindestens 5 Sekunden

## Hinweise

- Die Integration nutzt nur das lokale HTTP-API des Geraets.
- Keine Cloud-Abhaengigkeit.
- Diagnostics findest du in Home Assistant unter dem Geraet/der Integration.
