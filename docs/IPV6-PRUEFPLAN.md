# IPv4/IPv6 parallel: Untersuchung und sichere Einfuehrung

Stand: 2026-10-08. Ergebnis: Dual-Stack ist im vorhandenen SDK vorgesehen.
Funktion auf den konkreten ESP-Testgeraeten ist noch nicht nachgewiesen.

## 1. Belegte Ergebnisse

### Konfigurationen sind widerspruechlich

- [sdkconfig.defaults](../sdkconfig.defaults) deaktiviert IPv6 und lokales HTTPS.
- [sdkconfig.esp32c3_optical_meter](../sdkconfig.esp32c3_optical_meter)
  und der vorhandene generierte PlatformIO-Header deaktivieren IPv6.
- Das lokale, ignorierte `sdkconfig` aktiviert IPv4, IPv6 und SLAAC.
- [PlatformIO](../platformio.ini) verwendet die gepinnte 2024.06.00-Plattform
  und das installierte ESP-IDF-4.4.8-Paket.
- Die lokale VS-Code-ESP-IDF-Erweiterung verweist auf ESP-IDF 6.1.
- `build/CMakeCache.txt`, `build/project_description.json` und `build.ninja`
  belegen einen getrennten CMake-Buildpfad mit `C:/esp/v6.1/esp-idf`,
  `IDF_VER=v6.1` und Target C3. Das belegt die SDK-Auswahl, nicht ein
  erfolgreich gebautes oder geflashtes Image.
- Im Screenshot sind unten ESP-IDF v6.1.0, Target `esp32c3` und COM3
  eingestellt; links ist zugleich der PlatformIO-Taskbaum sichtbar. PlatformIO
  verwendet fuer eigene Build-Tasks weiterhin das SDK aus `platformio.ini`;
  die Statusleiste stellt diese Tasks nicht auf 6.1 um. COM6 wurde als
  Espressif CDC erkannt.
- Der ESP-IDF-Buildaufruf am 2026-10-08 meldete einen Fehler. `build/smartmeter.elf`
  fehlt, daher startete der zugehoerige ELF-Monitor nicht. Es wurde kein
  6.1-Image geflasht oder auf Hardware getestet.
- Historische IDF-6.1-Buildlogs vom 2026-09-24 scheitern beim Uebersetzen von
  `components/sml/src/sml_reader.cpp` mit `driver/uart.h: No such file or
  directory`. [SML-CMake](../components/sml/CMakeLists.txt) verlangt aktuell
  `driver`; IDF 6.1 stellt UART als `esp_driver_uart` bereit. Historische
  Reconfigure-Logs nennen ausserdem `json`/`mqtt` als unbekannte Component-
  Namen. Das sind keine exakten Diagnosen des Aufrufs vom 2026-10-08.
- Fuer den neuen separaten S3-Pfad wurden die IDF-6-Components fuer UART,
  mDNS, MQTT und cJSON deklariert. C3-PlatformIO bleibt auf dem bestehenden
  IDF-4.4-Pfad. S3-Kconfig und Build wurden mit
  `CONFIG_IDF_TARGET="esp32s3"` und 16-MB-Flash erfolgreich erzeugt. Das BIN
  ist 1.210.944 Byte bei 1.966.080 Byte OTA-Slotgroesse (38 Prozent Reserve).
  IRAM ist voll belegt und braucht Analyse vor Produktivfreigabe.
- Die IPv6-Anzeige und der Dual-Stack-HTTP-Listener sind im S3-Pfad umgesetzt:
  S3-Defaults aktivieren IPv6/SLAAC; die WLAN-Verbindungsanzeige liefert eine
  bevorzugte globale IPv6-Adresse, ersatzweise Link-Local. Der HTTP-Server
  bindet bei aktiviertem IPv6 an `::` (alle Interfaces); IPv4 bleibt ueber
  IPv4-mapped IPv6-Sockets erreichbar. Der Socket-Filter laesst standardmaessig
  IPv4-private, IPv6-ULA und Link-Local zu; globale IPv6-Praefixe muessen in
  der CIDR-Freigabeliste explizit erlaubt werden. Build mit IDF 6.1 erfolgreich;
  das Image ist 1.249.392 Byte, 36 Prozent OTA-Slotreserve. Effektives
  `sdkconfig` und `build/config/sdkconfig.h` bestaetigen IPv6, SLAAC und vier
  Adressslots. Der erste Boot verband sich mit IPv4 und startete den HTTP-
  Server auf IPv4/IPv6, aber der Link-Local-Aufruf bei `WIFI_EVENT_STA_CONNECTED`
  schlug mit `ESP_FAIL` fehl. IDF-6.1-Quellcode bestaetigt, dass dieser Aufruf
  fehlschlaegt, solange das lwIP-Netif noch nicht up ist. Der Aufruf wurde
  deshalb hinter `IP_EVENT_STA_GOT_IP` verschoben. Korrigierter Build wurde
  erneut erstellt und per UART erfolgreich geflasht. Der folgende Bootlog
  bestaetigt Link-Local (`fe80::3edc:75ff:fe59:d460`), globale SLAAC-Adresse
  (`2a00:6020:a105:c900:3edc:75ff:fe59:d460`) aus dem angekuendigten
  Heimnetz-Praefix und ULA (`fd95:12b8:d02c:0:b2f2:8ff:fe16:cdad`). Der S3
  erhaelt IPv6 parallel zu IPv4. Dashboardzugriff ueber IPv6 und ausgehendes
  HTTPS zum IPv6-only-Testserver sind noch separat abzunehmen.
- Das Dashboard verschiebt IPv4-, WLAN- und Uptime-Status in den Footer der
  Seitenleiste. Die Status-API liefert zusaetzlich alle bevorzugten IPv6-
  Adressen des STA-Interfaces, damit Link-Local, globale SLAAC-Adresse und ULA
  gemeinsam angezeigt werden koennen. Der UI-Build wurde auf S3 (1.251.792 Byte,
  36 Prozent OTA-Slotreserve) und C3 (64,1 Prozent Flash, 8,9 Prozent RAM)
  erfolgreich gebaut und das S3-Image per UART mit Hashverifikation geflasht.
  Das neue Footer-Layout nach diesem Flash ist noch nicht per Browser
  nachgeprueft.
- GitHub-Release-OTA ist im S3-Testbuild deaktiviert: UI blendet den GitHub-
  Bereich aus und die authentifizierten GitHub-OTA-Endpunkte antworten mit
  HTTP 403. Lokaler OTA-Upload bleibt aktiv; C3-Releasepfad ist nicht betroffen.
- COM6 meldet Espressif VID/PID 303A:4001, ist aber allein kein Chipnachweis.
  Die spaetere ESP-IDF-Verbindung auf COM7 identifizierte den angeschlossenen
  Chip als ESP32-S3 Revision v0.2 mit erkanntem 8-MB-PSRAM. Das neue Image
  wurde per UART geschrieben und sein Flash-Hash erfolgreich verifiziert. Das
  16-MB-Boardprofil von PlatformIO meldete fuer sein N8-Profil einen
  Flashgroessen-Mismatch und wurde verworfen.
- Der lokale generierte C3-IDF-6.1-Header hat `CONFIG_LWIP_IPV6=1`, waehrend
  `sdkconfig.defaults` IPv6 deaktiviert. Eine bereits vorhandene lokale
  `sdkconfig` uebersteuert Defaults; Configs muessen pro Buildpfad getrennt
  geprueft werden.

Eine Aenderung nur in `sdkconfig.defaults` beweist keine Aenderung des naechsten
Builds: vorhandene Konfigurationen koennen den Wert beibehalten. Immer
effektiven SDK-Pfad und generierten Header pruefen. Ein 6.1-Build prueft
nicht die Kompatibilitaet der gepinnten 4.4.8-Releasefirmware. Der 6.1-Port
muss vor IPv6-Aenderungen in einem isolierten C3-Build reproduzierbar werden;
SDK-/API-Portierung und IPv6-Netzwerkfehler nicht im selben Schritt vermischen.

### HTTP-Listener aendert sich mit IPv6

Der installierte 4.4.8-SDK-Quellcode `esp_http_server/src/httpd_main.c`
erstellt bei `CONFIG_LWIP_IPV6` einen PF_INET6-Listener, sonst PF_INET.
lwIP unterstuetzt Dual-Stack-Sockets und IPv4-mapped IPv6.

Der aktuelle [dashboard_network_filter](../src/app/web_server.cpp) akzeptiert
nur AF_INET und IPv4-CIDRs. Aktiviertes IPv6 ohne angepassten Filter kann native
IPv6-Clients und IPv4-Clients in gemappter Darstellung ablehnen.
Das erklaert moegliche Dashboard-Unerreichbarkeit, beweist aber keinen Bootcrash.

Der historische Stand `3be79de` enthielt IPv6- und mapped-Adressbehandlung
fuer private Adressen. Deshalb ist der heutige Filter kein Beweis fuer die
damalige Fehlerursache. IPv4-CIDRs fuer gemappte Peers muessen ebenfalls getestet
werden; nur private gemappte Adressen zu erkennen reicht nicht.

### Historischer IPv6-Start

Im Stand `1e64a18` wurde `esp_netif_create_ip6_linklocal()` unmittelbar nach
Netif-Erzeugung und vor WLAN-Initialisierung/-Verbindung aufgerufen.
Das installierte SDK-Beispiel `protocol_examples_common/connect.c` ruft es
stattdessen auf `WIFI_EVENT_STA_CONNECTED` auf. Fuer neue Implementierung
diesen lifecyclegerechten Weg verwenden und Rueckgabewerte pruefen.
Die fruehe historische Reihenfolge ist ein zu korrigierender Unterschied,
keine bewiesene Ursache des damaligen Nichtstarts.

Mit `a257009` wurden IPv6, lokales HTTPS und ACME zusammen aus dem Recoverypfad
entfernt. Der fehlerhafte Stand startete unter anderem einen HTTPS-Server mit
16-KiB-Stack und optional einen separaten ACME-Listener. Mehrere gleichzeitig
geaenderte Features erlauben keine eindeutige Fehlerzuordnung.
Vorhandene Buildlogs zeigen auch 4.4/6.1-Kompatibilitaetsfehler, aber keinen
seriellen Panic-/Backtrace-Nachweis fuer den gemeldeten Bootfehler.

### DNS und HTTPS koennen IPv6

ESP-TLS 4.4.8 verwendet `getaddrinfo(AF_UNSPEC)` und verarbeitet AF_INET6.
lwIP bevorzugt dabei A und versucht AAAA, wenn die IPv4-Aufloesung fehlschlaegt.
Das ist kein Happy Eyeballs: eine vorhandene, aber unerreichbare A-Adresse
garantiert keinen Wechsel zu AAAA. Fuer spaetere gemischte DNS-Endpunkte
Adressauswahl/-Retry gesondert pruefen; fuer das AAAA-only-Testziel passt
der vorhandene grundsaetzliche Codepfad.

### Freigegebener PC-Test

`smarthome.mkreutzer.de` lieferte bei der Untersuchung einen AAAA-Record
`2a00:6020:a105:c900:efe:39bc:c52f:caa4`, keinen A-Adressrecord.
Die Adresse ist ein zeitbezogener Nachweis, keine fest zu verdrahtende IP.

Mit `curl.exe -6`:

- HEAD: HTTP 405 mit `Allow: GET`; TCP/TLS-Verbindung dabei erfolgreich.
- GET: HTTP 200, `ssl_verify_result=0`, Gegenstelle die o.g. IPv6-Adresse.
- TCP-Verbindungszeit etwa 22 ms, TLS-Verbindungszeit etwa 54 ms.
- Antwortinhalt nicht als Artefakt gespeichert; keine Zugangsdaten verwendet.

Das beweist DNS, IPv6 und TLS vom Entwicklungs-PC. Nicht bewiesen sind
ESP-SLAAC, ESP-Routing, dessen CA-Bundle oder Speicherverbrauch.
Interne ULA-/Link-Local-Erreichbarkeit wurde nicht geprueft, da keine interne
Zieladresse und kein Scope festgelegt wurden.

## 2. Sicherer Implementierungsweg

1. Den ESP-IDF-6.1-C3-Port isoliert buildbar machen. Zunaechst UART-/GPIO-
   Component-Abhaengigkeiten und Managed-Components entsprechend der tatsaechlich
   aufgeloesten Component-Namen korrigieren; nachfolgende Compiler-/APIfehler
   einzeln portieren. PlatformIO bleibt bis zu einer formalen Migration auf
   dem gepinnten 4.4.8-Releasepfad.
2. C3 und S3 N16R8 als getrennte Targets fuehren. Der bisherige
   `esp32c3_optical_meter`/`lolin_c3_mini`-Build ist kein S3-Build.
3. Serielle Baseline fuer C3 sichern: Resetgrund, komplette Bootsequenz,
   Panic/Backtrace, freier/minimaler Heap, groesster freier Block, Task-Stacks.
4. Separate Dual-Stack-Testkonfiguration mit IPv6 und SLAAC, weiterhin IPv4.
   Release-Config nicht ungeprueft umstellen; lokales HTTPS/ACME bleiben aus.
5. Config und APIs mit dem jeweils tatsaechlichen SDK pruefen: 4.4.8 und 6.1
   nicht vermischen. `CONFIG_LWIP_IPV6`,
   `CONFIG_LWIP_IPV6_AUTOCONFIG`, `CONFIG_LWIP_IPV6_NUM_ADDRESSES`.
   Fuer Link-Local + ULA + GUA mindestens drei Slots, vier als Teststartwert
   fuer Prefixwechsel budgetieren. Mehr Adressen nicht ohne RAM-Messung.
6. Link-Local auf `WIFI_EVENT_STA_CONNECTED` erstellen, Ergebnisse loggen.
   `IP_EVENT_GOT_IP6` dem richtigen STA-Netif zuordnen.
7. WLAN-Assoziation, IPv4-Bereitschaft, IPv6-Adressen und Cloud-Erreichbarkeit
   getrennt modellieren. Keine Boot-/SML-Sperre bis zur IPv6-Zuteilung.
8. Disconnect/Reconnect invalidiert Adressen und Cloudstatus. Geteilte Zustands-
   daten synchronisieren, keine ungeschuetzten Strings im Eventhandler.
9. Adressart und Gueltigkeit unterscheiden: Link-Local, ULA, GUA,
   tentative/deprecated/preferred. Eine GUA allein beweist keine Route.
10. HTTP-Filter kanonisiert native IPv4 und IPv4-mapped IPv6 vor CIDR-Pruefung.
   RFC1918-Regeln erhalten, ULA/Link-Local bewusst behandeln; globale LAN-
   Praefixe nur explizit erlauben. Nicht ganz `2000::/3` freigeben. Der
   IPv6-Listener bindet an `::`; dadurch ist die TCP-Oberflaeche auf allen
   IPv6-Adressen vorhanden, aber ein GUA-Zugriff bleibt ohne passende CIDR-
   Freigabe durch den Socket-Filter blockiert.
11. Begrenzter HTTPS-GET im separaten Worker: 10 s Connect-/20 s Gesamtbudget
   als Ziel, kleine Streaming-Puffer, Hostname/SNI und CA-Verifikation.
   Reale APIs des SDK fuer Timeouts pruefen. Kein `skip_cert_common_name_check`,
   kein Insecure-TLS, keine endlosen Retries oder Cloud-Task im Eventhandler.
12. 12. Status um IPv6-Adressen, letzte TLS-/DNS-/Connectfehler, Zeitpunkt und
    Ressourcenwerte erweitern, ohne Secrets/Serverinhalt offenzulegen.

Ausgehendes HTTPS erfordert keinen HTTPS-Server oder ACME auf dem ESP.
Ein Dual-Stack-Listener darf das Dashboard nicht global oeffentlich machen.

## 3. Netzvoraussetzungen

- Externe IPv6-Adresse: nutzbare GUA am ESP, IPv6-Default-Router,
  funktionierende DNS-Aufloesung, Firewallzugang zu TCP 443.
- Interne ULA (`fc00::/7`): nur bei passender interner Route/Split-DNS,
  nicht automatisch im IPv6-Internet erreichbar.
- Link-Local (`fe80::/10`): nur auf demselben Link, Interface/Scope erforderlich.
  Fuer Cloud-Service-Endpunkte ungeeignet.
- SLAAC benoetigt passende Router Advertisements. Stateful DHCPv6-only ist
  kein Ersatz: das vorhandene lwIP-Kconfig unterstuetzt nur stateless DHCPv6.
- Fuer IPv6-only-WLAN DNS per RA/RDNSS oder geeignete Konfiguration pruefen.
  `CONFIG_LWIP_IPV6_RDNSS_MAX_DNS_SERVERS` ist im SDK vorhanden, Default 0.
  Im Dual-Stack-Netz kann IPv4-DNS ebenfalls AAAA aufloesen.
- ICMPv6 fuer ND/RA und Packet Too Big zulassen; ein TCP-443-Erfolg mit kleinen
  Antworten beweist noch keine funktionierende Path-MTU fuer TLS/Batches.
- Gueltige Uhr und CA-Bundle fuer Zertifikate erforderlich. Zeitversorgung
  ueber IPv6 und Cold-Boot ohne Netz separat pruefen.
- Kein IPv6 am Anschluss bedeutet kein Direktupload an einen IPv6-only-Server.
  Lokal puffern, sichtbar offline bleiben; keine Firmwareoption kann die
  fehlende Netzroute ersetzen. Ein Relay waere eine eigene spaetere Entscheidung.

## 4. Hardware-Abnahme

| Test | Erwartung |
|---|---|
| C3 IPv4-only, kein RA | Boot, SML, Dashboard und HA funktionieren |
| C3 Dual-Stack | IPv4 und bevorzugte ULA/GUA parallel vorhanden |
| C3 HTTPS AAAA-only | GET-Testziel, verifizierter TLS-Handshake |
| IPv4-Client auf v6-Listener | mapped Adresse erhaelt gleiche CIDR-Regeln |
| Native IPv6-Clients | ULA/Link-Local/erlaubte GUA korrekt gefiltert |
| Nicht erlaubte GUA | abgewiesen, kein unbeabsichtigtes oeffentliches Dashboard |
| Server down/DNS defekt | lokaler Betrieb bleibt aktiv, sichtbarer Fehler |
| Zertifikat falsch/Uhr ungueltig | TLS verweigert, kein Insecure-Fallback |
| RA verzoegert/Prefixwechsel | kein Bootblock, Zustand aktualisiert |
| WLAN-/Routerneustart | neue Adressen und Wiederaufnahme ohne Rebootloop |
| Kein DHCPv4, aber IPv6 vorhanden | STA nicht vorschnell als WLAN-Fehler behandeln |
| IPv6-Route/MTU gestoert | begrenzter Timeout, Puffer und Tasks freigegeben |
| HTTP + MQTT + TLS + SML | keine Starvation, ausreichender Heap/Stack |
| OTA bei Cloudaktivitaet | Upload begrenzt/pausiert, OTA und Recovery funktionieren |
| 20 Cold Boots / 20 Reconnects | keine Panic, kein Lockout |
| 72 Stunden C3-Dauertest | keine Rebootloops oder fortlaufende Heapabnahme |
| ESP32-S3 DevKitC-1 N16R8 | Chiprevision sowie Flash/PSRAM erkannt, gleiche Funktion, separate Ressourcenwerte |

Pro Test Build, SDK, generierte Config, Chiprevision, Netztyp, Messwerte und
serielle Diagnose festhalten. Hardware-Zeilen sind aktuell alle offen.
Den frueheren Crash erst mit Resetgrund/Backtrace als aufgeklaert markieren.

## 5. Referenzen

- [ESP-NETIF 4.4.8](https://docs.espressif.com/projects/esp-idf/en/v4.4.8/esp32c3/api-reference/network/esp_netif.html)
- [Aktueller WifiManager](../src/app/wifi_manager.cpp)
- [Aktueller WebServer und Zugriffsfilter](../src/app/web_server.cpp)
- Git-Historie: `1e64a18`, `3be79de`, `a257009`, `c876622`.
- Installierter SDK: `components/esp_http_server/src/httpd_main.c`,
  `components/esp-tls/esp_tls.c`, `components/lwip/lwip/src/api/netdb.c`,
  `components/lwip/Kconfig`, `examples/common_components/protocol_examples_common/connect.c`.
