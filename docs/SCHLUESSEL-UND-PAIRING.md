# eFuse, Device-Schluessel und Pairing

Stand: 2026-10-08. Zielmodell; noch kein implementiertes oder freigegebenes
Produktprotokoll. Dieses Dokument enthaelt absichtlich keine echten Secrets.
Schluessel entstehen auf dem jeweiligen Geraet bzw. in der App, nicht in Git.

## 1. Sicherheitsgrenzen

- Pro ESP ein zufaelliges 256-Bit-Root-Secret in einem nicht auslesbaren
  HMAC_UP-eFuse-Keyblock. Kein Root-Secret aus MAC oder Seriennummer.
- ESP32-C3 hat sechs Keybloecke. HMAC_UP liefert HMAC-SHA256 an Software;
  das Root selbst bleibt verborgen. Ableitungen liegen zeitweise im RAM.
- Software mit Zugriff auf HMAC_UP kann Keys ableiten. eFuse allein schuetzt
  nicht vor boesartiger Firmware; produktiver Schutz erfordert Secure Boot,
  passende Debug-/Downloadrestriktionen und geprueften Updatepfad.
- Flash Encryption und NVS-Verschluesselung schuetzen lokale Daten getrennt
  von E2E-Payloads. Coredumps und Diagnosen duerfen keine RAM-Keys exportieren.
- Entscheidung (2026-10-19, Nutzer): NVS wird nicht verschluesselt/geschuetzt.
  Geschuetzt wird nur der Geraeteschluessel (Root in eFuse HMAC_UP), mit dem
  eine App sicher gekoppelt wird. Folge: In NVS duerfen nur Daten liegen, die
  ohne Geheimnis nutzlos bleiben (Konfiguration, oeffentliche App-Keys,
  gewrappte Schluessel, Token nur als Hash). `K_store` und
  Flash-Encryption sind damit nicht Voraussetzung fuer das Pairing; wer
  Flash-Zugriff hat, kann NVS lesen und aendern (Konfiguration, App-Liste).
  Ausnahme/Uebergang: der Development-KeyProvider legt seinen Root in NVS ab
  und ist daher nicht produktiv. Signatur der App-Berechtigungseintraege
  (z. B. mit `K_auth`) ist bewusst auf spaeter verschoben (Nutzerentscheidung);
  bis dahin ist eine Manipulation der App-Liste bei Flash-Zugriff moeglich.
- Device und App verwenden verschiedene Keys fuer Daten, Authentifizierung,
  Pairing und lokale Speicherung.
- Der Server darf weder Root noch Data-Key oder entschluesseltes Energiedatum
  besitzen. Auch ein serverseitiges Monitoring-/Aggregate-Subsystem nicht.

## 2. KeyProvider und Provisionierung

Schnittstelle fuer Root-basierte HMAC-Ableitung; kein `read_root()` anbieten.
Virtuelle eFuses/Testprovider fuer Tests kennzeichnen. Produktiver Cloudbetrieb
darf sich mit einem Development-KeyProvider nicht aktivieren lassen.

### Reversibler Vorlauf

1. Chip/Revision/Board bestimmen, eFuse-Belegung und Schutzbits nur lesen.
2. Slotplan fuer HMAC_UP, Flash-Encryption-Key und Secure-Boot-Digests erstellen.
   Keine feste Slotnummer auf allen vorhandenen Geraeten voraussetzen.
3. SDK-Version und verfuegbare `esp_efuse_write_key`-/HMAC-APIs pruefen.
4. Virtuelle Tests fuer freie/belegte/falsch konfigurierte Slots, Stromausfall
   und erneute Provisionierung. Batch-Schreiben ist keine Stromausfallgarantie.
5. Bootloadergroesse, Partitionen, signierte Dual-OTA und Recovery testen.
6. Geraet, Slot, Zwecke, Schutzbits und irreversible Folgen schriftlich
   bestaetigen lassen. Erst danach physisch provisionieren.

### Geraetelokale Root-Erzeugung

1. Separater, physisch freigegebener Provisionierungsmodus; kein normaler
   Bootpfad und kein ungeschuetzter HTTP-Endpunkt.
2. Sichere Entropiebedingungen nach ESP-IDF nachweisen: aktive RF-Entropie
   oder kontrolliert aktivierte Bootloader-Entropiequelle. `esp_fill_random`
   ohne diese Bedingungen nicht automatisch als True-RNG betrachten.
3. 32 Byte Root in begrenztem RAM-Puffer erzeugen. Nicht ausgeben, senden oder
   dauerhaft als Datei/NVS-Wert speichern.
4. Freien freigegebenen Keyblock mit Purpose HMAC_UP beschreiben; mit den
   SDK-APIs Lese-/Schreibschutz und Purpose-Schreibschutz sicherstellen.
5. Purpose/Schutzstatus lesen, HMAC mit Testkontext berechnen und gegen die
   vor dem Brennen lokal berechnete Erwartung pruefen. Root nicht zuruecklesen.
6. Root- und Vergleichspuffer mit geeigneter nicht wegoptimierter Funktion
   loeschen. Status, Slot und oeffentliche Device-ID duerfen protokolliert werden.
7. Reboot, HMAC-Funktion und erhaltene Identitaet pruefen. Bestehenden korrekten
   Slot wiederverwenden; widerspruechlicher/partieller Zustand ist ein Fehler.
   Niemals blind einen weiteren Slot brennen.

SDK-4.4.8-Grundlage: `esp_efuse_write_key` schreibt Key, Purpose und Schutz
im Batchmodus. Der genaue Protection-Vertrag ist pro Purpose/SDK nachzulesen.
Keine rohe eFuse-Bitfolge oder pauschale Brennbefehle aus dieser Planung ableiten.

## 3. Device-ID und abgeleitete Keys

Alle HMAC-Eingaben erhalten eine kanonische, versionierte Serialisierung
mit festen Feldlaengen/Laengenpraefixen. Keine mehrdeutige Stringverkettung.
Vor Freigabe bytegenaue Testvektoren erstellen.

```text
device_id = trunc128(HMAC_root(context("smleasy/device-id/v1", factory_mac)))
K_auth    = HMAC_root(context("smleasy/server-auth/v1", device_id))
K_pair    = HMAC_root(context("smleasy/pairing/v1", device_id))
K_store   = HMAC_root(context("smleasy/local-storage/v1", device_id))
K_data    = HMAC_root(context("smleasy/energy-data/v1", device_id, key_epoch))
```

`factory_mac` bezeichnet eine stabile Hardwarekennung, nicht eine
konfigurierbare WLAN-MAC. `device_id` ist oeffentlich, kein Berechtigungsnachweis.
`key_epoch` ist eine zufaellige 128-Bit-Kennung; neben ihr kann eine lesbare
Generationsnummer gefuehrt werden. Generationsnummer allein ist nach einem
kompletten Speicherreset nicht sicher eindeutig.

- Jede neue Daten-Key-Epoche verwendet sichere Entropie und wird vor ihrer
  Nutzung crashfest committed.
- Alte Epochen bleiben fuer Historie dokumentiert und in Apps entschluesselbar.
- Nach vollstaendigem Speicherverlust neue zufaellige Epoche erzeugen und
  physische Recovery/Re-Pairing verlangen; nicht Generation 1 reproduzieren.
- Root bleibt erhalten, aber eFuse speichert nicht automatisch Sequenzstand,
  Epochenliste, App-Berechtigungen oder gewrappte Keys.
- Normale OTA/NVS-Konfigurationsresets sollen das separate Sicherheitsjournal
  erhalten. Ein vollstaendiges Reflash/Erase ist ein eigener Recoveryfall.

## 4. AEAD und Nonces

Ziel: AES-256-GCM aus mbedTLS, 96-Bit-Nonce, 128-Bit-Tag.
Die aktuelle 4.4.8-Config enthaelt GCM und P-256/ECDH; HKDF muss fuer das
gewaehlte Standard-Wrapping geprueft/aktiviert werden.

Empfohlener Nonce-Vertrag: 32-Bit feste Domaene plus 64-Bit monotoner
Verschluesselungszaehler pro Key-Epoche. Counterbereiche vor Nutzung crashfest
reservieren; nach Neustart nicht verwendete Reservierungen ueberspringen.
Counter ist nicht von gueltiger Uhrzeit abhaengig.

Bei verlorener/zweifelhafter Nonce-Persistenz niemals denselben Key weiter
verwenden: neue zufaellige Epoche oder verschluesselten Upload sperren.
Flash-/NVS-Rollback als eigenen Fall pruefen. Ein System, das beliebige
Snapshots seiner Zaehler unter gleichem Key restauriert, erfuellt die
Nonce-Garantie nicht. Falls Rollback nicht verhindert/erkannt werden kann,
vor jeder neuen Verschluesselungssitzung eine frische zufaellige Key-Epoche
als Subkey-Kontext verwenden; deren Wrapping-/Speicheraufwand separat abnehmen.

AAD bindet Version, Device-/ggf. Stream-ID, Recordtyp, Sequenz, Key-Epoche
und Payload-Schema bytegenau. Nonce und Tag werden uebertragen.
Zeit, Counter und optionale Energiequellen sind im Ciphertext.

Ein Record wird einmal verschluesselt. Fuer Retries denselben Envelope
wiederverwenden oder bewusst mit neuem Nonce verschluesseln und den Server-
Konfliktvertrag beachten. Kein veraenderter Klartext unter alter Nonce.
Messungen und Aggregate verwenden getrennte Key-/Nonce-Domaenen.

Pruefen: bekannte Testvektoren, falscher Tag/AAD/Key, Replay, Counterende,
Reset waehrend Reservierung, verlorenes Journal und Wiederholung.

## 5. Device- und App-Keypairs

Neben symmetrischen HMAC-Ableitungen benoetigt das Pairing eine authentifizierte
Device-Identitaet. Keine beliebige KDF-Ausgabe als privaten ECC-Key interpretieren.

- Device erzeugt ein P-256-Keypair ueber die Standardbibliothek mit sicherer
  Entropie. Privaten Key lokal unter K_store mit authentifizierter Verschluesselung
  speichern; seine Nonce-/Epochenverwaltung muss denselben Resetregeln genuegen.
- Device stellt fuer Pairing ein Zertifikat/oeffentliches Keymaterial bereit.
  Der QR-Code pinnt dessen oeffentlichen Schluessel/Fingerprint.
- App erzeugt separate Keypairs fuer ihre Identitaet und fuer Daten-Key-Wrapping,
  sofern die gewaehlte Standardbibliothek beide Rollen nicht sicher trennt.
- App-Private-Keys in Keychain/Android Keystore/Secure Storage; exportierbare
  Fallbacks nur mit erklaertem Risiko und verschluesseltem lokalen Speicher.
- Verlust des Device-Identitaetskeys nach vollem Erase erfordert neue physische
  QR-Vertrauensbindung. Historische Data-Keys in Apps bleiben verwendbar.

## 6. Pairing: Zielmodell und aktuelle Entscheidung

**Entscheidung 2026-10-19:** Kein QR-Code mehr. Besitzer startet einen
300-s-Koppelmodus im lokalen, authentifizierten Webfrontend. Die App entdeckt
das Geraet und stellt eine Anfrage; erst die ausdrueckliche Freigabe im
Webfrontend liefert ein App-Geheimnis und den Fingerprint. Die folgende
QR-/HPKE-Beschreibung bleibt historische Zielplanung, nicht der aktuelle
Implementierungsvertrag. Der Entwicklungsstand und seine Grenzen stehen in
Abschnitt 9a; produktives Key-Wrapping/eFuse sind dadurch nicht freigegeben.

Zielverfahren: kurzlebiger QR-Bootstrap plus TLS mit gepinntem Device-Key.
Nicht das heutige unverschluesselte Basic-Auth-HTTP fuer Key-Uebergabe verwenden.
Ein begrenzter, nur bei physischer Freigabe aktiver Pairing-TLS-Dienst ist
getrennt vom dauerhaften Dashboard zu budgetieren.

Fuer die App-Registrierung ist eine eigene Pairing-Seite auf dem Geraet
erforderlich. Sie darf den QR-Code erst nach physischer Freigabe waehrend des
aktiven Pairingfensters anzeigen, muss die verbleibende Gueltigkeit kenntlich
machen und den Code nach Ablauf oder erfolgreichem Pairing entfernen. Die Seite
ist kein Ersatz fuer den authentifizierten Pairing-Kanal und darf den QR-
Bootstrap nicht ueber einen oeffentlichen oder ungeschuetzten Fernzugriff
bereitstellen.

QR enthaelt Protokollversion, Device-ID, lokale Kontaktinformationen,
Device-Public-Key/Fingerprint, zufaellige Session-ID und zufaelliges
256-Bit-Einmal-Token. Keine Data-/Auth-/Root-Keys.
Token wird mittels sicherem RNG erzeugt; K_pair dient nur einem im
finalen Protokoll definierten Authentifizierungs-/Kontextzweck.

1. Physischer Ausloeser oeffnet ein maximal 120-s-Pairingfenster.
2. App scannt QR und erzeugt ihre Keypairs lokal.
3. App verbindet ueber TLS und prueft den gepinnten Device-Key. Kein
   ungesichertes Fallback; QR darf nicht ueber ungeschuetztes Remote-Dashboard
   zur Vertrauensquelle werden.
4. Einmal-Token, Session-ID und App-Public-Keys ueber den authentifizierten
   Kanal pruefen. Rate Limit, Sessionbindung und Replay-Sperre anwenden.
5. Neue App erst nach Proof of Possession und bewusstem Besitzer-Grant
   freigeben. Ein bekanntes Device-ID reicht nicht.
6. Aktuelle Data-Keys fuer den App-Wrapping-Key kapseln. Zielstandard HPKE
   nach RFC 9180 mit passender standardisierter Suite/Bibliothek; konkrete
   Library und C3-Ressourcen muessen vor Produktcode festgelegt werden.
   Kein selbst gebautes ECDH+AES-Protokoll als ungepruefter Ersatz.
7. App importiert Keys und einen verschluesselten Testrecord atomar, bestaetigt
   Besitz/Entschluesselung. ESP schliesst Session und invalidiert Token.
8. Abbruch-/Commit-Zustaende erlauben begrenzte Wiederaufnahme derselben
   authentifizierten Session, nicht mehrfache beliebige Token-Nutzung.
9. Weitere Apps benoetigen eigenes Pairing oder einen standardisierten
   verschluesselten Owner-Grant; bestehende Keys nicht an den Server geben.

Physischer Button und vertrauenswuerdige QR-Anzeige sind fuer die beiden
ESP32-C3- und ESP32-S3-Boards noch festzulegen. Ein Code auf unverschluesseltem
LAN-HTTP hat nicht dieselbe Vertrauensqualitaet wie eine direkte, lokale
physische Uebergabe.

## 7. Serverauthentifizierung und Registrierung

K_auth ist kein Data-Key. Fuer den ersten Entwurf getrennte TLS-geschuetzte
Registrierung in einem ausdruecklich autorisierten Provisionierungsvorgang.
Bei symmetrischem HMAC-Auth darf der Server K_auth besitzen, aber keinen
anderen Key. Registrierung darf nicht allein bekannte Device-IDs akzeptieren.

Request-Auth muss Methode, Pfad, Bodyhash und frische Challenge/Request-ID
binden; Replay verhindern, Nonce/Challenge nach Verarbeitung verbrauchen.
Nicht allein auf Uhrzeit vertrauen, solange der ESP seine Zeit nicht kennt.
Finales Verfahren und Speicherung von Server-Auth-Secrets vor P5 festlegen;
asymmetrische Auth/mTLS ist eine separat zu budgetierende Alternative.

App-Account-Authentifizierung und Device-Schreibrechte bleiben getrennt.
Der Server prueft Account -> Meter -> Device fuer jeden Download/Grant.

## 8. Entzug, Recovery und Device-Ersatz

- App entziehen: Berechtigung serverseitig sperren und neue Data-Key-Epoche
  nur an weiterhin berechtigte Apps wrappen. Eine lokal bekannte alte
  Schluesselkopie laesst sich nicht nachtraeglich unlesbar machen.
- Offline-Device: Entzug schuetzt neue Messungen erst nach durchgefuehrter
  Rotation; Status offen anzeigen, nicht sofortige Revocation behaupten.
- Historische Wrappings/Keys behalten, solange die Historie lesbar sein muss.
- Tote ESPs koennen keine neuen alten Keys liefern. Apps oder ein vorab
  erzeugtes Recoverybackup muessen diese bereits besitzen.
- Gewrappte Keys fuer einen verlorenen App-Private-Key sind allein nutzlos.
  Recoverybackup der Daten-Key-Epochen unter separatem, nutzerseitigem
  Recovery-Geheimnis vorsehen; Server darf dieses Geheimnis nicht besitzen.
- Behauptung "App-Private-Key verlaesst nie das Geraet" beibehalten:
  Recovery exportiert verschluesselte Data-Key-Epochen, nicht den App-Private-Key.
- Neues ESP hat neue Root-/Device-Identitaet und wird demselben Meter
  zugeordnet. Zeitliche Ueberlappungen/doppelte Messquellen bewusst aufloesen.
- Noch nicht hochgeladene Daten eines physisch zerstoerten ESP koennen verloren
  sein. Austauschbarkeit garantiert keine Rettung ungesicherter Flashdaten.

## 8a. Entscheidungen des Betreibers (2026-10-20, Entwurf Push-Relay)

Nur Entscheidungen, keine Implementierung. Offene Punkte stehen unten.

- Zwei Schluessel: Geraeteschluessel (Identitaet, nur fuer die Gueltigkeitspruefung
  gegenueber dem Server) und ein davon getrennter Datenschluessel fuer Verschluesselung
  und Signatur einzelner JSON-Teile. Der Datenschluessel wird beim lokalen Koppeln
  verschluesselt zwischen ESP und App geteilt und dem Server nie bekannt.
- Klartext im Paket nur, was der Server fuer Sequenz/Dedup braucht (Geraete-ID, epoch,
  seq, Zeitstempel, Stufe, q). Alles Weitere verschluesselt; Signatur ueber das ganze Paket.
- Aufbewahrung auf dem Server: nur das letzte Paket plus abgeschlossene Perioden
  (Tage = Monatslaenge, dann Wochen, Monate, Jahre); der Rest liegt in Home Assistant.
- Verlust/Tausch: keine Wiederherstellung der Cloud-Daten. Ein ersetztes Geraet wird neu
  gekoppelt. Ein Geraet, das 6 Wochen nicht pusht, wird samt Eintrag, Schluessel und Daten
  auf dem Server geloescht; es registriert sich nicht automatisch neu.
- Limit: 4 Apps/Clients pro ESP.
- Missbrauchsschutz: Bei erkanntem Missbrauch (z. B. zu viele Registrierungen, Pushes oder
  ungueltige Signaturen) sperrt der Server die Quelle 6 Stunden. Schwellwerte noch offen.
- Widerspruch zu Abschnitt 8 (Recoverybackup, Historie behalten) bewusst: gilt fuer das
  Relay nicht; Abschnitt 8 bleibt fuer einen spaeteren Server mit Wiederherstellung.

Offen: Verfahren (libsodium/HPKE/mbedTLS, Messung Flash/Heap auf dem C3), Schluesselversion
(`kid`) bei Rotation, Datenschluessel im Web-Client, Limit fuer Registrierungen/Pushes,
Warnung der App vor Ablauf der 6 Wochen, Webgo-Eignung (PHP-Version, sodium, Header, Cron).

## 8b. Relay-Transport und Speicherlayout (Entwurf, nicht implementiert)

Betriebsarten: Ohne eingerichteten Cloud-Zugang verbindet ein ESP nie nach aussen
(Verhalten wie heute, kein Mehrverbrauch an Heap). Mit eingerichtetem Zugang:

1. Tagesabschluss: Der ESP legt den abgeschlossenen Tag (mit `q`, Stufen Tag/Woche/Monat/Jahr)
   auf dem Server ab. Clients holen fehlende Perioden nach.
2. Live-Modus: Ein Client, der den ESP nicht direkt erreicht, setzt auf dem Server ein
   befristetes Flag (Lease, z. B. 10-15 min). Jeder Abruf des Clients verlaengert es, ohne
   Abrufe laeuft es ab. Ein Dauerflag gibt es nicht.
3. Der ESP fragt alle 5 min kurz nach dem Flag (kleiner Request, kein Keep-Alive noetig).
4. Ist das Flag gesetzt, pusht er alle 30 s den aktuellen Wert, mit TLS-Keep-Alive, solange der
   Server die Verbindung haelt; sonst Reconnect (ggf. Session-Resumption).
5. Die Flag-Antwort muss authentisiert sein (Signatur oder festes Server-Zertifikat), mit
   Maximum der Lease-Dauer und Mindestintervall am ESP, damit niemand Dauerpush erzwingt.
6. Serverausfall, DNS- oder TLS-Fehler duerfen den lokalen Betrieb nicht blockieren und keinen
   Neustart ausloesen; Fehler werden sichtbar angezeigt.

Speicherlayout auf Webgo (Variante Datei): ein Unterordner pro ESP ausserhalb des Web-Roots
oder per `.htaccess` gesperrt, Name = zufaellige Geraete-ID. Inhalt: letztes Paket,
abgeschlossene Perioden, oeffentlicher Geraeteschluessel, Token-Hashes, Metadaten (letzter Push,
Client-Anzahl, Sperrzeit). Zugriff nur ueber PHP; Schreiben mit `flock` und atomar (Temp-Datei,
umbenennen). 6-Wochen-Bereinigung loescht den Ordner. Alternative SQLite ist noch offen.

Noch zu messen: TLS-Handshake-Dauer C3 zu Webgo, Heap/Stack des ausgehenden Clients (kurz und
mit Keep-Alive), KeepAliveTimeout und Session-Ticket-Unterstuetzung bei Webgo, Flash-Zuwachs
durch die Kryptobibliothek.

## 9. Produktions-Gate und Tests

Secure Boot und Flash Encryption nicht automatisch aktivieren. C3-Chiprevision,
reservierte Slots, Schutzbits, signierte OTA, Bootloader-/Partitionlayout und
Recovery muessen zusammenpassen. Die 4.4.8-Dokumentation nennt Secure Boot V2
fuer C3 ECO3 aufwaerts; tatsaechliche Revision vor Planung/Brennen feststellen.
Das ESP32-S3-DevKitC-1-N16R8 benoetigt eine eigene chipbezogene Pruefung.

Die Brennreihenfolge gehoert zur Freigabe: benoetigte Secret-Keybloecke und
deren Leseschutz vor Schutzbits provisionieren, die spaetere Aenderungen an
Read-Protection verhindern koennen. Secure-Boot-Digests haben einen anderen
Protection-Vertrag als geheime HMAC-/Flash-Keys. Keine pauschale identische
Read-Protection fuer alle Slots anwenden.

Pflichttests: HMAC nach Reboot/NVS-Verlust/OTA; falscher Purpose/Slot;
physischer Provisionierungsabbruch; Pairing-MITM/Fingerprint/Replay/Timeout;
mehrere Apps und Entzug; AEAD/Nonce nach Stromausfall und vollem Erase;
Recovery ohne alten ESP/ohne alte App; signierte Dual-OTA; keine Secrets in
Logs/Coredumps/Git/Backend; TLS-/Pairing-Spitzenverbrauch auf C3.

## 9a. Implementierungsstand (IDF-6/S3-Pfad, Entwicklungsschlüssel)

Nicht produktiv; Root ist der Dev-Provider (NVS), kein eFuse-Root.

- **Secret-Verschlüsselung:** WLAN-, MQTT-, HA-Token und private TLS-/Fallback-/ACME-Schlüssel
  liegen als AES-256-GCM-Blob (`<key>.e`: Version | 96-Bit-Zufallsnonce | Ciphertext | Tag, AAD
  = Version + `namespace/key`) mit `K_store = HMAC_root("smleasy/local-storage/v1")`. Klartext
  wird beim Schreiben gelöscht; Lesen fällt auf Klartext zurück (Migration beim nächsten Speichern).
- **Web-Passwort:** nur HMAC-Prüfwert (`pv`), nicht entschlüsselbar. Default-/Klartext-Passwort wird
  beim ersten erfolgreichen Login migriert.
- **App-Token / kein QR (2026-10-19):** Das authentifizierte lokale Webfrontend
  oeffnet maximal 300 s einen TLS/TCP-Dienst auf Port 7384 und mDNS
  `_smleasy-pair._tcp`. Ein Task (10 KiB Stack, noch nicht laufzeitgemessen)
  bearbeitet genau eine Verbindung, mit maximal 10 Verbindungsversuchen je Fenster,
  5 s Handshake-/Nachrichtenbudget und maximal 2048 Byte je JSON-Nachricht.
  Timeout, Abbruch und Erfolg schliessen Dienst/Advertisement; Fehler sind im
  Webstatus und ohne Secrets im Log sichtbar. Der bestehende Netzfilter gilt.
- **Freigabe:** Jede TLS-Verbindung erhaelt eine zufaellige 128-Bit-Anfragekennung.
  Die App erzeugt einen 256-Bit-Nonce. Beide Seiten bilden unabhaengig die
  sechsstellige Vergleichsnummer aus SHA-256 ueber
  `smleasy-pair-v1:<SHA256-Zertifikat-DER-hex>:<Nonce-hex>:<Anfrage-ID-hex>`,
  jeweils lowercase Hex, erste 4 Hashbytes big-endian modulo 1000000,
  mit fuehrenden Nullen. Besitzer muss die Nummern vergleichen; Name/mDNS allein
  sind kein Identitaetsnachweis. Erst `POST /api/pairing/decide` mit Basic-Auth,
  JSON-Content-Type, aktueller Anfrage-ID und `approve:true` autorisiert diese
  Verbindung. Ablehnen/Trennen bindet keine spaetere Verbindung.
- **Nachrichtenvertrag v1:** 2 Byte unsigned Big-Endian-Laenge (1..2048),
  gefolgt von UTF-8-JSON, innerhalb TLS. App sendet
  `{"op":"pair","name":"...","nonce":"<64 hex>"}`; ESP antwortet mit
  `status:pending, request_id`. App fragt auf derselben TLS-Verbindung mit
  `op:poll, request_id`; Antworten sind `waiting`, `rejected`, `error` oder
  `approved`. Nur `approved` enthaelt `token` (256 Bit), `fingerprint`
  (SHA-256 des DER-Blattzertifikats), `device_id`, optionalen `fqdn`,
  `https_port:443`, `ipv4` und IPv6-Adressliste ohne fremden Interface-Scope.
  Uebertragung/Logs enthalten keine Messdaten, WLAN-/HA-Passwoerter oder Root-Keys.
  Verifikation wie bisher nur als HMAC im NVS (max. 4 Apps). Alte QR-Claims
  antworten absichtlich mit HTTP 410; bereits ausgegebene Tokens bleiben gueltig.
- **Geraeteverwaltung:** `/pair` listet Namen, Slot, optionalen Erstellzeitpunkt
  und Belegung. Umbenennen (max. 40 UTF-8-Bytes) und Widerruf pro App benoetigen
  Web-Authentifizierung und JSON-Content-Type (auch fuer Start/Abbruch).
  Zeitmetadaten `t0`..`t3` ergaenzen `app_tok`; alte
  Eintraege ohne Datum bleiben lesbar. Keine Partition-/Schluessel-Migration.
  Tokenzugriffe werden bei jeder Anfrage geprueft, nicht nur beim Verbindungsaufbau.
- **Netzfilter:** Port 80 filtert wie bisher beim Verbindungsaufbau. Port 443 prüft pro Request:
  Peers außerhalb der erlaubten Netze erreichen nur lesende Token-Routen (`/api/status`, `/api/stats`,
  `/api/history`, `/api/meter`, `/api/ext/values`, `/house.jpg`) mit gültigem `Authorization: Bearer`.
  Web-UI, Konfiguration, OTA und Pairing bleiben auf erlaubte Netze beschränkt.
- **Folgen/Rückweg:** Wechsel Dev→eFuse-Root macht Blobs und Token unlesbar (Passwörter neu eingeben,
  Apps neu koppeln). Rückweg auf IDF-4.4-Firmware verliert verschlüsselte Passwörter und Token.
- **Android:** speichert Token, Zertifikatspin und Kontaktinformationen gemeinsam.
  Hostname/AAAA, direkte IPv6 (mit erhaltenem lokalem Interface-Scope), dann IPv4.
  Mit Token/Pin kein HTTP-Fallback und keine HTTP-Redirects. Pinning authentifiziert
  auch selbstsignierte Zertifikate ohne DNS; der Vergleich im vertrauenswuerdigen
  Webfrontend ist dafuer zwingend. Ein Zertifikatswechsel, auch ACME-Erneuerung,
  erfordert aktuell erneutes lokales Koppeln. Sichere automatische Pinrotation ist offen.
- **Offen/ungetestet:** kein ESP-/Android-Hardwaretest und kein echter externer
  IPv6-Bearer-Test; keine Produktionsfreigabe oder HPKE/eFuse-Implementierung.
  Webfrontend auf Klartext-HTTP ist nur im vertrauenswuerdigen LAN geeignet,
  nicht als Schutz gegen einen Angreifer auf diesem Verwaltungsweg.
  Kein Rate-Limit fuer laufende Bearer-Fehlversuche. Reale 300-s-Abschaltung,
  Verbindungsabbruch/Slots voll/NVS-Fehler, Widerruf, mDNS/Scope auf Android,
  IPv6-Internet und TLS-Last auf C3/S3 bleiben Hardware-Abnahmetests.
  Build-/Hosttest- und Ressourcen-Nachweis:
  [Fortschritt 2026-10-19](ZIEL-DESIGN.md#2026-10-19-lokales-pairing-app-verwaltung-ipv6-first-und-kpi-leiste).

## 10. Quellen

- [HMAC ESP32-C3 / ESP-IDF 4.4.8](https://docs.espressif.com/projects/esp-idf/en/v4.4.8/esp32c3/api-reference/peripherals/hmac.html)
- [Random Number Generation](https://docs.espressif.com/projects/esp-idf/en/v4.4.8/esp32c3/api-reference/system/random.html)
- [eFuse API](https://docs.espressif.com/projects/esp-idf/en/v4.4.8/esp32c3/api-reference/system/efuse.html)
- [Secure Boot V2](https://docs.espressif.com/projects/esp-idf/en/v4.4.8/esp32c3/security/secure-boot-v2.html)
- [Flash Encryption](https://docs.espressif.com/projects/esp-idf/en/v4.4.8/esp32c3/security/flash-encryption.html)
- [HPKE / RFC 9180](https://www.rfc-editor.org/rfc/rfc9180)
