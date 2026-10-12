# SMLEasy: Arbeitsgrundlage fuer AI/KI

## Verbindliche Einstiegslektuere

Vor Architektur-, Firmware-, App- oder Backend-Aenderungen lesen:

1. [Ziel-Design und Reihenfolge](docs/ZIEL-DESIGN.md)
2. [IPv6-Untersuchung und Testplan](docs/IPV6-PRUEFPLAN.md)
3. [eFuse, Device-Schluessel und Pairing](docs/SCHLUESSEL-UND-PAIRING.md)

Diese Dokumente beschreiben ein Ziel, nicht bereits implementierte Features.
Den Implementierungsstand und die Nachweise bei jeder abgeschlossenen Phase
aktualisieren. Offene Entscheidungen nicht als bereits getroffen darstellen.

## Arbeitsregeln

- Die Reihenfolge P0 bis P7 im Ziel-Design beachten. Eine spaetere Phase nicht
  produktiv aktivieren, bevor ihre Voraussetzungen nachgewiesen sind.
- C3 mit 4 MB bleibt die verbindliche Mindestplattform. Das N16R8-Testgeraet
  ersetzt keine C3-Abnahme; Chip, Board und Pinbelegung vorher bestaetigen.
- IPv4, lokales Dashboard, SML, Home Assistant und Dual-OTA erhalten.
- IPv6-Unterstuetzung, lokales HTTPS, ACME, Partitionsmigration und eFuse-Brennen
  in getrennten Aenderungen behandeln. Kein gleichzeitiger SDK-Wechsel.
- SDK-Version und tatsaechlich generierte Konfiguration belegen. Der aktuelle
  PlatformIO-Releasepfad verwendet ESP-IDF 4.4.8; die lokale VS-Code-ESP-IDF-
  Erweiterung verweist auf 6.1.0. PlatformIO-Tasks verwenden nicht automatisch
  das SDK der VS-Code-Statusleiste. Diese Buildpfade nicht vermischen.
- ESP-IDF 6.1.0 bleibt ein isolierter Port-/Testpfad, bis Build, C3-Hardware,
  S3-N16R8-Target, IPv6, OTA und CI separat abgenommen sind. Kein S3-Board mit
  dem C3-Target/Partitionlayout flashen.
- Fehlendes IPv6, DNS-, TLS- oder Serverfehler duerfen den lokalen Betrieb
  nicht blockieren oder einen Neustart ausloesen. Fehler sichtbar machen.
- Alte API-Vertraege nicht ersetzen. Neue Historie unter `/api/v1/` ergaenzen.
- Keine Secrets, privaten Schluessel, Pairing-Tokens oder Klartext-Energiedaten
  in Git, Logs, Diagnosepakete, QR-Screenshots oder zentrale Dienste schreiben.
- Kryptografie ausschliesslich ueber etablierte Bibliotheken/Protokolle.
  Keine improvisierte ECDH-/Token-/AEAD-Kombination als Produktprotokoll.
- eFuse-, Secure-Boot-, Flash-Encryption- und Debug-Lock-Aenderungen sind
  irreversibel. Keine automatische Ausfuehrung beim normalen Boot und keine
  Ausfuehrung ohne geraetespezifische ausdrueckliche Freigabe.
- Reines Kompilieren und ein PC-HTTPS-Test sind keine Hardware-Abnahme.
  Nicht ausgefuehrte Tests mit Begruendung offen lassen.
- Ressourcenlimits und Testkriterien aus den Dokumenten messen, nicht schaetzen.
- Vor Builds das automatische Versionsinkrement in `version_increment.py`
  beruecksichtigen; fuer reine Validierung den vorhandenen CI-Modus verwenden.
- Neue Abhaengigkeiten, Boardprofile oder SDK-Upgrades nur gezielt und mit
  begruendetem Bedarf einfuehren.

## Abschluss jeder Aenderung

Dokumentieren: betroffene Phase, API-/Speichermigration, ausgefuehrte Tests,
Geraet/Chiprevision, SDK/Config/Build-Version, Flash-/Heap-/Stack-Messwerte,
verbleibende Risiken und Rueckweg. Keine Produktionstauglichkeit behaupten,
solange Hardware-, Recovery- oder Schluesseltests fehlen.
