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
- C3 auf ESP-IDF 6.1 (Build-Nachweis 2026-10-19, isoliert, kein Releasepfad):
  `idf.py -B build_c3_idf6 -DIDF_TARGET=esp32c3 -DSMLEASY_IDF6_APP=1
  -DSDKCONFIG=build_c3_idf6/sdkconfig
  "-DSDKCONFIG_DEFAULTS=sdkconfig.defaults;sdkconfig.defaults.c3_idf6" build`
  (IDF v6.1, `PYTHONUTF8=1`) wurde erfolgreich gebaut: `smartmeter.bin`
  1.753.056 Byte (0x1ac3e0), 11 Prozent des kleinsten App-Slots frei; die
  Config enthaelt `CONFIG_LWIP_IPV6=y` und HTTPS-Server. Offen: Flash/Boot auf
  echter C3-Hardware, IPv6-Laufzeit, Heap/Stack, OTA-Wechsel 4.4 -> 6.1 und
  CI. Der PlatformIO-/IDF-4.4.8-Releasepfad bleibt unveraendert.
- Tageswechsel/Energie-Board: Das Schreiben der Historie (64-KiB-Erase) erfolgt
  nicht mehr unter dem Statistik-Mutex, damit `/api/status` nicht blockiert.
  Das Board pollt Status und HA-Werte nacheinander mit Timeout, ohne
  parallele Dashboard-Abfragen, und plant den naechsten Abruf 1,5 s nach dem
  erwarteten Zaehler-/HA-Lesen (neue Felder `meter_age_ms`, `read_interval_s`,
  `poll_s`). Nicht auf Hardware ueber einen echten Tageswechsel getestet.
- Android-Board-App (`android/`, Kotlin nativ, Entwicklungsstand;
  Build-/Testnachweise unten, keine Android-Hardwareabnahme): zeigt das
  Energie-Board per REST (`/api/status`, `/api/ext/values`), cached `/house.jpg`
  und speichert Kachelposition/-groesse lokal.
  Kopplung (aktualisiert 2026-10-19): `/pair` (IDF 6, Basic-Auth, erlaubte
  Netze) startet maximal 300 s TLS/TCP auf Port 7384 mit mDNS-Suche.
  Kein QR/Code-Link, keine Abhaengigkeit von FQDN/oeffentlicher CA.
  App stellt Anfrage; Besitzer vergleicht die sechsstellige Nummer und
  bestaetigt im Webfrontend. Erst dann erhaelt diese Verbindung Token und
  Zertifikatsfingerprint. Geraeteverwaltung mit vier App-Slots, Umbenennen,
  optionalem Erstellzeitpunkt und Widerruf. Alt-Claim antwortet HTTP 410.
  Danach HTTPS ueber FQDN/IPv6/IPv4 mit Pinning, nie HTTP mit Token.
  Wechsel des Blattzertifikats erfordert derzeit lokales Re-Pairing.
  ABWEICHUNG von der urspruenglichen P4-Planung (Nutzerentscheidungen):
  Web-Button statt physischem Taster, lokale Suche und Web-Freigabe statt QR.
  HMAC-Tokenpruefung und Blattzertifikats-Pinning sind im Entwicklungsstand
  umgesetzt; HPKE, eFuse-HMAC_UP-Root und produktives Key-/Recovery-Protokoll
  bleiben offen. Keine produktive Aktivierung der P4-/P5-Gates.
  Bildschirmschoner: `BoardDream` (Daydream) nutzt dieselbe Abfrage. `ScreenCare`
  verschiebt den Inhalt einmal pro Kalendertag um max. 8 dp (9er-Muster) gegen
  Einbrennen und passt die Fensterhelligkeit per Lichtsensor an (5 % bis 100 %,
  log. Kennlinie). Ohne Sensor bleibt die Systemhelligkeit.
  iOS-App (spaeter): QR-Format und HTTPS+Basic-Auth sind plattformneutral; das
  spaetere Schluesselprotokoll wird einmal fuer beide Plattformen spezifiziert.
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
| SDK | entschieden: 4.4.8/PlatformIO-Releasepfad am 2026-10-29 vollstaendig entfernt, ESP-IDF 6.1.0 einziger Pfad (Nutzerentscheidung trotz zu diesem Zeitpunkt nur teilweise frisch nachgewiesener Gates, siehe Fortschrittsnachweis 2026-10-29) | P0 |
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

Die P0-Quellcode-/SDK-Untersuchung und der PC-IPv6-HTTPS-Test sind belegt.
Die folgenden Entwicklungs-Builds sind keine Freigabe der noch offenen
Hardware-/Recovery-Gates; kein eFuse-Brennen oder SDK-Wechsel durch diese Aenderung.

### 2026-10-29: IDF-4.4.8/PlatformIO-Pfad entfernt, LAN-Steuerrouten, Dashboard-/App-Autarkie-Korrektur, App-Icon

Betroffene Phase: Entwicklungsstand fuer P3/P4 (C3/S3, ESP-IDF 6.1); keine
produktive P5-Aktivierung. Partitionslayout, eFuse und OTA-Vertraege unveraendert.

**SDK-Entscheidung vollzogen (Nutzerfreigabe, nicht alle Gates frisch belegt):**
`platformio.ini`, `sdkconfig.esp32c3_optical_meter` und der `.pio`-Cache geloescht;
`CMakeLists.txt` waehlt jetzt unconditional `src/app_s3` als `EXTRA_COMPONENT_DIRS`
(kein IDF4.4.8-/No-ACME-Zweig mehr; `-DSMLEASY_IDF6_APP=1` dadurch wirkungslos,
aber unschaedlich). `README.md` und `.vscode/extensions.json` auf den
`idf.py`-Workflow umgestellt. `dependencies.lock` dadurch auf Component-Manager-
Aufloesung (espressif/cjson, espressif/mdns, espressif/mqtt, idf 6.1.0) aktualisiert.
Die §5-Tabelle "Noch erforderliche Entscheidungen" ist entsprechend angepasst.

**Firmware:** `kLanControlRoutes` (`web_server.cpp`/`.hpp`) ergaenzt die bestehenden
`kLanReadOnlyRoutes`: `/api/start`, `/api/start_continuous`, `/api/stop`,
`/api/reset_counters` bleiben per POST auf Port 80 erreichbar, auch wenn der Server
sonst auf HTTPS umleitet (Home-Assistant-Integration nutzt Port 80/HTTP ohne TLS).
`web_assets.hpp`: `EH_POS_DEFAULT`-Koordinaten fuer `pvo`/`pvw` (PV Ost/West) waren
seitenverkehrt und sind getauscht; die KPI "Eigenverbrauch" rechnete bisher die
PV-Eigenverbrauchsquote `(PV-Erzeugung - Einspeisung) / PV-Erzeugung` statt des vom
Nutzer erwarteten Autarkiegrads `(Hausverbrauch - Netzbezug) / Hausverbrauch` und
ist entsprechend korrigiert (Formel-Unterschied in Code-Kommentar dokumentiert).

**Android-App:** Dieselbe Autarkie-Korrektur in `BoardModel.kt` nachgezogen (die App
hatte unabhaengig vom Web-Dashboard dieselbe alte PV-Eigenverbrauchsquote-Formel;
Nutzer meldete weiterhin ~40 % in der App nach dem Web-Fix). Neues Launcher-Icon
(adaptiv fuer API 26+, Vektor-Fallback fuer die `legacy`-Variante mit minSdk 22) aus
dem bestehenden `kLogoSvg`-Markenzeichen erzeugt (`android/app/src/main/res/drawable`,
`mipmap`, `mipmap-anydpi-v26`); Gradient durch Volltonblau ersetzt, da
Vector-Drawable-Gradients erst ab API 24 unterstuetzt werden. `AndroidManifest.xml`
referenziert jetzt `android:icon`/`android:roundIcon`. Beide Debug-APKs
(`SMLEasy-Board-3.6.0-legacy-debug.apk`, `...-standard-debug.apk`) am Repo-Root neu
gebaut und ersetzt.

**Home-Assistant-Integration (`custom_components/smleasy`):** Geprueft, keine
Code-Aenderung noetig. Sie berechnet keine eigene Eigenverbrauchs-/Autarkie-Prozentzahl,
sondern liefert nur Rohwerte (`today_house_kwh`, `today_import_kwh`, `today_pv_kwh`,
`today_export_kwh` u. a. je Zeitraum); Routen/Port/TLS-Defaults (`/api/status` GET,
`/api/start`/`/api/stop`/`/api/reset_counters` POST, Port 80, SSL aus) passen bereits
zu den LAN-Ausnahmen.

| Nachweis | Ergebnis |
|---|---|
| Webtests (`node --test tests/energy_board.test.cjs`) | 4 bestanden (Autarkie-Formel-Fixtures angepasst) |
| C3, IDF v6.1, Clean-Build ohne `-DSMLEASY_IDF6_APP` (vereinfachtes `CMakeLists.txt`) | Build erfolgreich; BIN 1.797.120 Byte |
| OTA auf Live-Geraet `192.168.1.172`/`smartmeter.mkreutzer.de` | `{"ok":true}`, Neustart nach ca. 30 s (`reset_reason` 3); `app_version` 3.6.2, `idf_version` v6.1; `free_heap` 96.184 Byte, `min_free_heap` 74.128 Byte |
| Live-Verifikation Dashboard (Browser) | "Self-consumption"-KPI zeigt 98 % (`today_house_kwh` 14,466 kWh, `today_import_kwh` 0,264 kWh); Kachel "PV East" bei `left:90.6%`, "PV West" bei `left:18.5%` (korrekt getauscht) |
| Android (`./gradlew :app:assembleStandardDebug :app:assembleLegacyDebug`, JDK 17.0.20.1 unter `E:\Android\jdk`, SDK unter `E:\Android\sdk`) | Build erfolgreich (4 min 36 s); APKs 895.194 / 898.943 Byte |
| HA-Integration | alle `.py`-Dateien mit `py_compile` syntaktisch geprueft; keine automatisierten Tests im Projekt vorhanden |

**Offen:** Vollstaendiger `IPV6-PRUEFPLAN.md`-Durchlauf fuer diesen konkreten Build
nicht erneut ausgefuehrt (nur `ip6_sta`/`ip6_addresses` im Status-Snapshot als Indiz
geprueft). S3-Build nach der `CMakeLists.txt`-Vereinfachung in dieser Sitzung nicht
erneut kompiliert (sollte unveraendert funktionieren, da S3 bereits vorher `src/app_s3`
nutzte, aber nicht gegengeprueft). Android-App nicht auf echtem Geraet/Emulator
getestet (kein Android-Geraet/-Emulator in dieser Umgebung verfuegbar, nur
Gradle-Build + angepasster Unit-Test-Quelltext). Lokale Gradle-Wrapper-Dateien
(`gradlew`, `gradlew.bat`, `gradle-wrapper.jar`/`.properties`) wichen bereits vor
dieser Sitzung von der eingecheckten Version (8.7) auf 9.7.1 ab; das ist keine
Aenderung dieser Sitzung und wurde unveraendert gelassen/nicht committet.

Rueckweg: vorheriges GitHub-Release (`v3.6.0`) bzw. vorheriges OTA-Image auf dem
Live-Geraet; `CMakeLists.txt`/`platformio.ini`-Entfernung per Git-Revert umkehrbar,
falls der IDF-6.1-Pfad doch nicht ausreichend traegt.

### 2026-10-20: C3-Heap-Optimierung, Koppelmodus-Sperre, Neustart, Helligkeit

Betroffene Phase: Entwicklungsstand fuer P3/P4 (C3, ESP-IDF 6.1); keine
produktive P4-/P5-Aktivierung. Kein SDK-Wechsel, keine Partitions-/eFuse-Aenderung.
Aufgespielt per OTA auf das Geraet hinter `192.168.1.172`; laut seriellem Flash-Log
des Nutzers ESP32-C3 (Chiprevision und Flashgroesse nicht erfasst, ungeprueft).

Aenderungen: `MBEDTLS_DYNAMIC_BUFFER` (+`DYNAMIC_FREE_CONFIG_DATA`, `DYNAMIC_FREE_CA_CERT`)
in `sdkconfig.defaults.c3_idf6`; HTTPS `max_open_sockets` 3 -> 4, `recv/send_wait_timeout`
30 -> 8 s; Log-Ringpuffer 500 -> 80 Zeilen (`level`/`tag` als Festfelder);
Koppelmodus: nur noch eine Instanz, Toggle-Knopf, waehrend er offen ist darf nur die
IP des Startenden den HTTPS-Server nutzen (sonst 409; TLS-Handshake fremder
Verbindungen laeuft kurz an, kein hartes 1-Slot-Limit); `/api/reboot` mit Authentifizierung;
`/api/status` zusaetzlich `largest_free_block` und `stack_free` je Task. Android:
Neustart-Menue, Helligkeit (Automatik/fest/Nachtabsenkung mit Zeitplan, Standard
nachts 22-06 Uhr 5 %), Flavors `legacy` (minSdk 22) und `standard` (minSdk 24).

| Nachweis | Ergebnis |
|---|---|
| C3, IDF v6.1, Config-Neuerzeugung (`sdkconfig` geloescht) | Build erfolgreich; BIN 1.789.760 Byte (`0x1b4f40`), Slot 1.966.080 Byte, Reserve 176.320 Byte (9 %, unter dem 20-%-Ziel) |
| Heap nach Boot, vorher (ohne Aenderungen) | ca. 83 KB frei; Absturzschleife mit `min_free_heap` 1,6-13,7 KB |
| Heap nach Boot, nachher | ca. 97-113 KB frei, `min_free_heap` 72-98 KB |
| Betrieb mit App + Browser (4 Slots, Uptime ~1000 s) | ca. 104 KB frei, min 54 KB, groesster Block 78 KB |
| Last: 2/4/6 parallele neue TLS-Verbindungen je 5 Runden | 59 von 60 Antworten HTTP 200, eine Abbruch (`000`) bei 4 parallel; min_free_heap Tiefstwert 30.852 Byte, groesster Block 53.248 Byte; Erholung auf ca. 104 KB; kein Reset (`reset_reason` 3 = nach OTA) |
| Stack-Reserven (`stack_free`) | `meter_task` 9.624, `ext_poll` 6.336, `httpd` 6.364 Byte |
| Webtests | `node --test tests/energy_board.test.cjs`: 4 bestanden |
| Android | beide Flavors gebaut (`sdkVersion` 22 bzw. 24 per aapt); keine Geraete-/Fire-Abnahme |

**Offen:** Langzeitlauf (Fragmentierung, Leck), Messung mit offenem Koppelmodus und
mehreren Clients, Android-5.1-Test (TLS/Pairing/Helligkeit), Fire-HD-Installation,
Stack-Kuerzung nach Messung, Serial-Log des frueheren Absturzes (Ursache Heap nur
vermutet, nicht belegt). Keine Produktionsfreigabe. Hinweis Hardware: Das Shop-Geraet
(WiFi IR SMI V32) nutzt GPIO3 (RX), GPIO1 (TX), GPIO20/21 (zweiter Zaehler); S3-Eigenfertigung
siehe offene Entscheidungen, nichts davon umgesetzt.

Rueckweg: vorheriges IDF-6-Image per OTA; fuer die vorige Konfiguration die vier
`MBEDTLS_DYNAMIC_*`-Zeilen entfernen und `max_open_sockets` zuruecksetzen.

### 2026-10-21: serieller Notfallzugang (nur Dokumentation)

Betroffene Phase: Recovery/Betrieb, keine Code- oder Flash-Aenderung.

- Geraet: C3-Board "WiFi IR SMI V32" (Christians Technikshop). USB-C liefert nur
  Strom, ohne Datenweg zum C3 (Angabe des Betreibers, nicht am Geraet geprueft).
- Serieller Zugang ueber den Stecker "Zusatzmodul": RX, TX, GND und der GPIO fuer
  den Flash-Modus (Angabe des Betreibers). Pinreihenfolge und Steckerbelegung sind
  **nicht belegt** und muessen vor dem Anschluss aus Datenblatt/Schaltplan oder per
  Durchgangsmessung bestaetigt werden.
- Adapter: USB-UART-Modul mit 3,3 V (Betreiber). Kein 5-V-Pegel am C3.
- Allgemein beim ESP32-C3: Flash-Modus durch GPIO9 auf GND beim Reset, UART0 fuer
  den ROM-Bootloader. Fuer dieses Board nicht geprueft.
- OTA bleibt der Normalweg. Seriell nur bei nicht erreichbarem OTA; Flashen nur auf
  ausdrueckliche Freigabe, mit bestaetigtem Chip/Board/Pinbelegung.
- Offen: Pinbelegung, Test des seriellen Flashens und Wiederherstellung des
  Boot-Modus (kein Test ausgefuehrt).

### 2026-10-20 (nachts): Tageswerte aus Zaehlerstaenden, Korrektur-Endpunkt, Netz-Kreis, Summenzeile

Betroffene Phase: Statistik (Firmware) und lokales Web-Dashboard. Kein Relay, kein P4-P7.

- PV, Batterie laden/entladen und Wallbox: Tageswert = HA-Zaehlerstand minus
  Zaehlerstand zum Tagesstart (`day_start_cnt_wh_`, NVS-Keys `st_pv`, `st_chg`,
  `st_dis`, `st_wb`, i64 Wh, nur fuer den laufenden Tag). Beim Tageswechsel wird der
  letzte Zaehlerstand zum neuen Start. Ein dauerhaft (>= 120 s) niedrigerer Stand
  (Zaehler-Reset) oder ein Sprung >= 1000 kWh verschiebt nur den Start; der
  Tageswert bleibt erhalten. Ohne gespeicherten Start wird der letzte persistierte
  Zaehlerstand verwendet, so geht die Luecke ueber einen Neustart nicht verloren.
  `acc_*`/`cnt_*` bleiben unveraendert gespeichert (Rueckweg auf das alte Image).
- Ursache der falschen PV-Zahlen: HA-Rolle `pv_kwh` war ein Tageszaehler;
  umgestellt auf `sensor.goodwe_total_pv_generation` (Lebenszaehler, 1927,9 kWh).
- Neuer Endpunkt `POST /api/stats/pv` (Auth) `{"key":yyyymmdd,"pv_wh":n}`: setzt PV
  eines gespeicherten Tages (Ersparnis wird neu berechnet) oder des laufenden Tages
  (Start wird angepasst). `max_uri_handlers` 56 -> 64. Alte Vertraege unveraendert.
- Korrigierte Tage (manuell vom Betreiber): 17.10. 45,6; 18.10. 9,9; 19.10. 38,6;
  20.10. 34,4 kWh. 12.-16.10. unveraendert (nicht bekannt). Backup vor der Aenderung
  unter `%USERPROFILE%\smleasy-backup-20261020\b2` (nicht im Git).
- Wallbox-Ergebnis (`wb_milli_of`): kWh x (Erstattung - Netzanteil x Bezugspreis -
  PV-Anteil x Einspeisepreis). Der PV-Anteil ist geschaetzt (Eigenanteil des Hauses
  am Tag), nicht je Ladevorgang. Wirkung erst mit einer Wallbox-Rolle in HA; bisher
  sind die Wallbox-Spalten 0, nicht an einer echten Ladung getestet. Spaeter:
  KEBA-Sitzungsdaten je Karte (UDP 7090, `report 101..130`, noch nicht geprueft,
  Zugang offen). Web-Zeile "Gewinn" = Ersparnis - Netto-Kosten unter der Summe.
- Web: Netz-Kachel mit nichtlinearem Kreis (rot Bezug links, gruen Einspeisung
  rechts, 5 kW = 12 Uhr, Punkte 10/20 kW), rechtsbuendige Tabellenueberschriften,
  Summenzeile in Tage/Wochen/Monate/Jahre. Android-App unveraendert (/api/stats
  bleibt kompatibel).

Tests: Build IDF 6.1 OK (BIN 0x1b6470, 9 % Slot-Reserve, unter dem 20-%-Ziel); OTA
auf C3 192.168.1.172; Korrekturwerte nach zwei Neustarts unveraendert;
Reset-Grund 3; Heap frei 93.476 (min 62.960) kurz nach Boot. Nicht getestet: Tageswechsel
mit Lebenszaehler (erst morgen), Zaehler-Reset im Betrieb, Netz-Kreis und
Summenzeile optisch, Langzeitlauf, S3/andere Hardware.

Risiken: Heap-Minimum niedrig, Slot-Reserve knapp. Rueckweg: vorheriges Image per OTA
(unbekannte `st_*`-Keys werden ignoriert).

### 2026-10-20 (abends): gestufte Statistik-Aufbewahrung mit Jahresstufe und Sequenznummern

Betroffene Phase: Statistikspeicher (Firmware), kein Relay, kein P4-P7.

- Aufbewahrung: Tage = Anzahl Tage des laufenden Monats (28-31), danach Wochensummen bis 182 Tage, danach Monate bis 730 Tage, danach Jahressummen (neue Stufe 3 in `DayRec.day` Bits 14-15, Einheit 1000 Wh). Rest liegt in Home Assistant.
- Migration: `load_history()` faltet vorhandene Datensaetze beim Start einmalig (`compact_history()`) und schreibt sie neu (`history migrated`-Log). Nicht umkehrbar: ein altes Image liest die Datensaetze, kennt aber Stufe 3 nicht und wuerde Jahre als Monate lesen. Ruecksicherung: `/api/stats`, `/api/history`, `/api/status` vom 2026-10-20 unter `%USERPROFILE%\smleasy-backup-20261020` (nicht im Git).
- API (nur additiv): `/api/stats` hat je Eintrag das Feld `q` (Sequenznummer je Stufe: Tag/Woche/Monat/Jahr seit 2020, zustandslos aus dem Datum) und ein neues Array `years`. Bestehende Felder unveraendert. Dashboard und App zeigen `years` noch nicht an.
- Messung am C3 (192.168.1.172, ESP-IDF 6.1, BIN 1.790.xxx Byte, Slot-Reserve 9 %): nach OTA Heap 100.800 frei, min 57.908, groesster Block 73.728, Reset-Grund 3. Es gab nur 9 Tage Historie: keine sichtbare Faltung, Jahreseintrag 2026 wird aus den Monaten abgeleitet. Die Faltung ueber alte Daten und die Rundung bei Stufe 3 sind nicht auf dem Geraet getestet.
- Risiken: Rundung je Faltung (<= 0,5 kWh/Monat bei Jahressummen); Faltung nutzt den letzten Datensatz als "heute", bei falscher Uhr nicht korrekt.

### 2026-10-19: lokales Pairing, App-Verwaltung, IPv6-first und KPI-Leiste


Betroffene Phase: Entwicklungsstand fuer P3/P4; keine produktive P4-/P5-Aktivierung.
Kein Flashen, keine Aenderung an Partitionen, OTA-Slots oder eFuses.
Pairing-/API-/NVS-Vertrag und Risiken:
[Abschnitt 9a](SCHLUESSEL-UND-PAIRING.md#9a-implementierungsstand-idf-6s3-pfad-entwicklungsschlüssel).

| Nachweis | Ergebnis |
|---|---|
| S3, ESP-IDF v6.1, `build/`, Firmware 3.6.0 | Ninja-Build erfolgreich; BIN 1.679.712 Byte (`0x19a160`), kleinster OTA-Slot 1.966.080 Byte, Reserve 286.368 Byte (14,6 %) |
| Effektive S3-Config `build/config/sdkconfig.h` | `esp32s3`, 16-MB-Flash, IPv6/SLAAC und HTTPS aktiv, PSRAM aus; kein neuer Hardware-/Chiprevisionsnachweis |
| S3 statische Groessen (`esp_idf_size`) | Flash Code 922.414 Byte, Flash Data 651.324 Byte; DIRAM 114.182/341.760 Byte; IRAM 16.384/16.384 Byte |
| C3-Release, PlatformIO 2024.06.00, ESP-IDF 4.4.8 | Build erfolgreich mit `CI=true`, Version 3.6.0 unveraendert; Target `esp32c3`, 4 MB, IPv6/HTTPS weiterhin aus |
| C3 Groessen | BIN 1.513.584 Byte; PlatformIO Flash 1.508.744/1.966.080 Byte (76,7 %), RAM 29.268/327.680 Byte (8,9 %) |
| Android | JDK 17.0.20.1, Gradle 8.7, AGP 8.5.2; `assembleDebug testDebugUnitTest` erfolgreich; APK 861.462 Byte |
| Android/JVM-Tests | 5 bestanden: IPv6-Syntax/Interface-Scope, vier KPI-Werte/Update/fehlend vs. Null, echte TLS-Kopplung ueber `::1` mit Warten/Freigabe und Fingerprint, Vergleichsnummer-Bindung |
| Webtests | `node --test tests/energy_board.test.cjs`: 4 bestanden; Live-KPI-Werte/Update, fehlend vs. Null, frischer Status trotz Ext-Fehler, Pairing-/Verwaltungs-UI und JS-Syntax |

Die Web-KPI-Leiste wird unmittelbar aus `/api/status` gerendert, vor externen
Abfragen und Flussgrafiken, und auch durch den Dashboard-Statuspfad aktualisiert.
Die Android-Leiste verwendet dieselben vier Felder und dieselbe fachliche Formel
wie die Webansicht. Unbekannte Werte bleiben unbekannt statt als Null zu erscheinen.
Kein neuer Energie-/Tarif-Speichervertrag.

Buildumgebung: Der bereits veraenderte Gradle-Wrapper zeigt auf 9.7.1;
fuer den AGP-8.5.2-Nachweis wurde bewusst das vorhandene Gradle 8.7 verwendet,
ohne diesen fremden Wrapperstand zurueckzusetzen. Der Kotlin-Compiler lief
fuer die erfolgreiche Validierung mit
`-Pkotlin.compiler.execution.strategy=in-process`. Beim C3-Build wurde ein
bereits vorhandener IDF-6-only-ACME-Pause-Verweis im gemeinsamen Statushandler
abgegrenzt: C3 meldet `pause_s:0`, ohne ACME-Code zu aktivieren.

**Offen:** echtes Android-/ESP-Pairing inklusive 300-s-Abschaltung, Abbruch,
Ablehnen, volle Slots, NVS-Fehler, Widerruf und Reconnect; mDNS und Scope auf
realen Telefonen; HTTPS ueber das IPv6-Internet; SML/HA/OTA unter gleichzeitiger
TLS-Last. Kein Geraet mit diesem Stand geflasht, daher keine neuen Chiprevision-,
Heap-Minimum-, groesster-Block- oder Task-Stack-Messungen.
S3 unterschreitet das 20-%-OTA-Reserveziel und IRAM ist voll belegt:
keine Produktionsfreigabe. Das Zertifikatspinning erfordert bei
Zertifikatswechsel weiterhin erneutes lokales Pairing; sichere automatische
Rotation, HPKE und eFuse-Provider bleiben offen.

Rueckweg: vorheriges IDF-6-Image/App verwenden; neue optionale Token-Zeitmetadaten
koennen ignoriert werden, bestehende Tokenverifier bleiben unveraendert.
Ein Token, dessen Antwort bei Verbindungsabbruch verloren ging, in `/pair`
widerrufen und neu koppeln. Wechsel zur IDF-4.4-Firmware ist der bereits
dokumentierte getrennte Recoveryfall fuer verschluesselte Konfiguration/Token.

Jede Phase erhaelt Nachweise mit Datum, Build/SDK, Board/Chiprevision,
Testfaellen, Messwerten, Ergebnis und offenen Einschraenkungen.
Ein Gate gilt erst mit den benoetigten Hardware-/Recovery-Nachweisen als bestanden.
