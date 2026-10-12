# SMLEasy: nachvollziehbares Ziel-Design

Stand: 2026-10-08. Status: Architektur- und Implementierungsgrundlage.
Keine der folgenden neuen Funktionen wird durch dieses Dokument aktiviert.

## 1. Auftrag und Systemgrenzen

Bestehende SML-Firmware zu einem lokal funktionsfaehigen, offline-toleranten
Energy Gateway erweitern. Langfristige Energiedaten nur Ende-zu-Ende
verschluesselt auf der zentralen Plattform speichern. Nur berechtigte Apps
besitzen die Daten-Schluessel. Meter und ESP-Device sind getrennte Identitaeten.

Der verfuegbare Server ist nur ueber IPv6 erreichbar. Daher ist IPv6 parallel
zu IPv4 eine fruehe Voraussetzung, keine spaete optionale Erweiterung.
`https://smarthome.mkreutzer.de/` ist ausschliesslich als HTTPS-Testziel
freigegeben. Dort weder Messdaten noch Schluessel oder Provisionierungsdaten
ablegen. Eine Energy-API oder Nutzungsberechtigung fuer deren Deployment
ist damit nicht festgelegt.

Testgeraete:

- ESP32-C3: verbindliche Referenz fuer die bestehende 4-MB-Firmware.
- ESP32-S3 DevKitC-1 N16R8: vom Besitzer anhand des Produktlinks identifiziertes
  zusaetzliches Testgeraet mit nominell 16 MB Flash und 8 MB PSRAM. Die
  physische Chipkennung, Modulrevision, Flashgroesse, PSRAM-Erkennung und
  Pinbelegung bei der ersten seriellen Identifikation verifizieren. Kein
  S3-Image auf C3 flashen; UART-, USB-, Flash- und PSRAM-Layout pruefen.
- Beide Geraete zunaechst ohne irreversible Sicherheitskonfiguration testen.

## 2. Belegte Ausgangsbasis

| Bereich | Vorhanden | Noch zu entwickeln |
|---|---|---|
| SML/OBIS | Reader, CRC, Profile, aktuelle Werte | 64-Bit-Energiepfad, Qualitaetsregeln |
| Historie | 720 Leistungspunkte im RAM | persistente 5-Minuten-Zaehlerstaende |
| Statistik | Tagesbasis/Vortag teilweise in NVS | belastbare Zeitraeume und Aggregate |
| API | `/api/status`, `/api/history`, Konfiguration | paginierte Sequenz-Synchronisierung |
| Netzwerk | IPv4, lokales HTTP, ausgehendes HTTPS | getesteter Dual-Stack und IPv6-Upload |
| OTA | zwei Slots, manuell/GitHub | Boot-Gesundheit, Recovery, signierte OTA |
| Home Assistant | MQTT und HACS-Polling | zusaetzliche Energiequellen |
| Kryptografie | SDK-Funktionen verfuegbar | Provisionierung, Pairing, E2E-Protokoll |
| App/Backend | nicht als Produkt implementiert | lokale App und Ciphertext-Plattform |

Quellen im Repository: [AppState](../src/app/include/app_state.hpp),
[MeterTask](../src/app/meter_task.cpp), [EnergyStats](../src/app/energy_stats.cpp),
[WebServer](../src/app/web_server.cpp), [Partitionen](../partitions_4mb_ota.csv),
[Release-Build](../.github/workflows/firmware-release.yml).

Aktuelle Energiezaehler und Tagesbasen verwenden `int32_t`. Der Reader begrenzt
Werte auf dessen Grenzen. Fuer langlebige kumulative Werte ist das ungeeignet.
Die Migration muss Reader, AppState, Statistik, Persistenz, JSON, Dashboard,
MQTT und HA gemeinsam betrachten. Fehlender Export ist nicht automatisch Null.
Gesamtregister `1.8.0`/`2.8.0` und Tarifregister `1.8.1`/`1.8.2` bzw.
`2.8.1`/`2.8.2` sind nicht fachlich austauschbar. Beim bestehenden Aliasmapping
die Prioritaet und ggf. bewusste Summierung pruefen, bevor historische
Counterdifferenzen als Gesamtenergie interpretiert werden.

Der vorhandene lokale Build-Artefaktstand vom 2026-10-02 hat 1.259.136 Byte.
Ein OTA-Slot hat 1.966.080 Byte. Das ist ein historischer Anhaltspunkt,
keine aktuelle oder zukuenftige Groessengarantie.

### ESP-IDF-Versionen und Buildpfade (Pruefung 2026-10-08)

- VS-Code-ESP-IDF-Erweiterung ist auf ESP-IDF 6.1.0 eingestellt. Das
  Arbeits-Target wurde fuer den S3-Test auf `esp32s3` gestellt; COM6 ist der
  erkannte Espressif-USB-Port. Der Screenshot zeigte zugleich links PlatformIO-Tasks:
  dieser Aufgabenbaum baut nicht automatisch mit der unten in der Statusleiste
  angezeigten SDK-Version. COM6 war als Espressif USB CDC erkannt worden.
- `build/CMakeCache.txt`, `build/project_description.json` und `build.ninja`
  belegen, dass das separate CMake-Verzeichnis mit
  `C:/esp/v6.1/esp-idf` und `IDF_VER=v6.1` erzeugt wurde. `build/` ist
  ignoriert und sein Artefaktstand ist kein erfolgreicher Firmware-Nachweis.
- `platformio.ini` verwendet weiterhin die Plattform 2024.06.00 und deren
  eigenes ESP-IDF-4.4.8-Paket. Dieser PlatformIO-Pfad bleibt bis zur formalen
  Migration der Releasepfad; ESP-IDF 6.1 ist ein separater Port-/Testpfad.
- Das PlatformIO-Environment `lolin_c3_mini` bleibt ausschliesslich der
  C3-Releasepfad und verwendet ESP-IDF 4.4.8. Der S3-Test erfolgt getrennt
  ueber die lokale ESP-IDF-Erweiterung mit ESP-IDF 6.1.0. Ein erster 6.1-
  PlatformIO-Probelauf war trotz S3-Boardprofil noch ESP-IDF 4.4.8 und wird
  nicht als S3-/6.1-Erfolg gewertet. Nach den IDF-6-API-Aenderungen wurde der
  C3-Releasepfad mit ESP-IDF 4.4.8 erneut erfolgreich gebaut
  (`firmware.bin` 1.261.168 Byte, 63,9 Prozent des 1.966.080-Byte-Slots;
  RAM 29.100/327.680 Byte). Nach den gemeinsamen IPv6-UI-/Filteraenderungen
  wurde der C3-Pfad erneut erfolgreich gebaut (64,0 Prozent Flash, 8,9 Prozent
  RAM); IPv6 bleibt in diesem Releaseprofil deaktiviert. Die IDF-6-Header sind
  versionsbedingt eingebunden.
- Der IDF-6.1-Projektpfad verwendet fuer S3 eine separate App-Komponente und
  zielabhaengige UART-Abhaengigkeit (`esp_driver_uart` auf S3, `driver` auf
  dem bisherigen IDF-4.4-C3-Pfad). Die App-Komponente bindet mDNS, MQTT und
  cJSON als verwaltete IDF-Komponenten ein. Der S3-Component-Resolver lief
  mit `IDF_TARGET=esp32s3` durch; die 16-MB-Flashkonfiguration sowie das
  bestehende Dual-OTA-Layout werden generiert. PSRAM bleibt bis zur
  Hardwareverifikation deaktiviert.
- Der S3-Build erreichte die Uebersetzung der App-Quellen. Fuer IDF 6 wurden
  Firmwareversionsabfragen auf `esp_app_get_description()` umgestellt, und
  JSON-Formatstrings nutzen jetzt die passenden Integer-Makros. MQTT- und
  HTTPS-OTA-Konfiguration verwenden die IDF-6-APIs. Der Build wurde erfolgreich
  gelinkt und als ESP32-S3-BIN erzeugt: 1.210.944 Byte; der 1.966.080-Byte-
  OTA-Slot hat 38 Prozent Reserve. Flash-Code/Data und DIRAM sind gemessen;
  IRAM ist voll belegt (16.384/16.384 Byte) und muss vor Produktivfreigabe
  untersucht werden. Dies belegt nur einen erfolgreichen Build.
- Mit aktivierter S3-IPv6-/SLAAC-Konfiguration sowie IPv6-GUI und Dual-Stack-
  Listener wurde erneut erfolgreich gebaut. Das Image ist 1.249.392 Byte
  (`0x131070`), der 1.966.080-Byte-OTA-Slot hat 36 Prozent Reserve. IRAM bleibt
  voll belegt. Kein Flash- oder Laufzeitnachweis liegt vor.
- Im S3-Testbuild ist GitHub-Release-OTA absichtlich deaktiviert: das Dashboard
  laedt keine GitHub-Releases, und die GitHub-OTA-API antwortet mit HTTP 403.
  Manuelles lokales Firmware-OTA bleibt davon getrennt. C3/Releaseverhalten
  wird durch die Target-Bedingung nicht deaktiviert.
- COM7 identifizierte den ESP32-S3 als Revision v0.2 mit erkanntem 8-MB-PSRAM.
  Das initiale IPv6-Testimage wurde per UART geschrieben und sein Hash
  verifiziert. Beim ersten Boot starteten IPv4 und der IPv4/IPv6-HTTP-Listener;
  der IPv6-Link-Local-Aufruf schlug vor dem Netif-Up mit `ESP_FAIL` fehl.
- Link-Local-Erzeugung wurde auf das IPv4-GOT-IP-Ereignis verschoben. Der
  korrigierte IDF-6.1-S3-Build wurde erfolgreich erstellt und per UART geflasht
  (Image 1.249.488 Byte, 36 Prozent OTA-Slotreserve). Die Bootmeldungen nach
  diesem korrigierten Flash bestaetigen Link-Local, globale SLAAC-Adresse aus
  `2a00:6020:a105:c900::/64` sowie eine FRITZ!Box-ULA. Damit ist IPv6 parallel
  zu IPv4 am S3 zur Laufzeit nachgewiesen; IPv6-Dashboardzugriff und ausgehendes
  HTTPS zum IPv6-only-Server sind noch separat zu testen.
- Das separate `sdkconfig` des IDF-6.1-S3-Builds belegt Target `esp32s3`,
  16-MB-Flash und Custom-Partitionstabelle `partitions_4mb_ota.csv`;
  `CONFIG_SPIRAM` bleibt aus. Der S3-Testbuild aktiviert IPv6 und SLAAC
  target-spezifisch; der bestehende C3-PlatformIO-Pfad bleibt IPv4-only.
  HTTP lauscht bei IPv6 auf allen Interfaces. Der Socket-Filter erlaubt
  private IPv4-, IPv6-ULA- und Link-Local-Clients standardmaessig. Clients
  aus dem eigenen globalen /64-Praefix des ESP (SLAAC, wird bei jeder
  Verbindung gegen die aktuell zugewiesenen Adressen geprueft, also auch bei
  spaeter zugewiesenem Praefix) sind ebenfalls erlaubt; andere globale
  IPv6-Praefixe erfordern eine explizite CIDR-Freigabe. Die S3-Config ist
  lokal generiert und nicht der PlatformIO-C3-Config gleichzusetzen.
- Lokales HTTPS (Phase 1, nur S3/IDF 6): Ein zweiter HTTP-Server lauscht auf
  Port 443 parallel zu HTTP auf Port 80 und teilt sich Routen sowie den
  Netzwerkfilter. Er startet in einem eigenen Task (10-KiB-Stack) nach dem
  HTTP-Dashboard; ein Fehler dort laesst HTTP unberuehrt. Er braucht einen
  eigenen `ctrl_port` (32769), sonst kollidiert er mit dem HTTP-Server.
  Zertifikat: manuell hochgeladenes PEM-Paar (wird auf Zusammengehoerigkeit
  geprueft) oder ein beim ersten Start erzeugtes EC-P-256-Self-Signed-
  Zertifikat (SAN = FQDN, 5 Jahre, mbedTLS 4/PSA in `tls_certs.cpp`).
  Konfiguration ueber `/api/config/tls` und die Karte „HTTPS / TLS“ der
  Konfigurationsseite; Speichern startet nur den HTTPS-Listener neu.
  Let's Encrypt (ACME HTTP-01, Phase 2, S3): `components/acme_client` nutzt
  PSA (Account-/Zertifikatsschluessel, ES256-JWS, CSR). Der Account-Key bleibt
  als PEM kompatibel gespeichert. Fuer die Pruefung wird Port 80 kurz vom
  Dashboard auf einen reinen Challenge-Listener umgeschaltet (ohne
  Netzwerkfilter, nur `/.well-known/acme-challenge/*`), das Dashboard bleibt
  per HTTPS erreichbar. Danach startet HTTP wieder, HTTPS laedt das neue
  Zertifikat ohne Neustart. Erneuerung automatisch nach dem eingestellten
  Intervall (Standard 60 Tage). Staging ist voreingestellt; der Host braucht
  AAAA-Eintrag und eine IPv6-Freigabe von Port 80 zum ESP. Der C3-Build
  (IDF 4.4) schliesst die ACME-Komponente aus und bleibt HTTP-only.
- TLS-Untermenue `/tls` (S3): eigene Seite „HTTPS / Zertifikate“ in der
  Navigation. Bei Let's Encrypt zeigt sie die Nutzungsbedingungen (Link, URL
  aus der ACME-Directory) mit Zustimmung und drei Pruefungen: Internetzugang/
  Uhrzeit/Erreichbarkeit der CA, oeffentliches DNS (DoH `dns.google`, AAAA muss
  zur globalen IPv6 des ESP passen; nur A-Eintrag ist eine Warnung) und
  Erreichbarkeit von aussen ueber Port 80 (Testvalidierung in der
  LE-Staging-Umgebung mit Wegwerf-Konto, ohne Zertifikat). Der Anfordern-
  Button ist erst nach bestandenen Pruefungen frei; das Geraet erzwingt das
  serverseitig (gleicher Hostname, hoechstens 1 Stunde alt). Anforderung und
  Pruefung teilen sich Port 80 und laufen nie gleichzeitig.
- Externe Daten (S3, `/ext`): Werte aus Home Assistant fuer das Dashboard
  (PV-Batterie, PV-Leistung, Wallbox usw.), bis zu 8 frei benennbare Werte.
  Quelle je Wert: REST (`GET {url}/api/states/{entity_id}` mit Long-Lived
  Access Token, Abfrageintervall 10..600 s, Keep-Alive) oder MQTT (Topic am
  Broker der HA-Integration, Zahl oder JSON mit Schluessel `a.b`). Das Token
  liegt nur im NVS und wird nie an die UI zurueckgegeben. Das Dashboard zeigt
  die Karte „Haus & Energie“ (Kacheln, Balken bei Prozentwerten, Alter/Fehler).
  C3 bleibt ohne diese Funktion.
  Hinweis: `esp_https_server` wertet den Rueckgabewert des `open_fn` nicht
  aus; der Netzwerkfilter trennt blockierte Clients per `shutdown()` der
  Socket, daher bleibt der Zugriffsschutz auch auf Port 443 wirksam.

Naechste Freigabeschritte fuer den S3-Test: COM6 mit
esptool als ESP32-S3 identifizieren; reale 16-MB-Flash- und 8-MB-PSRAM-
Erkennung pruefen; danach erst ein ausdruecklich freigegebenes Flashen und
serielle Boot-/SML-/Wi-Fi-Tests. Bis diese Schritte bestanden sind, ist der
S3-Port buildbar, aber kein bestaetigt lauffaehiges Geraet. Ein sichtbares
`ESP-IDF v6.1.0` in der Statusleiste allein beweist nicht, dass eine
PlatformIO-Task dieses SDK verwendet.

## 3. Architektur

```text
SML -> validierter Live-Snapshot im RAM
       -> 5-Minuten-Scheduler -> HistoryStore/Sequenzjournal
       -> vorhandenes Dashboard/MQTT/HA
       -> versionierte lokale API -> App/SQLite
       -> KeyProvider/AEAD -> begrenzte Upload-Batches
                                -> HTTPS ueber IPv6
                                -> API -> PostgreSQL/Ciphertext
                                -> berechtigte App -> Entschluesselung
```

Die Cloud ist nie Voraussetzung fuer Zaehlermessung, Dashboard oder Historie.
Upload und Pairing laufen nicht im WLAN-Event-Handler oder SML-Lesezyklus.
Fuer lokale Historie und Upload begrenzte Puffer statt kompletter JSON-Historie
im RAM verwenden. App und Server erhalten getrennte neue Projektbereiche;
keinen Backend-Prozess in den Firmware-Webserver integrieren.

## 4. Reihenfolge und Abnahme

### P0: Baseline, IPv6 und verbindliche Vertrage

1. C3-Chiprevision/reale Flashgroesse und beim S3 N16R8-Chiprevision,
   Flashgroesse, PSRAM-Erkennung und USB-Port identifizieren.
2. ESP-IDF-6.1-C3-Port als getrennten Buildpfad reproduzierbar machen:
   UART-/GPIO-Component-Abhaengigkeiten und Managed-Components korrigieren,
   dann Compiler/APIfehler schrittweise portieren. PlatformIO-4.4.8-Releasepfad
   nicht parallel veraendern.
3. Targets strikt trennen: C3-Referenz und separater S3-N16R8-IDF-6.1-Test;
   S3 zuerst nur bauen und den Chip am USB-Port identifizieren. Vor jedem
   Flashen Target, Board und Flash-/PSRAM-Konfiguration nochmals pruefen.
4. Seriellen Boot-/Crashnachweis und Ressourcenbaseline erfassen.
5. Effektive IPv6-Config des konkret gewaehlten Buildpfads aus dem generierten
   Header belegen. Keine alte `sdkconfig`-Datei fuer eine andere SDK-Version
   als stillen Erfolgsnachweis verwenden.
6. Dual-Stack in isolierter Testkonfiguration ergaenzen:
   STA-Connected -> Link-Local -> SLAAC -> getrennte Adress-/Cloudzustaende.
7. HTTP-Zugriffsfilter inkl. IPv4-mapped IPv6 korrigieren und regressionspruefen.
8. Ausgehenden HTTPS-GET zum freigegebenen AAAA-Testziel testen. Lokales
   HTTPS/ACME dabei deaktiviert lassen.
9. Datensatz, Sequenz-/Resetregeln und Kryptografie-Vertrag schriftlich
   verabschieden, bevor deren persistente Formate produktiv werden.

Abnahme: [IPv6-Testmatrix](IPV6-PRUEFPLAN.md) auf C3 bestanden;
IPv4-only-Netz und fehlender Server verhindern weder Boot noch lokale Messung.
N16R8 dient als Vergleich, nicht als Ersatz. Ein PC-Test reicht nicht.

### P1: Flash-/OTA-Layout und Recovery

1. Firmware-, Heap-, Stack- und TLS-Spitzenverbrauch auf C3 messen.
2. Ringformat inkl. CRC, Commit-Markierung, Sektormetadaten und Reserve dimensionieren.
3. Zunaechst 14 Tage Retention; 30 Tage nur mit belegtem Speicherbudget.
4. Coredump-Nutzung entscheiden. Im aktuellen Release-Config ist Flash-Coredump
   deaktiviert, obwohl 192 KiB dafuer reserviert sind.
5. Migration bestehender Geraete, Rueckweg und Bootloader-Sicherheitsoptionen
   separat entwerfen und auf dem Testgeraet pruefen.

24 Byte Rohdaten x 288 x 14 = 96.768 Byte. Bei beispielsweise 32 Byte
Speicherrecord sind es 129.024 Byte, jeweils ohne Sektor-/Journalreserve.
Die AEAD-Uploadhuelle wird nicht in diese Rohdatenrechnung hineingeschaetzt.

Die bisherige Tabelle belegt 4 MB vollstaendig. Historie koennte aus dem
Coredump-Bereich entstehen; die konkrete Tabelle ist erst nach Format- und
Sicherheitsbudget freizugeben. Bootloadergroesse kann mit Secure Boot steigen.
Eine normale App-OTA aktualisiert nicht automatisch die Partitionstabelle
oder den Bootloader. Keine neue Partition in alter Tabelle voraussetzen.

Abnahme: Dual-OTA, Stromausfall, altes/neues Layout und Recovery nachgewiesen.
Ziel fuer Release-Images: mindestens 20 Prozent Slotreserve; bei Unterschreitung
Architektur/Features pruefen statt stillschweigend Slots vergroessern.

### P2: persistente Historie und Sequenz

1. Kumulative Energie auf validierte 64-Bit-Werte migrieren, Leistung vorzeichenbehaftet.
2. LiveSnapshot um Messzeit, monotone Messalterbestimmung und Qualitaet ergaenzen.
3. HistoryStore und Sequenzjournal mit versioniertem, explizit serialisiertem
   Format erstellen; keine compilerabhaengige Struct-Ablage.
4. Alle 300 Sekunden hoechstens einen gueltigen Snapshot persistieren.
   Veraltete oder unvollstaendige Werte kennzeichnen/auslassen; keine Messungen
   fuer verpasste Intervalle erfinden.
5. Wiederherstellung nach Stromausfall und Ringueberlauf implementieren.
6. Zaehlertausch/-reset, fehlende Uhrzeit und Sequenzreset explizit behandeln.

UTC und monotone Reihenfolge trennen. Unsichere Zeit nicht als gueltige Epoche
ausgeben. Spaetere Zeitsynchronisierung macht alte unbekannte Zeiten nicht
automatisch exakt. Lokale Kalendertage werden erst in der Auswertung gebildet.

Wichtige offene Vertragsentscheidung: Nach vollstaendiger Loeschung aller
beschreibbaren Speicher kann eFuse allein keinen letzten Sequenzstand liefern.
Entweder neue `stream_id` mit Schluessel `(device_id, stream_id, seq)` verwenden,
oder `(device_id, seq)` beibehalten und Wiederherstellung eines sicheren
High-Water-Marks voraussetzen; ohne diesen Sync sperren. Keine stillschweigende
Ruecksetzung auf 1. Vor Implementierung gemeinsam entscheiden.

Abnahme: 14 Tage Kapazitaet, Reboot, Stromausfall waehrend Write/Erase,
CRC-Fehler, Journalbeschaedigung, Zaehlerruecksprung und Uhrkorrektur getestet.

### P3: versionierte API und lokaler App-Sync

1. Bestehende Endpunkte fuer Dashboard/HA erhalten.
2. `/api/v1/live`, `/api/v1/history?after=...&limit=...` und
   `/api/v1/history/range` mit festem Ressourcenlimit ergaenzen.
3. Device-/ggf. Stream-Identitaet, Schema, `oldest_seq`, `latest_seq`,
   Fortsetzungs-Cursor und explizite Retention-/Reservierungsluecken liefern.
4. Startwert: maximal 128 Records pro Seite/Batch, hoechstens 64 KiB Request.
   Auf C3 messen und gegebenenfalls kleiner dimensionieren.
5. 64-Bit-Werte, die JSONs exakten Integerbereich uebersteigen koennen, als
   Dezimalstrings spezifizieren; App und Server verwenden denselben Vertrag.
6. Lokale App mit SQLite, deduplizierendem Import und atomarem Daten-/Cursor-
   Commit erstellen. Mehrere Devices unter einem Meter modellieren.
7. Gzip nach funktionalem Sync messen und nur bei belegtem Nutzen ergaenzen.

Abnahme: Wiederholung, Abbruch, konkurrierende Syncs und Cursorverlust erzeugen
weder Duplikate noch uebersehene Records. Alte HA-Integration funktioniert.

### P4: eFuse, Device-Keys, Pairing und lokale verschluesselte App

Das Schluesselmodell wird bereits in P0 festgelegt; seine produktive Aktivierung
folgt erst nach Speicher-/Recovery-Abnahme. Vollstaendiger Ablauf:
[Schluessel und Pairing](SCHLUESSEL-UND-PAIRING.md).

1. KeyProvider mit eindeutig als unsicher markiertem Testprovider erstellen.
2. Root-Provisionierung und HMAC_UP zuerst virtuell, dann nach Freigabe auf
   einem identifizierten realen Testgeraet pruefen.
3. Device-ID, getrennte Daten/Auth/Pairing/Storage-Domaenen und Key-Epochen.
4. AES-256-GCM, kanonische AAD, Nonce-/Resetstrategie und Testvektoren.
5. Eigene Pairing-Seite fuer die App-Registrierung erstellen. Sie zeigt nur
   waehrend eines physisch gestarteten, kurzlebigen Pairingfensters einen
   QR-Code mit Bootstrap-Daten und Ablaufzeit; keine Data-, Auth- oder
   Root-Schluessel im QR-Code.
6. App-Key-Wrapping, mehrere Apps, Entzug, Backup und Ersatzgeraet.
7. Secure Boot, Flash Encryption, NVS-Schutz und signierte OTA als
   zusammenhaengendes Produktionsprofil separat abnehmen.

Abnahme: kein Klartext-Key beim Server; falsche Pins/Tags, Replay, Reset,
App-Verlust und Device-Defekt werden korrekt behandelt. Kein Brennen auf
beiden Testgeraeten, bevor ein reversibler Recovery-Vergleich abgeschlossen ist.

### P5: IPv6-only-Backend und Ende-zu-Ende-Datenpfad

1. Separaten API-DNS-Namen und Test-/Produktivumgebung festlegen.
2. HTTPS, gueltige Zertifikate, Account-/Device-Authentifizierung und
   serverseitige Meter-/Device-Zugriffspruefung.
3. PostgreSQL fuer Ciphertext, Key-Wrappings und Zuordnungsmetadaten.
4. Idempotente Batches, atomarer Commit, bestaetigter Sync-Cursor und Konflikte.
   Derselbe Record mit abweichendem Ciphertext ist ein Konflikt, kein Upsert.
5. App-Download, Entschluesselung, Backfill und Client-Aggregate.
6. Monitoring ohne Payloads/Keys, Rate Limits, verschluesselte Backups und Restore.

Server-Cursor darf nicht nur `MAX(seq)` sein. Sequenzreservierungen, echte
Luecken und geloeschte Historie benoetigen einen definierten Protokollzustand.
Bei Retentionluecken nicht endlos auf eine unerreichbare Sequenz warten.
ESP bewahrt seine lokale Retention auch nach ACK auf: asynchroner DB-Failover
kann bereits bestaetigte Daten verlieren. Server-Cursor neu abgleichen.

Abnahme: Offlinephase, Nachlieferung, widerspruechlicher Batch, Serverrestore,
fehlende IPv6-Route und abgelaufenes Zertifikat; lokale Messung laeuft weiter.

### P6: Hochverfuegbarkeit und Multi-Service

Erst Backups/Restore, dann PostgreSQL-WAL-Replikation, Failover mit Fencing und
Split-Brain-Schutz. Eine Generation allein verhindert keine parallelen Writer.
RPO/RTO verbindlich messen; aelterer Verlust als die lokale Retention ist nicht
automatisch reparierbar. Clients kennen nur API-Endpunkte, nie Datenbanken.

Danach DNS/Bootstrap, reproduzierbare Kandidatenreihenfolge, Preferred Endpoint,
Cooldown und exponentieller Backoff mit Jitter. Bootstrap/Endpoint-Konfiguration
authentifizieren; keine Messdaten an unbestaetigte Umleitungsziele senden.
HA-/Multi-Service-Aufwand erst nach Last- und Ausfallnachweisen.

### P7: Energieoekosystem, Auswertungen und Retention

PV, Wallbox und Batterie ueber kumulative Quellen mit Herkunft/Zeitraum erfassen.
Tarife mit Gueltigkeitsintervallen, Zeitzone, Einheiten und Rundung versionieren.
Tag/Woche/Monat/Jahr aus Counter-Differenzen und Qualitaetsinformationen bilden.
Keine exakte Intervallzuordnung behaupten, wenn Grenzwerte fehlen.

PV-Eigenverbrauch = PV minus Export ist nur unter passendem Anlagenmodell
gueltig. Batterie, weitere Erzeuger und Netzladung erfordern eigene Energiefluesse.
Wallbox-PV-Anteil nicht ohne geeignete Messung behaupten.

Zielretention: Tagesaggregate 0-4 Jahre, Wochenaggregate 4-6 Jahre,
danach Loeschung; exakte 5-Minuten-Serverretention noch festzulegen.
Nur Clients/vertrauenswuerdige Endgeraete koennen E2E-Daten verdichten.
Ohne deren Aktivitaet entstehen nicht automatisch Wochenaggregate.
Loeschplanung benoetigt offen definierte Retention-Metadaten oder muss rein
nach Empfangszeit arbeiten; verschluesselte Messzeit ist serverseitig unbekannt.
Tarifkorrekturen nicht durch ausschliesslich vorab berechnete Kosten verhindern.

## 5. Noch erforderliche Entscheidungen

| Entscheidung | Status / Empfehlung | Spaetestens |
|---|---|---|
| S3 N16R8 physische Revision/Flash/PSRAM/Pins | Boardfamilie vom Besitzer identifiziert; Hardwarewerte seriell bestaetigen | P0 |
| SDK | 4.4.8 fuer isolierte Untersuchung; Upgrade separat bewerten | P0 |
| Reset und Sequenzidentitaet | stream_id oder gesperrte High-Water-Recovery | P0/P2 |
| Partitionsmigration/Coredump | nach realem Speicherbudget | P1 |
| App-Plattform/Framework | offen, SQLite und Secure Storage erforderlich | P3 |
| Pairing/Wrapping-Protokoll | Standardbibliothek, Sicherheitspruefung vor Freigabe | P4 |
| Physischer Pairing-Ausloeser | Boardbutton/serieller Vorgang festlegen | P4 |
| eFuse-Slots und Securityprofil | pro Chiprevision/Belegung freigeben | P4 |
| Recovery fuer App-Verlust | verschluesseltes Offlinebackup mit Recovery-Geheimnis | P4 |
| Energy-API-DNS/Stack/PostgreSQL | getrennt vom freigegebenen HTTPS-Testziel | P5 |
| zentrale Feinretention, RPO/RTO | gemeinsam festlegen | P5/P6 |

## 6. Fortschrittsnachweis

Aktuell: P0 Quellcode-/SDK-Untersuchung und PC-IPv6-HTTPS-Test abgeschlossen;
Hardwaretests und alle Implementierungsphasen offen. Keine neue Firmware,
Schluessel oder eFuse-Konfiguration durch diese Dokumentation erzeugt.

Jede Phase erhaelt Nachweise mit Datum, Build/SDK, Board/Chiprevision,
Testfaellen, Messwerten, Ergebnis und offenen Einschraenkungen.
Ein Gate gilt erst mit den benoetigten Hardware-/Recovery-Nachweisen als bestanden.
