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

1. Kopiere den Ordner `smleasy` nach:
   - `<config>/custom_components/smleasy`
2. Starte Home Assistant neu.
3. Gehe zu Einstellungen -> Geraete & Dienste -> Integration hinzufuegen.
4. Suche nach `SMLEasy`.

### Option B: HACS (Custom Repository)

1. In HACS -> Integrationen -> Benutzerdefiniertes Repository.
2. Repository-URL eintragen.
3. Kategorie `Integration` waehlen.
4. `SMLEasy` installieren und Home Assistant neu starten.

## Konfiguration

- Host/IP: LAN-IP oder DNS-Name deines Smartmeters
- Port: 80 im HTTP-Recovery-Modus, 443 mit aktivem TLS-Zertifikat
- HTTPS: aktivieren, wenn ein TLS-Zertifikat installiert ist
- Scan-Intervall: mindestens 5 Sekunden

## Hinweise

- Die Integration nutzt nur die lokale REST-API des Geraets.
- Fuer HTTPS muss Home Assistant dem Zertifikat vertrauen. Das selbstsignierte Fallback wird standardmaessig abgelehnt; nutze fuer HACS ein oeffentlich vertrautes Let's-Encrypt-Zertifikat.
- Beim Let's-Encrypt HTTP-01-Verfahren wird Port 80 ausschliesslich fuer die Challenge verwendet, waehrend Dashboard und API ueber HTTPS-Port 443 laufen.
- Keine Cloud-Abhaengigkeit.
- Diagnostics findest du in Home Assistant unter dem Geraet/der Integration.
