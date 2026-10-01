#pragma once
// Embedded web assets — included once by web_server.cpp.
// Use raw string literals so HTML/CSS/JS do not need escaping.

namespace app {

// Compact vector logo (SMLEasy) — used inline in the header and as favicon.
// A hand-drawn SVG keeps this well under 1KB, far smaller than any embeddable
// raster export of the product artwork, while staying crisp at any size.
static const char kLogoSvg[] = R"rawsvg(<svg viewBox="0 0 100 100" xmlns="http://www.w3.org/2000/svg" role="img" aria-label="SMLEasy">
<defs><linearGradient id="smlG" x1="0%" y1="0%" x2="100%" y2="100%">
<stop offset="0%" stop-color="#22d3ee"/><stop offset="100%" stop-color="#2563eb"/>
</linearGradient></defs>
<circle cx="50" cy="50" r="42" fill="none" stroke="#0b1220" stroke-width="9"/>
<path d="M50 8 A42 42 0 0 1 90.5 40" fill="none" stroke="url(#smlG)" stroke-width="9" stroke-linecap="round"/>
<path d="M8 55 A42 42 0 0 0 40 91.5" fill="none" stroke="url(#smlG)" stroke-width="9" stroke-linecap="round"/>
<path d="M12 40 A42 42 0 0 1 30 15" fill="none" stroke="url(#smlG)" stroke-width="9" stroke-linecap="round" opacity=".85"/>
<rect x="34" y="55" width="7" height="16" rx="1.5" fill="url(#smlG)"/>
<rect x="44" y="48" width="7" height="23" rx="1.5" fill="url(#smlG)"/>
<rect x="54" y="40" width="7" height="31" rx="1.5" fill="url(#smlG)"/>
<path d="M72 30 L58 55 H68 L60 78 L80 50 H69 Z" fill="url(#smlG)"/>
</svg>
)rawsvg";

static const char kStyleCss[] = R"rawcss(
*{box-sizing:border-box;margin:0;padding:0}
*[hidden]{display:none!important}
.register-decimal{color:#c73434}
.grid>.hero-card{grid-column:1/span 5;grid-row:1}.grid>.meter-card{grid-column:1/span 5;grid-row:2}.grid>.connection-card{grid-column:10/span 3;grid-row:1}@media(max-width:1100px){.grid>.hero-card,.grid>.meter-card{grid-column:1/-1;grid-row:auto}.grid>.connection-card{grid-column:span 6;grid-row:auto}}@media(max-width:760px){.grid>.hero-card,.grid>.meter-card,.grid>.connection-card{grid-column:1/-1;grid-row:auto}}
.hero-card{grid-column:1/span 5;grid-row:1}.meter-card{grid-column:1/span 5;grid-row:2}.connection-card{grid-column:10/span 3;grid-row:1}.register-card{padding:18px 20px}.register-heading{display:flex;align-items:center;justify-content:space-between;gap:12px}.register-heading h2{margin:0}.register-live{display:inline-flex;align-items:center;gap:7px;color:#286348;background:#e8f4ed;border-radius:20px;padding:6px 10px;font-size:.65rem;font-weight:700;white-space:nowrap}.register-live:before{content:"";width:7px;height:7px;border-radius:50%;background:#258459}.register-subtitle{margin-top:7px;color:var(--text-secondary);font-size:.76rem}.register-meter{margin-top:14px;padding:12px;border:1px solid #48504c;border-radius:10px;color:#e3e8e4;background:linear-gradient(145deg,#343a38,#151918)}.register-brand{display:flex;justify-content:space-between;align-items:center;margin:0 3px 10px;color:#dce2dd;font-size:.58rem;font-weight:700;letter-spacing:.09em}.register-led{width:9px;height:9px;border-radius:50%;background:#74bd78;box-shadow:0 0 0 4px rgba(116,189,120,.2)}.register-displays{display:grid;grid-template-columns:repeat(2,minmax(0,1fr));gap:9px}.register-window{min-width:0;padding:9px;border:1px solid #68716b;border-radius:6px;background:#111513}.register-label{min-height:26px;color:#aeb8b1;font-size:.58rem;font-weight:700;letter-spacing:.05em;line-height:1.3}.register-reading{display:block;margin-top:5px;padding:9px 3px;border:1px solid #a8a38f;border-radius:4px;color:#242821;background:linear-gradient(#f5f0dc,#d8d2bd);font-family:"Cascadia Code","JetBrains Mono",monospace;font-size:clamp(.9rem,1.3vw,1.15rem);font-weight:700;font-variant-numeric:tabular-nums;text-align:center;white-space:nowrap;overflow:hidden}.register-unit{text-align:right;margin:4px 1px 0;color:#d9dfda;font-size:.68rem;font-weight:600}.register-time{display:flex;justify-content:space-between;gap:8px;margin-top:10px;color:var(--text-secondary);font-size:.68rem}.register-time .val{color:var(--text-primary);text-align:right}@media(max-width:1100px){.hero-card,.meter-card{grid-column:1/-1;grid-row:auto}.connection-card{grid-column:span 6;grid-row:auto}}@media(max-width:760px){.hero-card,.meter-card,.connection-card{grid-column:1/-1;grid-row:auto}.register-card{padding:16px}.register-meter{padding:10px}.register-window{padding:7px}.register-label{font-size:.54rem}.register-reading{font-size:clamp(.78rem,3.5vw,1rem)}}
:root{--bg-page:#f4f8fc;--bg-sidebar:rgba(255,255,255,.9);--bg-card:rgba(255,255,255,.9);--bg-card-hover:#fff;--text-primary:#132238;--text-secondary:#60758f;--text-muted:#8ca0b8;--primary:#168cff;--primary-light:#31c7ff;--primary-dark:#135bd8;--success:#24bf6b;--warning:#f4a928;--danger:#e85858;--border:#dce7f2;--border-strong:#c7d7e8;--shadow-card:0 10px 30px rgba(40,76,120,.06);--sidebar-width:240px;--header-height:72px;--radius-sm:8px;--radius-md:12px;--radius-lg:16px}
html{scroll-behavior:smooth}body{font-family:"Segoe UI",system-ui,sans-serif;color:var(--text-primary);background:radial-gradient(circle at 70% -10%,rgba(49,199,255,.12),transparent 30%),var(--bg-page);min-height:100vh}button,input,select{font:inherit}button{cursor:pointer}
.app{min-height:100vh;display:grid;grid-template-columns:var(--sidebar-width) 1fr}.sidebar{position:sticky;top:0;height:100vh;padding:24px 16px;background:var(--bg-sidebar);border-right:1px solid var(--border);backdrop-filter:blur(18px);display:flex;flex-direction:column;z-index:2}.brand{display:flex;align-items:center;gap:10px;margin:0 8px 34px;color:var(--text-primary);font-weight:750;font-size:1.05rem}.brand svg{width:38px;height:38px}.brand small{display:block;color:var(--text-muted);font-size:.66rem;font-weight:600;letter-spacing:.1em;text-transform:uppercase}.nav-title{padding:0 12px 8px;color:var(--text-muted);font-size:.67rem;font-weight:700;letter-spacing:.1em;text-transform:uppercase}.nav-item{min-height:44px;display:flex;align-items:center;gap:12px;padding:0 14px;border-radius:var(--radius-md);color:var(--text-secondary);text-decoration:none;font-size:.9rem;transition:background .16s,color .16s}.nav-item:hover{background:rgba(22,140,255,.06);color:var(--primary)}.nav-item.active{color:var(--primary);background:linear-gradient(90deg,rgba(22,140,255,.12),rgba(49,199,255,.04));font-weight:650}.nav-item svg{width:18px;height:18px;stroke:currentColor;fill:none;stroke-width:1.8}.sidebar-footer{margin-top:auto;padding:14px 12px;border-top:1px solid var(--border);color:var(--text-muted);font-size:.76rem}.status{display:inline-flex;align-items:center;gap:7px;color:var(--success);font-weight:650}.status-dot{width:8px;height:8px;border-radius:50%;background:currentColor;box-shadow:0 0 0 4px rgba(36,191,107,.1)}
.workspace{min-width:0}.topbar{height:var(--header-height);display:flex;align-items:center;justify-content:space-between;padding:0 32px;border-bottom:1px solid var(--border);background:rgba(255,255,255,.55);backdrop-filter:blur(14px)}.topbar h1{font-size:1.28rem;font-weight:700}.topbar-meta{display:flex;align-items:center;gap:18px;color:var(--text-muted);font-size:.78rem}.main-content{max-width:1500px;margin:0 auto;padding:28px 32px 40px}.eyebrow,.label{font-size:.7rem;font-weight:700;letter-spacing:.09em;text-transform:uppercase;color:var(--text-secondary)}.page-intro{display:flex;align-items:flex-end;justify-content:space-between;margin-bottom:22px}.page-intro h2{font-size:1.65rem;margin-top:5px}.page-intro p{color:var(--text-secondary);font-size:.88rem;margin-top:5px}.grid{display:grid;grid-template-columns:repeat(12,minmax(0,1fr));gap:16px}.card{position:relative;min-width:0;padding:20px;background:var(--bg-card);border:1px solid var(--border);border-radius:var(--radius-lg);box-shadow:var(--shadow-card);backdrop-filter:blur(14px);overflow:hidden}.card:before{content:"";position:absolute;inset:0 auto auto 0;width:80px;height:2px;background:linear-gradient(90deg,var(--primary-light),transparent)}.card h2{font-size:.78rem;font-weight:700;letter-spacing:.08em;text-transform:uppercase;color:var(--text-secondary);margin:0 0 16px}.card-inner{padding:0}.hero-card{grid-column:span 5;min-height:210px}.meter-card{grid-column:span 4}..connection-card{grid-column:span 3}.kpi-grid{grid-column:1/-1;display:grid;grid-template-columns:repeat(4,minmax(0,1fr));gap:16px}.kpi-card{padding:18px 20px}.metric-value,.kpi-value,.mono,.data-table td.value{font-family:"Cascadia Code","JetBrains Mono",monospace;font-variant-numeric:tabular-nums}.metric-value{font-size:clamp(2.5rem,4vw,4rem);font-weight:650;line-height:1;color:var(--text-primary);margin:20px 0 7px}.metric-unit{font-size:1rem;color:var(--text-secondary);margin-left:6px}.metric-sub{color:var(--text-secondary);font-size:.8rem}.direction-row{display:flex;gap:28px;margin-top:22px}.direction-row strong{display:block;font-family:"Cascadia Code","JetBrains Mono",monospace;font-size:1rem;margin-top:5px}.import{color:var(--primary)}.export{color:var(--success)}.kpi-value{font-size:1.55rem;font-weight:650;margin-top:10px}.kpi-change{font-size:.77rem;color:var(--text-secondary);margin-top:8px}.kpi-change.positive{color:var(--success)}.chart-card{grid-column:span 8;min-height:300px}.split-card{grid-column:span 4}.table-card{grid-column:span 9}.system-card{grid-column:span 3}.chart-wrap{height:220px;margin-top:8px}.chart-wrap svg{width:100%;height:100%;overflow:visible}.chart-grid{stroke:var(--border);stroke-width:1}.chart-line{fill:none;stroke:var(--primary);stroke-width:3;stroke-linecap:round;stroke-linejoin:round}.chart-area{fill:rgba(49,199,255,.13)}.chart-empty{fill:var(--text-muted);font-size:12px}.donut{width:150px;height:150px;margin:12px auto 18px;border-radius:50%;background:conic-gradient(var(--primary) 0 68%,#8bdcf4 68% 85%,var(--border) 85%);display:grid;place-items:center}.donut:after{content:"";width:92px;height:92px;border-radius:50%;background:#fff}.legend{display:grid;gap:10px;color:var(--text-secondary);font-size:.8rem}.legend span{display:flex;justify-content:space-between}.legend i{width:9px;height:9px;border-radius:50%;display:inline-block;margin-right:8px}.data-table{width:100%;border-collapse:collapse}.data-table th{padding:0 8px 10px;text-align:left;font-size:.68rem;font-weight:700;color:var(--text-muted);letter-spacing:.06em;text-transform:uppercase;border-bottom:1px solid var(--border)}.data-table td{padding:10px 8px;border-bottom:1px solid var(--border);font-size:.83rem}.data-table td.value{text-align:right;color:var(--text-primary)}.system-list{display:grid;gap:0}.system-list .kv{padding:10px 0}.kv{display:flex;justify-content:space-between;align-items:baseline;gap:12px;border-bottom:1px solid var(--border);font-size:.8rem}.kv .lbl{color:var(--text-secondary)}.kv .val{color:var(--text-primary);font-family:"Cascadia Code","JetBrains Mono",monospace;font-size:.8rem;font-weight:650;text-align:right}.btn{min-height:40px;padding:0 16px;border:1px solid transparent;border-radius:var(--radius-sm);font-size:.82rem;font-weight:650;transition:filter .15s,transform .15s}.btn:hover{filter:brightness(.97);transform:translateY(-1px)}.btn-green,.btn-blue{color:#fff;background:linear-gradient(135deg,var(--primary-dark),var(--primary-light));box-shadow:0 4px 12px rgba(22,140,255,.18)}.btn-red{color:#fff;background:var(--danger)}.btn-gray{color:var(--text-secondary);background:#fff;border-color:var(--border-strong)}.btn-row{display:flex;gap:8px;flex-wrap:wrap;margin-bottom:14px}.control-card{grid-column:1/-1}.control-card .card-inner{display:flex;align-items:center;justify-content:space-between;gap:16px}.control-meta{display:flex;gap:20px;flex-wrap:wrap}.control-meta .kv{border:0;display:grid;gap:3px}.control-meta .val{text-align:left}.card.peach,.card.orange{grid-column:span 6}.card.orange[style],.card.peach[style]{grid-column:1/-1}.card h2.orange,.card h2.tan{background:none;color:var(--text-secondary);padding:0}.card h2.orange{color:var(--primary-dark)}label{display:block;font-size:.72rem;font-weight:650;color:var(--text-secondary);margin:12px 0 5px;text-transform:uppercase;letter-spacing:.06em}input[type=text],input[type=password],input[type=number],input[type=url],select{width:100%;min-height:42px;padding:0 12px;background:#fff;border:1px solid var(--border-strong);border-radius:var(--radius-sm);color:var(--text-primary);font-size:.9rem}input:focus,select:focus{outline:none;border-color:var(--primary);box-shadow:0 0 0 3px rgba(22,140,255,.1)}.save-row{margin-top:14px}.lcars-bar{height:1px;background:var(--border);margin:18px 0}.progress{height:7px;background:var(--border);border-radius:8px;overflow:hidden;margin-top:6px}.progress-bar{height:100%;background:linear-gradient(90deg,var(--primary),var(--primary-light));width:0;transition:width .3s}#ota-result{margin-top:8px;font-size:.82rem;color:var(--primary-dark)}#log{background:#f8fbfe;border-radius:var(--radius-sm);padding:10px;height:300px;overflow-y:auto;font-size:.78rem;font-family:monospace;border:1px solid var(--border)}.log-I{color:var(--primary-dark)}.log-W{color:var(--warning)}.log-E{color:var(--danger)}.log-D{color:var(--text-muted)}footer{color:var(--text-muted)!important;border-top:1px solid var(--border)!important}footer a,footer span{color:var(--text-secondary)!important}
@media(max-width:1100px){.hero-card,.meter-card,.connection-card{grid-column:span 6}.chart-card,.table-card{grid-column:span 8}.split-card,.system-card{grid-column:span 4}.sidebar{width:72px;padding:24px 10px}.app{grid-template-columns:72px 1fr}.brand{justify-content:center;margin:0 0 34px}.brand>div,.nav-title,.nav-item span,.sidebar-footer div:not(.status){display:none}.nav-item{justify-content:center;padding:0}.sidebar-footer{padding:14px 0;text-align:center}.main-content{padding:24px}}
@media(max-width:760px){.app{display:block}.sidebar{display:none}.topbar{padding:0 18px}.topbar-meta{gap:8px}.main-content{padding:20px 14px 32px}.page-intro{display:block}.grid{grid-template-columns:1fr}.hero-card,.meter-card,.connection-card,.chart-card,.split-card,.table-card,.system-card,.card.peach,.card.orange{grid-column:1/-1}.kpi-grid{grid-template-columns:repeat(2,minmax(0,1fr));gap:12px}.control-card .card-inner{display:block}.control-meta{margin-top:14px}.data-table{min-width:560px}.table-card{overflow-x:auto}}
@media(max-width:430px){.kpi-grid{grid-template-columns:1fr}.metric-value{font-size:2.7rem}}.connection-card{grid-column:span 3}@media(max-width:1100px){.connection-card{grid-column:span 6}}@media(max-width:760px){.connection-card{grid-column:1/-1}}header{display:flex;align-items:center;min-height:var(--header-height);padding:0 24px;border-bottom:1px solid var(--border);background:rgba(255,255,255,.72)}.lcars-header-elbow{display:flex;align-items:center;justify-content:center;width:52px;height:52px;flex:0 0 52px}.lcars-header-elbow.sml-logo svg{width:38px;height:38px;display:block}.lcars-header-bar{display:flex;align-items:center;gap:16px;flex:1;margin-left:14px}.lcars-header-bar h1{font-size:1.15rem;color:var(--text-primary);font-weight:700}.lcars-header-bar .btn{margin-left:auto}.overview-info{display:flex;justify-content:center;align-items:center;gap:8px;padding:20px 8px;color:var(--text-muted);font-size:.76rem;text-align:center}.overview-info strong{color:var(--text-secondary);font-family:"Cascadia Code","JetBrains Mono",monospace}
.register-meter{margin-top:14px;padding:12px;border:1px solid var(--border);border-radius:10px;color:var(--text-primary);background:rgba(22,140,255,.045)}.register-brand{margin:0 3px 10px;color:var(--text-secondary);font-size:.62rem;letter-spacing:.08em}.register-window{border-color:var(--border);background:rgba(255,255,255,.82)}.register-label{color:var(--text-secondary)}.register-reading{border-color:var(--border-strong);color:var(--text-primary);background:#fff}.register-unit{color:var(--text-secondary)}
.kpi-grid{grid-template-columns:repeat(3,minmax(0,1fr))}@media(max-width:760px){.kpi-grid{grid-template-columns:repeat(2,minmax(0,1fr))}}@media(max-width:430px){.kpi-grid{grid-template-columns:1fr}}
)rawcss";

static const char kAppJs[] = R"rawjs(
'use strict';
let logSeq = 0;
let statusTimer, logTimer;
let logPaused = false;

// ── Reconnect state ───────────────────────────────────────────────────────────
let _offline = false;
let _reconnectTimer = null;
let _reconnectDeadline = 0;
let _reconnectCountdown = null;

const UI_LANGUAGES = ['en', 'de', 'nl', 'fr', 'pl'];
const UI_TEXT = {
  'SMLEasy Dashboard': ['SMLEasy Dashboard', 'SMLEasy Dashboard', 'SMLEasy Dashboard', 'Tableau de bord SMLEasy', 'Panel SMLEasy'],
  'SMLEasy — Konfiguration': ['SMLEasy — Settings', 'SMLEasy — Konfiguration', 'SMLEasy — Instellingen', 'SMLEasy — Configuration', 'SMLEasy — Konfiguracja'],
  'Übersicht': ['Overview', 'Übersicht', 'Overzicht', 'Vue d’ensemble', 'Przegląd'],
  'Dashboard': ['Dashboard', 'Dashboard', 'Dashboard', 'Tableau de bord', 'Panel'],
  '← Dashboard': ['← Dashboard', '← Dashboard', '← Dashboard', '← Tableau de bord', '← Panel'],
  'Zählerwerte': ['Meter readings', 'Zählerwerte', 'Meterstanden', 'Relevés du compteur', 'Wskazania licznika'],
  'Verlauf': ['History', 'Verlauf', 'Historie', 'Historique', 'Historia'],
  'Konfiguration': ['Settings', 'Konfiguration', 'Instellingen', 'Configuration', 'Konfiguracja'],
  'Verbindung & Einstellungen': ['Connection & settings', 'Verbindung & Einstellungen', 'Verbinding & instellingen', 'Connexion et paramètres', 'Połączenie i ustawienia'],
  'Smart energy': ['Smart energy', 'Smart energy', 'Slimme energie', 'Énergie intelligente', 'Inteligentna energia'],
  'Smart meter / Übersicht': ['Smart meter / overview', 'Smart meter / Übersicht', 'Slimme meter / overzicht', 'Compteur intelligent / vue d’ensemble', 'Licznik energii / przegląd'],
  'Live monitoring': ['Live monitoring', 'Live-Überwachung', 'Livebewaking', 'Surveillance en direct', 'Monitoring na żywo'],
  'Verbrauch auf einen Blick': ['Energy at a glance', 'Verbrauch auf einen Blick', 'Energie in één oogopslag', 'L’énergie en un coup d’œil', 'Energia w skrócie'],
  'Aktuelle Messwerte und Energiefluss deines Zählers.': ['Current readings and energy flow from your meter.', 'Aktuelle Messwerte und Energiefluss deines Zählers.', 'Actuele meetwaarden en energiestroom van je meter.', 'Mesures actuelles et flux d’énergie de votre compteur.', 'Aktualne odczyty i przepływ energii z licznika.'],
  'Jetzt auslesen': ['Read now', 'Jetzt auslesen', 'Nu uitlezen', 'Lire maintenant', 'Odczytaj teraz'],
  'Aktuelle Leistung': ['Current power', 'Aktuelle Leistung', 'Huidig vermogen', 'Puissance actuelle', 'Aktualna moc'],
  'Bezug': ['Import', 'Bezug', 'Afname', 'Soutirage', 'Pobór'],
  'Einspeisung': ['Feed-in', 'Einspeisung', 'Teruglevering', 'Injection', 'Oddawanie'],
  'Zählerstände': ['Meter registers', 'Zählerstände', 'Meterstanden', 'Index du compteur', 'Wskazania licznika'],
  'Zähler Live': ['Meter live', 'Zähler Live', 'Meter live', 'Compteur en direct', 'Licznik na żywo'],
  'Bezug und Einspeisung als Zählwerke': ['Import and feed-in registers', 'Bezug und Einspeisung als Zählwerke', 'Afname- en terugleverregisters', 'Index de soutirage et d’injection', 'Rejestry poboru i oddawania'],
  'ZÄHLWERKE · OBIS 1.8.0 / 2.8.0': ['REGISTERS · OBIS 1.8.0 / 2.8.0', 'ZÄHLWERKE · OBIS 1.8.0 / 2.8.0', 'REGISTERS · OBIS 1.8.0 / 2.8.0', 'REGISTRES · OBIS 1.8.0 / 2.8.0', 'REJESTRY · OBIS 1.8.0 / 2.8.0'],
  '1.8.0 / BEZUG GESAMT': ['1.8.0 / TOTAL IMPORT', '1.8.0 / BEZUG GESAMT', '1.8.0 / TOTALE AFNAME', '1.8.0 / TOTAL SOUTIRÉ', '1.8.0 / POBÓR ŁĄCZNIE'],
  '2.8.0 / EINSPEISUNG GESAMT': ['2.8.0 / TOTAL FEED-IN', '2.8.0 / EINSPEISUNG GESAMT', '2.8.0 / TOTALE TERUGLEVERING', '2.8.0 / TOTAL INJECTÉ', '2.8.0 / ODDANIE ŁĄCZNIE'],
  'Zählerzeit': ['Meter time', 'Zählerzeit', 'Metertijd', 'Heure du compteur', 'Czas licznika'],
  'Verbindung': ['Connection', 'Verbindung', 'Verbinding', 'Connexion', 'Połączenie'],
  'IPv4': ['IPv4', 'IPv4', 'IPv4', 'IPv4', 'IPv4'],
  'IPv6': ['IPv6', 'IPv6', 'IPv6', 'IPv6', 'IPv6'],
  'Warte auf IPv6 vom Router': ['Waiting for router IPv6', 'Warte auf IPv6 vom Router', 'Wachten op IPv6 van de router', 'En attente de l’IPv6 du routeur', 'Oczekiwanie na IPv6 z routera'],
  'WLAN': ['Wi-Fi', 'WLAN', 'Wifi', 'Wi-Fi', 'Wi-Fi'],
  'Uptime': ['Uptime', 'Laufzeit', 'Uptime', 'Disponibilité', 'Czas pracy'],
  'Verbrauch heute': ['Consumption today', 'Verbrauch heute', 'Verbruik vandaag', 'Consommation du jour', 'Zużycie dzisiaj'],
  'Importierte Energie': ['Imported energy', 'Importierte Energie', 'Afgenomen energie', 'Énergie soutirée', 'Energia pobrana'],
  'Einspeisung heute': ['Feed-in today', 'Einspeisung heute', 'Teruglevering vandaag', 'Injection du jour', 'Oddano dzisiaj'],
  'Exportierte Energie': ['Exported energy', 'Exportierte Energie', 'Teruggeleverde energie', 'Énergie injectée', 'Energia oddana'],
  'Nettokosten heute': ['Net cost today', 'Nettokosten heute', 'Netto kosten vandaag', 'Coût net du jour', 'Koszt netto dzisiaj'],
  'Bezug minus Einspeisevergütung': ['Import cost minus feed-in credit', 'Bezug minus Einspeisevergütung', 'Afnamekosten min terugleververgoeding', 'Coût soutiré moins crédit d’injection', 'Koszt poboru minus rozliczenie oddania'],
  'Abfrageintervall': ['Polling interval', 'Abfrageintervall', 'Meetinterval', 'Intervalle de lecture', 'Interwał odczytu'],
  'Automatische Aktualisierung': ['Automatic refresh', 'Automatische Aktualisierung', 'Automatisch vernieuwen', 'Actualisation automatique', 'Automatyczne odświeżanie'],
  'Leistungsverlauf': ['Power history', 'Leistungsverlauf', 'Vermogensgeschiedenis', 'Historique de puissance', 'Historia mocy'],
  'Letzte Messpunkte': ['Recent samples', 'Letzte Messpunkte', 'Recente metingen', 'Dernières mesures', 'Ostatnie pomiary'],
  'Verbrauchsaufteilung': ['Consumption breakdown', 'Verbrauchsaufteilung', 'Verbruiksverdeling', 'Répartition de la consommation', 'Podział zużycia'],
  'Bezug heute': ['Import today', 'Bezug heute', 'Afname vandaag', 'Soutirage aujourd’hui', 'Pobór dzisiaj'],
  'Monat gesamt': ['Month total', 'Monat gesamt', 'Maandtotaal', 'Total du mois', 'Suma miesiąca'],
  'Aktuelle Zählerwerte': ['Current meter readings', 'Aktuelle Zählerwerte', 'Actuele meterstanden', 'Relevés actuels du compteur', 'Aktualne wskazania licznika'],
  'OBIS': ['OBIS', 'OBIS', 'OBIS', 'OBIS', 'OBIS'],
  'Beschreibung': ['Description', 'Beschreibung', 'Omschrijving', 'Description', 'Opis'],
  'Wert': ['Value', 'Wert', 'Waarde', 'Valeur', 'Wartość'],
  'Einheit': ['Unit', 'Einheit', 'Eenheid', 'Unité', 'Jednostka'],
  'Bezug gesamt': ['Total import', 'Bezug gesamt', 'Totale afname', 'Total soutiré', 'Pobór łącznie'],
  'Einspeisung gesamt': ['Total feed-in', 'Einspeisung gesamt', 'Totale teruglevering', 'Total injecté', 'Oddanie łącznie'],
  'Strom L1': ['Current L1', 'Strom L1', 'Stroom L1', 'Courant L1', 'Prąd L1'],
  'Strom L2': ['Current L2', 'Strom L2', 'Stroom L2', 'Courant L2', 'Prąd L2'],
  'Strom L3': ['Current L3', 'Strom L3', 'Stroom L3', 'Courant L3', 'Prąd L3'],
  'Systemstatus': ['System status', 'Systemstatus', 'Systeemstatus', 'État du système', 'Stan systemu'],
  'Hersteller': ['Manufacturer', 'Hersteller', 'Fabrikant', 'Fabricant', 'Producent'],
  'Modell': ['Model', 'Modell', 'Model', 'Modèle', 'Model'],
  'Firmware': ['Firmware', 'Firmware', 'Firmware', 'Micrologiciel', 'Oprogramowanie'],
  'Seriennr.': ['Serial number', 'Seriennr.', 'Serienummer', 'Numéro de série', 'Numer seryjny'],
  'Login': ['Login', 'Login', 'Aanmelding', 'Connexion', 'Logowanie'],
  'Alarme': ['Alarms', 'Alarme', 'Alarmen', 'Alarmes', 'Alarmy'],
  'Keine': ['None', 'Keine', 'Geen', 'Aucune', 'Brak'],
  'Running': ['Running', 'Läuft', 'Actief', 'En cours', 'Działa'],
  'Done': ['Done', 'Fertig', 'Gereed', 'Terminé', 'Gotowe'],
  'Error': ['Error', 'Fehler', 'Fout', 'Erreur', 'Błąd'],
  'Idle': ['Idle', 'Bereit', 'Inactief', 'Inactif', 'Bezczynny'],
  'Dauerhaft lesen': ['Continuous reading', 'Dauerhaft lesen', 'Continu uitlezen', 'Lecture continue', 'Odczyt ciągły'],
  'Stoppen': ['Stop', 'Stoppen', 'Stoppen', 'Arrêter', 'Zatrzymaj'],
  'Zähler reset': ['Reset counters', 'Zähler reset', 'Tellers resetten', 'Réinitialiser les compteurs', 'Resetuj liczniki'],
  'Neustart': ['Restart', 'Neustart', 'Opnieuw starten', 'Redémarrer', 'Uruchom ponownie'],
  'Konfiguration': ['Settings', 'Konfiguration', 'Instellingen', 'Configuration', 'Konfiguracja'],
  'Copyright Michael Kreutzer 2026': ['Copyright Michael Kreutzer 2026', 'Copyright Michael Kreutzer 2026', 'Copyright Michael Kreutzer 2026', 'Copyright Michael Kreutzer 2026', 'Copyright Michael Kreutzer 2026'],
  'Lizenz: GNU GPLv3': ['License: GNU GPLv3', 'Lizenz: GNU GPLv3', 'Licentie: GNU GPLv3', 'Licence : GNU GPLv3', 'Licencja: GNU GPLv3'],
  'Sprache': ['Language', 'Sprache', 'Taal', 'Langue', 'Język'],
  'Sprache für die Oberfläche': ['Interface language', 'Sprache für die Oberfläche', 'Taal van de interface', 'Langue de l’interface', 'Język interfejsu'],
  'Zugriff auf das Zähler-Frontend': ['Meter interface access', 'Zugriff auf das Zähler-Frontend', 'Toegang tot de meterinterface', 'Accès à l’interface du compteur', 'Dostęp do interfejsu licznika'],
  'Zusätzliche erlaubte CIDR-Netze': ['Additional allowed CIDR networks', 'Zusätzliche erlaubte CIDR-Netze', 'Extra toegestane CIDR-netwerken', 'Réseaux CIDR autorisés supplémentaires', 'Dodatkowe dozwolone sieci CIDR'],
  'Ein CIDR-Netz pro Zeile. Private IPv4- und IPv6-Netze sind immer erlaubt.': ['One CIDR network per line. Private IPv4 and IPv6 networks are always allowed.', 'Ein CIDR-Netz pro Zeile. Private IPv4- und IPv6-Netze sind immer erlaubt.', 'Eén CIDR-netwerk per regel. Privé-IPv4- en IPv6-netwerken zijn altijd toegestaan.', 'Un réseau CIDR par ligne. Les réseaux IPv4 et IPv6 privés sont toujours autorisés.', 'Jedna sieć CIDR w wierszu. Prywatne sieci IPv4 i IPv6 są zawsze dozwolone.'],
  'Beispiele: 192.168.1.0/24 oder 2a00:6020:a105:c900::/64': ['Examples: 192.168.1.0/24 or 2a00:6020:a105:c900::/64', 'Beispiele: 192.168.1.0/24 oder 2a00:6020:a105:c900::/64', 'Voorbeelden: 192.168.1.0/24 of 2a00:6020:a105:c900::/64', 'Exemples : 192.168.1.0/24 ou 2a00:6020:a105:c900::/64', 'Przykłady: 192.168.1.0/24 lub 2a00:6020:a105:c900::/64'],
  'Netzwerke speichern': ['Save networks', 'Netzwerke speichern', 'Netwerken opslaan', 'Enregistrer les réseaux', 'Zapisz sieci'],
  'Netzwerkfilter gespeichert.': ['Network filter saved.', 'Netzwerkfilter gespeichert.', 'Netwerkfilter opgeslagen.', 'Filtre réseau enregistré.', 'Zapisano filtr sieci.'],
  'Netzwerkfilter konnte nicht gespeichert werden.': ['Could not save network filter.', 'Netzwerkfilter konnte nicht gespeichert werden.', 'Netwerkfilter kon niet worden opgeslagen.', 'Impossible d’enregistrer le filtre réseau.', 'Nie udało się zapisać filtra sieci.'],
  'Browser (automatisch)': ['Browser (automatic)', 'Browser (automatisch)', 'Browser (automatisch)', 'Navigateur (automatique)', 'Przeglądarka (automatycznie)'],
  'English': ['English', 'English', 'Engels', 'Anglais', 'Angielski'],
  'Deutsch': ['German', 'Deutsch', 'Duits', 'Allemand', 'Niemiecki'],
  'Nederlands': ['Dutch', 'Nederlands', 'Nederlands', 'Néerlandais', 'Niderlandzki'],
  'Français': ['French', 'Français', 'Frans', 'Français', 'Francuski'],
  'Polski': ['Polish', 'Polski', 'Pools', 'Polonais', 'Polski'],
  'Zähler-Konfiguration': ['Meter settings', 'Zähler-Konfiguration', 'Meterinstellingen', 'Configuration du compteur', 'Ustawienia licznika'],
  'Leseintervall (s)': ['Read interval (s)', 'Leseintervall (s)', 'Leesinterval (s)', 'Intervalle de lecture (s)', 'Interwał odczytu (s)'],
  'UART Raw-Debug (hex dump TX/RX)': ['UART raw debug (hex dump TX/RX)', 'UART Raw-Debug (hex dump TX/RX)', 'UART raw-debug (hex-dump TX/RX)', 'Débogage UART brut (hex TX/RX)', 'Surowy debug UART (hex TX/RX)'],
  'Login-Befehl vor Auslesung (optional)': ['Login command before reading (optional)', 'Login-Befehl vor Auslesung (optional)', 'Aanmeldopdracht vóór uitlezen (optioneel)', 'Commande de connexion avant lecture (facultatif)', 'Polecenie logowania przed odczytem (opcjonalne)'],
  'PIN (optional, nur beim Speichern gesendet)': ['PIN (optional, sent only when saving)', 'PIN (optional, nur beim Speichern gesendet)', 'PIN (optioneel, alleen verzonden bij opslaan)', 'PIN (facultatif, envoyé uniquement à l’enregistrement)', 'PIN (opcjonalny, wysyłany tylko przy zapisie)'],
  '(leer lassen = unverändert)': ['(leave blank to keep unchanged)', '(leer lassen = unverändert)', '(leeg laten = ongewijzigd)', '(laisser vide = inchangé)', '(pozostaw puste, aby nie zmieniać)'],
  'PIN-Status': ['PIN status', 'PIN-Status', 'PIN-status', 'État du PIN', 'Stan PIN-u'],
  'Gespeicherten PIN löschen': ['Delete saved PIN', 'Gespeicherten PIN löschen', 'Opgeslagen PIN verwijderen', 'Supprimer le PIN enregistré', 'Usuń zapisany PIN'],
  'Wartezeit nach Login-Befehl (ms)': ['Wait after login command (ms)', 'Wartezeit nach Login-Befehl (ms)', 'Wachttijd na aanmeldopdracht (ms)', 'Attente après commande de connexion (ms)', 'Czas oczekiwania po poleceniu logowania (ms)'],
  'OBIS-Mapping (Alias-Liste per Komma)': ['OBIS mapping (comma-separated aliases)', 'OBIS-Mapping (Alias-Liste per Komma)', 'OBIS-koppeling (aliassen gescheiden door komma’s)', 'Correspondance OBIS (alias séparés par des virgules)', 'Mapowanie OBIS (aliasy oddzielone przecinkami)'],
  'Default: MT631/MS2020': ['Default: MT631/MS2020', 'Default: MT631/MS2020', 'Standaard: MT631/MS2020', 'Par défaut : MT631/MS2020', 'Domyślnie: MT631/MS2020'],
  'Profil auswählen': ['Select profile', 'Profil auswählen', 'Profiel kiezen', 'Choisir un profil', 'Wybierz profil'],
  'Profil anwenden': ['Apply profile', 'Profil anwenden', 'Profiel toepassen', 'Appliquer le profil', 'Zastosuj profil'],
  'GitHub Profil-URL (JSON)': ['GitHub profile URL (JSON)', 'GitHub Profil-URL (JSON)', 'GitHub-profiel-URL (JSON)', 'URL du profil GitHub (JSON)', 'Adres URL profilu GitHub (JSON)'],
  'Von URL laden': ['Load from URL', 'Von URL laden', 'Laden vanaf URL', 'Charger depuis l’URL', 'Wczytaj z URL'],
  'Katalog-Status': ['Catalog status', 'Katalog-Status', 'Catalogusstatus', 'État du catalogue', 'Stan katalogu'],
  'Keine externen Profile geladen': ['No external profiles loaded', 'Keine externen Profile geladen', 'Geen externe profielen geladen', 'Aucun profil externe chargé', 'Nie wczytano profili zewnętrznych'],
  'Profil-Hinweis': ['Profile note', 'Profil-Hinweis', 'Profielnotitie', 'Remarque sur le profil', 'Informacja o profilu'],
  'PIN optional. Bei Bedarf {PIN} im Login-Befehl verwenden.': ['PIN is optional. Use {PIN} in the login command if needed.', 'PIN optional. Bei Bedarf {PIN} im Login-Befehl verwenden.', 'PIN is optioneel. Gebruik zo nodig {PIN} in de aanmeldopdracht.', 'PIN facultatif. Utilisez {PIN} dans la commande si nécessaire.', 'PIN jest opcjonalny. W razie potrzeby użyj {PIN} w poleceniu logowania.'],
  'Custom 1 Name': ['Custom 1 name', 'Custom 1 Name', 'Naam aangepast 1', 'Nom personnalisé 1', 'Nazwa własna 1'],
  'Custom 2 Name': ['Custom 2 name', 'Custom 2 Name', 'Naam aangepast 2', 'Nom personnalisé 2', 'Nazwa własna 2'],
  'Custom 1': ['Custom 1', 'Custom 1', 'Aangepast 1', 'Personnalisé 1', 'Własny 1'],
  'Custom 2': ['Custom 2', 'Custom 2', 'Aangepast 2', 'Personnalisé 2', 'Własny 2'],
  'Aktuelle Werte als Custom 1 speichern': ['Save current values as Custom 1', 'Aktuelle Werte als Custom 1 speichern', 'Huidige waarden opslaan als aangepast 1', 'Enregistrer les valeurs comme personnalisé 1', 'Zapisz bieżące wartości jako własne 1'],
  'Aktuelle Werte als Custom 2 speichern': ['Save current values as Custom 2', 'Aktuelle Werte als Custom 2 speichern', 'Huidige waarden opslaan als aangepast 2', 'Enregistrer les valeurs comme personnalisé 2', 'Zapisz bieżące wartości jako własne 2'],
  'Import Energie (Wh)': ['Import energy (Wh)', 'Import Energie (Wh)', 'Afgenomen energie (Wh)', 'Énergie soutirée (Wh)', 'Energia pobrana (Wh)'],
  'Export Energie (Wh)': ['Feed-in energy (Wh)', 'Export Energie (Wh)', 'Teruggeleverde energie (Wh)', 'Énergie injectée (Wh)', 'Energia oddana (Wh)'],
  'Nettoleistung (W, signed)': ['Net power (W, signed)', 'Nettoleistung (W, signed)', 'Nettovermogen (W, signed)', 'Puissance nette (W, signée)', 'Moc netto (W, ze znakiem)'],
  'Bezug Leistung (W)': ['Import power (W)', 'Bezug Leistung (W)', 'Afnamevermogen (W)', 'Puissance soutirée (W)', 'Moc pobierana (W)'],
  'Einspeisung Leistung (W)': ['Feed-in power (W)', 'Einspeisung Leistung (W)', 'Terugleververmogen (W)', 'Puissance injectée (W)', 'Moc oddawana (W)'],
  'Spannung L1/L2/L3 (V)': ['Voltage L1/L2/L3 (V)', 'Spannung L1/L2/L3 (V)', 'Spanning L1/L2/L3 (V)', 'Tension L1/L2/L3 (V)', 'Napięcie L1/L2/L3 (V)'],
  'Strom L1/L2/L3 (A)': ['Current L1/L2/L3 (A)', 'Strom L1/L2/L3 (A)', 'Stroom L1/L2/L3 (A)', 'Courant L1/L2/L3 (A)', 'Prąd L1/L2/L3 (A)'],
  'Frequenz (Hz)': ['Frequency (Hz)', 'Frequenz (Hz)', 'Frequentie (Hz)', 'Fréquence (Hz)', 'Częstotliwość (Hz)'],
  'Leistungsfaktor L1': ['Power factor L1', 'Leistungsfaktor L1', 'Vermogensfactor L1', 'Facteur de puissance L1', 'Współczynnik mocy L1'],
  'Speichern': ['Save', 'Speichern', 'Opslaan', 'Enregistrer', 'Zapisz'],
  'WLAN-Konfiguration': ['Wi-Fi settings', 'WLAN-Konfiguration', 'Wifi-instellingen', 'Paramètres Wi-Fi', 'Ustawienia Wi-Fi'],
  'Verfügbare WLANs': ['Available Wi-Fi networks', 'Verfügbare WLANs', 'Beschikbare wifi-netwerken', 'Réseaux Wi-Fi disponibles', 'Dostępne sieci Wi-Fi'],
  'Netzwerk auswählen': ['Select network', 'Netzwerk auswählen', 'Netwerk kiezen', 'Choisir un réseau', 'Wybierz sieć'],
  'Scannen': ['Scan', 'Scannen', 'Scannen', 'Rechercher', 'Skanuj'],
  'Scanstatus': ['Scan status', 'Scanstatus', 'Scanstatus', 'État de recherche', 'Stan skanowania'],
  'Noch nicht gescannt': ['Not scanned yet', 'Noch nicht gescannt', 'Nog niet gescand', 'Pas encore recherché', 'Jeszcze nie skanowano'],
  'SSID': ['SSID', 'SSID', 'SSID', 'SSID', 'SSID'],
  'Passwort': ['Password', 'Passwort', 'Wachtwoord', 'Mot de passe', 'Hasło'],
  'Tarife': ['Tariffs', 'Tarife', 'Tarieven', 'Tarifs', 'Taryfy'],
  'Bezug / Verbrauch (EUR/kWh)': ['Import / consumption (EUR/kWh)', 'Bezug / Verbrauch (EUR/kWh)', 'Afname / verbruik (EUR/kWh)', 'Soutirage / consommation (EUR/kWh)', 'Pobór / zużycie (EUR/kWh)'],
  'Einspeisung (EUR/kWh)': ['Feed-in (EUR/kWh)', 'Einspeisung (EUR/kWh)', 'Teruglevering (EUR/kWh)', 'Injection (EUR/kWh)', 'Oddawanie (EUR/kWh)'],
  'Währung': ['Currency', 'Währung', 'Valuta', 'Devise', 'Waluta'],
  'Tarife speichern': ['Save tariffs', 'Tarife speichern', 'Tarieven opslaan', 'Enregistrer les tarifs', 'Zapisz taryfy'],
  'Home Assistant Integration': ['Home Assistant integration', 'Home Assistant Integration', 'Home Assistant-integratie', 'Intégration Home Assistant', 'Integracja Home Assistant'],
  'Status': ['Status', 'Status', 'Status', 'État', 'Status'],
  'Aktiviert': ['Enabled', 'Aktiviert', 'Ingeschakeld', 'Activé', 'Włączone'],
  'MQTT Broker URI': ['MQTT broker URI', 'MQTT Broker URI', 'MQTT-broker-URI', 'URI du broker MQTT', 'URI brokera MQTT'],
  'Benutzername': ['Username', 'Benutzername', 'Gebruikersnaam', 'Nom d’utilisateur', 'Nazwa użytkownika'],
  'Gerätename': ['Device name', 'Gerätename', 'Apparaatnaam', 'Nom de l’appareil', 'Nazwa urządzenia'],
  'HA Discovery-Präfix': ['HA discovery prefix', 'HA Discovery-Präfix', 'HA-discoveryvoorvoegsel', 'Préfixe de découverte HA', 'Prefiks wykrywania HA'],
  'Weboberfläche schützen': ['Protect web interface', 'Weboberfläche schützen', 'Webinterface beveiligen', 'Protéger l’interface web', 'Chroń interfejs WWW'],
  'Neues Passwort': ['New password', 'Neues Passwort', 'Nieuw wachtwoord', 'Nouveau mot de passe', 'Nowe hasło'],
  'Neues Passwort wiederholen': ['Repeat new password', 'Neues Passwort wiederholen', 'Herhaal nieuw wachtwoord', 'Répéter le nouveau mot de passe', 'Powtórz nowe hasło'],
  'Passwort ändern': ['Change password', 'Passwort ändern', 'Wachtwoord wijzigen', 'Modifier le mot de passe', 'Zmień hasło'],
  'Zertifikat und HTTPS': ['Certificate and HTTPS', 'Zertifikat und HTTPS', 'Certificaat en HTTPS', 'Certificat et HTTPS', 'Certyfikat i HTTPS'],
  'Zertifikatsmodus': ['Certificate mode', 'Zertifikatsmodus', 'Certificaatmodus', 'Mode de certificat', 'Tryb certyfikatu'],
  'Manueller PEM-Import': ['Manual PEM import', 'Manueller PEM-Import', 'Handmatige PEM-import', 'Import PEM manuel', 'Ręczny import PEM'],
  'FQDN / Hostname': ['FQDN / hostname', 'FQDN / Hostname', 'FQDN / hostnaam', 'FQDN / nom d’hôte', 'FQDN / nazwa hosta'],
  'Kontakt-E-Mail für Let’s Encrypt': ['Contact email for Let’s Encrypt', 'Kontakt-E-Mail für Let’s Encrypt', 'Contact-e-mail voor Let’s Encrypt', 'E-mail de contact pour Let’s Encrypt', 'E-mail kontaktowy Let’s Encrypt'],
  'Erneuerungsintervall (Tage)': ['Renewal interval (days)', 'Erneuerungsintervall (Tage)', 'Vernieuwingsinterval (dagen)', 'Intervalle de renouvellement (jours)', 'Interwał odnowienia (dni)'],
  'ACME-Umgebung': ['ACME environment', 'ACME-Umgebung', 'ACME-omgeving', 'Environnement ACME', 'Środowisko ACME'],
  'Staging (Test, Browserwarnung)': ['Staging (test, browser warning)', 'Staging (Test, Browserwarnung)', 'Staging (test, browserwaarschuwing)', 'Staging (test, avertissement navigateur)', 'Staging (test, ostrzeżenie przeglądarki)'],
  'Produktion (gültiges Browser-Zertifikat)': ['Production (browser-trusted certificate)', 'Produktion (gültiges Browser-Zertifikat)', 'Productie (door browser vertrouwd certificaat)', 'Production (certificat reconnu par le navigateur)', 'Produkcja (certyfikat zaufany przez przeglądarkę)'],
  'Staging-Zertifikate sind nur für Tests und werden von normalen Browsern nicht als vertrauenswürdig erkannt.': ['Staging certificates are for testing and are not trusted by normal browsers.', 'Staging-Zertifikate sind nur für Tests und werden von normalen Browsern nicht als vertrauenswürdig erkannt.', 'Staging-certificaten zijn alleen voor tests en worden niet vertrouwd door normale browsers.', 'Les certificats staging sont réservés aux tests et ne sont pas reconnus par les navigateurs classiques.', 'Certyfikaty staging służą do testów i nie są zaufane przez zwykłe przeglądarki.'],
  'Let’s Encrypt empfiehlt bei 90 Tagen Laufzeit eine Erneuerung alle 60 Tage.': ['Let’s Encrypt recommends renewal every 60 days for 90-day certificates.', 'Let’s Encrypt empfiehlt bei 90 Tagen Laufzeit eine Erneuerung alle 60 Tage.', 'Let’s Encrypt raadt bij certificaten van 90 dagen vernieuwing om de 60 dagen aan.', 'Let’s Encrypt recommande un renouvellement tous les 60 jours pour les certificats de 90 jours.', 'Let’s Encrypt zaleca odnawianie co 60 dni dla certyfikatów ważnych 90 dni.'],
  'Zertifikatskette (Fullchain PEM)': ['Certificate chain (fullchain PEM)', 'Zertifikatskette (Fullchain PEM)', 'Certificaatketen (fullchain PEM)', 'Chaîne de certificats (fullchain PEM)', 'Łańcuch certyfikatów (fullchain PEM)'],
  'Privater Schlüssel (PEM)': ['Private key (PEM)', 'Privater Schlüssel (PEM)', 'Privésleutel (PEM)', 'Clé privée (PEM)', 'Klucz prywatny (PEM)'],
  'TLS-Konfiguration speichern': ['Save TLS settings', 'TLS-Konfiguration speichern', 'TLS-instellingen opslaan', 'Enregistrer les paramètres TLS', 'Zapisz ustawienia TLS'],
  'TLS-Einstellungen gespeichert; Gerät startet neu …': ['TLS settings saved; device restarting …', 'TLS-Einstellungen gespeichert; Gerät startet neu …', 'TLS-instellingen opgeslagen; apparaat start opnieuw …', 'Paramètres TLS enregistrés ; redémarrage …', 'Zapisano TLS; urządzenie uruchamia się ponownie …'],
  'TLS-Zertifikat ist gespeichert; PEM-Schlüssel werden nicht erneut angezeigt.': ['TLS certificate is stored; PEM keys are not shown again.', 'TLS-Zertifikat ist gespeichert; PEM-Schlüssel werden nicht erneut angezeigt.', 'TLS-certificaat is opgeslagen; PEM-sleutels worden niet opnieuw getoond.', 'Le certificat TLS est enregistré ; les clés PEM ne sont pas réaffichées.', 'Certyfikat TLS jest zapisany; klucze PEM nie są ponownie wyświetlane.'],
  'TLS-Konfiguration konnte nicht gespeichert werden.': ['Could not save TLS settings.', 'TLS-Konfiguration konnte nicht gespeichert werden.', 'TLS-instellingen konden niet worden opgeslagen.', 'Impossible d’enregistrer les paramètres TLS.', 'Nie udało się zapisać ustawień TLS.'],
  'PEM-Zertifikat und privater Schlüssel passen nicht zusammen.': ['PEM certificate and private key do not match.', 'PEM-Zertifikat und privater Schlüssel passen nicht zusammen.', 'PEM-certificaat en privésleutel komen niet overeen.', 'Le certificat PEM et la clé privée ne correspondent pas.', 'Certyfikat PEM i klucz prywatny nie pasują do siebie.'],
  'Automatische ACME-Ausstellung wird vorbereitet.': ['Automatic ACME issuance is being prepared.', 'Automatische ACME-Ausstellung wird vorbereitet.', 'Automatische ACME-uitgifte wordt voorbereid.', 'L’émission ACME automatique est en préparation.', 'Automatyczne wydawanie ACME jest przygotowywane.'],
  'Let’s-Encrypt-Zertifikat anfordern': ['Request Let’s Encrypt certificate', 'Let’s-Encrypt-Zertifikat anfordern', 'Let’s Encrypt-certificaat aanvragen', 'Demander un certificat Let’s Encrypt', 'Poproś o certyfikat Let’s Encrypt'],
  'ACME-Anforderung läuft …': ['ACME request running …', 'ACME-Anforderung läuft …', 'ACME-aanvraag loopt …', 'Demande ACME en cours …', 'Trwa żądanie ACME …'],
  'Zertifikat ausgestellt; Gerät startet neu …': ['Certificate issued; device restarting …', 'Zertifikat ausgestellt; Gerät startet neu …', 'Certificaat uitgegeven; apparaat start opnieuw …', 'Certificat émis ; redémarrage …', 'Certyfikat wydany; urządzenie uruchamia się ponownie …'],
  'ACME-Anforderung fehlgeschlagen. DNS und Routerfreigabe prüfen.': ['ACME request failed. Check DNS and router access.', 'ACME-Anforderung fehlgeschlagen. DNS und Routerfreigabe prüfen.', 'ACME-aanvraag mislukt. Controleer DNS en routertoegang.', 'Échec de la demande ACME. Vérifiez le DNS et le routeur.', 'Żądanie ACME nie powiodło się. Sprawdź DNS i router.'],
  "Let's Encrypt (HTTP-01)": ["Let's Encrypt (HTTP-01)", "Let's Encrypt (HTTP-01)", "Let's Encrypt (HTTP-01)", "Let's Encrypt (HTTP-01)", "Let's Encrypt (HTTP-01)"],
  "Voraussetzungen für Let's Encrypt": ["Let's Encrypt requirements", "Voraussetzungen für Let's Encrypt", "Vereisten voor Let's Encrypt", "Conditions requises pour Let's Encrypt", "Wymagania Let's Encrypt"],
  'Vor Erstanforderung und jeder Erneuerung muss der FQDN öffentlich per A (IPv4) oder AAAA (IPv6) auf die aktuelle Adresse des ESP zeigen.': ['Before initial issuance and every renewal, the FQDN must resolve publicly via A (IPv4) or AAAA (IPv6) to the ESP’s current address.', 'Vor Erstanforderung und jeder Erneuerung muss der FQDN öffentlich per A (IPv4) oder AAAA (IPv6) auf die aktuelle Adresse des ESP zeigen.', 'Voor de eerste uitgifte en elke verlenging moet de FQDN openbaar via A (IPv4) of AAAA (IPv6) naar het actuele adres van de ESP verwijzen.', 'Avant la première émission et chaque renouvellement, le FQDN doit pointer publiquement via A (IPv4) ou AAAA (IPv6) vers l’adresse actuelle de l’ESP.', 'Przed pierwszym wydaniem i każdym odnowieniem FQDN musi wskazywać publicznie przez A (IPv4) lub AAAA (IPv6) aktualny adres ESP.'],
  'Am Router: IPv4-TCP-Port 80 direkt auf ESP-Port 80 weiterleiten; kein abweichender Zielport.': ['At the router: forward IPv4 TCP port 80 directly to ESP port 80; do not translate to another destination port.', 'Am Router: IPv4-TCP-Port 80 direkt auf ESP-Port 80 weiterleiten; kein abweichender Zielport.', 'Op de router: stuur IPv4-TCP-poort 80 rechtstreeks door naar ESP-poort 80; geen afwijkende doelpoort.', 'Sur le routeur : redirigez directement le port TCP IPv4 80 vers le port 80 de l’ESP ; aucun port de destination différent.', 'Na routerze: przekieruj IPv4 TCP 80 bezpośrednio na port 80 ESP; bez zmiany portu docelowego.'],
  'Bei AAAA: IPv6-Port 80 direkt in der Router-Firewall für die globale IPv6-Adresse des ESP freigeben; IPv6 übersetzt keine Ports.': ['For AAAA: allow IPv6 port 80 directly in the router firewall for the ESP’s global IPv6 address; IPv6 does not translate ports.', 'Bei AAAA: IPv6-Port 80 direkt in der Router-Firewall für die globale IPv6-Adresse des ESP freigeben; IPv6 übersetzt keine Ports.', 'Bij AAAA: sta IPv6-poort 80 rechtstreeks toe in de routerfirewall voor het globale IPv6-adres van de ESP; IPv6 vertaalt geen poorten.', 'Avec AAAA : autorisez directement le port IPv6 80 dans le pare-feu du routeur pour l’adresse IPv6 globale de l’ESP ; IPv6 ne traduit pas les ports.', 'Dla AAAA: zezwól bezpośrednio na port IPv6 80 w zaporze routera dla globalnego adresu IPv6 ESP; IPv6 nie tłumaczy portów.'],
  'Port 80 dient nur der HTTP-01-Challenge von Let’s Encrypt; Dashboard und Konfiguration laufen über HTTPS auf Port 443.': ['Port 80 is only for the Let’s Encrypt HTTP-01 challenge; the dashboard and settings use HTTPS on port 443.', 'Port 80 dient nur der HTTP-01-Challenge von Let’s Encrypt; Dashboard und Konfiguration laufen über HTTPS auf Port 443.', 'Poort 80 is alleen voor de Let’s Encrypt HTTP-01-uitdaging; dashboard en instellingen gebruiken HTTPS op poort 443.', 'Le port 80 sert uniquement au défi HTTP-01 de Let’s Encrypt ; le tableau de bord et les paramètres utilisent HTTPS sur le port 443.', 'Port 80 służy tylko do wyzwania HTTP-01 Let’s Encrypt; panel i konfiguracja działają przez HTTPS na porcie 443.'],
  'DNS und Routerfreigabe müssen auch bei jeder Erneuerung erreichbar sein.': ['DNS and router access must also be available for every renewal.', 'DNS und Routerfreigabe müssen auch bei jeder Erneuerung erreichbar sein.', 'DNS en routertoegang moeten ook bij elke verlenging beschikbaar zijn.', 'Le DNS et l’accès au routeur doivent également être disponibles à chaque renouvellement.', 'DNS i dostęp przez router muszą być dostępne przy każdym odnowieniu.'],
  'Hinweis: Die automatische Let’s-Encrypt-Ausstellung und Erneuerung muss noch aktiviert werden.': ['Note: automatic Let’s Encrypt issuance and renewal still need to be enabled.', 'Hinweis: Die automatische Let’s-Encrypt-Ausstellung und Erneuerung muss noch aktiviert werden.', 'Let op: automatische uitgifte en verlenging van Let’s Encrypt moeten nog worden ingeschakeld.', 'Remarque : l’émission et le renouvellement automatiques de Let’s Encrypt doivent encore être activés.', 'Uwaga: automatyczne wydawanie i odnawianie Let’s Encrypt wymaga jeszcze włączenia.'],
  'OTA-Update (Firmware)': ['OTA update (firmware)', 'OTA-Update (Firmware)', 'OTA-update (firmware)', 'Mise à jour OTA (micrologiciel)', 'Aktualizacja OTA (oprogramowanie)'],
  'Hinweis an den Admin:': ['Note for the administrator:', 'Hinweis an den Admin:', 'Opmerking voor de beheerder:', 'Remarque pour l’administrateur :', 'Uwaga dla administratora:'],
  'Die Firmware nutzt kein Safeboot und ist für das direkte Flashen ab Adresse': ['This firmware does not use safeboot and is intended for direct flashing from address', 'Die Firmware nutzt kein Safeboot und ist für das direkte Flashen ab Adresse', 'Deze firmware gebruikt geen safeboot en is bedoeld om direct te flashen vanaf adres', 'Ce micrologiciel n’utilise pas Safeboot et doit être flashé directement à partir de l’adresse', 'To oprogramowanie nie używa Safeboot i jest przeznaczone do bezpośredniego flashowania od adresu'],
  'Flashen': ['Flash', 'Flashen', 'Flashen', 'Flasher', 'Wgraj'],
  'Browser-Firmware': ['Firmware from GitHub', 'Firmware von GitHub', 'Firmware van GitHub', 'Micrologiciel GitHub', 'Firmware z GitHub'],
  'Verfügbare GitHub-Versionen': ['Available GitHub versions', 'Verfügbare GitHub-Versionen', 'Beschikbare GitHub-versies', 'Versions GitHub disponibles', 'Dostępne wersje GitHub'],
  'GitHub-Versionen laden': ['Load GitHub versions', 'GitHub-Versionen laden', 'GitHub-versies laden', 'Charger les versions GitHub', 'Wczytaj wersje GitHub'],
  'Ausgewählte Version installieren': ['Install selected version', 'Ausgewählte Version installieren', 'Geselecteerde versie installeren', 'Installer la version sélectionnée', 'Zainstaluj wybraną wersję'],
  'Firmware-Versionen werden geladen …': ['Loading firmware versions …', 'Firmware-Versionen werden geladen …', 'Firmwareversies laden …', 'Chargement des versions du micrologiciel …', 'Wczytywanie wersji firmware …'],
  'GitHub-Versionen geladen': ['GitHub versions loaded', 'GitHub-Versionen geladen', 'GitHub-versies geladen', 'Versions GitHub chargées', 'Wczytano wersje GitHub'],
  'Keine passende Firmware gefunden.': ['No matching firmware found.', 'Keine passende Firmware gefunden.', 'Geen passende firmware gevonden.', 'Aucun micrologiciel correspondant trouvé.', 'Nie znaleziono pasującego firmware.'],
  'Fehler beim Laden der GitHub-Versionen.': ['Could not load GitHub versions.', 'Fehler beim Laden der GitHub-Versionen.', 'GitHub-versies laden mislukt.', 'Impossible de charger les versions GitHub.', 'Nie udało się wczytać wersji GitHub.'],
  'Firmware-Download gestartet …': ['Firmware download started …', 'Firmware-Download gestartet …', 'Firmwaredownload gestart …', 'Téléchargement du micrologiciel démarré …', 'Rozpoczęto pobieranie firmware …'],
  'Firmware-Update läuft …': ['Firmware update in progress …', 'Firmware-Update läuft …', 'Firmware-update bezig …', 'Mise à jour du micrologiciel en cours …', 'Aktualizacja firmware trwa …'],
  'Gerät startet mit neuer Firmware neu …': ['Device restarting with new firmware …', 'Gerät startet mit neuer Firmware neu …', 'Apparaat start opnieuw met nieuwe firmware …', 'Redémarrage avec le nouveau micrologiciel …', 'Urządzenie uruchamia się z nowym firmware …'],
  'GitHub-OTA fehlgeschlagen.': ['GitHub OTA failed.', 'GitHub-OTA fehlgeschlagen.', 'GitHub OTA mislukt.', 'Échec de la mise à jour GitHub.', 'Aktualizacja GitHub OTA nie powiodła się.'],
  'Bitte zuerst eine GitHub-Version laden.': ['Load GitHub versions first.', 'Bitte zuerst eine GitHub-Version laden.', 'Laad eerst GitHub-versies.', 'Chargez d’abord les versions GitHub.', 'Najpierw wczytaj wersje GitHub.'],
  'Installation wirklich starten?': ['Start installation?', 'Installation wirklich starten?', 'Installatie starten?', 'Lancer l’installation ?', 'Rozpocząć instalację?'],
  'Browser bevorzugt; EN ist der Fallback.': ['Uses browser preferences; falls back to English.', 'Browser bevorzugt; EN ist der Fallback.', 'Gebruikt browservoorkeuren; valt terug op Engels.', 'Utilise les préférences du navigateur ; anglais par défaut.', 'Używa preferencji przeglądarki; domyślnie angielski.'],
  'Language saved': ['Language saved', 'Sprache gespeichert', 'Taal opgeslagen', 'Langue enregistrée', 'Zapisano język'],
  'Language could not be saved': ['Language could not be saved', 'Sprache konnte nicht gespeichert werden', 'Taal kon niet worden opgeslagen', 'Impossible d’enregistrer la langue', 'Nie udało się zapisać języka'],
  'Verbunden': ['Connected', 'Verbunden', 'Verbonden', 'Connecté', 'Połączono'],
  'Getrennt': ['Disconnected', 'Getrennt', 'Verbinding verbroken', 'Déconnecté', 'Rozłączono'],
  'Dauerlesen aktiv': ['Continuous reading active', 'Dauerlesen aktiv', 'Continu uitlezen actief', 'Lecture continue active', 'Odczyt ciągły aktywny'],
  'Letzter Login erfolgreich': ['Last login successful', 'Letzter Login erfolgreich', 'Laatste aanmelding geslaagd', 'Dernière connexion réussie', 'Ostatnie logowanie udane'],
  'Letzter Login fehlgeschlagen / ausstehend': ['Last login failed / pending', 'Letzter Login fehlgeschlagen / ausstehend', 'Laatste aanmelding mislukt / in afwachting', 'Dernière connexion échouée / en attente', 'Ostatnie logowanie nieudane / oczekujące'],
  'Gerät nicht erreichbar. Bitte Seite manuell neu laden.': ['Device unreachable. Reload the page manually.', 'Gerät nicht erreichbar. Bitte Seite manuell neu laden.', 'Apparaat niet bereikbaar. Laad de pagina handmatig opnieuw.', 'Appareil inaccessible. Rechargez la page manuellement.', 'Urządzenie niedostępne. Odśwież stronę ręcznie.'],
  'Verbindung unterbrochen — Wiederverbindung in': ['Connection lost — reconnecting in', 'Verbindung unterbrochen — Wiederverbindung in', 'Verbinding verbroken — opnieuw verbinden over', 'Connexion perdue — reconnexion dans', 'Utracono połączenie — ponowne za minut'],
  'Weiter': ['Resume', 'Weiter', 'Doorgaan', 'Reprendre', 'Wznów'],
  'Pause': ['Pause', 'Pause', 'Pauze', 'Pause', 'Pauza'],
  'Log in Zwischenablage kopiert': ['Log copied to clipboard', 'Log in Zwischenablage kopiert', 'Log gekopieerd naar klembord', 'Journal copié dans le presse-papiers', 'Skopiowano log do schowka'],
  'Kopieren fehlgeschlagen': ['Copy failed', 'Kopieren fehlgeschlagen', 'Kopiëren mislukt', 'Échec de la copie', 'Kopiowanie nie powiodło się'],
  'Ablesung läuft bereits.': ['A reading is already running.', 'Ablesung läuft bereits.', 'Er wordt al uitgelezen.', 'Une lecture est déjà en cours.', 'Odczyt jest już uruchomiony.'],
  'Gerät neu starten?': ['Restart the device?', 'Gerät neu starten?', 'Apparaat opnieuw starten?', 'Redémarrer l’appareil ?', 'Uruchomić ponownie urządzenie?'],
  'Bitte eine URL eintragen.': ['Enter a URL.', 'Bitte eine URL eintragen.', 'Voer een URL in.', 'Saisissez une URL.', 'Wpisz adres URL.'],
  'Lade Profilkatalog ...': ['Loading profile catalog ...', 'Lade Profilkatalog ...', 'Profielcatalogus laden ...', 'Chargement du catalogue de profils ...', 'Wczytywanie katalogu profili ...'],
  'Fehler beim Laden': ['Loading failed', 'Fehler beim Laden', 'Laden mislukt', 'Échec du chargement', 'Błąd wczytywania'],
  'Suche nach Netzwerken ...': ['Searching for networks ...', 'Suche nach Netzwerken ...', 'Netwerken zoeken ...', 'Recherche de réseaux ...', 'Wyszukiwanie sieci ...'],
  'Netzwerk auswählen': ['Select network', 'Netzwerk auswählen', 'Netwerk kiezen', 'Choisir un réseau', 'Wybierz sieć'],
  'Noch nicht gescannt': ['Not scanned yet', 'Noch nicht gescannt', 'Nog niet gescand', 'Pas encore recherché', 'Jeszcze nie skanowano'],
  'Übertrage …': ['Transferring …', 'Übertrage …', 'Bezig met overdragen …', 'Transfert …', 'Przesyłanie …'],
  'Verbindungsfehler': ['Connection error', 'Verbindungsfehler', 'Verbindingsfout', 'Erreur de connexion', 'Błąd połączenia'],
  'Update erfolgreich! Gerät startet neu …': ['Update successful! Device is restarting …', 'Update erfolgreich! Gerät startet neu …', 'Update geslaagd! Apparaat start opnieuw …', 'Mise à jour réussie ! Redémarrage …', 'Aktualizacja udana! Urządzenie uruchamia się ponownie …'],
  'Fehler beim Speichern': ['Save failed', 'Fehler beim Speichern', 'Opslaan mislukt', 'Échec de l’enregistrement', 'Błąd zapisu'],
  'Zähler-Konfiguration gespeichert': ['Meter settings saved', 'Zähler-Konfiguration gespeichert', 'Meterinstellingen opgeslagen', 'Configuration du compteur enregistrée', 'Zapisano ustawienia licznika'],
  'Tarife gespeichert.': ['Tariffs saved.', 'Tarife gespeichert.', 'Tarieven opgeslagen.', 'Tarifs enregistrés.', 'Zapisano taryfy.'],
  'Tarife konnten nicht gespeichert werden.': ['Could not save tariffs.', 'Tarife konnten nicht gespeichert werden.', 'Tarieven konden niet worden opgeslagen.', 'Impossible d’enregistrer les tarifs.', 'Nie udało się zapisać taryf.'],
  'PIN ist gesetzt': ['PIN is set', 'PIN ist gesetzt', 'PIN is ingesteld', 'Le PIN est défini', 'PIN jest ustawiony'],
  'PIN ist nicht gesetzt': ['PIN is not set', 'PIN ist nicht gesetzt', 'PIN is niet ingesteld', 'Le PIN n’est pas défini', 'PIN nie jest ustawiony'],
  'Speichern fehlgeschlagen': ['Save failed', 'Speichern fehlgeschlagen', 'Opslaan mislukt', 'Échec de l’enregistrement', 'Zapis nie powiódł się'],
  'Profil anwenden': ['Apply profile', 'Profil anwenden', 'Profiel toepassen', 'Appliquer le profil', 'Zastosuj profil'],
  'External profiles loaded': ['external profiles loaded', 'externe Profile geladen', 'externe profielen geladen', 'profils externes chargés', 'profili zewnętrznych wczytano'],
  'Profile catalog could not be loaded: ': ['Could not load profile catalog: ', 'Profilkatalog konnte nicht geladen werden: ', 'Profielcatalogus kon niet worden geladen: ', 'Impossible de charger le catalogue de profils : ', 'Nie udało się wczytać katalogu profili: '],
  'Custom profile ': ['Custom profile ', 'Custom-Profil ', 'Aangepast profiel ', 'Profil personnalisé ', 'Profil własny '],
  ' saved.': [' saved.', ' gespeichert.', ' opgeslagen.', ' enregistré.', ' zapisano.'],
  'Invalid OBIS format for ': ['Invalid OBIS format for ', 'Ungültiges OBIS-Format bei ', 'Ongeldig OBIS-formaat bij ', 'Format OBIS invalide pour ', 'Nieprawidłowy format OBIS dla '],
  'Expected: 1-0:1.8.0*255': ['Expected: 1-0:1.8.0*255', 'Erwartet: 1-0:1.8.0*255', 'Verwacht: 1-0:1.8.0*255', 'Attendu : 1-0:1.8.0*255', 'Oczekiwano: 1-0:1.8.0*255'],
  'Meter settings saved': ['Meter settings saved', 'Zähler-Konfiguration gespeichert', 'Meterinstellingen opgeslagen', 'Configuration du compteur enregistrée', 'Zapisano ustawienia licznika'],
  'Wi-Fi saved and connected.': ['Wi-Fi saved and connected.', 'WLAN gespeichert und verbunden.', 'Wifi opgeslagen en verbonden.', 'Wi-Fi enregistré et connecté.', 'Zapisano Wi-Fi i połączono.'],
  'Wi-Fi saved, connection failed. AP remains active.': ['Wi-Fi saved, connection failed. AP remains active.', 'WLAN gespeichert, Verbindung fehlgeschlagen. AP bleibt aktiv.', 'Wifi opgeslagen, verbinden mislukt. AP blijft actief.', 'Wi-Fi enregistré, connexion échouée. Le point d’accès reste actif.', 'Zapisano Wi-Fi, połączenie nieudane. Punkt dostępowy pozostaje aktywny.'],
  'Password must match and be at least 8 characters.': ['Passwords must match and be at least 8 characters.', 'Passwörter müssen übereinstimmen und mindestens 8 Zeichen lang sein.', 'Wachtwoorden moeten overeenkomen en minstens 8 tekens lang zijn.', 'Les mots de passe doivent correspondre et contenir au moins 8 caractères.', 'Hasła muszą być zgodne i mieć co najmniej 8 znaków.'],
  'Password saved. The new password will be used on next access.': ['Password saved. The new password will be used on next access.', 'Passwort gespeichert. Beim nächsten Zugriff wird das neue Passwort verwendet.', 'Wachtwoord opgeslagen. Het nieuwe wachtwoord wordt bij de volgende toegang gebruikt.', 'Mot de passe enregistré. Le nouveau mot de passe sera utilisé à la prochaine connexion.', 'Hasło zapisano. Nowe hasło będzie używane przy następnym dostępie.'],
  'Password could not be saved.': ['Password could not be saved.', 'Passwort konnte nicht gespeichert werden.', 'Wachtwoord kon niet worden opgeslagen.', 'Impossible d’enregistrer le mot de passe.', 'Nie udało się zapisać hasła.'],
  'Searching for networks ...': ['Searching for networks ...', 'Suche nach Netzwerken ...', 'Netwerken zoeken ...', 'Recherche de réseaux ...', 'Wyszukiwanie sieci ...'],
  'networks found': ['networks found', 'Netzwerke gefunden', 'netwerken gevonden', 'réseaux trouvés', 'znaleziono sieci'],
  'Scan failed': ['Scan failed', 'Scan fehlgeschlagen', 'Scannen mislukt', 'Échec de la recherche', 'Skanowanie nie powiodło się'],
  'Tariffs saved.': ['Tariffs saved.', 'Tarife gespeichert.', 'Tarieven opgeslagen.', 'Tarifs enregistrés.', 'Zapisano taryfy.'],
  'Could not save tariffs.': ['Could not save tariffs.', 'Tarife konnten nicht gespeichert werden.', 'Tarieven konden niet worden opgeslagen.', 'Impossible d’enregistrer les tarifs.', 'Nie udało się zapisać taryf.'],
  'HA integration saved.': ['Home Assistant integration saved.', 'HA-Integration gespeichert.', 'Home Assistant-integratie opgeslagen.', 'Intégration Home Assistant enregistrée.', 'Zapisano integrację Home Assistant.'],
  'PIN is set': ['PIN is set', 'PIN ist gesetzt', 'PIN is ingesteld', 'Le PIN est défini', 'PIN jest ustawiony'],
  'PIN is not set': ['PIN is not set', 'PIN ist nicht gesetzt', 'PIN is niet ingesteld', 'Le PIN n’est pas défini', 'PIN nie jest ustawiony'],
  'Transferring …': ['Transferring …', 'Übertrage …', 'Bezig met overdragen …', 'Transfert …', 'Przesyłanie …'],
  'Update successful! Device is restarting …': ['Update successful! Device is restarting …', 'Update erfolgreich! Gerät startet neu …', 'Update geslaagd! Apparaat start opnieuw …', 'Mise à jour réussie ! Redémarrage …', 'Aktualizacja udana! Urządzenie uruchamia się ponownie …'],
  'Connection error': ['Connection error', 'Verbindungsfehler', 'Verbindingsfout', 'Erreur de connexion', 'Błąd połączenia'],
  'Error: ': ['Error: ', 'Fehler: ', 'Fout: ', 'Erreur : ', 'Błąd: '],
  'Version': ['Version', 'Version', 'Versie', 'Version', 'Wersja']
};

let activeUiLanguage = 'en';
let configuredUiLanguage = 'auto';
const translatedTextNodes = new Set();
const textNodeStates = new WeakMap();
const translatedAttributes = new Map();
const attributeStates = new WeakMap();
let uiTextObserver = null;

function browserUiLanguage() {
  for (const candidate of navigator.languages || [navigator.language]) {
    const code = String(candidate || '').toLowerCase().split('-')[0];
    if (UI_LANGUAGES.includes(code)) return code;
  }
  return 'en';
}

function localizedText(source) {
  const values = UI_TEXT[source];
  if (!values) return source;
  const index = UI_LANGUAGES.indexOf(activeUiLanguage);
  return values[index < 0 ? 0 : index];
}

function translateTextNode(node) {
  if (!node.parentElement || node.parentElement.closest('script,style')) return;
  const previous = textNodeStates.get(node);
  const source = previous && node.nodeValue === previous.rendered ? previous.source : node.nodeValue;
  const trimmed = source.trim();
  if (!trimmed) return;
  const rendered = source.replace(trimmed, localizedText(trimmed));
  textNodeStates.set(node, { source, rendered });
  translatedTextNodes.add(node);
  if (node.nodeValue !== rendered) node.nodeValue = rendered;
}

function translateAttribute(element, attribute) {
  const state = attributeStates.get(element) || {};
  const current = element.getAttribute(attribute);
  if (current == null) return;
  const previous = state[attribute];
  const source = previous && current === previous.rendered ? previous.source : current;
  const rendered = localizedText(source);
  state[attribute] = { source, rendered };
  attributeStates.set(element, state);
  let attributes = translatedAttributes.get(element);
  if (!attributes) {
    attributes = new Set();
    translatedAttributes.set(element, attributes);
  }
  attributes.add(attribute);
  if (current !== rendered) element.setAttribute(attribute, rendered);
}

function collectUiText(root) {
  const walker = document.createTreeWalker(root, NodeFilter.SHOW_TEXT);
  while (walker.nextNode()) translateTextNode(walker.currentNode);
  root.querySelectorAll('[placeholder],[title],[aria-label]').forEach(element => {
    ['placeholder', 'title', 'aria-label'].forEach(attribute => translateAttribute(element, attribute));
  });
}

function applyUiLanguage(language) {
  activeUiLanguage = UI_LANGUAGES.includes(language) ? language : 'en';
  document.documentElement.lang = activeUiLanguage;
  translatedTextNodes.forEach(translateTextNode);
  translatedAttributes.forEach((attributes, element) => {
    attributes.forEach(attribute => translateAttribute(element, attribute));
  });
}

async function initUiLanguage() {
  collectUiText(document.head);
  collectUiText(document.body);
  applyUiLanguage(browserUiLanguage());
  if (uiTextObserver) uiTextObserver.disconnect();
  uiTextObserver = new MutationObserver(records => {
    for (const record of records) {
      if (record.type === 'characterData') translateTextNode(record.target);
      record.addedNodes && record.addedNodes.forEach(node => {
        if (node.nodeType === Node.TEXT_NODE) translateTextNode(node);
        else if (node.nodeType === Node.ELEMENT_NODE) collectUiText(node);
      });
      if (record.type === 'attributes') translateAttribute(record.target, record.attributeName);
    }
  });
  uiTextObserver.observe(document.body, { childList: true, characterData: true, subtree: true, attributes: true, attributeFilter: ['placeholder', 'title', 'aria-label'] });
  try {
    const response = await fetch('/api/config/language');
    if (response.ok) configuredUiLanguage = (await response.json()).language || 'auto';
  } catch (_) {}
  const selector = document.getElementById('ui-language-select');
  if (selector) selector.value = configuredUiLanguage;
  applyUiLanguage(configuredUiLanguage === 'auto' ? browserUiLanguage() : configuredUiLanguage);
}

async function saveUiLanguagePreference(language) {
  const status = document.getElementById('ui-language-status');
  const selector = document.getElementById('ui-language-select');
  try {
    const response = await fetch('/api/config/language', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ language })
    });
    if (!response.ok || !(await response.json()).ok) throw new Error('save failed');
    configuredUiLanguage = language;
    applyUiLanguage(language === 'auto' ? browserUiLanguage() : language);
    if (status) status.textContent = localizedText('Language saved');
  } catch (_) {
    if (selector) selector.value = configuredUiLanguage;
    if (status) status.textContent = localizedText('Language could not be saved');
  }
}

async function loadAccessNetworks() {
  try {
    const response = await fetch('/api/config/networks');
    if (!response.ok) return;
    const config = await response.json();
    const input = document.getElementById('allowed-networks');
    if (input) input.value = config.allowed_networks || '';
  } catch (_) {}
}

async function saveAccessNetworks(event) {
  event.preventDefault();
  const input = document.getElementById('allowed-networks');
  const response = await fetch('/api/config/networks', {
    method: 'POST',
    headers: { 'Content-Type': 'application/json' },
    body: JSON.stringify({ allowed_networks: input ? input.value : '' })
  });
  const result = response.ok ? await response.json() : { ok: false };
  alert(localizedText(result.ok ? 'Netzwerkfilter gespeichert.' : 'Netzwerkfilter konnte nicht gespeichert werden.'));
}

function updateTlsMode(mode) {
  const letsEncrypt = mode === 'letsencrypt';
  const prerequisites = document.getElementById('le-prerequisites');
  const requestButton = document.getElementById('acme-request-button');
  const stagingWarning = document.getElementById('acme-staging-warning');
  if (prerequisites) prerequisites.hidden = !letsEncrypt;
  if (requestButton) requestButton.hidden = !letsEncrypt;
  if (stagingWarning) stagingWarning.hidden = !letsEncrypt || document.getElementById('tls-acme-environment').value !== 'staging';
}

async function loadTlsConfig() {
  try {
    const response = await fetch('/api/config/tls');
    if (!response.ok) return;
    const config = await response.json();
    document.getElementById('tls-mode').value = config.mode || 'manual';
    document.getElementById('tls-fqdn').value = config.fqdn || '';
    document.getElementById('tls-email').value = config.email || '';
    document.getElementById('tls-renewal-days').value = config.renewal_interval_days || 60;
    document.getElementById('tls-acme-environment').value = config.acme_staging === false ? 'production' : 'staging';
    const certificateStatus = document.getElementById('tls-current-cert-status');
    if (certificateStatus && config.has_certificate)
      certificateStatus.textContent = localizedText('TLS-Zertifikat ist gespeichert; PEM-Schlüssel werden nicht erneut angezeigt.');
    updateTlsMode(config.mode || 'manual');
  } catch (_) {}
}

async function saveTlsConfig(event) {
  event.preventDefault();
  const certificate = document.getElementById('tls-certificate').value.trim();
  const privateKey = document.getElementById('tls-private-key').value.trim();
  const result = document.getElementById('tls-save-status');
  if (Boolean(certificate) !== Boolean(privateKey)) {
    alert(localizedText('PEM-Zertifikat und privater Schlüssel passen nicht zusammen.'));
    return;
  }
  const payload = {
    mode: document.getElementById('tls-mode').value,
    fqdn: document.getElementById('tls-fqdn').value.trim(),
    email: document.getElementById('tls-email').value.trim(),
    renewal_interval_days: Number(document.getElementById('tls-renewal-days').value),
    acme_staging: document.getElementById('tls-acme-environment').value === 'staging',
    certificate_pem: certificate,
    private_key_pem: privateKey
  };
  try {
    const response = await fetch('/api/config/tls', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify(payload)
    });
    const saved = response.ok ? await response.json() : { ok: false };
    if (!saved.ok) throw new Error('save failed');
    if (result) result.textContent = localizedText('TLS-Einstellungen gespeichert; Gerät startet neu …');
  } catch (_) {
    if (result) result.textContent = localizedText('TLS-Konfiguration konnte nicht gespeichert werden.');
  }
}

async function requestAcmeCertificate() {
  const status = document.getElementById('acme-request-status');
  const button = document.getElementById('acme-request-button');
  if (document.getElementById('tls-mode').value !== 'letsencrypt') return;
  if (button) button.disabled = true;
  if (status) status.textContent = localizedText('ACME-Anforderung läuft …');
  try {
    const response = await fetch('/api/config/tls/acme', { method: 'POST' });
    const result = response.ok ? await response.json() : null;
    if (!result || !result.ok) throw new Error('ACME request failed');
    const poll = setInterval(async () => {
      try {
        const statusResponse = await fetch('/api/config/tls/acme/status');
        const current = statusResponse.ok ? await statusResponse.json() : null;
        if (!current) throw new Error('status unavailable');
        if (current.state === 'issued') {
          if (status) status.textContent = localizedText('Zertifikat ausgestellt; Gerät startet neu …');
          clearInterval(poll);
        } else if (current.state === 'failed') {
          if (status) status.textContent = localizedText('ACME-Anforderung fehlgeschlagen. DNS und Routerfreigabe prüfen.');
          if (button) button.disabled = false;
          clearInterval(poll);
        }
      } catch (_) {
        if (status) status.textContent = localizedText('Zertifikat ausgestellt; Gerät startet neu …');
        clearInterval(poll);
      }
    }, 2000);
  } catch (_) {
    if (status) status.textContent = localizedText('ACME-Anforderung fehlgeschlagen. DNS und Routerfreigabe prüfen.');
    if (button) button.disabled = false;
  }
}

function _startReconnect() {
  if (_offline) return;                  // already in reconnect mode
  _offline = true;
  clearInterval(statusTimer);
  clearInterval(logTimer);
  _reconnectDeadline = Date.now() + 2 * 60 * 1000;  // 2 minutes

  // Show banner
  let banner = document.getElementById('_reconnect-banner');
  if (!banner) {
    banner = document.createElement('div');
    banner.id = '_reconnect-banner';
    banner.style.cssText =
      'position:fixed;top:0;left:0;right:0;z-index:9999;padding:10px 16px;' +
      'background:#FF8C00;color:#000;font-weight:700;font-family:monospace;' +
      'font-size:.85rem;text-align:center;letter-spacing:.05em';
    document.body.prepend(banner);
  }

  function attempt() {
    const remaining = Math.max(0, Math.ceil((_reconnectDeadline - Date.now()) / 1000));
    if (remaining <= 0) {
      banner.textContent = '\u26A0 ' + localizedText('Gerät nicht erreichbar. Bitte Seite manuell neu laden.');
      banner.style.background = '#CC0000';
      banner.style.color = '#fff';
      return;
    }
    banner.textContent = '\u21BB ' + localizedText('Verbindung unterbrochen — Wiederverbindung in') + ' ' + remaining + ' s …';
    fetch('/api/status')
      .then(r => { if (!r.ok) throw new Error(); return r.json(); })
      .then(() => {
        // Device is back
        _offline = false;
        clearTimeout(_reconnectTimer);
        clearInterval(_reconnectCountdown);
        banner.remove();
        logSeq = 0;
        statusTimer = setInterval(fetchStatus, 30000);
        logTimer    = setInterval(fetchLog, 30000);
        fetchStatus();
        fetchLog();
      })
      .catch(() => {
        _reconnectTimer = setTimeout(attempt, 5000);
      });
  }

  // Start countdown ticker (updates banner every second)
  _reconnectCountdown = setInterval(() => {
    const remaining = Math.max(0, Math.ceil((_reconnectDeadline - Date.now()) / 1000));
    if (_offline && remaining > 0) {
      const banner2 = document.getElementById('_reconnect-banner');
      if (banner2 && !banner2.textContent.includes('nicht erreichbar'))
        banner2.textContent = '\u21BB ' + localizedText('Verbindung unterbrochen — Wiederverbindung in') + ' ' + remaining + ' s …';
    } else if (remaining <= 0) {
      clearInterval(_reconnectCountdown);
    }
  }, 1000);

  // First attempt after 5 s
  _reconnectTimer = setTimeout(attempt, 5000);
}

function toggleLogPause() {
  logPaused = !logPaused;
  const btn = document.getElementById('log-pause-btn');
  btn.textContent = logPaused ? '\u25B6 ' + localizedText('Weiter') : '\u23F8 ' + localizedText('Pause');
  btn.style.background = logPaused ? '#0f9d58' : '';
}

function copyLog() {
  const lines = document.getElementById('log').innerText;
  navigator.clipboard.writeText(lines).then(
    () => alert(localizedText('Log in Zwischenablage kopiert')),
    () => alert(localizedText('Kopieren fehlgeschlagen'))
  );
}

function pill(state) {
  const p = document.getElementById('status-pill');
  if (!p) return;
  p.innerHTML = '<i class="status-dot"></i>' + localizedText(state);
  p.className = 'status ' + (state === 'Running' ? 'running' : state === 'Error' ? 'error' : '');
}

function displayNumber(value, digits, unit) {
  return value == null || Number.isNaN(Number(value)) ? '—' : Number(value).toFixed(digits) + (unit ? ' ' + unit : '');
}

function updateHistory(points) {
  const line = document.getElementById('history-line');
  const area = document.getElementById('history-area');
  if (!line || !area || !Array.isArray(points) || !points.length) return;
  const values = points.map(p => Number(p.p) || 0);
  const max = Math.max(1, ...values.map(Math.abs));
  const width = 800, base = 185, height = 165;
  const coords = values.map((value, index) => {
    const x = values.length === 1 ? 0 : index * width / (values.length - 1);
    const y = base - Math.max(-1, Math.min(1, value / max)) * height / 2 - height / 2;
    return x.toFixed(1) + ',' + y.toFixed(1);
  });
  line.setAttribute('points', coords.join(' '));
  area.setAttribute('d', 'M' + coords[0] + 'L' + coords.join('L') + 'L800,185L0,185Z');
}

async function fetchHistory() {
  try {
    const r = await fetch('/api/history');
    if (r.ok) updateHistory(await r.json());
  } catch (_) {}
}

function kv(id, val) {
  const el = document.getElementById(id);
  if (el) el.textContent = (val == null || val === '') ? '—' : val;
}

function setRegisterValue(id, valueWh, hasValue) {
  const el = document.getElementById(id);
  if (!el) return;
  if (!hasValue || valueWh == null || !Number.isFinite(Number(valueWh))) {
    el.textContent = '—';
    return;
  }
  const [whole, fraction] = (Number(valueWh) / 1000).toFixed(3).split('.');
  const decimal = document.createElement('span');
  decimal.className = 'register-decimal';
  decimal.textContent = '.' + fraction;
  el.replaceChildren(whole.padStart(7, '0'), decimal);
}

function setRowVisible(id, visible) {
  const el = document.getElementById(id);
  if (!el) return;
  const row = el.closest('tr') || el.closest('.kv');
  if (row) row.hidden = !visible;
}

function setConnectionStatus(online) {
  document.querySelectorAll('.connection-card .status, .sidebar-footer .status').forEach(el => {
    el.classList.toggle('offline', !online);
    const label = el.querySelector('span');
    if (label) label.textContent = online ? 'Verbunden' : 'Getrennt';
  });
}

function updateControlButtons(state) {
  const continuous = state.continuous === true;
  const running = state.job_state === 'Running';
  const singleButton = document.querySelector('button[onclick="doStart()"]');
  const continuousButton = document.querySelector('button[onclick="doStartContinuous()"]');
  const stopButton = document.querySelector('button[onclick="doStop()"]');
  if (singleButton) singleButton.disabled = continuous || running;
  if (continuousButton) {
    continuousButton.disabled = continuous || running;
    continuousButton.textContent = continuous ? 'Dauerlesen aktiv' : '↻ Dauerhaft lesen';
    continuousButton.setAttribute('aria-pressed', String(continuous));
  }
  if (stopButton) stopButton.disabled = !continuous && !running;
}

async function fetchStatus() {
  try {
    const r = await fetch('/api/status');
    if (!r.ok) throw new Error('status request failed');
    const d = await r.json();
    setConnectionStatus(d.wifi_connected === true);
    pill(d.job_state ?? 'Idle');
    updateControlButtons(d);
    const netPower = (Number(d.fwd_w) || 0) - (Number(d.rev_w) || 0);
    const powerValue = document.getElementById('current-power');
    if (powerValue && powerValue.firstChild) powerValue.firstChild.nodeValue = d.has_power ? String(Math.round(netPower)) : '—';
    kv('power-kilowatt', d.has_power ? (netPower / 1000).toFixed(2) + ' kW netto' : null);
    const heroCard = document.querySelector('.hero-card');
    if (heroCard) heroCard.hidden = !d.has_power;
    const meterCard = document.querySelector('.meter-card');
    if (meterCard) meterCard.hidden = !d.has_fwd_active_wh && !d.has_rev_active_wh;
    const kpiGrid = document.querySelector('.kpi-grid');
    if (kpiGrid) kpiGrid.hidden = !d.populated;
    const historyCard = document.querySelector('.chart-card');
    if (historyCard) historyCard.hidden = !d.populated;
    const splitCard = document.querySelector('.split-card');
    if (splitCard) splitCard.hidden = !d.populated;
    kv('today-import', displayNumber(d.today_import_kwh, 2, 'kWh'));
    kv('today-export', displayNumber(d.today_export_kwh, 2, 'kWh'));
    kv('cost-today', displayNumber(d.cost_today, 2, d.currency || 'EUR'));
    const importCost = (Number(d.today_import_kwh) || 0) * (Number(d.price_import_kwh) || 0);
    const exportCredit = (Number(d.today_export_kwh) || 0) * (Number(d.price_export_kwh) || 0);
    kv('cost-breakdown', 'Bezug ' + importCost.toFixed(2) + ' - Einspeisung ' + exportCredit.toFixed(2) + ' ' + (d.currency || 'EUR'));
    kv('legend-import', displayNumber(d.today_import_kwh, 2, 'kWh'));
    kv('legend-month', displayNumber(d.month_import_kwh, 2, 'kWh'));
    kv('table-import', d.has_fwd_active_wh ? displayNumber(Number(d.fwd_active_wh) / 1000, 2, '') : null);
    kv('table-export', d.has_rev_active_wh ? displayNumber(Number(d.rev_active_wh) / 1000, 2, '') : null);
    kv('table-power', d.has_power ? displayNumber(netPower, 0, '') : null);
    kv('table-current-l1', d.has_i_l1 ? displayNumber(Number(d.i_l1_ma) / 1000, 2, '') : null);
    kv('table-current-l2', d.has_i_l2 ? displayNumber(Number(d.i_l2_ma) / 1000, 2, '') : null);
    kv('table-current-l3', d.has_i_l3 ? displayNumber(Number(d.i_l3_ma) / 1000, 2, '') : null);
    setRowVisible('table-import', d.has_fwd_active_wh);
    setRowVisible('table-export', d.has_rev_active_wh);
    setRowVisible('table-power', d.has_power);
    setRowVisible('table-current-l1', d.has_i_l1);
    setRowVisible('table-current-l2', d.has_i_l2);
    setRowVisible('table-current-l3', d.has_i_l3);
    const meterTable = document.querySelector('.table-card');
    if (meterTable) meterTable.hidden = ![d.has_fwd_active_wh, d.has_rev_active_wh, d.has_power, d.has_i_l1, d.has_i_l2, d.has_i_l3].some(Boolean);
    kv('m-mfr',   d.manufacturer);
    kv('m-model', d.model);
    kv('m-fw',    d.fw_version);
    kv('m-ser',   d.serial_bcd);
    kv('m-iser',  d.utility_serial);
    kv('m-time',  d.meter_time);
    setRegisterValue('m-fwh', d.fwd_active_wh, d.has_fwd_active_wh);
    setRegisterValue('m-rwh', d.rev_active_wh, d.has_rev_active_wh);
    kv('m-irh',   d.import_react_varh != null ? d.import_react_varh + ' varh' : null);
    kv('m-erh',   d.export_react_varh != null ? d.export_react_varh + ' varh' : null);
    kv('m-v1',    d.v_l1_mv != null ? (d.v_l1_mv/1000).toFixed(2) + ' V' : null);
    kv('m-v2',    d.v_l2_mv != null ? (d.v_l2_mv/1000).toFixed(2) + ' V' : null);
    kv('m-v3',    d.v_l3_mv != null ? (d.v_l3_mv/1000).toFixed(2) + ' V' : null);
    kv('m-i1',    d.i_l1_ma != null ? (d.i_l1_ma/1000).toFixed(3) + ' A' : null);
    kv('m-i2',    d.i_l2_ma != null ? (d.i_l2_ma/1000).toFixed(3) + ' A' : null);
    kv('m-i3',    d.i_l3_ma != null ? (d.i_l3_ma/1000).toFixed(3) + ' A' : null);
    kv('m-freq',  d.freq_mhz != null ? (d.freq_mhz/1000).toFixed(2) + ' Hz' : null);
    kv('m-pf',    d.pf_l1 != null ? (d.pf_l1/1000).toFixed(3) : null);
    kv('m-alarm', d.has_alarms ? (d.alarm_list || '!') : 'Keine');
    ['m-mfr', 'm-model', 'm-fw', 'm-ser', 'm-iser', 'm-time'].forEach(id => {
      const el = document.getElementById(id);
      setRowVisible(id, !!(el && el.textContent && el.textContent !== '—'));
    });
    const loginRow = document.getElementById('last-login-ok');
    setRowVisible('last-login-ok', d.login_required === true);
    const systemCard = document.querySelector('.system-card');
    if (systemCard) systemCard.hidden = !d.populated && d.login_required !== true && !d.has_alarms;
    // counters
    kv('c-tx', d.tx_bytes); kv('c-rx', d.rx_bytes);
    kv('c-tf', d.tx_frames); kv('c-rf', d.rx_frames);
    kv('c-crc', d.crc_errors); kv('c-nak', d.nak_count);
    kv('c-ack', d.ack_count); kv('c-wk', d.wakeup_count);
    kv('c-fi', d.flag_ident  ? '\u2713' : '\u2717');
    kv('c-fl', d.flag_logon  ? '\u2713' : '\u2717');
    kv('c-fa', d.flag_auth   ? '\u2713' : '\u2717');
    if (d.last_login_ok != null) {
      const el = document.getElementById('last-login-ok');
      if (el) {
        el.textContent = d.last_login_ok ? '\u2713 Letzter Login erfolgreich' : '\u2717 Letzter Login fehlgeschlagen / ausstehend';
        el.style.color = d.last_login_ok ? 'var(--lc-green,#0f0)' : 'var(--lc-orange,#f80)';
      }
    }
    kv('wifi-ip', d.ip); kv('wifi-ipv6', d.ipv6 || localizedText('Warte auf IPv6 vom Router')); kv('wifi-ssid', d.ssid);
    // Show both IPs if both modes active
    if (d.ip_sta && d.ip_ap) {
      kv('wifi-ip',   'STA: ' + d.ip_sta + '  |  AP: ' + d.ip_ap);
      kv('wifi-ssid', d.ssid_sta + '  |  AP: ' + d.ssid_ap);
    } else if (d.ip_sta) {
      kv('wifi-ip', d.ip_sta); kv('wifi-ssid', d.ssid_sta);
    } else if (d.ip_ap) {
      kv('wifi-ip', d.ip_ap); kv('wifi-ssid', 'AP: ' + d.ssid_ap);
    }
    if (d.uptime_s != null) {
      let t = Math.floor(d.uptime_s);
      const s = t % 60; t = Math.floor(t / 60);
      const m = t % 60; t = Math.floor(t / 60);
      const h = t % 24; t = Math.floor(t / 24);
      const d2 = t % 7;  const w = Math.floor(t / 7);
      let parts = [];
      if (w)  parts.push(w  + 'W');
      if (d2) parts.push(d2 + 'T');
      parts.push(String(h).padStart(2,'0') + ':' + String(m).padStart(2,'0') + ':' + String(s).padStart(2,'0'));
      kv('uptime', parts.join(' '));
    }
    if (d.app_version) {
      kv('fw-ver', 'v' + d.app_version);
      kv('fw-ver-ctrl', 'v' + d.app_version);
      const el = document.getElementById('fw-ver-footer');
      if (el) el.textContent = 'v' + d.app_version;
      const overviewVersion = document.getElementById('app-version');
      if (overviewVersion) overviewVersion.textContent = 'v' + d.app_version;
    }
  } catch(_) {
    setConnectionStatus(false);
    _startReconnect();
  }
}

async function fetchLog() {
  try {
    const r = await fetch('/api/log?after=' + logSeq);
    if (!r.ok) return;
    const lines = await r.json();
    // Always advance the sequence so we don't re-fetch old lines after unpause
    for (const l of lines) { if (l.seq > logSeq) logSeq = l.seq; }
    if (logPaused) return;
    const box = document.getElementById('log');
    for (const l of lines) {
      const div = document.createElement('div');
      div.className = 'log-' + l.level;
      div.textContent = '[' + l.level + '] ' + l.tag + ': ' + l.msg;
      box.appendChild(div);
    }
    box.scrollTop = box.scrollHeight;
  } catch(_) {}
}

async function doStart() {
  const r = await fetch('/api/start', {method:'POST'});
  const j = await r.json();
  if (!j.ok && j.reason === 'already_running') {
    alert(localizedText('Ablesung läuft bereits.'));
    return;
  }
  logSeq = 0; // reset log view so we see the new cycle from the start
  fetchStatus();
  fetchLog();
}
async function doStartContinuous() {
  const r = await fetch('/api/start_continuous', {method:'POST'});
  const j = await r.json();
  if (!j.ok && j.reason === 'already_running') {
    alert(localizedText('Ablesung läuft bereits.'));
    return;
  }
  logSeq = 0;
  fetchStatus();
  fetchLog();
}
async function doStop() {
  await fetch('/api/stop', {method:'POST'});
  fetchStatus();
}
async function doReboot() {
  if (!confirm(localizedText('Gerät neu starten?'))) return;
  await fetch('/api/reboot', {method:'POST'});
  _startReconnect();
}
async function doResetCounters() {
  await fetch('/api/reset_counters', {method:'POST'});
  fetchStatus();
}

const PROFILE_FIELDS = [
  'import_wh','export_wh','power_net_w','power_import_w','power_export_w',
  'voltage_l1_v','voltage_l2_v','voltage_l3_v',
  'current_l1_a','current_l2_a','current_l3_a','frequency_hz','pf_l1'
];

const PROFILE_URL_DEFAULT = 'https://raw.githubusercontent.com/<user>/<repo>/main/profiles.json';

const BUILTIN_METER_PROFILES = {
  mt631_ms2020: {
    name: 'Iskraemeco MT631 / MS2020',
    pin_required: false,
    login_cmd: '',
    login_wait_ms: 250,
    obis: {
      import_wh: '1-0:1.8.0*255,1-0:1.8.1*255,1-0:1.8.2*255',
      export_wh: '1-0:2.8.0*255,1-0:2.8.1*255,1-0:2.8.2*255',
      power_net_w: '1-0:16.7.0*255',
      power_import_w: '1-0:1.7.0*255',
      power_export_w: '1-0:2.7.0*255',
      voltage_l1_v: '1-0:32.7.0*255',
      voltage_l2_v: '1-0:52.7.0*255',
      voltage_l3_v: '1-0:72.7.0*255',
      current_l1_a: '1-0:31.7.0*255',
      current_l2_a: '1-0:51.7.0*255',
      current_l3_a: '1-0:71.7.0*255',
      frequency_hz: '1-0:14.7.0*255,1-0:14.4.0*255',
      pf_l1: '1-0:13.7.0*255'
    }
  },
  dtz541: {
    name: 'Holley DTZ541',
    pin_required: false,
    login_cmd: '',
    login_wait_ms: 250,
    obis: {
      import_wh: '1-0:1.8.0*255',
      export_wh: '1-0:2.8.0*255',
      power_net_w: '1-0:16.7.0*255,1-0:1.7.0*255',
      power_import_w: '1-0:1.7.0*255',
      power_export_w: '1-0:2.7.0*255',
      voltage_l1_v: '1-0:32.7.0*255',
      voltage_l2_v: '',
      voltage_l3_v: '',
      current_l1_a: '1-0:31.7.0*255',
      current_l2_a: '',
      current_l3_a: '',
      frequency_hz: '1-0:14.7.0*255',
      pf_l1: '1-0:13.7.0*255'
    }
  },
  ed300l: {
    name: 'EMH ED300L',
    pin_required: false,
    login_cmd: '',
    login_wait_ms: 250,
    obis: {
      import_wh: '1-0:1.8.0*255',
      export_wh: '1-0:2.8.0*255',
      power_net_w: '1-0:16.7.0*255,1-0:1.7.0*255',
      power_import_w: '1-0:1.7.0*255',
      power_export_w: '1-0:2.7.0*255',
      voltage_l1_v: '1-0:32.7.0*255',
      voltage_l2_v: '',
      voltage_l3_v: '',
      current_l1_a: '1-0:31.7.0*255',
      current_l2_a: '',
      current_l3_a: '',
      frequency_hz: '1-0:14.7.0*255',
      pf_l1: '1-0:13.7.0*255'
    }
  },
  ebz_dd3: {
    name: 'eBZ DD3',
    pin_required: false,
    login_cmd: '',
    login_wait_ms: 250,
    obis: {
      import_wh: '1-0:1.8.0*255',
      export_wh: '1-0:2.8.0*255',
      power_net_w: '1-0:16.7.0*255',
      power_import_w: '1-0:1.7.0*255',
      power_export_w: '1-0:2.7.0*255',
      voltage_l1_v: '1-0:32.7.0*255',
      voltage_l2_v: '',
      voltage_l3_v: '',
      current_l1_a: '1-0:31.7.0*255',
      current_l2_a: '',
      current_l3_a: '',
      frequency_hz: '1-0:14.7.0*255',
      pf_l1: '1-0:13.7.0*255'
    }
  },
  landis_e350: {
    name: 'Landis+Gyr E350',
    pin_required: false,
    login_cmd: '',
    login_wait_ms: 300,
    obis: {
      import_wh: '1-0:1.8.0*255,1-0:1.8.1*255',
      export_wh: '1-0:2.8.0*255,1-0:2.8.1*255',
      power_net_w: '1-0:16.7.0*255',
      power_import_w: '1-0:1.7.0*255',
      power_export_w: '1-0:2.7.0*255',
      voltage_l1_v: '1-0:32.7.0*255',
      voltage_l2_v: '1-0:52.7.0*255',
      voltage_l3_v: '1-0:72.7.0*255',
      current_l1_a: '1-0:31.7.0*255',
      current_l2_a: '1-0:51.7.0*255',
      current_l3_a: '1-0:71.7.0*255',
      frequency_hz: '1-0:14.7.0*255',
      pf_l1: '1-0:13.7.0*255'
    }
  },
  easymeter_q3a: {
    name: 'EasyMeter Q3A / Q3D',
    pin_required: false,
    login_cmd: '/?\\r\\n',
    login_wait_ms: 300,
    obis: {
      import_wh: '1-0:1.8.0*255',
      export_wh: '1-0:2.8.0*255',
      power_net_w: '1-0:16.7.0*255',
      power_import_w: '1-0:1.7.0*255',
      power_export_w: '1-0:2.7.0*255',
      voltage_l1_v: '1-0:32.7.0*255',
      voltage_l2_v: '',
      voltage_l3_v: '',
      current_l1_a: '1-0:31.7.0*255',
      current_l2_a: '',
      current_l3_a: '',
      frequency_hz: '1-0:14.7.0*255',
      pf_l1: ''
    }
  },
  generic_three_phase: {
    name: 'Generisch 3-phasig OBIS',
    pin_required: false,
    login_cmd: '',
    login_wait_ms: 250,
    obis: {
      import_wh: '1-0:1.8.0*255',
      export_wh: '1-0:2.8.0*255',
      power_net_w: '1-0:16.7.0*255',
      power_import_w: '1-0:1.7.0*255',
      power_export_w: '1-0:2.7.0*255',
      voltage_l1_v: '1-0:32.7.0*255',
      voltage_l2_v: '1-0:52.7.0*255',
      voltage_l3_v: '1-0:72.7.0*255',
      current_l1_a: '1-0:31.7.0*255',
      current_l2_a: '1-0:51.7.0*255',
      current_l3_a: '1-0:71.7.0*255',
      frequency_hz: '1-0:14.7.0*255',
      pf_l1: '1-0:13.7.0*255'
    }
  },
  generic_single_phase: {
    name: 'Generisch 1-phasig OBIS',
    pin_required: false,
    login_cmd: '',
    login_wait_ms: 250,
    obis: {
      import_wh: '1-0:1.8.0*255',
      export_wh: '1-0:2.8.0*255',
      power_net_w: '1-0:16.7.0*255,1-0:1.7.0*255',
      power_import_w: '1-0:1.7.0*255',
      power_export_w: '1-0:2.7.0*255',
      voltage_l1_v: '1-0:32.7.0*255',
      voltage_l2_v: '',
      voltage_l3_v: '',
      current_l1_a: '1-0:31.7.0*255',
      current_l2_a: '',
      current_l3_a: '',
      frequency_hz: '1-0:14.7.0*255',
      pf_l1: '1-0:13.7.0*255'
    }
  },
  iec62056_pin_mode: {
    name: 'IEC62056 Login mit PIN',
    pin_required: true,
    login_cmd: '/?{PIN}!\\r\\n',
    login_wait_ms: 500,
    obis: {
      import_wh: '1-0:1.8.0*255',
      export_wh: '1-0:2.8.0*255',
      power_net_w: '1-0:16.7.0*255',
      power_import_w: '1-0:1.7.0*255',
      power_export_w: '1-0:2.7.0*255',
      voltage_l1_v: '1-0:32.7.0*255',
      voltage_l2_v: '1-0:52.7.0*255',
      voltage_l3_v: '1-0:72.7.0*255',
      current_l1_a: '1-0:31.7.0*255',
      current_l2_a: '1-0:51.7.0*255',
      current_l3_a: '1-0:71.7.0*255',
      frequency_hz: '1-0:14.7.0*255',
      pf_l1: '1-0:13.7.0*255'
    }
  }
};

function getCustomProfile(slot) {
  try {
    const raw = localStorage.getItem('smleasy.custom_profile_' + slot);
    if (!raw) return null;
    const p = JSON.parse(raw);
    if (!p || !p.obis) return null;
    return p;
  } catch (_) {
    return null;
  }
}

function getRemoteProfiles() {
  try {
    const raw = localStorage.getItem('smleasy.remote_profiles');
    if (!raw) return {};
    const parsed = JSON.parse(raw);
    if (!parsed || typeof parsed !== 'object') return {};
    return parsed;
  } catch (_) {
    return {};
  }
}

function saveRemoteProfiles(profiles) {
  localStorage.setItem('smleasy.remote_profiles', JSON.stringify(profiles || {}));
}

function getRemoteProfileUrl() {
  return localStorage.getItem('smleasy.profile_catalog_url') || '';
}

function saveRemoteProfileUrl(url) {
  localStorage.setItem('smleasy.profile_catalog_url', url || '');
}

function normalizeGitHubRawUrl(url) {
  const u = (url || '').trim();
  if (!u) return '';
  const m = u.match(/^https?:\/\/github\.com\/([^\/]+)\/([^\/]+)\/blob\/([^\/]+)\/(.+)$/i);
  if (m) {
    return 'https://raw.githubusercontent.com/' + m[1] + '/' + m[2] + '/' + m[3] + '/' + m[4];
  }
  return u;
}

function emptyObisMap() {
  return {
    import_wh:'', export_wh:'', power_net_w:'', power_import_w:'', power_export_w:'',
    voltage_l1_v:'', voltage_l2_v:'', voltage_l3_v:'', current_l1_a:'', current_l2_a:'',
    current_l3_a:'', frequency_hz:'', pf_l1:''
  };
}

function sanitizeProfileProfile(entry, fallbackName) {
  const p = entry || {};
  const obis = emptyObisMap();
  for (const key of PROFILE_FIELDS) {
    const v = p.obis && p.obis[key] != null ? p.obis[key] : '';
    obis[key] = String(v);
  }

  const placeholders = Array.isArray(p.placeholders)
    ? p.placeholders.filter(x => typeof x === 'string' && x.length > 0)
    : [];

  return {
    name: String(p.name || fallbackName || 'Externes Profil'),
    pin_required: !!p.pin_required,
    login_cmd: String(p.login_cmd || ''),
    login_wait_ms: Number.isFinite(parseInt(p.login_wait_ms, 10)) ? parseInt(p.login_wait_ms, 10) : 250,
    meter_pin: p.meter_pin != null ? String(p.meter_pin) : undefined,
    key_hint: p.key_hint != null ? String(p.key_hint) : '',
    key_value: p.key_value != null ? String(p.key_value) : undefined,
    placeholders,
    obis
  };
}

function parseExternalProfilesJson(data) {
  const out = {};
  if (!data) return out;

  if (Array.isArray(data)) {
    for (let i = 0; i < data.length; i++) {
      const e = data[i] || {};
      const id = e.id ? String(e.id) : ('profile_' + (i + 1));
      out[id] = sanitizeProfileProfile(e, e.name || id);
    }
    return out;
  }

  if (Array.isArray(data.profiles)) {
    for (let i = 0; i < data.profiles.length; i++) {
      const e = data.profiles[i] || {};
      const id = e.id ? String(e.id) : ('profile_' + (i + 1));
      out[id] = sanitizeProfileProfile(e, e.name || id);
    }
    return out;
  }

  if (data.profiles && typeof data.profiles === 'object') {
    for (const [id, e] of Object.entries(data.profiles)) {
      out[String(id)] = sanitizeProfileProfile(e, e && e.name ? e.name : String(id));
    }
    return out;
  }

  if (typeof data === 'object') {
    for (const [id, e] of Object.entries(data)) {
      out[String(id)] = sanitizeProfileProfile(e, e && e.name ? e.name : String(id));
    }
  }
  return out;
}

async function loadProfilesFromUrl() {
  const input = document.getElementById('profile-url-input');
  const status = document.getElementById('profile-url-status');
  if (!input) return;

  const urlRaw = (input.value || '').trim();
  if (!urlRaw) {
    alert(localizedText('Bitte eine URL eintragen.'));
    return;
  }
  const url = normalizeGitHubRawUrl(urlRaw);
  input.value = url;
  saveRemoteProfileUrl(url);

  if (status) status.textContent = 'Lade Profilkatalog ...';

  try {
    const r = await fetch(url, { cache: 'no-store' });
    if (!r.ok) throw new Error('HTTP ' + r.status);
    const data = await r.json();
    const parsed = parseExternalProfilesJson(data);
    const count = Object.keys(parsed).length;
    if (!count) throw new Error('Keine Profile im JSON gefunden');

    saveRemoteProfiles(parsed);
    renderProfileOptions();
    if (status) status.textContent = count + ' ' + localizedText('External profiles loaded');
  } catch (err) {
    if (status) status.textContent = 'Fehler beim Laden';
    alert(localizedText('Profile catalog could not be loaded: ') + (err && err.message ? err.message : err));
  }
}

function saveCustomProfile(slot, profile) {
  localStorage.setItem('smleasy.custom_profile_' + slot, JSON.stringify(profile));
}

function getAllProfiles() {
  const merged = Object.assign({}, BUILTIN_METER_PROFILES);
  const remote = getRemoteProfiles();
  for (const [rid, rp] of Object.entries(remote)) {
    merged['ext_' + rid] = rp;
  }
  const c1 = getCustomProfile(1);
  const c2 = getCustomProfile(2);
  merged.custom1 = c1 || {
    name: 'Custom 1 (leer)', pin_required: false, login_cmd: '', login_wait_ms: 250,
    obis: emptyObisMap()
  };
  merged.custom2 = c2 || {
    name: 'Custom 2 (leer)', pin_required: false, login_cmd: '', login_wait_ms: 250,
    obis: emptyObisMap()
  };
  return merged;
}

function renderProfileOptions() {
  const sel = document.getElementById('meter-profile-select');
  if (!sel) return;
  const keep = sel.value;
  const profiles = getAllProfiles();
  const c1Name = document.getElementById('custom-profile-name-1');
  const c2Name = document.getElementById('custom-profile-name-2');
  if (c1Name) c1Name.value = (profiles.custom1 && profiles.custom1.name) ? profiles.custom1.name : '';
  if (c2Name) c2Name.value = (profiles.custom2 && profiles.custom2.name) ? profiles.custom2.name : '';
  const ids = Object.keys(profiles);
  sel.innerHTML = '';
  for (const id of ids) {
    const o = document.createElement('option');
    o.value = id;
    o.textContent = (id.indexOf('ext_') === 0 ? '[GitHub] ' : '') + profiles[id].name;
    sel.appendChild(o);
  }
  if (ids.includes(keep)) sel.value = keep;
}

function applyProfileById(profileId) {
  const f = document.getElementById('meter-form');
  if (!f) return;
  const profiles = getAllProfiles();
  const p = profiles[profileId];
  if (!p) return;

  for (const key of PROFILE_FIELDS) {
    const field = f['obis_' + key];
    if (field) field.value = (p.obis && p.obis[key]) ? p.obis[key] : '';
  }

  if (f.login_cmd && typeof p.login_cmd === 'string') f.login_cmd.value = p.login_cmd;
  if (f.login_wait_ms && p.login_wait_ms != null) f.login_wait_ms.value = p.login_wait_ms;
  if (f.meter_pin && p.meter_pin) f.meter_pin.value = p.meter_pin;

  const hint = document.getElementById('profile-pin-hint');
  if (hint) {
    const parts = [];
    if (p.pin_required) {
      parts.push('Dieses Profil benötigt typischerweise eine PIN. Nutze {PIN} im Login-Befehl.');
    } else {
      parts.push('PIN optional. Bei Bedarf {PIN} im Login-Befehl verwenden.');
    }
    if (p.key_hint) parts.push('Key-Hinweis: ' + p.key_hint);
    if (Array.isArray(p.placeholders) && p.placeholders.length) {
      parts.push('Platzhalter: ' + p.placeholders.join(', '));
    }
    hint.textContent = parts.join(' ');
  }

  const sel = document.getElementById('meter-profile-select');
  if (sel) sel.value = profileId;
}

function applySelectedProfile() {
  const sel = document.getElementById('meter-profile-select');
  if (!sel) return;
  applyProfileById(sel.value);
}

function saveCurrentAsCustom(slot) {
  const f = document.getElementById('meter-form');
  if (!f) return;
  const nameInput = document.getElementById('custom-profile-name-' + slot);
  const customName = nameInput && nameInput.value.trim().length
    ? nameInput.value.trim()
    : ('Custom ' + slot);

  const obis = {};
  for (const key of PROFILE_FIELDS) {
    const field = f['obis_' + key];
    obis[key] = field ? field.value.trim() : '';
  }

  const profile = {
    name: customName,
    pin_required: (f.login_cmd.value || '').indexOf('{PIN}') >= 0,
    login_cmd: (f.login_cmd.value || ''),
    login_wait_ms: parseInt((f.login_wait_ms.value || '250'), 10) || 250,
    obis
  };
  saveCustomProfile(slot, profile);
  renderProfileOptions();
  const sel = document.getElementById('meter-profile-select');
  if (sel) sel.value = 'custom' + slot;
  alert(localizedText('Custom profile ') + slot + localizedText(' saved.'));
}

async function saveMeter(e) {
  e.preventDefault();
  const fd = new FormData(e.target);
  const obis = {
    import_wh:      (fd.get('obis_import_wh')      || '').toString().trim(),
    export_wh:      (fd.get('obis_export_wh')      || '').toString().trim(),
    power_net_w:    (fd.get('obis_power_net_w')    || '').toString().trim(),
    power_import_w: (fd.get('obis_power_import_w') || '').toString().trim(),
    power_export_w: (fd.get('obis_power_export_w') || '').toString().trim(),
    voltage_l1_v:   (fd.get('obis_voltage_l1_v')   || '').toString().trim(),
    voltage_l2_v:   (fd.get('obis_voltage_l2_v')   || '').toString().trim(),
    voltage_l3_v:   (fd.get('obis_voltage_l3_v')   || '').toString().trim(),
    current_l1_a:   (fd.get('obis_current_l1_a')   || '').toString().trim(),
    current_l2_a:   (fd.get('obis_current_l2_a')   || '').toString().trim(),
    current_l3_a:   (fd.get('obis_current_l3_a')   || '').toString().trim(),
    frequency_hz:   (fd.get('obis_frequency_hz')   || '').toString().trim(),
    pf_l1:          (fd.get('obis_pf_l1')          || '').toString().trim(),
  };

  // Basic format validation: 1-0:1.8.0*255,1-0:1.8.1*255
  const entryRx = /^\d{1,3}-\d{1,3}:\d{1,3}\.\d{1,3}\.\d{1,3}\*\d{1,3}$/;
  for (const [key, value] of Object.entries(obis)) {
    if (!value) continue;
    const parts = value.split(/[;,\s]+/).filter(Boolean);
    for (const p of parts) {
      if (!entryRx.test(p)) {
        alert(localizedText('Invalid OBIS format for ') + key + ': ' + p + '\n' + localizedText('Expected: 1-0:1.8.0*255'));
        return;
      }
    }
  }

  const body = JSON.stringify({
    interval_s:   parseInt(fd.get('interval_s'), 10),
    uart_debug:        fd.get('uart_debug')        === 'on',
    login_cmd:         (fd.get('login_cmd') || '').toString(),
    login_wait_ms:     parseInt((fd.get('login_wait_ms') || '250').toString(), 10),
    clear_meter_pin:   fd.get('clear_meter_pin') === 'on',
    obis,
  });
  const pin = (fd.get('meter_pin') || '').toString();
  const payload = JSON.parse(body);
  if (pin.length > 0) payload.meter_pin = pin;
  const r = await fetch('/api/config/meter', {method:'POST',
    headers:{'Content-Type':'application/json'}, body: JSON.stringify(payload)});
  alert(r.ok ? localizedText('Meter settings saved') : localizedText('Fehler beim Speichern'));
}

function applyDefaultObisProfile() {
  applyProfileById('mt631_ms2020');
}


async function saveWifi(e) {
  e.preventDefault();
  const fd = new FormData(e.target);
  const body = JSON.stringify({ ssid: fd.get('ssid'), password: fd.get('password') });
  const r = await fetch('/api/config/wifi', {method:'POST',
    headers:{'Content-Type':'application/json'}, body});
  const result = r.ok ? await r.json() : {ok:false};
  alert(result.ok && result.connected
    ? localizedText('Wi-Fi saved and connected.')
    : result.ok ? localizedText('Wi-Fi saved, connection failed. AP remains active.') : localizedText('Fehler beim Speichern'));
}

async function saveWebPassword(e) {
  e.preventDefault();
  const password = e.target.password.value;
  const confirmation = e.target.password_confirm.value;
  if (password.length < 8 || password !== confirmation) {
    alert(localizedText('Password must match and be at least 8 characters.'));
    return;
  }
  const r = await fetch('/api/config/auth', {method:'POST', headers:{'Content-Type':'application/json'}, body:JSON.stringify({password})});
  if (r.ok && (await r.json()).ok) {
    e.target.reset();
    alert(localizedText('Password saved. The new password will be used on next access.'));
  } else alert(localizedText('Password could not be saved.'));
}

async function scanWifi() {
  const button = document.getElementById('wifi-scan-btn');
  const select = document.getElementById('wifi-network-select');
  const status = document.getElementById('wifi-scan-status');
  if (!button || !select) return;
  button.disabled = true;
  if (status) status.textContent = localizedText('Searching for networks ...');
  try {
    const r = await fetch('/api/wifi/scan');
    const data = await r.json();
    select.innerHTML = '<option value="">Netzwerk auswählen</option>';
    (data.networks || []).sort((a, b) => b.rssi - a.rssi).forEach(network => {
      const option = document.createElement('option');
      option.value = network.ssid;
      option.textContent = network.ssid + '  (' + network.rssi + ' dBm)';
      select.appendChild(option);
    });
    if (status) status.textContent = (data.networks || []).length + ' ' + localizedText('networks found');
  } catch (_) {
    if (status) status.textContent = localizedText('Scan failed');
  } finally {
    button.disabled = false;
  }
}

function selectWifiNetwork() {
  const select = document.getElementById('wifi-network-select');
  const input = document.querySelector('#wifi-form input[name="ssid"]');
  if (select && input && select.value) input.value = select.value;
}

async function fetchTariffCfg() {
  try {
    const r = await fetch('/api/config/tariff');
    if (!r.ok) return;
    const d = await r.json();
    const f = document.getElementById('tariff-form');
    if (!f) return;
    f.price_import_kwh.value = d.price_import_kwh ?? 0.30;
    f.price_export_kwh.value = d.price_export_kwh ?? 0.0664;
    f.currency.value = d.currency ?? 'EUR';
  } catch (_) {}
}

async function saveTariff(e) {
  e.preventDefault();
  const fd = new FormData(e.target);
  const body = JSON.stringify({
    price_import_kwh: Number(fd.get('price_import_kwh')),
    price_export_kwh: Number(fd.get('price_export_kwh')),
    currency: fd.get('currency')
  });
  const r = await fetch('/api/config/tariff', {method:'POST', headers:{'Content-Type':'application/json'}, body});
  alert(r.ok && (await r.json()).ok ? localizedText('Tariffs saved.') : localizedText('Could not save tariffs.'));
}

async function fetchHaCfg() {
  try {
    const r = await fetch('/api/config/ha');
    if (!r.ok) return;
    const d = await r.json();
    const f = document.getElementById('ha-form');
    if (!f) return;
    f.enabled.checked      = d.enabled      ?? false;
    f.broker_uri.value     = d.broker_uri   ?? 'mqtt://192.168.1.1';
    f.username.value       = d.username     ?? '';
    f.device_name.value    = d.device_name  ?? 'Smartmeter';
    f.ha_prefix.value      = d.ha_prefix    ?? 'homeassistant';
    const pill = document.getElementById('ha-status-pill');
    if (pill) pill.textContent = d.connected ? '● ' + localizedText('Verbunden') : '○ ' + localizedText('Getrennt');
  } catch(_) {}
}

async function saveHa(e) {
  e.preventDefault();
  const fd = new FormData(e.target);
  const body = JSON.stringify({
    enabled:      e.target.enabled.checked,
    broker_uri:   fd.get('broker_uri'),
    username:     fd.get('username'),
    password:     fd.get('password') || undefined,
    device_name:  fd.get('device_name'),
    ha_prefix:    fd.get('ha_prefix'),
  });
  const r = await fetch('/api/config/ha', {method:'POST',
    headers:{'Content-Type':'application/json'}, body});
  const j = r.ok ? await r.json() : null;
  alert(j && j.ok ? localizedText('HA integration saved.') : localizedText('Fehler beim Speichern'));
  fetchHaCfg();
}

async function loadConfig() {
  try {
    const r = await fetch('/api/config/meter');
    if (!r.ok) return;
    const d = await r.json();
    const f = document.getElementById('meter-form');
    if (!f) return;
    f.interval_s.value = d.interval_s ?? 30;
    f.uart_debug.checked      = d.uart_debug      ?? false;
    f.login_cmd.value = d.login_cmd ?? '';
    f.login_wait_ms.value = d.login_wait_ms ?? 250;
    f.clear_meter_pin.checked = false;
    const pinInfo = document.getElementById('meter-pin-info');
    if (pinInfo) pinInfo.textContent = d.has_meter_pin ? localizedText('PIN is set') : localizedText('PIN is not set');
    const ob = d.obis || {};
    f.obis_import_wh.value      = ob.import_wh      ?? '';
    f.obis_export_wh.value      = ob.export_wh      ?? '';
    f.obis_power_net_w.value    = ob.power_net_w    ?? '';
    f.obis_power_import_w.value = ob.power_import_w ?? '';
    f.obis_power_export_w.value = ob.power_export_w ?? '';
    f.obis_voltage_l1_v.value   = ob.voltage_l1_v   ?? '';
    f.obis_voltage_l2_v.value   = ob.voltage_l2_v   ?? '';
    f.obis_voltage_l3_v.value   = ob.voltage_l3_v   ?? '';
    f.obis_current_l1_a.value   = ob.current_l1_a   ?? '';
    f.obis_current_l2_a.value   = ob.current_l2_a   ?? '';
    f.obis_current_l3_a.value   = ob.current_l3_a   ?? '';
    f.obis_frequency_hz.value   = ob.frequency_hz   ?? '';
    f.obis_pf_l1.value          = ob.pf_l1          ?? '';
    const profileUrlInput = document.getElementById('profile-url-input');
    if (profileUrlInput) {
      profileUrlInput.placeholder = PROFILE_URL_DEFAULT;
      profileUrlInput.value = getRemoteProfileUrl();
    }
    renderProfileOptions();
    const sel = document.getElementById('meter-profile-select');
    if (sel && !sel.value) sel.value = 'mt631_ms2020';
  } catch(_) {}
}

let githubOtaTimer = null;

async function loadGithubVersions() {
  const select = document.getElementById('github-ota-select');
  const status = document.getElementById('github-ota-result');
  const loadButton = document.getElementById('github-ota-load');
  if (!select || !status) return;
  if (loadButton) loadButton.disabled = true;
  status.textContent = localizedText('Firmware-Versionen werden geladen …');
  select.replaceChildren();
  try {
    const response = await fetch('https://api.github.com/repos/ip6constructor/SMLEasy/releases?per_page=20', {
      headers: { Accept: 'application/vnd.github+json' },
      cache: 'no-store'
    });
    if (!response.ok) throw new Error('HTTP ' + response.status);
    const releases = await response.json();
    for (const release of releases) {
      if (release.draft) continue;
      for (const asset of release.assets || []) {
        const match = asset.name.match(/^SMLEasy-(\d+\.\d+\.\d+)\.bin$/);
        if (!match || !asset.browser_download_url) continue;
        const option = document.createElement('option');
        option.value = asset.browser_download_url;
        option.dataset.version = match[1];
        option.textContent = 'v' + match[1] + (release.prerelease ? ' (preview)' : '');
        select.appendChild(option);
      }
    }
    status.textContent = select.options.length
      ? select.options.length + ' ' + localizedText('GitHub-Versionen geladen')
      : localizedText('Keine passende Firmware gefunden.');
  } catch (_) {
    status.textContent = localizedText('Fehler beim Laden der GitHub-Versionen.');
  } finally {
    if (loadButton) loadButton.disabled = false;
  }
}

async function installGithubVersion() {
  const select = document.getElementById('github-ota-select');
  const status = document.getElementById('github-ota-result');
  const installButton = document.getElementById('github-ota-install');
  if (!select || !select.value || !status) {
    alert(localizedText('Bitte zuerst eine GitHub-Version laden.'));
    return;
  }
  const version = select.selectedOptions[0].dataset.version;
  if (!confirm(localizedText('Installation wirklich starten?') + ' v' + version)) return;
  if (installButton) installButton.disabled = true;
  status.textContent = localizedText('Firmware-Download gestartet …');
  try {
    const response = await fetch('/api/ota/github', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ url: select.value })
    });
    const result = response.ok ? await response.json() : null;
    if (!result || !result.ok) throw new Error('OTA request failed');
    watchGithubOtaStatus();
  } catch (_) {
    status.textContent = localizedText('GitHub-OTA fehlgeschlagen.');
    if (installButton) installButton.disabled = false;
  }
}

function watchGithubOtaStatus() {
  const status = document.getElementById('github-ota-result');
  const installButton = document.getElementById('github-ota-install');
  clearInterval(githubOtaTimer);
  githubOtaTimer = setInterval(async () => {
    try {
      const response = await fetch('/api/ota/github/status');
      if (!response.ok) throw new Error('status unavailable');
      const result = await response.json();
      if (result.state === 'downloading' && status)
        status.textContent = localizedText('Firmware-Update läuft …');
      else if (result.state === 'restarting') {
        if (status) status.textContent = localizedText('Gerät startet mit neuer Firmware neu …');
        clearInterval(githubOtaTimer);
      } else if (result.state === 'failed') {
        if (status) status.textContent = localizedText('GitHub-OTA fehlgeschlagen.');
        if (installButton) installButton.disabled = false;
        clearInterval(githubOtaTimer);
      }
    } catch (_) {
      if (status) status.textContent = localizedText('Gerät startet mit neuer Firmware neu …');
      clearInterval(githubOtaTimer);
    }
  }, 2000);
}

// OTA upload
function startOta(file) {
  if (!file) return;
  const bar = document.getElementById('ota-bar');
  const res = document.getElementById('ota-result');
  const btn = document.getElementById('ota-btn');
  btn.disabled = true;
  bar.style.width = '0%';
  res.textContent = localizedText('Transferring …');
  const xhr = new XMLHttpRequest();
  xhr.upload.addEventListener('progress', ev => {
    if (ev.lengthComputable)
      bar.style.width = (ev.loaded / ev.total * 100) + '%';
  });
  xhr.onload = () => {
    bar.style.width = '100%';
    btn.disabled = false;
    res.textContent = xhr.status === 200
      ? localizedText('Update successful! Device is restarting …')
      : localizedText('Error: ') + xhr.responseText;
  };
  xhr.onerror = () => { btn.disabled = false; res.textContent = localizedText('Connection error'); };
  xhr.open('POST', '/api/ota');
  xhr.send(file);
}
document.addEventListener('DOMContentLoaded', () => {
  initUiLanguage();
  renderProfileOptions();
  if (document.getElementById('github-ota-select')) loadGithubVersions();
  if (document.getElementById('access-networks-form')) loadAccessNetworks();
  if (document.getElementById('tls-config-form')) loadTlsConfig();
  const otaForm = document.getElementById('ota-form');
  if (otaForm) {
    otaForm.addEventListener('submit', (e) => {
      e.preventDefault();
      startOta(document.getElementById('ota-file').files[0]);
    });
  }
  if (document.getElementById('meter-form')) loadConfig();
  if (document.getElementById('tariff-form')) fetchTariffCfg();
  if (document.getElementById('ha-form')) fetchHaCfg();
  fetchStatus();
  fetchHistory();
  fetchLog();
  statusTimer = setInterval(fetchStatus, 5000);
  setInterval(fetchHistory, 30000);
  logTimer    = setInterval(fetchLog, 30000);
});
)rawjs";

static const char kIndexHtml[] = R"rawhtml(<!DOCTYPE html>
<html lang="de"><head><meta charset="UTF-8"><meta name="viewport" content="width=device-width,initial-scale=1"><title>SMLEasy Dashboard</title><link rel="stylesheet" href="/style.css"><link rel="icon" href="/favicon.ico" type="image/svg+xml"></head>
<body><div class="app">
<aside class="sidebar"><div class="brand">)rawhtml" R"rawsvg(<svg viewBox="0 0 100 100" xmlns="http://www.w3.org/2000/svg" aria-hidden="true"><defs><linearGradient id="smlGh" x1="0%" y1="0%" x2="100%" y2="100%"><stop offset="0%" stop-color="#22d3ee"/><stop offset="100%" stop-color="#2563eb"/></linearGradient></defs><circle cx="50" cy="50" r="42" fill="none" stroke="#132238" stroke-width="9"/><path d="M50 8 A42 42 0 0 1 90.5 40M8 55 A42 42 0 0 0 40 91.5M12 40 A42 42 0 0 1 30 15" fill="none" stroke="url(#smlGh)" stroke-width="9" stroke-linecap="round"/><path d="M72 30 L58 55 H68 L60 78 L80 50 H69 Z" fill="url(#smlGh)"/></svg>)rawsvg" R"rawhtml(<div>EasySML<small>Smart energy</small></div></div>
<div class="nav-title">Übersicht</div><a class="nav-item active" href="#dashboard"><svg viewBox="0 0 24 24"><rect x="3" y="3" width="7" height="7"/><rect x="14" y="3" width="7" height="7"/><rect x="3" y="14" width="7" height="7"/><rect x="14" y="14" width="7" height="7"/></svg><span>Dashboard</span></a><a class="nav-item" href="#meter-values"><svg viewBox="0 0 24 24"><path d="M4 19V5m0 14h16M8 16v-4m4 4V8m4 8V5"/></svg><span>Zählerwerte</span></a><a class="nav-item" href="#history"><svg viewBox="0 0 24 24"><path d="M4 18 9 12l4 3 7-9"/><path d="M4 4v14h17"/></svg><span>Verlauf</span></a><div class="nav-title" style="margin-top:24px">Konfiguration</div><a class="nav-item" href="/config"><svg viewBox="0 0 24 24"><path d="M12 15.5a3.5 3.5 0 1 0 0-7 3.5 3.5 0 0 0 0 7Z"/><path d="m19.4 15 .1.1a2 2 0 1 1-2.8 2.8l-.1-.1a2 2 0 0 0-3.4 1.4v.3a2 2 0 1 1-4 0v-.2A2 2 0 0 0 5.8 18l-.1.1a2 2 0 1 1-2.8-2.8l.1-.1A2 2 0 0 0 1.6 12H1.5a2 2 0 1 1 0-4h.2A2 2 0 0 0 3 4.6l-.1-.1a2 2 0 1 1 2.8-2.8l.1.1A2 2 0 0 0 9.2.5v-.1a2 2 0 1 1 4 0v.2A2 2 0 0 0 16.6 2l.1-.1a2 2 0 1 1 2.8 2.8l-.1.1A2 2 0 0 0 20.8 8h.2a2 2 0 1 1 0 4h-.2a2 2 0 0 0-1.4 3Z"/></svg><span>Verbindung & Einstellungen</span></a><div class="sidebar-footer"><div class="status"><i class="status-dot"></i><span>Verbunden</span></div><div style="margin-top:8px">EasySML <span id="fw-ver-footer">v1.0</span></div></div></aside>
<div class="workspace"><header class="topbar"><div><div class="eyebrow">Smart meter / Übersicht</div><h1>Dashboard</h1></div><div class="topbar-meta"><span id="fw-ver">—</span><span id="status-pill" class="status"><i class="status-dot"></i>Idle</span></div></header>
<main id="dashboard" class="main-content"><div class="page-intro"><div><div class="eyebrow">Live monitoring</div><h2>Verbrauch auf einen Blick</h2><p>Aktuelle Messwerte und Energiefluss deines Zählers.</p></div><button class="btn btn-blue" onclick="doStart()">&#9654; Jetzt auslesen</button></div>
<section class="grid">
<article class="card hero-card"><h2>Aktuelle Leistung</h2><div class="metric-value" id="current-power">—<span class="metric-unit">W</span></div><div class="metric-sub" id="power-kilowatt">— kW netto</div></article>
<article class="card meter-card register-card"><div class="register-heading"><h2>Zählerstände</h2><span class="register-live">Zähler Live</span></div><div class="register-subtitle">Bezug und Einspeisung als Zählwerke</div><div class="register-meter"><div class="register-brand"><span>ZÄHLWERKE · OBIS 1.8.0 / 2.8.0</span></div><div class="register-displays"><div class="register-window"><div class="register-label">1.8.0 / BEZUG GESAMT</div><span class="register-reading" id="m-fwh">—</span><div class="register-unit">kWh</div></div><div class="register-window"><div class="register-label">2.8.0 / EINSPEISUNG GESAMT</div><span class="register-reading" id="m-rwh">—</span><div class="register-unit">kWh</div></div></div></div><div class="register-time"><span>Zählerzeit</span><span class="val" id="m-time">—</span></div></article>
<article class="card connection-card"><h2>Verbindung</h2><div class="status"><i class="status-dot"></i><span>—</span></div><div class="kv" style="margin-top:20px"><span class="lbl">IPv4</span><span class="val" id="wifi-ip">—</span></div><div class="kv"><span class="lbl">IPv6</span><span class="val" id="wifi-ipv6">—</span></div><div class="kv"><span class="lbl">WLAN</span><span class="val" id="wifi-ssid">—</span></div><div class="kv"><span class="lbl">Uptime</span><span class="val" id="uptime">—</span></div></article>
<div class="kpi-grid"><article class="card kpi-card"><div class="label">Verbrauch heute</div><div class="kpi-value" id="today-import">—</div><div class="kpi-change positive">Importierte Energie</div></article><article class="card kpi-card"><div class="label">Einspeisung heute</div><div class="kpi-value" id="today-export">—</div><div class="kpi-change">Exportierte Energie</div></article><article class="card kpi-card"><div class="label">Nettokosten heute</div><div class="kpi-value" id="cost-today">—</div><div class="kpi-change" id="cost-breakdown">Bezug minus Einspeisevergütung</div></article></div>
<article id="history" class="card chart-card"><h2>Leistungsverlauf <span style="float:right;font-weight:400;letter-spacing:0;text-transform:none">Letzte Messpunkte</span></h2><div class="chart-wrap"><svg viewBox="0 0 800 220" preserveAspectRatio="none" aria-label="Leistungsverlauf"><path class="chart-grid" d="M0 20H800M0 75H800M0 130H800M0 185H800"/><path id="history-area" class="chart-area" d="M0 185L0 185L800 185Z"/><polyline id="history-line" class="chart-line" points="0,185 800,185"/></svg></div></article>
<article class="card split-card"><h2>Verbrauchsaufteilung</h2><div class="donut" aria-label="Verbrauchsaufteilung"></div><div class="legend"><span><b><i style="background:var(--primary)"></i>Bezug heute</b><strong id="legend-import">—</strong></span><span><b><i style="background:#8bdcf4"></i>Monat gesamt</b><strong id="legend-month">—</strong></span></div></article>
<article id="meter-values" class="card table-card"><h2>Aktuelle Zählerwerte</h2><table class="data-table"><thead><tr><th>OBIS</th><th>Beschreibung</th><th style="text-align:right">Wert</th><th>Einheit</th></tr></thead><tbody><tr><td>1.8.0</td><td>Bezug gesamt</td><td class="value" id="table-import">—</td><td>kWh</td></tr><tr><td>2.8.0</td><td>Einspeisung gesamt</td><td class="value" id="table-export">—</td><td>kWh</td></tr><tr><td>16.7.0</td><td>Aktuelle Leistung</td><td class="value" id="table-power">—</td><td>W</td></tr><tr><td>31.7.0</td><td>Strom L1</td><td class="value" id="table-current-l1">—</td><td>A</td></tr><tr><td>51.7.0</td><td>Strom L2</td><td class="value" id="table-current-l2">—</td><td>A</td></tr><tr><td>71.7.0</td><td>Strom L3</td><td class="value" id="table-current-l3">—</td><td>A</td></tr></tbody></table></article>
<article class="card system-card"><h2>Systemstatus</h2><div class="system-list"><div class="kv"><span class="lbl">Hersteller</span><span class="val" id="m-mfr">—</span></div><div class="kv"><span class="lbl">Modell</span><span class="val" id="m-model">—</span></div><div class="kv"><span class="lbl">Firmware</span><span class="val" id="m-fw">—</span></div><div class="kv"><span class="lbl">Seriennr.</span><span class="val" id="m-ser">—</span></div><div class="kv"><span class="lbl">Login</span><span class="val" id="last-login-ok">—</span></div><div class="kv"><span class="lbl">Alarme</span><span class="val" id="m-alarm">Keine</span></div></div></article>
<article class="card control-card"><h2>Steuerung</h2><div class="card-inner"><div class="btn-row"><button class="btn btn-blue" onclick="doStartContinuous()">&#8635; Dauerhaft lesen</button><button class="btn btn-red" onclick="doStop()">&#9646;&#9646; Stoppen</button><button class="btn btn-gray" onclick="doResetCounters()">Zähler reset</button><button class="btn btn-gray" onclick="doReboot()">Neustart</button></div><div class="control-meta"><div class="kv"><span class="lbl">Firmware</span><span class="val" id="fw-ver-ctrl">—</span></div><a href="/config" class="btn btn-gray">Konfiguration</a></div></div></article>
</section><footer class="overview-info">Copyright Michael Kreutzer 2026 <span>·</span> Lizenz: GNU GPLv3 <span>·</span> Version <strong id="app-version">—</strong></footer></main></div></div><script src="/app.js"></script></body></html>
)rawhtml";

static const char kConfigHtml[] = R"rawhtml(<!DOCTYPE html>
<html lang="de">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>SMLEasy — Konfiguration</title>
<link rel="stylesheet" href="/style.css">
<link rel="icon" href="/favicon.ico" type="image/svg+xml">
</head>
<body>
<header>
  <div class="lcars-header-elbow sml-logo">)rawhtml" R"rawsvg(<svg viewBox="0 0 100 100" xmlns="http://www.w3.org/2000/svg" role="img" aria-label="SMLEasy"><defs><linearGradient id="smlGc" x1="0%" y1="0%" x2="100%" y2="100%"><stop offset="0%" stop-color="#22d3ee"/><stop offset="100%" stop-color="#2563eb"/></linearGradient></defs><circle cx="50" cy="50" r="42" fill="none" stroke="#0b1220" stroke-width="9"/><path d="M50 8 A42 42 0 0 1 90.5 40" fill="none" stroke="url(#smlGc)" stroke-width="9" stroke-linecap="round"/><path d="M8 55 A42 42 0 0 0 40 91.5" fill="none" stroke="url(#smlGc)" stroke-width="9" stroke-linecap="round"/><path d="M12 40 A42 42 0 0 1 30 15" fill="none" stroke="url(#smlGc)" stroke-width="9" stroke-linecap="round" opacity=".85"/><rect x="34" y="55" width="7" height="16" rx="1.5" fill="url(#smlGc)"/><rect x="44" y="48" width="7" height="23" rx="1.5" fill="url(#smlGc)"/><rect x="54" y="40" width="7" height="31" rx="1.5" fill="url(#smlGc)"/><path d="M72 30 L58 55 H68 L60 78 L80 50 H69 Z" fill="url(#smlGc)"/></svg>)rawsvg" R"rawhtml(</div>
  <div class="lcars-header-bar">
    <h1>SMLEasy &mdash; Konfiguration</h1>
    <a href="/" class="btn btn-gray" style="text-decoration:none">← Dashboard</a>
  </div>
</header>
<main>
  <div class="card peach">
    <h2 class="tan">Sprache</h2>
    <div class="card-inner">
      <label for="ui-language-select">Sprache für die Oberfläche</label>
      <select id="ui-language-select" onchange="saveUiLanguagePreference(this.value)">
        <option value="auto">Browser (automatisch)</option>
        <option value="en">English</option>
        <option value="de">Deutsch</option>
        <option value="nl">Nederlands</option>
        <option value="fr">Français</option>
        <option value="pl">Polski</option>
      </select>
      <div class="kv" style="margin-top:8px"><span class="val" id="ui-language-status">Browser bevorzugt; EN ist der Fallback.</span></div>
    </div>
  </div>

  <div class="card peach">
    <h2 class="tan">Zugriff auf das Zähler-Frontend</h2>
    <div class="card-inner">
      <form id="access-networks-form" onsubmit="saveAccessNetworks(event)">
        <label for="allowed-networks">Zusätzliche erlaubte CIDR-Netze</label>
        <textarea id="allowed-networks" rows="4" maxlength="512" placeholder="192.168.1.0/24&#10;2a00:6020:a105:c900::/64"></textarea>
        <p style="margin-top:8px;color:var(--lc-dim);font-size:.78rem">Ein CIDR-Netz pro Zeile. Private IPv4- und IPv6-Netze sind immer erlaubt.</p>
        <p style="margin-top:4px;color:var(--lc-dim);font-size:.74rem">Beispiele: 192.168.1.0/24 oder 2a00:6020:a105:c900::/64</p>
        <div class="save-row"><button class="btn btn-blue" type="submit">Netzwerke speichern</button></div>
      </form>
    </div>
  </div>

  <!-- Meter config -->
  <div class="card peach">
    <h2 class="tan">Zähler-Konfiguration</h2>
    <div class="card-inner">
    <form id="meter-form" onsubmit="saveMeter(event)">
      <label>Leseintervall (s)</label>
      <input type="number" name="interval_s" min="5" max="3600" value="30">
      <div style="margin-top:10px;display:flex;flex-direction:column;gap:6px">
        <label style="display:flex;align-items:center;gap:8px;color:var(--lc-peach);font-size:.88rem">
          <input type="checkbox" name="uart_debug"> UART Raw-Debug (hex dump TX/RX)
        </label>
      </div>
      <div class="lcars-bar"></div>
      <label>Login-Befehl vor Auslesung (optional)</label>
      <input type="text" name="login_cmd" maxlength="256" placeholder="/?!\\r\\n oder /?{PIN}!\\r\\n">
      <label>PIN (optional, nur beim Speichern gesendet)</label>
      <input type="password" name="meter_pin" maxlength="64" autocomplete="new-password" placeholder="(leer lassen = unverändert)">
      <div class="kv" style="margin-top:6px"><span class="lbl">PIN-Status</span><span class="val" id="meter-pin-info">—</span></div>
      <label style="display:flex;align-items:center;gap:8px;color:var(--lc-peach);font-size:.88rem">
        <input type="checkbox" name="clear_meter_pin"> Gespeicherten PIN löschen
      </label>
      <label>Wartezeit nach Login-Befehl (ms)</label>
      <input type="number" name="login_wait_ms" min="20" max="5000" value="250">
      <div class="lcars-bar"></div>
      <div style="display:flex;justify-content:space-between;align-items:center;gap:8px;flex-wrap:wrap">
        <label style="margin:0;color:var(--lc-peach);font-size:.86rem;letter-spacing:.06em">OBIS-Mapping (Alias-Liste per Komma)</label>
        <button class="btn btn-gray" type="button" onclick="applyDefaultObisProfile()">Default: MT631/MS2020</button>
      </div>
      <div style="display:grid;grid-template-columns:1fr auto;gap:8px;margin-top:8px;align-items:end">
        <div>
          <label>Profil auswählen</label>
          <select id="meter-profile-select"></select>
        </div>
        <button class="btn btn-blue" type="button" onclick="applySelectedProfile()">Profil anwenden</button>
      </div>
      <div style="display:grid;grid-template-columns:1fr auto;gap:8px;margin-top:8px;align-items:end">
        <div>
          <label>GitHub Profil-URL (JSON)</label>
          <input type="url" id="profile-url-input" placeholder="https://raw.githubusercontent.com/<user>/<repo>/main/profiles.json">
        </div>
        <button class="btn btn-gray" type="button" onclick="loadProfilesFromUrl()">Von URL laden</button>
      </div>
      <div class="kv" style="margin-top:6px"><span class="lbl">Katalog-Status</span><span class="val" id="profile-url-status">Keine externen Profile geladen</span></div>
      <div class="kv" style="margin-top:8px"><span class="lbl">Profil-Hinweis</span><span class="val" id="profile-pin-hint">PIN optional. Bei Bedarf {PIN} im Login-Befehl verwenden.</span></div>
      <div style="display:grid;grid-template-columns:1fr 1fr;gap:8px;margin-top:8px">
        <div>
          <label>Custom 1 Name</label>
          <input type="text" id="custom-profile-name-1" maxlength="48" placeholder="Custom 1">
          <div class="save-row"><button class="btn btn-gray" type="button" onclick="saveCurrentAsCustom(1)">Aktuelle Werte als Custom 1 speichern</button></div>
        </div>
        <div>
          <label>Custom 2 Name</label>
          <input type="text" id="custom-profile-name-2" maxlength="48" placeholder="Custom 2">
          <div class="save-row"><button class="btn btn-gray" type="button" onclick="saveCurrentAsCustom(2)">Aktuelle Werte als Custom 2 speichern</button></div>
        </div>
      </div>
      <label>Import Energie (Wh)</label>
      <input type="text" name="obis_import_wh" maxlength="192" placeholder="1-0:1.8.0*255,1-0:1.8.1*255">
      <label>Export Energie (Wh)</label>
      <input type="text" name="obis_export_wh" maxlength="192" placeholder="1-0:2.8.0*255,1-0:2.8.1*255">
      <label>Nettoleistung (W, signed)</label>
      <input type="text" name="obis_power_net_w" maxlength="96" placeholder="1-0:16.7.0*255">
      <label>Bezug Leistung (W)</label>
      <input type="text" name="obis_power_import_w" maxlength="96" placeholder="1-0:1.7.0*255">
      <label>Einspeisung Leistung (W)</label>
      <input type="text" name="obis_power_export_w" maxlength="96" placeholder="1-0:2.7.0*255">
      <label>Spannung L1/L2/L3 (V)</label>
      <input type="text" name="obis_voltage_l1_v" maxlength="96" placeholder="1-0:32.7.0*255">
      <input type="text" name="obis_voltage_l2_v" maxlength="96" placeholder="1-0:52.7.0*255">
      <input type="text" name="obis_voltage_l3_v" maxlength="96" placeholder="1-0:72.7.0*255">
      <label>Strom L1/L2/L3 (A)</label>
      <input type="text" name="obis_current_l1_a" maxlength="96" placeholder="1-0:31.7.0*255">
      <input type="text" name="obis_current_l2_a" maxlength="96" placeholder="1-0:51.7.0*255">
      <input type="text" name="obis_current_l3_a" maxlength="96" placeholder="1-0:71.7.0*255">
      <label>Frequenz (Hz)</label>
      <input type="text" name="obis_frequency_hz" maxlength="128" placeholder="1-0:14.7.0*255,1-0:14.4.0*255">
      <label>Leistungsfaktor L1</label>
      <input type="text" name="obis_pf_l1" maxlength="96" placeholder="1-0:13.7.0*255">
      <div class="save-row"><button class="btn btn-blue" type="submit">Speichern</button></div>
    </form>
    </div>
  </div>

  <!-- WiFi config -->
  <div class="card peach">
    <h2 class="tan">WLAN-Konfiguration</h2>
    <div class="card-inner">
    <form id="wifi-form" onsubmit="saveWifi(event)">
      <div style="display:flex;gap:8px;align-items:end">
        <div style="flex:1"><label>Verfügbare WLANs</label><select id="wifi-network-select" onchange="selectWifiNetwork()"><option value="">Netzwerk auswählen</option></select></div>
        <button class="btn btn-gray" id="wifi-scan-btn" type="button" onclick="scanWifi()">&#8635; Scannen</button>
      </div>
      <div id="wifi-scan-status" class="kv" style="margin-top:8px"><span class="lbl">Scanstatus</span><span class="val">Noch nicht gescannt</span></div>
      <label>SSID</label>
      <input type="text" name="ssid" maxlength="32">
      <label>Passwort</label>
      <input type="password" name="password" maxlength="64" autocomplete="new-password">
      <div class="save-row"><button class="btn btn-blue" type="submit">Speichern</button></div>
    </form>
    </div>
  </div>

  <!-- Tariff config -->
  <div class="card peach">
    <h2 class="tan">Tarife</h2>
    <div class="card-inner">
    <form id="tariff-form" onsubmit="saveTariff(event)">
      <label>Bezug / Verbrauch (EUR/kWh)</label>
      <input type="number" name="price_import_kwh" min="0" step="0.0001" value="0.30" required>
      <label>Einspeisung (EUR/kWh)</label>
      <input type="number" name="price_export_kwh" min="0" step="0.0001" value="0.0664" required>
      <label>Währung</label>
      <input type="text" name="currency" maxlength="8" value="EUR" required>
      <div class="save-row"><button class="btn btn-blue" type="submit">Tarife speichern</button></div>
    </form>
    </div>
  </div>

  <!-- Home Assistant config -->
  <div class="card peach">
    <h2 class="tan">Home Assistant Integration</h2>
    <div class="card-inner">
    <div class="kv" style="margin-bottom:4px"><span class="lbl">Status</span><span class="val" id="ha-status-pill">—</span></div>
    <form id="ha-form" onsubmit="saveHa(event)">
      <label style="display:flex;align-items:center;gap:8px;margin-bottom:6px">
        <input type="checkbox" name="enabled"> Aktiviert
      </label>
      <label>MQTT Broker URI</label>
      <input type="text" name="broker_uri" maxlength="128" placeholder="mqtt://192.168.1.1">
      <label>Benutzername</label>
      <input type="text" name="username" maxlength="64" autocomplete="username">
      <label>Passwort</label>
      <input type="password" name="password" maxlength="64" autocomplete="new-password" placeholder="(leer lassen = unverändert)">
      <label>Gerätename</label>
      <input type="text" name="device_name" maxlength="64" placeholder="Smartmeter">
      <label>HA Discovery-Präfix</label>
      <input type="text" name="ha_prefix" maxlength="64" placeholder="homeassistant">
      <div class="save-row"><button class="btn btn-blue" type="submit">Speichern</button></div>
    </form>
    </div>
  </div>

  <!-- Web authentication -->
  <div class="card peach">
    <h2 class="tan">Weboberfläche schützen</h2>
    <div class="card-inner">
    <div class="kv"><span class="lbl">Benutzername</span><span class="val">admin</span></div>
    <form id="web-auth-form" onsubmit="saveWebPassword(event)">
      <label>Neues Passwort</label>
      <input type="password" name="password" minlength="8" autocomplete="new-password" required>
      <label>Neues Passwort wiederholen</label>
      <input type="password" name="password_confirm" minlength="8" autocomplete="new-password" required>
      <div class="save-row"><button class="btn btn-blue" type="submit">Passwort ändern</button></div>
    </form>
    </div>
  </div>

  <div class="card peach">
    <h2 class="tan">Zertifikat und HTTPS</h2>
    <div class="card-inner">
      <form id="tls-config-form" onsubmit="saveTlsConfig(event)">
        <label for="tls-mode">Zertifikatsmodus</label>
        <select id="tls-mode" name="mode" onchange="updateTlsMode(this.value)">
          <option value="manual">Manueller PEM-Import</option>
          <option value="letsencrypt">Let's Encrypt (HTTP-01)</option>
        </select>
        <label for="tls-fqdn">FQDN / Hostname</label>
        <input type="text" id="tls-fqdn" maxlength="253" placeholder="zaehler.example.net" required>
        <label for="tls-email">Kontakt-E-Mail für Let’s Encrypt</label>
        <input type="email" id="tls-email" maxlength="254" placeholder="admin@example.net" required>
        <label for="tls-renewal-days">Erneuerungsintervall (Tage)</label>
        <input type="number" id="tls-renewal-days" min="1" max="60" value="60" required>
        <p style="margin-top:6px;color:var(--lc-dim);font-size:.78rem">Let’s Encrypt empfiehlt bei 90 Tagen Laufzeit eine Erneuerung alle 60 Tage.</p>
        <section id="le-prerequisites" hidden style="margin-top:12px">
          <label for="tls-acme-environment">ACME-Umgebung</label>
          <select id="tls-acme-environment" onchange="updateTlsMode(document.getElementById('tls-mode').value)">
            <option value="staging">Staging (Test, Browserwarnung)</option>
            <option value="production">Produktion (gültiges Browser-Zertifikat)</option>
          </select>
          <p id="acme-staging-warning" style="margin-top:6px;color:var(--lc-orange);font-size:.78rem">Staging-Zertifikate sind nur für Tests und werden von normalen Browsern nicht als vertrauenswürdig erkannt.</p>
          <h3 style="font-size:.9rem;margin:12px 0 8px">Voraussetzungen für Let's Encrypt</h3>
          <ol style="padding-left:22px;color:var(--lc-dim);font-size:.82rem;line-height:1.55">
            <li>Vor Erstanforderung und jeder Erneuerung muss der FQDN öffentlich per A (IPv4) oder AAAA (IPv6) auf die aktuelle Adresse des ESP zeigen.</li>
            <li>Am Router: IPv4-TCP-Port 80 direkt auf ESP-Port 80 weiterleiten; kein abweichender Zielport.</li>
            <li>Bei AAAA: IPv6-Port 80 direkt in der Router-Firewall für die globale IPv6-Adresse des ESP freigeben; IPv6 übersetzt keine Ports.</li>
            <li>Port 80 dient nur der HTTP-01-Challenge von Let’s Encrypt; Dashboard und Konfiguration laufen über HTTPS auf Port 443.</li>
            <li>DNS und Routerfreigabe müssen auch bei jeder Erneuerung erreichbar sein.</li>
          </ol>
          <p style="margin-top:10px;color:var(--lc-dim);font-size:.78rem">Nach der Erstausstellung versucht das Gerät die Erneuerung im konfigurierten Intervall; ACME verwendet dabei HTTP-01 auf Port 80.</p>
          <button class="btn btn-blue" id="acme-request-button" type="button" onclick="requestAcmeCertificate()">Let’s-Encrypt-Zertifikat anfordern</button>
          <div id="acme-request-status" style="margin-top:8px;font-size:.82rem;color:var(--lc-dim)"></div>
        </section>
        <label for="tls-certificate" style="margin-top:12px">Zertifikatskette (Fullchain PEM)</label>
        <textarea id="tls-certificate" rows="7" maxlength="3800" placeholder="-----BEGIN CERTIFICATE-----&#10;...&#10;-----END CERTIFICATE-----"></textarea>
        <label for="tls-private-key">Privater Schlüssel (PEM)</label>
        <textarea id="tls-private-key" rows="6" maxlength="3800" autocomplete="off" placeholder="-----BEGIN PRIVATE KEY-----&#10;...&#10;-----END PRIVATE KEY-----"></textarea>
        <div class="save-row"><button class="btn btn-blue" type="submit">TLS-Konfiguration speichern</button></div>
        <div id="tls-save-status" style="margin-top:8px;font-size:.82rem;color:var(--lc-dim)"></div>
      </form>
    </div>
  </div>

  <!-- OTA -->
  <div class="card orange" style="grid-column:1/-1">
    <h2 class="orange">OTA-Update (Firmware)</h2>
    <div class="card-inner">
    <label for="github-ota-select">Verfügbare GitHub-Versionen</label>
    <div style="display:flex;gap:8px;align-items:end;flex-wrap:wrap">
      <select id="github-ota-select" style="flex:1;min-width:220px"></select>
      <button class="btn btn-gray" id="github-ota-load" type="button" onclick="loadGithubVersions()">GitHub-Versionen laden</button>
      <button class="btn btn-blue" id="github-ota-install" type="button" onclick="installGithubVersion()">Ausgewählte Version installieren</button>
    </div>
    <div id="github-ota-result" style="margin-top:8px;font-size:.82rem;color:var(--primary-dark)"></div>
    <div class="lcars-bar" style="margin:14px 0"></div>
    <p style="font-size:.82rem;color:var(--lc-dim);margin-bottom:10px">
      <strong style="color:var(--lc-peach)">Hinweis an den Admin:</strong>
      Die Firmware nutzt kein Safeboot und ist f&uuml;r das direkte Flashen ab Adresse
      <code style="color:var(--lc-orange)">0x000000</code> vorgesehen.
    </p>
    <form id="ota-form">
      <input type="file" id="ota-file" accept=".bin">
      <div class="progress" style="margin-top:8px"><div class="progress-bar" id="ota-bar"></div></div>
      <div id="ota-result"></div>
      <div class="save-row"><button class="btn btn-blue" id="ota-btn" type="button" onclick="startOta(document.getElementById('ota-file').files[0])">Flashen</button></div>
    </form>
    </div>
  </div>
</main>
<script src="/app.js"></script>
</body>
</html>
)rawhtml";

} // namespace app
