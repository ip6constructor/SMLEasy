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
.grid>.hero-card{grid-column:1/span 6;grid-row:1}.grid>.meter-card{grid-column:7/span 6;grid-row:1}
.network-addresses{flex-direction:column;gap:2px;padding:0 8px 28px}
.network-address-row{display:flex;flex-wrap:wrap;justify-content:center;align-items:baseline;gap:0 6px;width:100%;text-align:center}
.network-address-row .lbl{color:var(--text-muted);font-size:.76rem;font-weight:400;letter-spacing:0;text-transform:none;white-space:nowrap}
.side-conn{margin-top:8px;display:flex;flex-direction:column;gap:2px;font-size:.74rem}.side-conn .kv{display:flex;gap:6px;justify-content:space-between}.side-conn .val{color:var(--text-secondary);text-align:right;word-break:break-all}a[data-cfg-tab].active{background:var(--primary,#168cff);color:#fff}
.network-ip-list{display:inline-flex;flex-wrap:wrap;justify-content:center;align-items:baseline;min-width:0;color:var(--text-muted);font-family:"Cascadia Code","JetBrains Mono",monospace;font-size:.76rem;font-variant-numeric:tabular-nums}
.network-ip-list span{min-width:0;overflow-wrap:anywhere}
.network-ip-separator{white-space:pre}
.continuous-control{display:inline-flex;align-items:center;gap:12px;min-height:42px;padding:0 14px;border:1px solid var(--border);border-radius:var(--radius-md);color:var(--text-primary);background:var(--bg-card);font-size:.86rem;font-weight:650;cursor:pointer}
.continuous-switch{position:relative;display:inline-flex;width:42px;height:24px;flex:0 0 42px}
.continuous-switch input{position:absolute;width:1px;height:1px;opacity:0}
.continuous-switch-track{position:absolute;inset:0;border-radius:999px;background:#aab8c8;transition:background .18s}
.continuous-switch-track:before{content:"";position:absolute;left:3px;top:3px;width:18px;height:18px;border-radius:50%;background:#fff;box-shadow:0 1px 3px rgba(19,34,56,.25);transition:transform .18s}
.continuous-switch input:checked+.continuous-switch-track{background:var(--success)}
.continuous-switch input:checked+.continuous-switch-track:before{transform:translateX(18px)}
.continuous-switch input:focus-visible+.continuous-switch-track{outline:3px solid rgba(22,140,255,.3);outline-offset:2px}
.continuous-switch input:disabled+.continuous-switch-track{opacity:.55}
@media(max-width:1100px){.grid>.hero-card,.grid>.meter-card{grid-column:1/-1;grid-row:auto}}
.btn:disabled{color:var(--text-muted);background:#e6ebf0;border-color:var(--border);box-shadow:none;cursor:not-allowed;filter:none;transform:none}.acme-section{margin-top:14px;padding:12px 14px;border:1px solid var(--border);border-radius:10px;background:rgba(22,140,255,.03)}.acme-section h3{margin:0 0 6px;font-size:.78rem;letter-spacing:.06em;text-transform:uppercase;color:var(--text-secondary)}
.check-list{margin:8px 0}.check-row{display:grid;grid-template-columns:28px 1fr;gap:2px 8px;padding:8px 0;border-bottom:1px solid var(--border);font-size:.84rem}.check-icon{font-weight:700;text-align:center}.check-detail{grid-column:2;color:var(--text-secondary);font-size:.76rem;overflow-wrap:anywhere}.check-row.pass .check-icon{color:var(--success)}.check-row.fail .check-icon{color:var(--danger)}.check-row.warn .check-icon{color:var(--warning)}.check-row.running .check-icon{color:var(--primary)}.check-row.pending .check-icon,.check-row.skipped .check-icon{color:var(--text-muted)}
.ext-row{display:grid;grid-template-columns:1.3fr .9fr 1.6fr .5fr .9fr 1fr;gap:8px;margin-bottom:8px}.ext-row input,.ext-row select{min-width:0}.ext-head{color:var(--text-muted);font-size:.68rem;font-weight:700;letter-spacing:.06em;text-transform:uppercase;margin-bottom:6px}@media(max-width:760px){.ext-row{grid-template-columns:1fr 1fr}.ext-head{display:none}}
.ext-card{grid-column:1/-1}.ext-grid{display:grid;grid-template-columns:repeat(3,minmax(0,1fr));gap:12px}@media(max-width:900px){.ext-grid{grid-template-columns:repeat(2,minmax(0,1fr))}}@media(max-width:560px){.ext-grid{grid-template-columns:1fr}}.ext-tile{padding:12px 14px;border:1px solid var(--border);border-radius:12px;background:rgba(255,255,255,.7)}.ext-tile-label{font-size:.7rem;font-weight:700;letter-spacing:.06em;text-transform:uppercase;color:var(--text-secondary)}.ext-tile-value{margin-top:8px;font-family:"Cascadia Code","JetBrains Mono",monospace;font-size:1.45rem;font-weight:650;font-variant-numeric:tabular-nums}.ext-tile-age{margin-top:6px;font-size:.68rem;color:var(--text-muted)}.ext-bar{height:6px;margin-top:8px;border-radius:6px;background:var(--border);overflow:hidden}.ext-bar i{display:block;height:100%;background:linear-gradient(90deg,var(--primary),var(--primary-light))}.ext-stale .ext-tile-value{color:var(--text-muted)}.ext-error{border-color:var(--danger)}.ext-error .ext-tile-value{color:var(--text-muted)}.ext-waiting .ext-tile-value{color:var(--text-muted)}.kpi-grid .kpi-card{background-repeat:no-repeat;background-position:right 18px center;background-size:auto 80%}.kpi-grid .kpi-card:nth-child(1){background-image:url("data:image/svg+xml,%3Csvg xmlns=%22http://www.w3.org/2000/svg%22 viewBox=%220 0 120 120%22 fill=%22none%22 stroke=%22%23168cff%22 stroke-opacity=%22.28%22 stroke-width=%223%22 stroke-linecap=%22round%22 stroke-linejoin=%22round%22%3E%3Cpath d=%22M60 8 36 112M60 8l24 104M22 36h76M32 70h56M38 36l44 34M82 36 38 70M4 36h18M98 36h18%22/%3E%3C/svg%3E")}.kpi-grid .kpi-card:nth-child(2){background-image:url("data:image/svg+xml,%3Csvg xmlns=%22http://www.w3.org/2000/svg%22 viewBox=%220 0 140 110%22%3E%3Ccircle cx=%22112%22 cy=%2224%22 r=%2212%22 fill=%22%23f5b301%22 fill-opacity=%22.45%22/%3E%3Cg stroke=%22%23f5b301%22 stroke-opacity=%22.45%22 stroke-width=%223%22 stroke-linecap=%22round%22%3E%3Cpath d=%22M112 4v6M112 38v6M92 24h6M126 24h6M98 10l4 4M122 34l4 4M126 10l-4 4M102 34l-4 4%22/%3E%3C/g%3E%3Cpath d=%22M18 100 36 48h86l-18 52z%22 fill=%22%23168cff%22 fill-opacity=%22.16%22 stroke=%22%23168cff%22 stroke-opacity=%22.38%22 stroke-width=%223%22 stroke-linejoin=%22round%22/%3E%3Cpath d=%22M27 74h86M32 61h86M54 48 40 100M76 48 64 100M98 48 88 100%22 stroke=%22%23168cff%22 stroke-opacity=%22.38%22 stroke-width=%222%22/%3E%3C/svg%3E")}.kpi-grid .kpi-card:nth-child(3){background-image:url("data:image/svg+xml,%3Csvg xmlns=%22http://www.w3.org/2000/svg%22 viewBox=%220 0 120 120%22%3E%3Ccircle cx=%2260%22 cy=%2260%22 r=%2246%22 fill=%22%232e9e5b%22 fill-opacity=%22.12%22 stroke=%22%232e9e5b%22 stroke-opacity=%22.38%22 stroke-width=%224%22/%3E%3Ctext x=%2260%22 y=%2280%22 text-anchor=%22middle%22 font-family=%22Arial,sans-serif%22 font-size=%2262%22 font-weight=%22700%22 fill=%22%232e9e5b%22 fill-opacity=%22.4%22%3E%26%238364%3B%3C/text%3E%3C/svg%3E")}.pbar{position:relative;height:16px;margin-top:22px;border-radius:9px;background:var(--border);overflow:hidden}.pbar-fill{height:100%;width:0;border-radius:9px;background:linear-gradient(90deg,#2f7de1,#6fb5ff);transition:width .6s ease}.pbar-fill.export{background:linear-gradient(90deg,#2e9e5b,#7fd6a0)}.pbar-scale{display:flex;justify-content:space-between;margin-top:6px;font-size:.72rem;color:var(--text-secondary)}.pstats{display:grid;grid-template-columns:repeat(3,1fr);gap:10px;margin-top:18px;padding-top:14px;border-top:1px solid var(--border)}.pstats span{display:block;font-size:.68rem;font-weight:700;letter-spacing:.05em;text-transform:uppercase;color:var(--text-secondary)}.pstats b{display:block;margin-top:4px;font-family:"Cascadia Code","JetBrains Mono",monospace;font-size:1.05rem}.register-row{display:flex;align-items:center;gap:12px;margin-top:10px}.register-card .register-reading{flex:1;margin:0;padding:0;border:0;border-radius:0;background:none;box-shadow:none;text-shadow:none;color:var(--text-primary);font-size:1.25rem;font-weight:700;letter-spacing:0;text-align:right}.register-reading .register-decimal{color:inherit}.register-reading .ext-zero{color:var(--text-muted);font-weight:500}.register-window{overflow:visible}.register-meter{background:linear-gradient(145deg,rgba(22,140,255,.09),rgba(22,140,255,.03));border-color:rgba(22,140,255,.28);border-radius:14px}.register-card .register-window{background:linear-gradient(145deg,rgba(22,140,255,.09),rgba(22,140,255,.03));border-color:rgba(22,140,255,.28);border-radius:12px}.ext-grid .ext-tile{background:linear-gradient(145deg,rgba(22,140,255,.09),rgba(22,140,255,.03));border-color:rgba(22,140,255,.28);border-radius:14px}.ext-row{display:flex;align-items:center;gap:12px;margin-top:10px}.ext-ico{flex:none;width:42px;height:42px;border-radius:50%;display:flex;align-items:center;justify-content:center}.ext-counter{display:flex;align-items:baseline;gap:8px;flex:1;min-width:0;justify-content:space-between}.ext-num{font-family:"Cascadia Code","JetBrains Mono",monospace;font-size:1.1rem;font-weight:700;color:var(--text-primary);font-variant-numeric:tabular-nums;white-space:nowrap}.ext-zero{color:var(--text-muted);font-weight:500}.ext-unit{font-size:.82rem;color:var(--text-secondary)}.ext-big .ext-today{justify-content:flex-end;align-items:baseline;gap:8px;margin-top:10px;padding-top:8px;border-top:1px solid rgba(22,140,255,.2);font-size:.85rem;font-weight:600;color:var(--text-secondary)}.ext-big .ext-today b{font-size:1.1rem;font-weight:700;color:var(--text-primary)}.ext-big .ext-tile-value{margin-top:0}.batt{display:flex;align-items:center;gap:14px;margin-top:12px}.batt-cells{display:flex;flex-direction:column-reverse;gap:3px;padding:3px;border:2px solid var(--text-secondary);border-radius:6px;position:relative}.batt-cells:after{content:"";position:absolute;top:-7px;left:30%;height:4px;width:40%;border-radius:2px 2px 0 0;background:var(--text-secondary)}.batt-cells i{width:34px;height:12px;border-radius:2px;background:var(--border)}.batt-cells i.red{background:#d93025}.batt-cells i.yellow{background:#f0b400}.batt-cells i.green{background:#2e9e5b}.batt-car{font-size:1.1rem;line-height:1;margin-bottom:2px}.batt-pct{margin-left:6px;font-family:"Cascadia Code","JetBrains Mono",monospace;font-size:1.3rem;font-weight:650}.batt-trend{font-size:1.8rem;font-weight:800;line-height:1;min-width:1.2ch;text-align:center}.batt-trend.up{color:#2e9e5b}.batt-trend.down{color:#d93025}.ext-today{margin-top:8px;display:flex;justify-content:space-between;font-size:.78rem;color:var(--text-secondary)}.ext-today b{font-family:"Cascadia Code","JetBrains Mono",monospace;color:var(--text-primary)}.ext-tile .register-reading{margin-top:8px}.ext-tile .register-unit{margin-top:4px}

body.eh-full{overflow:hidden}
body.eh-full #energy-view{position:fixed;inset:0;z-index:1000;background:#000;display:flex;align-items:center;justify-content:center}
body.eh-full .eh-bar,body.eh-full #energy-view .page-intro{display:none}
body.eh-full .eh-stage{width:min(100vw,calc(100vh*var(--ehr,1.7778)));border-radius:0;border:0}
.eh-menu{display:none;position:absolute;top:10px;left:10px;z-index:5;width:42px;height:42px;padding:0;border-radius:12px;border:1px solid rgba(148,163,184,.4);background:rgba(8,15,30,.7);color:#fff;cursor:pointer;align-items:center;justify-content:center}
.eh-menu svg{width:24px;height:24px;stroke:currentColor;fill:none;stroke-width:2;stroke-linecap:round}
body.eh-full .eh-menu{display:flex}
#eh-fs{left:62px}
#energy-view{display:none}
body.view-energy #energy-view{display:block}
body.view-energy .page-intro,body.view-energy section.grid{display:none}
.eh-bar{display:flex;gap:8px;flex-wrap:wrap;align-items:center;margin:0 0 10px}
.eh-bar button{padding:6px 12px;border-radius:8px;border:1px solid #334155;background:#0f172a;color:#e2e8f0;cursor:pointer}
.eh-bar button.on{background:#2563eb}
.eh-clock{margin-left:auto;font-variant-numeric:tabular-nums;color:#94a3b8}
.eh-stage{position:relative;width:100%;aspect-ratio:16/9;border-radius:14px;overflow:hidden;background:#0b1220 center/cover no-repeat;border:1px solid #1e293b;touch-action:none}
#eh-flows{position:absolute;inset:0;width:100%;height:100%;pointer-events:none}
.eh-flow{fill:none;stroke-linecap:round;stroke-dasharray:2 7;animation:ehflow 1s linear infinite;filter:drop-shadow(0 0 3px currentColor)}
@keyframes ehflow{to{stroke-dashoffset:-9}}
.eh-tile{position:absolute;transform:translate(-50%,-50%);min-width:120px;max-width:30%;padding:8px 12px;border-radius:12px;background:rgba(8,15,30,.78);border:1px solid rgba(148,163,184,.35);color:#e2e8f0;backdrop-filter:blur(3px);font-size:clamp(11px,1.3vw,15px);line-height:1.35;user-select:none}
.eh-tile b{display:block;font-size:1.5em;color:#fff}
.eh-tile small{color:#94a3b8;display:block}
.eh-tile h3{margin:0 0 2px;font-size:.95em;color:#cbd5e1}
body.eh-edit .eh-tile{cursor:move;outline:2px dashed #38bdf8}
.eh-ic{width:1.5em;height:1.5em;fill:none;stroke:currentColor;stroke-width:1.8;stroke-linecap:round;stroke-linejoin:round;flex:none}
.eh-top{display:flex;align-items:center;gap:.6em}
.eh-top .eh-ic{width:2.6em;height:2.6em}
.eh-top h3{margin:0;font-size:.95em;color:#e2e8f0}
.eh-top small{display:block}
.eh-val{display:block;font-size:1.6em;font-weight:700;line-height:1.15}
.eh-rows{border-top:1px solid rgba(148,163,184,.3);margin-top:6px;padding-top:4px}
.eh-row{display:flex;justify-content:space-between;gap:1em;color:#94a3b8;white-space:nowrap}
.eh-row span:last-child{color:#fff}
.eh-hd{display:flex;align-items:center;gap:.45em;margin-bottom:2px}
.eh-hd h3{margin:0}
.eh-ar{display:block;font-weight:700;font-size:.95em}
.eh-kpis div{display:grid;grid-template-columns:auto 1fr;align-items:center;column-gap:.6em}
.eh-kpis .eh-ic{grid-row:1/3;width:2em;height:2em}
.eh-kpis{position:absolute;left:4px;right:4px;bottom:4px;display:grid;grid-template-columns:repeat(4,minmax(0,1fr));gap:8px;padding:12px 20px;border-radius:12px;background:rgba(0,0,0,.93);font-size:clamp(14px,2.4vw,28px)}
.eh-kpis div{padding:0}
.eh-kpis small{display:block;color:#94a3b8}
.eh-kpis div{color:#e2e8f0}
.eh-kpis b{font-size:1.3em;color:#fff}
.eh-kpis[data-error]::after{content:attr(data-error);grid-column:1/-1;color:#fca5a5;font-size:.65em}
.eh-pbar{height:.8em;border-radius:3px;background:rgba(148,163,184,.25);margin:6px 0 2px;overflow:hidden}.eh-pbar i{display:block;height:100%;background:#fbbf24}
.eh-soc{display:flex;gap:3px;margin:6px 0 2px}
.eh-flowbar{position:relative;height:1.3em;margin:8px 0 4px;background:rgba(148,163,184,.18);overflow:hidden;clip-path:polygon(0 0,calc(100% - 1em) 0,100% 50%,calc(100% - 1em) 100%,0 100%)}
.eh-flowbar.l{clip-path:polygon(1em 0,100% 0,100% 100%,1em 100%,0 50%)}
.eh-flowbar i{position:absolute;inset:0;background:repeating-linear-gradient(90deg,var(--c) 0 .9em,transparent .9em 1.5em);background-size:1.5em 100%;animation:ehrun .9s linear infinite}
.eh-flowbar.l i{animation-direction:reverse}.eh-flowbar.idle i{animation:none;background:rgba(148,163,184,.25)}
@keyframes ehrun{to{background-position:1.5em 0}}
.eh-tile[data-k=grid]{min-width:230px}.eh-tile[data-k=grid] .eh-row{font-size:1.25em;padding:3px 0}.eh-tile[data-k=grid] .eh-row span:last-child{font-weight:700}
.eh-gauge{display:block;width:130px;height:130px;margin:6px auto 2px}.eh-gauge path{fill:none;stroke-width:8}.eh-gauge .bg{stroke:#334155}.eh-gauge .fl{transition:stroke-dasharray .9s ease}.eh-gauge circle{transition:fill .5s}
.eh-soc i{flex:1;height:.8em;border-radius:3px;background:rgba(148,163,184,.25)}
.eh-soc i.red{background:#ef4444}.eh-soc i.yellow{background:#f59e0b}.eh-soc i.green{background:#22c55e}
)rawcss";

static const char kAppJs[] = R"rawjs(
'use strict';
let logSeq = 0;
let statusTimer, logTimer;
let logPaused = false;
let controlRequestPending = false;
let selectedMeterProfileId = 'mt631_ms2020';
let selectedMeterProfileData = null;
let previousMeterProfile = null;

// ── Reconnect state ───────────────────────────────────────────────────────────
let _offline = false;
let _reconnectTimer = null;
let _reconnectDeadline = 0;
let _reconnectCountdown = null;

// ── Server-announced pause (certificate check/request) ───────────────────────
let _pauseUntil = 0;
let _pauseProbe = null;
const _nativeFetch = window.fetch.bind(window);
window.fetch = async function(input, init) {
  const url = typeof input === 'string' ? input : (input && input.url) || '';
  if (_pauseUntil && Date.now() < _pauseUntil && url.indexOf('/api/acme/') < 0) throw new TypeError('paused');
  const r = await _nativeFetch(input, init);
  const p = Number(r.headers.get('X-Pause'));
  if (p > 0) holdForPause(p);
  return r;
};
function holdForPause(seconds) {
  if (_pauseProbe) return;
  _pauseUntil = Date.now() + seconds * 1000;
  clearInterval(statusTimer);
  clearInterval(logTimer);
  clearTimeout(ehTimer);
  ehTimer = null;
  ehGen++;
  setAcmePause(true);
  const started = Date.now();
  _pauseProbe = setInterval(async () => {
    if (Date.now() - started < 10000) return;
    let back = Date.now() >= _pauseUntil;
    if (!back) {
      try {
        const r = await _nativeFetch('/api/status', {cache: 'no-store'});
        back = r.ok && !(Number(r.headers.get('X-Pause')) > 0);
      } catch (_) {}
    }
    if (back) endPause();
  }, 5000);
}
function endPause() {
  clearInterval(_pauseProbe);
  _pauseProbe = null;
  _pauseUntil = 0;
  setAcmePause(false);
  statusTimer = setInterval(fetchStatus, 5000);
  logTimer = setInterval(fetchLog, 30000);
  fetchStatus();
  if (location.hash === '#energy') ehRoute();
}
const UI_LANGUAGES = ['en', 'de', 'nl', 'fr', 'pl'];
const UI_TEXT = {
  'Modus': ['Mode', 'Modus', 'Modus', 'Mode', 'Tryb'],
  'Leistung': ['Power', 'Leistung', 'Vermogen', 'Puissance', 'Moc'],
  'Ladevorgang': ['Charging session', 'Ladevorgang', 'Laadsessie', 'Session de charge', 'Sesja \u0142adowania'],
  'Gesamt': ['Total', 'Gesamt', 'Totaal', 'Total', 'Razem'],
  'Woche': ['Week', 'Woche', 'Week', 'Semaine', 'Tydzie\u0144'],
  'Geladen': ['Charged', 'Geladen', 'Geladen', 'Charg\u00e9', 'Na\u0142adowano'],
  'PV S\u00fcd': ['PV South', 'PV S\u00fcd', 'PV Zuid', 'PV Sud', 'PV Po\u0142udnie'],
  'Vollbild': ['Fullscreen', 'Vollbild', 'Volledig scherm', 'Plein \u00e9cran', 'Pe\u0142ny ekran'],
  'PV West': ['PV West', 'PV West', 'PV West', 'PV Ouest', 'PV Zach\u00f3d'],
  'PV Ost': ['PV East', 'PV Ost', 'PV Oost', 'PV Est', 'PV Wsch\u00f3d'],
  'Ladezustand': ['State of charge', 'Ladezustand', 'Laadtoestand', 'Niveau de charge', 'Stan na\u0142adowania'],
  'Energiehaus': ['Energy house', 'Energiehaus', 'Energiehuis', 'Maison énergie', 'Dom energii'],
  'Mein Energiehaus': ['My energy house', 'Mein Energiehaus', 'Mijn energiehuis', 'Ma maison énergie', 'Mój dom energii'],
  'Foto wählen': ['Choose photo', 'Foto wählen', 'Foto kiezen', 'Choisir une photo', 'Wybierz zdjęcie'],
  'Foto-Link': ['Photo link', 'Foto-Link', 'Fotolink', 'Lien photo', 'Link do zdjęcia'],
  'Kacheln verschieben': ['Move tiles', 'Kacheln verschieben', 'Tegels verplaatsen', 'Déplacer les tuiles', 'Przesuń kafelki'],
  'Zurücksetzen': ['Reset', 'Zurücksetzen', 'Resetten', 'Réinitialiser', 'Resetuj'],
  'Foto zu groß für den Browser-Speicher': ['Photo too large for browser storage', 'Foto zu groß für den Browser-Speicher', 'Foto te groot voor browseropslag', 'Photo trop grande pour le stockage du navigateur', 'Zdjęcie za duże dla pamięci przeglądarki'],
  'Stromnetz': ['Grid', 'Stromnetz', 'Stroomnet', 'Réseau', 'Sieć'],
  'Bezug': ['Import', 'Bezug', 'Afname', 'Soutirage', 'Pobór'],
  'Einspeisung': ['Export', 'Einspeisung', 'Teruglevering', 'Injection', 'Oddawanie'],
  'Batterie': ['Battery', 'Batterie', 'Batterij', 'Batterie', 'Bateria'],
  'Laden': ['Charging', 'Laden', 'Laden', 'Charge', 'Ładowanie'],
  'Entladen': ['Discharging', 'Entladen', 'Ontladen', 'Décharge', 'Rozładowanie'],
  'Wallbox': ['Wallbox', 'Wallbox', 'Wallbox', 'Wallbox', 'Wallbox'],
  'E-Auto': ['EV', 'E-Auto', 'EV', 'VE', 'EV'],
  'Eigenverbrauch': ['Self-consumption', 'Eigenverbrauch', 'Eigenverbruik', 'Autoconsommation', 'Autokonsumpcja'],
  'Gesamterzeugung': ['Total generation', 'Gesamterzeugung', 'Totale opwekking', 'Production totale', 'Całkowita produkcja'],
  'Summe': ['Total', 'Summe', 'Totaal', 'Total', 'Suma'],
  'Gewinn': ['Profit', 'Gewinn', 'Winst', 'Bénéfice', 'Zysk'],
  'Netzkosten heute': ['Grid cost today', 'Netzkosten heute', 'Netkosten vandaag', 'Coût réseau aujourd\'hui', 'Koszt sieci dzisiaj'],
  'Monat': ['Month', 'Monat', 'Maand', 'Mois', 'Miesiąc'],
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
  'Einmal auslesen': ['Read once', 'Einmal auslesen', 'Eenmalig uitlezen', 'Lire une fois', 'Odczytaj raz'],
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
  'Nicht verfügbar': ['Not available', 'Nicht verfügbar', 'Niet beschikbaar', 'Indisponible', 'Niedostępny'],
  'WLAN': ['Wi-Fi', 'WLAN', 'Wifi', 'Wi-Fi', 'Wi-Fi'],
  'Allgemein': ['General', 'Allgemein', 'Algemeen', 'Général', 'Ogólne'],
  'Zähler': ['Meter', 'Zähler', 'Meter', 'Compteur', 'Licznik'],
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
  'Dauerlesen': ['Continuous reading', 'Dauerlesen', 'Continu uitlezen', 'Lecture continue', 'Odczyt ciągły'],
  'Aktiv': ['Active', 'Aktiv', 'Actief', 'Actif', 'Aktywny'],
  'Inaktiv': ['Inactive', 'Inaktiv', 'Inactief', 'Inactif', 'Nieaktywny'],
  'Zähler reset': ['Reset counters', 'Zähler reset', 'Tellers resetten', 'Réinitialiser les compteurs', 'Resetuj liczniki'],
  'Neustart': ['Restart', 'Neustart', 'Opnieuw starten', 'Redémarrer', 'Uruchom ponownie'],
  'Konfiguration': ['Settings', 'Konfiguration', 'Instellingen', 'Configuration', 'Konfiguracja'],
  'Copyright Michael Kreutzer 2026': ['Copyright Michael Kreutzer 2026', 'Copyright Michael Kreutzer 2026', 'Copyright Michael Kreutzer 2026', 'Copyright Michael Kreutzer 2026', 'Copyright Michael Kreutzer 2026'],
  'Lizenz: GNU AGPLv3': ['License: GNU AGPLv3', 'Lizenz: GNU AGPLv3', 'Licentie: GNU AGPLv3', 'Licence : GNU AGPLv3', 'Licencja: GNU AGPLv3'],
  'Quellcode': ['Source code', 'Quellcode', 'Broncode', 'Code source', 'Kod źródłowy'],
  'Lizenztext': ['License text', 'Lizenztext', 'Licentietekst', 'Texte de la licence', 'Tekst licencji'],
  'Keine Gewährleistung': ['No warranty', 'Keine Gewährleistung', 'Geen garantie', 'Aucune garantie', 'Brak gwarancji'],
  'Sprache': ['Language', 'Sprache', 'Taal', 'Langue', 'Język'],
  'Sprache für die Oberfläche': ['Interface language', 'Sprache für die Oberfläche', 'Taal van de interface', 'Langue de l’interface', 'Język interfejsu'],
  'Zugriff auf das Zähler-Frontend': ['Meter interface access', 'Zugriff auf das Zähler-Frontend', 'Toegang tot de meterinterface', 'Accès à l’interface du compteur', 'Dostęp do interfejsu licznika'],
  'Zusätzliche erlaubte CIDR-Netze': ['Additional allowed CIDR networks', 'Zusätzliche erlaubte CIDR-Netze', 'Extra toegestane CIDR-netwerken', 'Réseaux CIDR autorisés supplémentaires', 'Dodatkowe dozwolone sieci CIDR'],
  'IPv4- oder IPv6-CIDR-Netze pro Zeile. Standardmäßig erlaubt: private IPv4-Netze, IPv6-ULA (fd95…), Link-Local (fe80::/10) und das eigene globale IPv6-/64-Präfix des ESP, auch wenn der Router es erst später zuweist. Andere globale Netze müssen hier explizit freigegeben werden.': ['One IPv4 or IPv6 CIDR network per line. Allowed by default: private IPv4 networks, IPv6 ULA (fd95…), link-local (fe80::/10) and the ESP\'s own global IPv6 /64 prefix, even if the router assigns it later. Other global networks must be explicitly allowed here.', 'IPv4- oder IPv6-CIDR-Netze pro Zeile. Standardmäßig erlaubt: private IPv4-Netze, IPv6-ULA (fd95…), Link-Local (fe80::/10) und das eigene globale IPv6-/64-Präfix des ESP, auch wenn der Router es erst später zuweist. Andere globale Netze müssen hier explizit freigegeben werden.', 'Eén IPv4- of IPv6-CIDR-netwerk per regel. Standaard toegestaan: privé-IPv4-netwerken, IPv6-ULA (fd95…), link-local (fe80::/10) en het eigen globale IPv6-/64-prefix van de ESP, ook als de router dit later toewijst. Andere globale netwerken moeten hier expliciet worden toegestaan.', 'Un réseau CIDR IPv4 ou IPv6 par ligne. Autorisés par défaut : réseaux IPv4 privés, ULA IPv6 (fd95…), link-local (fe80::/10) et le préfixe IPv6 global /64 propre à l\'ESP, même si le routeur l\'attribue plus tard. Les autres réseaux globaux doivent être explicitement autorisés ici.', 'Jedna sieć IPv4 lub IPv6 CIDR w wierszu. Domyślnie dozwolone: prywatne sieci IPv4, IPv6 ULA (fd95…), link-local (fe80::/10) oraz własny globalny prefiks IPv6 /64 ESP, także gdy router przydzieli go później. Inne sieci globalne trzeba tu jawnie dopuścić.'],
  'Beispiel: 192.168.1.0/24': ['Example: 192.168.1.0/24', 'Beispiel: 192.168.1.0/24', 'Voorbeeld: 192.168.1.0/24', 'Exemple : 192.168.1.0/24', 'Przykład: 192.168.1.0/24'],
  'Beispiele: 192.168.1.0/24, 2a00:6020:a105:c900::/64': ['Examples: 192.168.1.0/24, 2a00:6020:a105:c900::/64', 'Beispiele: 192.168.1.0/24, 2a00:6020:a105:c900::/64', 'Voorbeelden: 192.168.1.0/24, 2a00:6020:a105:c900::/64', 'Exemples : 192.168.1.0/24, 2a00:6020:a105:c900::/64', 'Przykłady: 192.168.1.0/24, 2a00:6020:a105:c900::/64'],
  'Netzwerke speichern': ['Save networks', 'Netzwerke speichern', 'Netwerken opslaan', 'Enregistrer les réseaux', 'Zapisz sieci'],
  'Netzwerkfilter gespeichert.': ['Network filter saved.', 'Netzwerkfilter gespeichert.', 'Netwerkfilter opgeslagen.', 'Filtre réseau enregistré.', 'Zapisano filtr sieci.'],
  'Netzwerkfilter konnte nicht gespeichert werden.': ['Could not save network filter.', 'Netzwerkfilter konnte nicht gespeichert werden.', 'Netwerkfilter kon niet worden opgeslagen.', 'Impossible d’enregistrer le filtre réseau.', 'Nie udało się zapisać filtra sieci.'],
  'HTTPS / TLS': ['HTTPS / TLS', 'HTTPS / TLS', 'HTTPS / TLS', 'HTTPS / TLS', 'HTTPS / TLS'],
  'Hostname (FQDN)': ['Hostname (FQDN)', 'Hostname (FQDN)', 'Hostnaam (FQDN)', 'Nom d’hôte (FQDN)', 'Nazwa hosta (FQDN)'],
  'Zertifikat (PEM, optional)': ['Certificate (PEM, optional)', 'Zertifikat (PEM, optional)', 'Certificaat (PEM, optioneel)', 'Certificat (PEM, facultatif)', 'Certyfikat (PEM, opcjonalnie)'],
  'Privater Schlüssel (PEM, optional)': ['Private key (PEM, optional)', 'Privater Schlüssel (PEM, optional)', 'Privésleutel (PEM, optioneel)', 'Clé privée (PEM, facultatif)', 'Klucz prywatny (PEM, opcjonalnie)'],
  'Eigenes Zertifikat entfernen (Self-Signed verwenden)': ['Remove own certificate (use self-signed)', 'Eigenes Zertifikat entfernen (Self-Signed verwenden)', 'Eigen certificaat verwijderen (zelfondertekend gebruiken)', 'Supprimer le certificat propre (utiliser l’auto-signé)', 'Usuń własny certyfikat (użyj samopodpisanego)'],
  'HTTPS speichern': ['Save HTTPS', 'HTTPS speichern', 'HTTPS opslaan', 'Enregistrer HTTPS', 'Zapisz HTTPS'],
  'HTTPS läuft parallel zu HTTP auf Port 443. Ohne eigenes Zertifikat erzeugt das Gerät ein Self-Signed-Zertifikat; der Browser zeigt dafür eine Warnung. Zertifikat und privater Schlüssel müssen zusammenpassen.': ['HTTPS runs next to HTTP on port 443. Without your own certificate the device creates a self-signed certificate, for which the browser shows a warning. Certificate and private key must match.', 'HTTPS läuft parallel zu HTTP auf Port 443. Ohne eigenes Zertifikat erzeugt das Gerät ein Self-Signed-Zertifikat; der Browser zeigt dafür eine Warnung. Zertifikat und privater Schlüssel müssen zusammenpassen.', 'HTTPS draait naast HTTP op poort 443. Zonder eigen certificaat maakt het apparaat een zelfondertekend certificaat aan, waarvoor de browser een waarschuwing toont. Certificaat en privésleutel moeten bij elkaar passen.', 'HTTPS fonctionne en parallèle de HTTP sur le port 443. Sans certificat propre, l’appareil crée un certificat auto-signé, pour lequel le navigateur affiche un avertissement. Le certificat et la clé privée doivent correspondre.', 'HTTPS działa równolegle do HTTP na porcie 443. Bez własnego certyfikatu urządzenie tworzy certyfikat samopodpisany, dla którego przeglądarka wyświetla ostrzeżenie. Certyfikat i klucz prywatny muszą do siebie pasować.'],
  'HTTPS aktiv (Port 443)': ['HTTPS active (port 443)', 'HTTPS aktiv (Port 443)', 'HTTPS actief (poort 443)', 'HTTPS actif (port 443)', 'HTTPS aktywny (port 443)'],
  'HTTPS nicht aktiv': ['HTTPS not active', 'HTTPS nicht aktiv', 'HTTPS niet actief', 'HTTPS inactif', 'HTTPS nieaktywny'],
  'Zertifikat: eigenes': ['Certificate: own', 'Zertifikat: eigenes', 'Certificaat: eigen', 'Certificat : propre', 'Certyfikat: własny'],
  'Zertifikat: Self-Signed': ['Certificate: self-signed', 'Zertifikat: Self-Signed', 'Certificaat: zelfondertekend', 'Certificat : auto-signé', 'Certyfikat: samopodpisany'],
  'HTTPS-Einstellungen gespeichert. Der HTTPS-Server startet neu.': ['HTTPS settings saved. The HTTPS server is restarting.', 'HTTPS-Einstellungen gespeichert. Der HTTPS-Server startet neu.', 'HTTPS-instellingen opgeslagen. De HTTPS-server herstart.', 'Paramètres HTTPS enregistrés. Le serveur HTTPS redémarre.', 'Zapisano ustawienia HTTPS. Serwer HTTPS uruchamia się ponownie.'],
  'HTTPS-Einstellungen konnten nicht gespeichert werden.': ['Could not save HTTPS settings.', 'HTTPS-Einstellungen konnten nicht gespeichert werden.', 'HTTPS-instellingen konden niet worden opgeslagen.', 'Impossible d’enregistrer les paramètres HTTPS.', 'Nie udało się zapisać ustawień HTTPS.'],
  'Zertifikatsquelle': ['Certificate source', 'Zertifikatsquelle', 'Certificaatbron', 'Source du certificat', 'Źródło certyfikatu'],
  'Manuell / Self-Signed': ['Manual / self-signed', 'Manuell / Self-Signed', 'Handmatig / zelfondertekend', 'Manuel / auto-signé', 'Ręcznie / samopodpisany'],
  'Let\'s Encrypt (ACME HTTP-01)': ['Let\'s Encrypt (ACME HTTP-01)', 'Let\'s Encrypt (ACME HTTP-01)', 'Let\'s Encrypt (ACME HTTP-01)', 'Let\'s Encrypt (ACME HTTP-01)', 'Let\'s Encrypt (ACME HTTP-01)'],
  'E-Mail für Let\'s Encrypt': ['Email for Let\'s Encrypt', 'E-Mail für Let\'s Encrypt', 'E-mail voor Let\'s Encrypt', 'E-mail pour Let\'s Encrypt', 'E-mail dla Let\'s Encrypt'],
  'Nutzungsbedingungen von Let\'s Encrypt akzeptieren': ['Accept the Let\'s Encrypt terms of service', 'Nutzungsbedingungen von Let\'s Encrypt akzeptieren', 'Gebruiksvoorwaarden van Let\'s Encrypt accepteren', 'Accepter les conditions d’utilisation de Let\'s Encrypt', 'Zaakceptuj warunki korzystania z Let\'s Encrypt'],
  'Staging-Umgebung (Test, Zertifikat nicht vertrauenswürdig)': ['Staging environment (test, certificate not trusted)', 'Staging-Umgebung (Test, Zertifikat nicht vertrauenswürdig)', 'Staging-omgeving (test, certificaat niet vertrouwd)', 'Environnement de test (staging, certificat non approuvé)', 'Środowisko testowe (staging, certyfikat niezaufany)'],
  'Erneuerung nach (Tagen)': ['Renew after (days)', 'Erneuerung nach (Tagen)', 'Vernieuwen na (dagen)', 'Renouveler après (jours)', 'Odnów po (dniach)'],
  'Let\'s Encrypt prüft den Hostnamen über Port 80 (HTTP-01). Der Hostname muss auf dieses Gerät zeigen und Port 80 muss aus dem Internet erreichbar sein, bei IPv6-only per AAAA-Eintrag und Firewall-Freigabe. Zuerst die Staging-Umgebung testen.': ['Let\'s Encrypt validates the hostname over port 80 (HTTP-01). The hostname must point to this device and port 80 must be reachable from the internet, for IPv6-only via an AAAA record and a firewall rule. Test the staging environment first.', 'Let\'s Encrypt prüft den Hostnamen über Port 80 (HTTP-01). Der Hostname muss auf dieses Gerät zeigen und Port 80 muss aus dem Internet erreichbar sein, bei IPv6-only per AAAA-Eintrag und Firewall-Freigabe. Zuerst die Staging-Umgebung testen.', 'Let\'s Encrypt controleert de hostnaam via poort 80 (HTTP-01). De hostnaam moet naar dit apparaat wijzen en poort 80 moet vanaf internet bereikbaar zijn, bij alleen IPv6 via een AAAA-record en firewallregel. Test eerst de staging-omgeving.', 'Let\'s Encrypt valide le nom d’hôte via le port 80 (HTTP-01). Le nom d’hôte doit pointer vers cet appareil et le port 80 doit être joignable depuis Internet, en IPv6 seul via un enregistrement AAAA et une règle de pare-feu. Testez d’abord l’environnement de test.', 'Let\'s Encrypt weryfikuje nazwę hosta przez port 80 (HTTP-01). Nazwa hosta musi wskazywać to urządzenie, a port 80 musi być osiągalny z internetu, przy samym IPv6 przez rekord AAAA i regułę zapory. Najpierw przetestuj środowisko testowe.'],
  'Zertifikat jetzt anfordern': ['Request certificate now', 'Zertifikat jetzt anfordern', 'Certificaat nu aanvragen', 'Demander le certificat maintenant', 'Zamów certyfikat teraz'],
  'Zertifikat: Let\'s Encrypt': ['Certificate: Let\'s Encrypt', 'Zertifikat: Let\'s Encrypt', 'Certificaat: Let\'s Encrypt', 'Certificat : Let\'s Encrypt', 'Certyfikat: Let\'s Encrypt'],
  'Zertifikat wird angefordert …': ['Requesting certificate …', 'Zertifikat wird angefordert …', 'Certificaat wordt aangevraagd …', 'Demande du certificat en cours …', 'Zamawianie certyfikatu …'],
  'Zertifikat ausgestellt. HTTPS wird neu gestartet.': ['Certificate issued. HTTPS is restarting.', 'Zertifikat ausgestellt. HTTPS wird neu gestartet.', 'Certificaat uitgegeven. HTTPS herstart.', 'Certificat émis. HTTPS redémarre.', 'Certyfikat wystawiony. HTTPS uruchamia się ponownie.'],
  'Anforderung fehlgeschlagen': ['Request failed', 'Anforderung fehlgeschlagen', 'Aanvraag mislukt', 'Échec de la demande', 'Zamówienie nie powiodło się'],
  'HTTPS / Zertifikate': ['HTTPS / Certificates', 'HTTPS / Zertifikate', 'HTTPS / Certificaten', 'HTTPS / Certificats', 'HTTPS / Certyfikaty'],
  'SMLEasy — HTTPS / Zertifikate': ['SMLEasy — HTTPS / Certificates', 'SMLEasy — HTTPS / Zertifikate', 'SMLEasy — HTTPS / Certificaten', 'SMLEasy — HTTPS / Certificats', 'SMLEasy — HTTPS / Certyfikaty'],
  '1. Nutzungsbedingungen': ['1. Terms of service', '1. Nutzungsbedingungen', '1. Gebruiksvoorwaarden', '1. Conditions d’utilisation', '1. Warunki korzystania'],
  'Mit dem Anfordern eines Zertifikats stimmst du den Nutzungsbedingungen von Let\'s Encrypt zu. Das Gerät legt dafür ein Konto bei Let\'s Encrypt an.': ['By requesting a certificate you agree to the Let\'s Encrypt terms of service. The device creates an account with Let\'s Encrypt for this.', 'Mit dem Anfordern eines Zertifikats stimmst du den Nutzungsbedingungen von Let\'s Encrypt zu. Das Gerät legt dafür ein Konto bei Let\'s Encrypt an.', 'Door een certificaat aan te vragen ga je akkoord met de gebruiksvoorwaarden van Let\'s Encrypt. Het apparaat maakt hiervoor een account bij Let\'s Encrypt aan.', 'En demandant un certificat, vous acceptez les conditions d’utilisation de Let\'s Encrypt. L’appareil crée pour cela un compte chez Let\'s Encrypt.', 'Zamawiając certyfikat, akceptujesz warunki korzystania z Let\'s Encrypt. Urządzenie zakłada w tym celu konto w Let\'s Encrypt.'],
  'Nutzungsbedingungen lesen': ['Read the terms of service', 'Nutzungsbedingungen lesen', 'Gebruiksvoorwaarden lezen', 'Lire les conditions d’utilisation', 'Przeczytaj warunki korzystania'],
  '2. Angaben': ['2. Details', '2. Angaben', '2. Gegevens', '2. Informations', '2. Dane'],
  '3. Tests': ['3. Checks', '3. Tests', '3. Tests', '3. Tests', '3. Testy'],
  'Vor der Anforderung prüft das Gerät Internetzugang, DNS und die Erreichbarkeit von außen. Die Erreichbarkeit wird mit einer Testvalidierung in der Staging-Umgebung von Let\'s Encrypt bewiesen; dabei wird kein Zertifikat ausgestellt.': ['Before the request the device checks internet access, DNS and reachability from outside. Reachability is proven with a test validation in the Let\'s Encrypt staging environment; no certificate is issued.', 'Vor der Anforderung prüft das Gerät Internetzugang, DNS und die Erreichbarkeit von außen. Die Erreichbarkeit wird mit einer Testvalidierung in der Staging-Umgebung von Let\'s Encrypt bewiesen; dabei wird kein Zertifikat ausgestellt.', 'Voor de aanvraag controleert het apparaat internettoegang, DNS en bereikbaarheid van buitenaf. De bereikbaarheid wordt bewezen met een testvalidatie in de staging-omgeving van Let\'s Encrypt; er wordt geen certificaat uitgegeven.', 'Avant la demande, l’appareil vérifie l’accès Internet, le DNS et l’accessibilité depuis l’extérieur. L’accessibilité est prouvée par une validation de test dans l’environnement staging de Let\'s Encrypt ; aucun certificat n’est émis.', 'Przed zamówieniem urządzenie sprawdza dostęp do internetu, DNS i osiągalność z zewnątrz. Osiągalność potwierdza testowa walidacja w środowisku staging Let\'s Encrypt; certyfikat nie jest wystawiany.'],
  'Tests starten': ['Start checks', 'Tests starten', 'Tests starten', 'Lancer les tests', 'Uruchom testy'],
  '4. Zertifikat anfordern': ['4. Request certificate', '4. Zertifikat anfordern', '4. Certificaat aanvragen', '4. Demander le certificat', '4. Zamów certyfikat'],
  'Internetzugang und Uhrzeit': ['Internet access and clock', 'Internetzugang und Uhrzeit', 'Internettoegang en tijd', 'Accès Internet et heure', 'Dostęp do internetu i zegar'],
  'DNS: Hostname zeigt auf dieses Gerät': ['DNS: hostname points to this device', 'DNS: Hostname zeigt auf dieses Gerät', 'DNS: hostnaam wijst naar dit apparaat', 'DNS : le nom d’hôte pointe vers cet appareil', 'DNS: nazwa hosta wskazuje to urządzenie'],
  'Erreichbarkeit von außen über Port 80 (HTTP)': ['Reachable from outside on port 80 (HTTP)', 'Erreichbarkeit von außen über Port 80 (HTTP)', 'Bereikbaar van buitenaf via poort 80 (HTTP)', 'Accessible de l’extérieur via le port 80 (HTTP)', 'Osiągalność z zewnątrz przez port 80 (HTTP)'],
  'Zuerst die Nutzungsbedingungen akzeptieren.': ['Accept the terms of service first.', 'Zuerst die Nutzungsbedingungen akzeptieren.', 'Accepteer eerst de gebruiksvoorwaarden.', 'Acceptez d’abord les conditions d’utilisation.', 'Najpierw zaakceptuj warunki korzystania.'],
  'Gültigen Hostnamen und E-Mail-Adresse eintragen.': ['Enter a valid hostname and email address.', 'Gültigen Hostnamen und E-Mail-Adresse eintragen.', 'Vul een geldige hostnaam en e-mailadres in.', 'Saisissez un nom d’hôte et une adresse e-mail valides.', 'Wpisz poprawną nazwę hosta i adres e-mail.'],
  'Erst nach bestandenen Tests freigegeben.': ['Only enabled after the checks have passed.', 'Erst nach bestandenen Tests freigegeben.', 'Pas beschikbaar nadat de tests zijn geslaagd.', 'Disponible uniquement après la réussite des tests.', 'Dostępne dopiero po pomyślnych testach.'],
  'Tests bestanden: Die Anforderung ist freigegeben (1 Stunde gültig).': ['Checks passed: the request is enabled (valid for 1 hour).', 'Tests bestanden: Die Anforderung ist freigegeben (1 Stunde gültig).', 'Tests geslaagd: de aanvraag is vrijgegeven (1 uur geldig).', 'Tests réussis : la demande est autorisée (valable 1 heure).', 'Testy zaliczone: zamówienie jest odblokowane (ważne 1 godzinę).'],
  'Tests laufen …': ['Checks running …', 'Tests laufen …', 'Tests worden uitgevoerd …', 'Tests en cours …', 'Testy w toku …'],
  'Alle Tests bestanden. Das Zertifikat kann jetzt angefordert werden.': ['All checks passed. The certificate can now be requested.', 'Alle Tests bestanden. Das Zertifikat kann jetzt angefordert werden.', 'Alle tests geslaagd. Het certificaat kan nu worden aangevraagd.', 'Tous les tests sont réussis. Le certificat peut maintenant être demandé.', 'Wszystkie testy zaliczone. Można teraz zamówić certyfikat.'],
  'Mindestens ein Test ist fehlgeschlagen. Bitte die Hinweise beachten und die Tests wiederholen.': ['At least one check failed. Please follow the hints and repeat the checks.', 'Mindestens ein Test ist fehlgeschlagen. Bitte die Hinweise beachten und die Tests wiederholen.', 'Minstens één test is mislukt. Volg de aanwijzingen en herhaal de tests.', 'Au moins un test a échoué. Suivez les indications et répétez les tests.', 'Co najmniej jeden test nie powiódł się. Zastosuj wskazówki i powtórz testy.'],
  'Tests konnten nicht gestartet werden.': ['Checks could not be started.', 'Tests konnten nicht gestartet werden.', 'Tests konden niet worden gestart.', 'Les tests n’ont pas pu être lancés.', 'Nie udało się uruchomić testów.'],
  'Externe Daten': ['External data', 'Externe Daten', 'Externe gegevens', 'Données externes', 'Dane zewnętrzne'],
  'SMLEasy — Externe Daten': ['SMLEasy — External data', 'SMLEasy — Externe Daten', 'SMLEasy — Externe gegevens', 'SMLEasy — Données externes', 'SMLEasy — Dane zewnętrzne'],
  'Externe Daten (Home Assistant)': ['External data (Home Assistant)', 'Externe Daten (Home Assistant)', 'Externe gegevens (Home Assistant)', 'Données externes (Home Assistant)', 'Dane zewnętrzne (Home Assistant)'],
  'Werte aus Home Assistant anzeigen': ['Show values from Home Assistant', 'Werte aus Home Assistant anzeigen', 'Waarden uit Home Assistant tonen', 'Afficher les valeurs de Home Assistant', 'Pokaż wartości z Home Assistant'],
  'REST-Abfrage': ['REST query', 'REST-Abfrage', 'REST-opvraag', 'Requête REST', 'Zapytanie REST'],
  'Home-Assistant-URL': ['Home Assistant URL', 'Home-Assistant-URL', 'Home Assistant-URL', 'URL de Home Assistant', 'Adres URL Home Assistant'],
  'Zugriffstoken (Long-Lived Access Token)': ['Access token (long-lived access token)', 'Zugriffstoken (Long-Lived Access Token)', 'Toegangstoken (long-lived access token)', 'Jeton d’accès (jeton longue durée)', 'Token dostępu (długoterminowy)'],
  'Leer lassen, um das gespeicherte Token zu behalten': ['Leave empty to keep the stored token', 'Leer lassen, um das gespeicherte Token zu behalten', 'Leeg laten om het opgeslagen token te behouden', 'Laisser vide pour conserver le jeton enregistré', 'Zostaw puste, aby zachować zapisany token'],
  'Gesetzt – leer lassen, um es zu behalten': ['Set – leave empty to keep it', 'Gesetzt – leer lassen, um es zu behalten', 'Ingesteld – leeg laten om te behouden', 'Défini – laisser vide pour le conserver', 'Ustawiony – zostaw puste, aby zachować'],
  'Gespeichertes Token entfernen': ['Remove stored token', 'Gespeichertes Token entfernen', 'Opgeslagen token verwijderen', 'Supprimer le jeton enregistré', 'Usuń zapisany token'],
  'Abfrageintervall (Sekunden)': ['Polling interval (seconds)', 'Abfrageintervall (Sekunden)', 'Opvraaginterval (seconden)', 'Intervalle d’interrogation (secondes)', 'Interwał odpytywania (sekundy)'],
  'Anzuzeigende Werte (bis zu 8)': ['Values to show (up to 8)', 'Anzuzeigende Werte (bis zu 8)', 'Weer te geven waarden (maximaal 8)', 'Valeurs à afficher (jusqu’à 8)', 'Wartości do wyświetlenia (do 8)'],
  'Bezeichnung': ['Label', 'Bezeichnung', 'Naam', 'Libellé', 'Nazwa'],
  'Quelle': ['Source', 'Quelle', 'Bron', 'Source', 'Źródło'],
  'Entity-ID / Topic': ['Entity ID / topic', 'Entity-ID / Topic', 'Entity-ID / topic', 'ID d’entité / topic', 'ID encji / temat'],
  'Einheit': ['Unit', 'Einheit', 'Eenheid', 'Unité', 'Jednostka'],
  'JSON-Schlüssel (MQTT)': ['JSON key (MQTT)', 'JSON-Schlüssel (MQTT)', 'JSON-sleutel (MQTT)', 'Clé JSON (MQTT)', 'Klucz JSON (MQTT)'],
  'REST (Entity-ID)': ['REST (entity ID)', 'REST (Entity-ID)', 'REST (entity-ID)', 'REST (ID d’entité)', 'REST (ID encji)'],
  'MQTT (Topic)': ['MQTT (topic)', 'MQTT (Topic)', 'MQTT (topic)', 'MQTT (topic)', 'MQTT (temat)'],
  'PV-Batterie Ladestand': ['PV battery charge', 'PV-Batterie Ladestand', 'PV-batterij laadstand', 'Charge de la batterie PV', 'Poziom naładowania baterii PV'],
  'PV-Leistung gesamt': ['PV power total', 'PV-Leistung gesamt', 'PV-vermogen totaal', 'Puissance PV totale', 'Moc PV łącznie'],
  'Wallbox gesamt': ['Wallbox total', 'Wallbox gesamt', 'Wallbox totaal', 'Wallbox total', 'Wallbox łącznie'],
  'Hausverbrauch': ['House consumption', 'Hausverbrauch', 'Huisverbruik', 'Consommation de la maison', 'Zużycie domu'],
  'REST: Das Gerät fragt Home Assistant über die REST-API ab. Ein Token erzeugst du in Home Assistant unter Profil → Sicherheit → Langlebige Zugriffstoken. MQTT: Das Gerät abonniert ein Topic am Broker der Home-Assistant-Integration (Einstellungen → Home Assistant); eine Automation in Home Assistant muss den Wert dort veröffentlichen. Bei JSON-Nachrichten gibt der Schlüssel das Feld an, zum Beispiel battery.soc.': ['REST: the device queries Home Assistant through its REST API. Create a token in Home Assistant under Profile → Security → Long-lived access tokens. MQTT: the device subscribes to a topic on the broker of the Home Assistant integration (Settings → Home Assistant); an automation in Home Assistant must publish the value there. For JSON messages the key names the field, for example battery.soc.', 'REST: Das Gerät fragt Home Assistant über die REST-API ab. Ein Token erzeugst du in Home Assistant unter Profil → Sicherheit → Langlebige Zugriffstoken. MQTT: Das Gerät abonniert ein Topic am Broker der Home-Assistant-Integration (Einstellungen → Home Assistant); eine Automation in Home Assistant muss den Wert dort veröffentlichen. Bei JSON-Nachrichten gibt der Schlüssel das Feld an, zum Beispiel battery.soc.', 'REST: het apparaat vraagt Home Assistant op via de REST-API. Een token maak je in Home Assistant onder Profiel → Beveiliging → Langlevende toegangstokens. MQTT: het apparaat abonneert zich op een topic bij de broker van de Home Assistant-integratie (Instellingen → Home Assistant); een automatisering in Home Assistant moet de waarde daar publiceren. Bij JSON-berichten geeft de sleutel het veld aan, bijvoorbeeld battery.soc.', 'REST : l’appareil interroge Home Assistant via l’API REST. Créez un jeton dans Home Assistant sous Profil → Sécurité → Jetons d’accès longue durée. MQTT : l’appareil s’abonne à un topic sur le broker de l’intégration Home Assistant (Paramètres → Home Assistant) ; une automatisation de Home Assistant doit y publier la valeur. Pour les messages JSON, la clé désigne le champ, par exemple battery.soc.', 'REST: urządzenie odpytuje Home Assistant przez API REST. Token utworzysz w Home Assistant w Profil → Zabezpieczenia → Długoterminowe tokeny dostępu. MQTT: urządzenie subskrybuje temat na brokerze integracji Home Assistant (Ustawienia → Home Assistant); automatyzacja w Home Assistant musi tam publikować wartość. W wiadomościach JSON klucz wskazuje pole, na przykład battery.soc.'],
  'Externe Daten speichern': ['Save external data', 'Externe Daten speichern', 'Externe gegevens opslaan', 'Enregistrer les données externes', 'Zapisz dane zewnętrzne'],
  'Jetzt abrufen': ['Fetch now', 'Jetzt abrufen', 'Nu ophalen', 'Interroger maintenant', 'Pobierz teraz'],
  'Aktuelle Werte': ['Current values', 'Aktuelle Werte', 'Actuele waarden', 'Valeurs actuelles', 'Bieżące wartości'],
  'Keine Werte konfiguriert.': ['No values configured.', 'Keine Werte konfiguriert.', 'Geen waarden ingesteld.', 'Aucune valeur configurée.', 'Brak skonfigurowanych wartości.'],
  'ok': ['ok', 'ok', 'ok', 'ok', 'ok'],
  'veraltet': ['stale', 'veraltet', 'verouderd', 'périmé', 'nieaktualne'],
  'Fehler': ['Error', 'Fehler', 'Fout', 'Erreur', 'Błąd'],
  'wartet': ['waiting', 'wartet', 'wachten', 'en attente', 'oczekuje'],
  'MQTT ist ausgeschaltet (Home-Assistant-Integration aktivieren, um MQTT-Werte zu nutzen).': ['MQTT is off (enable the Home Assistant integration to use MQTT values).', 'MQTT ist ausgeschaltet (Home-Assistant-Integration aktivieren, um MQTT-Werte zu nutzen).', 'MQTT staat uit (schakel de Home Assistant-integratie in om MQTT-waarden te gebruiken).', 'MQTT est désactivé (activez l’intégration Home Assistant pour utiliser les valeurs MQTT).', 'MQTT jest wyłączony (włącz integrację Home Assistant, aby używać wartości MQTT).'],
  'MQTT-Broker verbunden.': ['MQTT broker connected.', 'MQTT-Broker verbunden.', 'MQTT-broker verbonden.', 'Broker MQTT connecté.', 'Broker MQTT połączony.'],
  'MQTT-Broker nicht verbunden.': ['MQTT broker not connected.', 'MQTT-Broker nicht verbunden.', 'MQTT-broker niet verbonden.', 'Broker MQTT non connecté.', 'Broker MQTT niepołączony.'],
  'Externe Daten gespeichert.': ['External data saved.', 'Externe Daten gespeichert.', 'Externe gegevens opgeslagen.', 'Données externes enregistrées.', 'Zapisano dane zewnętrzne.'],
  'Externe Daten konnten nicht gespeichert werden.': ['External data could not be saved.', 'Externe Daten konnten nicht gespeichert werden.', 'Externe gegevens konden niet worden opgeslagen.', 'Impossible d’enregistrer les données externes.', 'Nie udało się zapisać danych zewnętrznych.'],
  'Heute': ['Today', 'Heute', 'Vandaag', 'Aujourd\'hui', 'Dzisiaj'],
  'Hausverbrauch heute': ['Home consumption today', 'Hausverbrauch heute', 'Huisverbruik vandaag', 'Consommation du foyer aujourd\'hui', 'Zużycie domu dzisiaj'],
  'Haus & Energie': ['Home & energy', 'Haus & Energie', 'Huis & energie', 'Maison & énergie', 'Dom i energia'],
  'Live-Protokoll nur per HTTPS sichtbar (Port 80 ist während der Prüfung belegt).': ['Live log is only visible over HTTPS (port 80 is in use during validation).', 'Live-Protokoll nur per HTTPS sichtbar (Port 80 ist während der Prüfung belegt).', 'Livelogboek alleen zichtbaar via HTTPS (poort 80 is tijdens de controle in gebruik).', 'Journal en direct visible uniquement en HTTPS (le port 80 est occupé pendant la validation).', 'Dziennik na żywo jest widoczny tylko przez HTTPS (port 80 jest zajęty podczas weryfikacji).'],
  'Produktivumgebung ohne Staging-Test: Fehlversuche zählen gegen die Rate-Limits von Let\'s Encrypt. Trotzdem fortfahren?': ['Production environment without a staging test: failed attempts count against the Let\'s Encrypt rate limits. Continue anyway?', 'Produktivumgebung ohne Staging-Test: Fehlversuche zählen gegen die Rate-Limits von Let\'s Encrypt. Trotzdem fortfahren?', 'Productieomgeving zonder staging-test: mislukte pogingen tellen mee voor de limieten van Let\'s Encrypt. Toch doorgaan?', 'Environnement de production sans test staging : les échecs comptent dans les limites de Let\'s Encrypt. Continuer quand même ?', 'Środowisko produkcyjne bez testu staging: nieudane próby wliczają się do limitów Let\'s Encrypt. Kontynuować mimo to?'],
  'Zertifikat jetzt bei Let\'s Encrypt anfordern? Port 80 muss aus dem Internet erreichbar sein; das Dashboard ist währenddessen nur per HTTPS erreichbar.': ['Request the certificate from Let\'s Encrypt now? Port 80 must be reachable from the internet; meanwhile the dashboard is only available over HTTPS.', 'Zertifikat jetzt bei Let\'s Encrypt anfordern? Port 80 muss aus dem Internet erreichbar sein; das Dashboard ist währenddessen nur per HTTPS erreichbar.', 'Certificaat nu bij Let\'s Encrypt aanvragen? Poort 80 moet vanaf internet bereikbaar zijn; het dashboard is intussen alleen via HTTPS bereikbaar.', 'Demander maintenant le certificat à Let\'s Encrypt ? Le port 80 doit être joignable depuis Internet ; entre-temps, le tableau de bord n’est accessible qu’en HTTPS.', 'Zamówić teraz certyfikat w Let\'s Encrypt? Port 80 musi być osiągalny z internetu; w tym czasie panel jest dostępny tylko przez HTTPS.'],
  'Anforderung konnte nicht gestartet werden. Zuerst Einstellungen speichern (Hostname, E-Mail, Bedingungen).': ['Request could not be started. Save the settings first (hostname, email, terms).', 'Anforderung konnte nicht gestartet werden. Zuerst Einstellungen speichern (Hostname, E-Mail, Bedingungen).', 'Aanvraag kon niet worden gestart. Sla eerst de instellingen op (hostnaam, e-mail, voorwaarden).', 'La demande n’a pas pu être lancée. Enregistrez d’abord les paramètres (nom d’hôte, e-mail, conditions).', 'Nie udało się rozpocząć zamówienia. Najpierw zapisz ustawienia (nazwa hosta, e-mail, warunki).'],
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
  'Vorheriges Profil': ['Previous profile', 'Vorheriges Profil', 'Vorig profiel', 'Profil précédent', 'Poprzedni profil'],
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
  'Ich stimme den Let’s-Encrypt-Nutzungsbedingungen zu.': ['I agree to the Let’s Encrypt terms of service.', 'Ich stimme den Let’s-Encrypt-Nutzungsbedingungen zu.', 'Ik ga akkoord met de gebruiksvoorwaarden van Let’s Encrypt.', 'J’accepte les conditions d’utilisation de Let’s Encrypt.', 'Akceptuję warunki korzystania z Let’s Encrypt.'],
  'Staging-Zertifikate sind nur für Tests und werden von normalen Browsern nicht als vertrauenswürdig erkannt.': ['Staging certificates are for testing and are not trusted by normal browsers.', 'Staging-Zertifikate sind nur für Tests und werden von normalen Browsern nicht als vertrauenswürdig erkannt.', 'Staging-certificaten zijn alleen voor tests en worden niet vertrouwd door normale browsers.', 'Les certificats staging sont réservés aux tests et ne sont pas reconnus par les navigateurs classiques.', 'Certyfikaty staging służą do testów i nie są zaufane przez zwykłe przeglądarki.'],
  'Let’s Encrypt empfiehlt bei 90 Tagen Laufzeit eine Erneuerung alle 60 Tage.': ['Let’s Encrypt recommends renewal every 60 days for 90-day certificates.', 'Let’s Encrypt empfiehlt bei 90 Tagen Laufzeit eine Erneuerung alle 60 Tage.', 'Let’s Encrypt raadt bij certificaten van 90 dagen vernieuwing om de 60 dagen aan.', 'Let’s Encrypt recommande un renouvellement tous les 60 jours pour les certificats de 90 jours.', 'Let’s Encrypt zaleca odnawianie co 60 dni dla certyfikatów ważnych 90 dni.'],
  'Zertifikatskette (Fullchain PEM)': ['Certificate chain (fullchain PEM)', 'Zertifikatskette (Fullchain PEM)', 'Certificaatketen (fullchain PEM)', 'Chaîne de certificats (fullchain PEM)', 'Łańcuch certyfikatów (fullchain PEM)'],
  'Privater Schlüssel (PEM)': ['Private key (PEM)', 'Privater Schlüssel (PEM)', 'Privésleutel (PEM)', 'Clé privée (PEM)', 'Klucz prywatny (PEM)'],
  'Selbstsigniertes HTTPS-Fallback aktivieren (Browserwarnung)': ['Enable self-signed HTTPS fallback (browser warning)', 'Selbstsigniertes HTTPS-Fallback aktivieren (Browserwarnung)', 'Zelfondertekende HTTPS-fallback inschakelen (browserwaarschuwing)', 'Activer le HTTPS auto-signé de secours (avertissement navigateur)', 'Włącz samopodpisany HTTPS awaryjny (ostrzeżenie przeglądarki)'],
  'HTTPS bleibt aus, solange kein Zertifikat aktiviert ist.': ['HTTPS remains off until a certificate is activated.', 'HTTPS bleibt aus, solange kein Zertifikat aktiviert ist.', 'HTTPS blijft uit totdat een certificaat is geactiveerd.', 'HTTPS reste désactivé tant qu’aucun certificat n’est activé.', 'HTTPS pozostaje wyłączony, dopóki certyfikat nie zostanie aktywowany.'],
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
  'Tagesstart-Zählerstände': ['Daily-start meter readings', 'Tagesstart-Zählerstände', 'Meterstanden bij dagstart', 'Index au début de la journée', 'Wskazania licznika na początek dnia'],
  'Tagesstart-Hinweis': ['The daily totals are calculated from these readings and saved in device flash.', 'Tageswerte werden aus diesen Ständen berechnet und im Gerätespeicher gesichert.', 'Dagtotalen worden uit deze standen berekend en in het apparaatgeheugen opgeslagen.', 'Les totaux journaliers sont calculés à partir de ces relevés et enregistrés dans la mémoire de l’appareil.', 'Sumy dzienne są obliczane z tych wskazań i zapisywane w pamięci urządzenia.'],
  'Bezug zum Tagesstart (kWh)': ['Import at day start (kWh)', 'Bezug zum Tagesstart (kWh)', 'Afname bij dagstart (kWh)', 'Soutirage au début de journée (kWh)', 'Pobór na początku dnia (kWh)'],
  'Einspeisung zum Tagesstart (kWh)': ['Feed-in at day start (kWh)', 'Einspeisung zum Tagesstart (kWh)', 'Teruglevering bij dagstart (kWh)', 'Injection au début de journée (kWh)', 'Oddawanie na początku dnia (kWh)'],
  'Aktuelle Zählerstände': ['Current meter readings', 'Aktuelle Zählerstände', 'Huidige meterstanden', 'Relevés actuels du compteur', 'Aktualne wskazania licznika'],
  'Tagesstart laden': ['Load daily start', 'Tagesstart laden', 'Dagstart laden', 'Charger le début de journée', 'Wczytaj początek dnia'],
  'Tagesstart im Flash speichern': ['Save daily start to flash', 'Tagesstart im Flash speichern', 'Dagstart opslaan in flash', 'Enregistrer dans la mémoire flash', 'Zapisz początek dnia w pamięci flash'],
  'Tagesstartwerte gespeichert.': ['Daily-start readings saved.', 'Tagesstartwerte gespeichert.', 'Dagstartwaarden opgeslagen.', 'Relevés de début de journée enregistrés.', 'Zapisano wskazania początkowe dnia.'],
  'Tagesstartwerte konnten nicht gespeichert werden.': ['Could not save daily-start readings.', 'Tagesstartwerte konnten nicht gespeichert werden.', 'Dagstartwaarden konden niet worden opgeslagen.', 'Impossible d’enregistrer les relevés de début de journée.', 'Nie udało się zapisać wskazań początkowych dnia.'],
  'Tagesstart muss positiv sein und innerhalb ±400 kWh des aktuellen Standes liegen.': ['Daily start must be positive and within ±400 kWh of the current reading.', 'Tagesstart muss positiv sein und innerhalb ±400 kWh des aktuellen Standes liegen.', 'Dagstart moet positief zijn en binnen ±400 kWh van de huidige stand liggen.', 'Le relevé de début doit être positif et à moins de ±400 kWh du relevé actuel.', 'Wskazanie początkowe musi być dodatnie i mieścić się w zakresie ±400 kWh od aktualnego.'],
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
  'Version': ['Version', 'Version', 'Versie', 'Version', 'Wersja'],
  'Bezugspreis (je kWh)': ['Import price (per kWh)', 'Bezugspreis (je kWh)', 'Afnameprijs (per kWh)', 'Prix d’achat (par kWh)', 'Cena poboru (za kWh)'],
  'Einspeisevergütung (je kWh)': ['Feed-in tariff (per kWh)', 'Einspeisevergütung (je kWh)', 'Terugleververgoeding (per kWh)', 'Rémunération d’injection (par kWh)', 'Wynagrodzenie za oddawanie (za kWh)'],
  'Wallbox-Erstattung (je kWh)': ['Wallbox reimbursement (per kWh)', 'Wallbox-Erstattung (je kWh)', 'Wallbox-vergoeding (per kWh)', 'Remboursement wallbox (par kWh)', 'Zwrot za wallbox (za kWh)'],
  'Rolle für Statistik': ['Statistics role', 'Rolle für Statistik', 'Rol voor statistiek', 'Rôle pour les statistiques', 'Rola w statystykach'],
  'Statistik': ['Statistics', 'Statistik', 'Statistieken', 'Statistiques', 'Statystyki'],
  'Steuerung': ['Control', 'Steuerung', 'Besturing', 'Commande', 'Sterowanie'],
  'Tage': ['Days', 'Tage', 'Dagen', 'Jours', 'Dni'],
  'Wochen': ['Weeks', 'Wochen', 'Weken', 'Semaines', 'Tygodnie'],
  'Monate': ['Months', 'Monate', 'Maanden', 'Mois', 'Miesiące'],
  'Kurzname / Entity-ID / Topic': ['Short name / entity ID / topic', 'Kurzname / Entity-ID / Topic', 'Korte naam / entiteit-ID / topic', 'Nom court / ID d’entité / topic', 'Nazwa skrócona / ID encji / temat'],
  'REST-Abfrage': ['REST query', 'REST-Abfrage', 'REST-opvraag', 'Requête REST', 'Zapytanie REST'],
  'Tageswerte werden aus diesen Ständen berechnet und im Gerätespeicher gesichert.': ['Daily values are calculated from these readings and stored in device memory.', 'Tageswerte werden aus diesen Ständen berechnet und im Gerätespeicher gesichert.', 'Dagwaarden worden uit deze standen berekend en in het apparaatgeheugen opgeslagen.', 'Les valeurs journalières sont calculées à partir de ces relevés et enregistrées dans la mémoire de l’appareil.', 'Wartości dzienne są obliczane z tych odczytów i zapisywane w pamięci urządzenia.'],
  'Home-Assistant-Automation erzeugen': ['Create Home Assistant automation', 'Home-Assistant-Automation erzeugen', 'Home Assistant-automatisering maken', 'Créer une automatisation Home Assistant', 'Utwórz automatyzację Home Assistant'],
  'Aktuelle Zählerstände werden geladen …': ['Loading current meter readings …', 'Aktuelle Zählerstände werden geladen …', 'Actuele meterstanden worden geladen …', 'Chargement des relevés actuels …', 'Wczytywanie aktualnych odczytów …'],
  'GNU AGPLv3: Weitergabe und Änderung erlaubt': ['GNU AGPLv3: redistribution and modification permitted', 'GNU AGPLv3: Weitergabe und Änderung erlaubt', 'GNU AGPLv3: verspreiden en wijzigen toegestaan', 'GNU AGPLv3 : redistribution et modification autorisées', 'GNU AGPLv3: dozwolone rozpowszechnianie i modyfikacja'],
  'GNU AGPLv3: Weitergabe und Änderung erlaubt. Keine Gewährleistung.': ['GNU AGPLv3: redistribution and modification permitted. No warranty.', 'GNU AGPLv3: Weitergabe und Änderung erlaubt. Keine Gewährleistung.', 'GNU AGPLv3: verspreiden en wijzigen toegestaan. Geen garantie.', 'GNU AGPLv3 : redistribution et modification autorisées. Aucune garantie.', 'GNU AGPLv3: dozwolone rozpowszechnianie i modyfikacja. Bez gwarancji.'],
  'Spitze Bezug (1 h)': ['Peak import (1 h)', 'Spitze Bezug (1 h)', 'Piek afname (1 u)', 'Pic de soutirage (1 h)', 'Szczyt poboru (1 h)'],
  'Spitze Einspeisung (1 h)': ['Peak feed-in (1 h)', 'Spitze Einspeisung (1 h)', 'Piek teruglevering (1 u)', 'Pic d’injection (1 h)', 'Szczyt oddawania (1 h)'],
  'Strom je Phase': ['Current per phase', 'Strom je Phase', 'Stroom per fase', 'Courant par phase', 'Prąd na fazę'],
  'Ersparnis heute': ['Savings today', 'Ersparnis heute', 'Besparing vandaag', 'Économie aujourd\'hui', 'Oszczędność dzisiaj'],
  'kW netto': ['kW net', 'kW netto', 'kW netto', 'kW net', 'kW netto']
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

function tlsModeChanged() {
  const mode = document.getElementById('tls-mode');
  const acme = document.getElementById('tls-acme-fields');
  const manual = document.getElementById('tls-manual-fields');
  const letsEncrypt = !!mode && mode.value === 'letsencrypt';
  if (acme) acme.hidden = !letsEncrypt;
  if (manual) manual.hidden = letsEncrypt;
  updateAcmeGate();
}

let acmeStatus = null;
let tlsCertInfo = null;
let acmePollTimer = null;

const ACME_CHECK_LABELS = {
  internet: 'Internetzugang und Uhrzeit',
  dns: 'DNS: Hostname zeigt auf dieses Gerät',
  http: 'Erreichbarkeit von außen über Port 80 (HTTP)'
};
const ACME_CHECK_ICONS = { pass: '✔', fail: '✖', warn: '⚠', running: '…', pending: '○', skipped: '–' };

function tlsFormValue(id) {
  const element = document.getElementById(id);
  return element ? element.value.trim() : '';
}

function tlsFormChecked(id) {
  const element = document.getElementById(id);
  return !!(element && element.checked);
}

function acmeBusy() {
  return !!acmeStatus && (acmeStatus.busy || acmeStatus.state === 'requesting' || acmeStatus.checks_state === 'running');
}

function acmeChecksPassed() {
  return !!acmeStatus && acmeStatus.checks_state === 'passed' &&
    acmeStatus.checks_fqdn === tlsFormValue('tls-fqdn') &&
    acmeStatus.checks_age_s >= 0 && acmeStatus.checks_age_s <= 3600;
}

function updateAcmeGate() {
  const checkButton = document.getElementById('acme-check-btn');
  const requestButton = document.getElementById('acme-request-btn');
  const hint = document.getElementById('acme-gate-hint');
  if (!checkButton || !requestButton) return;
  const termsOk = tlsFormChecked('tls-terms');
  const fqdnOk = /^[A-Za-z0-9-]+(\.[A-Za-z0-9-]+)+$/.test(tlsFormValue('tls-fqdn'));
  const emailOk = /^[^@\s]+@[^@\s]+\.[^@\s]+$/.test(tlsFormValue('tls-email'));
  const ready = termsOk && fqdnOk && emailOk && !acmeBusy();
  checkButton.disabled = !ready;
  requestButton.disabled = !(ready && acmeChecksPassed());
  if (hint) {
    hint.textContent = !termsOk ? localizedText('Zuerst die Nutzungsbedingungen akzeptieren.')
      : !(fqdnOk && emailOk) ? localizedText('Gültigen Hostnamen und E-Mail-Adresse eintragen.')
      : !acmeChecksPassed() ? localizedText('Erst nach bestandenen Tests freigegeben.')
      : localizedText('Tests bestanden: Die Anforderung ist freigegeben (1 Stunde gültig).');
  }
}

function renderAcmeChecks(status) {
  const list = document.getElementById('acme-checks');
  if (!list) return;
  const checks = status.checks_state === 'idle' || !Array.isArray(status.checks) ? [] : status.checks;
  list.replaceChildren();
  checks.forEach(check => {
    const row = document.createElement('div');
    row.className = 'check-row ' + (check.state || 'pending');
    const icon = document.createElement('span');
    icon.className = 'check-icon';
    icon.textContent = ACME_CHECK_ICONS[check.state] || ACME_CHECK_ICONS.pending;
    const label = document.createElement('span');
    label.textContent = localizedText(ACME_CHECK_LABELS[check.id] || check.id);
    row.append(icon, label);
    if (check.detail) {
      const detail = document.createElement('span');
      detail.className = 'check-detail';
      detail.textContent = check.detail;
      row.appendChild(detail);
    }
    list.appendChild(row);
  });
}

function renderAcmeLog(lines) {
  const box = document.getElementById('acme-log');
  if (!box) return;
  const list = Array.isArray(lines) ? lines : [];
  box.hidden = list.length === 0;
  if (box.hidden) return;
  box.textContent = list.join('\n');
  box.scrollTop = box.scrollHeight;
}

function renderAcmeStatus(status) {
  acmeStatus = status;
  if (status.state === 'issued') status.log = [];
  renderAcmeChecks(status);
  renderAcmeLog(status.log);
  const link = document.getElementById('acme-terms-link');
  if (link && /^https:\/\//.test(status.terms_url || '')) link.href = status.terms_url;
  const line = document.getElementById('acme-status');
  if (line) {
    line.textContent = status.state === 'requesting' ? localizedText('Zertifikat wird angefordert …')
      : status.checks_state === 'running' ? localizedText('Tests laufen …')
      : status.state === 'issued' ? localizedText('Zertifikat ausgestellt. HTTPS wird neu gestartet.')
      : status.state === 'failed' ? localizedText('Anforderung fehlgeschlagen') + (status.error ? ' (' + status.error + ')' : '')
      : status.checks_state === 'passed' ? localizedText('Alle Tests bestanden. Das Zertifikat kann jetzt angefordert werden.')
      : status.checks_state === 'failed' ? localizedText('Mindestens ein Test ist fehlgeschlagen. Bitte die Hinweise beachten und die Tests wiederholen.')
      : '';
  }
  updateAcmeGate();
}

async function refreshAcmeStatus() {
  try {
    const response = await fetch('/api/acme/status');
    if (!response.ok) return null;
    const status = await response.json();
    renderAcmeStatus(status);
    return status;
  } catch (_) {
    return null;
  }
}

async function pollAcmeStatus() {
  const status = await refreshAcmeStatus();
  // While port 80 is swapped for the validation listener a page loaded over HTTP cannot poll; keep trying.
  if (status && !acmeBusy()) {
    setAcmePause(false);
    clearInterval(acmePollTimer);
    acmePollTimer = null;
    if (status.state === 'issued') {
      acmeStatus = null;
      renderAcmeChecks({});
      renderAcmeLog([]);
      const line = document.getElementById('acme-status');
      if (line) line.textContent = '';
      updateAcmeGate();
      setTimeout(loadTlsConfig, 3000);
    }
  }
}

function setAcmePause(show) {
  let box = document.getElementById('acme-pause');
  if (!show) { if (box) box.remove(); return; }
  if (box) return;
  box = document.createElement('div');
  box.id = 'acme-pause';
  box.style.cssText = 'position:fixed;inset:0;z-index:9999;background:rgba(0,0,0,.65);display:flex;align-items:center;justify-content:center;padding:16px';
  const card = document.createElement('div');
  card.style.cssText = 'background:#fff;color:#111;max-width:420px;padding:20px 24px;border-radius:10px;font:15px/1.4 sans-serif;text-align:center';
  card.textContent = localizedText('Die Verbindung zum Gerät wird jetzt pausiert und kommt gleich wieder. Smartmeter-Auslesung, MQTT und HTTPS ruhen während der Prüfung, damit der Test genug Speicher hat. Diese Meldung verschwindet automatisch.');
  box.appendChild(card);
  document.body.appendChild(box);
  setTimeout(() => setAcmePause(false), 240000);
}

function startAcmePolling() {
  setAcmePause(true);
  if (!acmePollTimer) acmePollTimer = setInterval(pollAcmeStatus, 2000);
}

async function loadTlsConfig() {
  try {
    const response = await fetch('/api/config/tls');
    if (!response.ok) return;
    const config = await response.json();
    tlsCertInfo = config;
    const editing = id => document.activeElement === document.getElementById(id);
    const setValue = (id, value) => {
      const element = document.getElementById(id);
      if (element && !editing(id)) element.value = value;
    };
    const setChecked = (id, value) => {
      const element = document.getElementById(id);
      if (element) element.checked = !!value;
    };
    setValue('tls-fqdn', config.fqdn || '');
    setValue('tls-mode', config.mode === 'letsencrypt' ? 'letsencrypt' : 'manual');
    setValue('tls-email', config.email || '');
    setValue('tls-interval', config.renewal_interval_days || 60);
    setChecked('tls-terms', config.acme_terms_accepted);
    setChecked('tls-staging', config.acme_staging);
    const status = document.getElementById('tls-status');
    if (status) {
      const source = config.certificate_source === 'letsencrypt' ? 'Zertifikat: Let\'s Encrypt'
        : config.certificate_source === 'manual' ? 'Zertifikat: eigenes' : 'Zertifikat: Self-Signed';
      let text = localizedText(config.https_active ? 'HTTPS aktiv (Port 443)' : 'HTTPS nicht aktiv') +
        ' · ' + localizedText(source);
      if (config.last_issued_epoch > 0) text += ' · ' + new Date(config.last_issued_epoch * 1000).toLocaleDateString();
      status.textContent = text;
    }
    const acme = await refreshAcmeStatus();
    tlsModeChanged();
    if (acme && acmeBusy()) startAcmePolling();
  } catch (_) {}
}

function tlsFormPayload() {
  return {
    mode: tlsFormValue('tls-mode') || 'manual',
    fqdn: tlsFormValue('tls-fqdn'),
    email: tlsFormValue('tls-email'),
    acme_terms_accepted: tlsFormChecked('tls-terms'),
    acme_staging: tlsFormChecked('tls-staging'),
    renewal_interval_days: parseInt(tlsFormValue('tls-interval'), 10) || 60,
    certificate_pem: tlsFormValue('tls-cert'),
    private_key_pem: tlsFormValue('tls-key'),
    clear_certificate: tlsFormChecked('tls-clear')
  };
}

async function postTlsConfig() {
  try {
    const response = await fetch('/api/config/tls', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify(tlsFormPayload())
    });
    if (response.ok) return await response.json();
  } catch (_) {}
  return { ok: false };
}

async function saveTlsConfig(event) {
  event.preventDefault();
  const result = await postTlsConfig();
  alert(localizedText(result.ok ? 'HTTPS-Einstellungen gespeichert. Der HTTPS-Server startet neu.' : 'HTTPS-Einstellungen konnten nicht gespeichert werden.'));
  if (result.ok) {
    ['tls-cert', 'tls-key'].forEach(id => { const element = document.getElementById(id); if (element) element.value = ''; });
    const clear = document.getElementById('tls-clear');
    if (clear) clear.checked = false;
    setTimeout(loadTlsConfig, 3000);
  }
}

async function runAcmeChecks() {
  const line = document.getElementById('acme-status');
  const saved = await postTlsConfig();
  if (!saved.ok) {
    if (line) line.textContent = localizedText('HTTPS-Einstellungen konnten nicht gespeichert werden.');
    return;
  }
  let ok = false;
  let reason = '';
  try {
    const response = await fetch('/api/acme/check', { method: 'POST' });
    ok = response.ok;
    if (!ok) reason = (await response.json()).reason || '';
  } catch (_) {}
  if (!ok) {
    if (line) line.textContent = localizedText('Tests konnten nicht gestartet werden.') + (reason ? ' (' + reason + ')' : '');
    return;
  }
  if (line) {
    line.textContent = localizedText('Tests laufen …') +
      (location.protocol === 'http:' ? ' ' + localizedText('Live-Protokoll nur per HTTPS sichtbar (Port 80 ist während der Prüfung belegt).') : '');
  }
  acmeStatus = Object.assign({}, acmeStatus || {}, { checks_state: 'running' });
  updateAcmeGate();
  startAcmePolling();
}

async function requestAcmeCertificate() {
  const staging = document.getElementById('tls-staging');
  if (staging && !staging.checked &&
      !confirm(localizedText('Produktivumgebung ohne Staging-Test: Fehlversuche zählen gegen die Rate-Limits von Let\'s Encrypt. Trotzdem fortfahren?'))) return;
  const info = tlsCertInfo;
  const fqdn = tlsFormValue('tls-fqdn').toLowerCase();
  if (info && info.certificate_source === 'letsencrypt' && !info.cert_staging &&
      info.cert_days_left >= 30 && (info.cert_cn || '').toLowerCase() === fqdn &&
      !confirm(localizedText('Für "') + info.cert_cn + localizedText('" ist bereits ein gültiges Let\'s-Encrypt-Zertifikat vorhanden (noch ') +
        info.cert_days_left + localizedText(' Tage gültig). Bei einer Neuausstellung wird das aktuelle Zertifikat jetzt entfernt und die HTTPS-Verbindung des Browsers bricht kurz ab. Neu ausstellen?'))) return;
  const line = document.getElementById('acme-status');
  const saved = await postTlsConfig();
  if (!saved.ok) {
    if (line) line.textContent = localizedText('HTTPS-Einstellungen konnten nicht gespeichert werden.');
    return;
  }
  if (saved.https_restarting) {
    // The HTTPS listener restarts after a certificate-relevant change; a request sent now would be dropped.
    if (line) line.textContent = localizedText('Einstellungen gespeichert, warte auf den HTTPS-Neustart …');
    await new Promise(resolve => setTimeout(resolve, 4000));
    for (let i = 0; i < 12; i++) {
      try { if ((await fetch('/api/status', { cache: 'no-store' })).ok) break; } catch (_) {}
      await new Promise(resolve => setTimeout(resolve, 2000));
    }
  }
  let ok = false;
  let reason = '';
  let reached = false;
  for (let attempt = 0; attempt < 3 && !reached; attempt++) {
    try {
      const response = await fetch('/api/acme/request', { method: 'POST' });
      reached = true;
      ok = response.ok;
      if (!ok) reason = (await response.json()).reason || '';
    } catch (_) {
      await new Promise(resolve => setTimeout(resolve, 3000));
    }
  }
  if (!ok) {
    if (line) line.textContent = reached
      ? localizedText('Anforderung konnte nicht gestartet werden. Zuerst Einstellungen speichern (Hostname, E-Mail, Bedingungen).') + (reason ? ' (' + reason + ')' : '')
      : localizedText('Das Gerät hat nicht geantwortet. Bitte die Seite neu laden und die Anforderung wiederholen.');
    return;
  }
  if (line) {
    line.textContent = localizedText('Zertifikat wird angefordert …') +
      (location.protocol === 'http:' ? ' ' + localizedText('Live-Protokoll nur per HTTPS sichtbar (Port 80 ist während der Prüfung belegt).') : '');
  }
  acmeStatus = Object.assign({}, acmeStatus || {}, { state: 'requesting' });
  updateAcmeGate();
  startAcmePolling();
}

const EXT_PRESETS = [
  { label: 'PV-Erzeugung gesamt', ref: 'sensor.pv_energy_total', role: 'pv_kwh' },
  { label: 'Batterie geladen gesamt', ref: 'sensor.battery_charge_total', role: 'bat_chg_kwh' },
  { label: 'Batterie entladen gesamt', ref: 'sensor.battery_discharge_total', role: 'bat_dis_kwh' },
  { label: 'PV-Batterie Ladestand', ref: 'sensor.battery_state_of_charge' },
  { label: 'Wallbox gesamt', ref: 'sensor.wallbox_energy_total', role: 'wb_kwh' },
  { label: 'PV Ost', ref: 'sensor.pv_east_power', role: 'pv_east' },
  { label: 'PV S\u00fcd', ref: 'sensor.pv_south_power', role: 'pv_south' },
  { label: 'PV West', ref: 'sensor.pv_west_power', role: 'pv_west' },
  { label: 'PV-Leistung gesamt', ref: 'sensor.pv_power', role: 'pv' },
  { label: 'Batterie-Leistung', ref: 'sensor.battery_power', role: 'battery' },
  { label: 'Ladeleistung', ref: 'sensor.wallbox_power', role: 'wb_power' },
  { label: 'Auto Name', ref: 'sensor.car_name', role: 'car_name' },
  { label: 'Auto Ladestand', ref: 'sensor.car_battery_level' },
  { label: 'Ladevorgang', ref: 'sensor.wallbox_session_energy', role: 'wb_session_kwh' },
  { label: 'Lademodus', ref: 'sensor.wallbox_mode', role: 'wb_mode' }
];
const EXT_STATE_TEXT = { ok: 'ok', stale: 'veraltet', error: 'Fehler', waiting: 'wartet' };
let extLiveTimer = null;

function extInput(className, value, placeholder, maxLength) {
  const input = document.createElement('input');
  input.type = 'text';
  input.className = className;
  input.maxLength = maxLength;
  input.value = value || '';
  if (placeholder) input.placeholder = placeholder;
  return input;
}

function renderExtRows(slots) {
  const box = document.getElementById('ext-rows');
  if (!box) return;
  box.replaceChildren();
  for (let index = 0; index < 16; index++) {
    const slot = slots[index] || {};
    const preset = EXT_PRESETS[index] || { label: '', ref: '' };
    const row = document.createElement('div');
    row.className = 'ext-row';
    const source = document.createElement('select');
    source.className = 'ext-source';
    [['rest', 'REST (Entity-ID)'], ['mqtt', 'MQTT (Topic)']].forEach(([value, text]) => {
      const option = document.createElement('option');
      option.value = value;
      option.textContent = text;
      source.appendChild(option);
    });
    source.value = slot.source === 'mqtt' ? 'mqtt' : 'rest';
    row.append(
      extInput('ext-label', slot.label, preset.label, 40),
      source,
      extInput('ext-ref', slot.ref, preset.ref, 128),
      extInput('ext-unit', slot.unit, '', 12),
      extInput('ext-key', slot.key, '', 40)
    );
    const role = document.createElement('select');
    role.className = 'ext-role';
    [['', 'Nur anzeigen'], ['pv_kwh', 'PV-Zähler (kWh gesamt)'], ['bat_chg_kwh', 'Batterie geladen (kWh gesamt)'], ['bat_dis_kwh', 'Batterie entladen (kWh gesamt)'], ['pv', 'PV-Leistung (W)'], ['battery', 'Batterie (+ = laden)'], ['battery_neg', 'Batterie (+ = entladen)'], ['pv_east', 'PV Ost (Leistung)'], ['pv_south', 'PV S\u00fcd (Leistung)'], ['pv_west', 'PV West (Leistung)'], ['wb_kwh', 'Wallbox-Z\u00e4hler (kWh gesamt)'], ['wb_power', 'Ladeleistung Wallbox'], ['wb_session_kwh', 'Ladevorgang (kWh)'], ['car_name', 'Auto-Name (Text)'], ['wb_mode', 'Lademodus (PV-Only/Both/Fast)']].forEach(([value, text]) => {
      const option = document.createElement('option');
      option.value = value;
      option.textContent = text;
      role.appendChild(option);
    });
    role.value = (slot.ref || slot.label) ? (slot.role || '') : (preset.role || '');
    row.appendChild(role);
    box.appendChild(row);
  }
}

function extFormPayload() {
  const value = id => {
    const element = document.getElementById(id);
    return element ? element.value.trim() : '';
  };
  const checked = id => {
    const element = document.getElementById(id);
    return !!(element && element.checked);
  };
  const slots = Array.from(document.querySelectorAll('#ext-rows .ext-row')).map(row => ({
    label: row.querySelector('.ext-label').value.trim(),
    source: row.querySelector('.ext-source').value,
    ref: row.querySelector('.ext-ref').value.trim(),
    unit: row.querySelector('.ext-unit').value.trim(),
    key: row.querySelector('.ext-key').value.trim(),
    role: row.querySelector('.ext-role').value
  })).filter(slot => slot.ref || slot.label);
  return {
    enabled: checked('ext-enabled'),
    ha_url: value('ext-url'),
    ha_token: value('ext-token'),
    clear_token: checked('ext-clear-token'),
    poll_s: parseInt(value('ext-poll'), 10) || 30,
    slots: slots
  };
}

function formatExtValue(value) {
  const magnitude = Math.abs(value);
  const digits = magnitude >= 100 ? 0 : magnitude >= 10 ? 1 : 2;
  return value.toLocaleString(undefined, { maximumFractionDigits: digits });
}

function formatExtAge(seconds) {
  if (seconds < 0) return '—';
  return seconds < 90 ? seconds + ' s' : Math.round(seconds / 60) + ' min';
}

function renderExtLive(data) {
  const list = document.getElementById('ext-live');
  if (!list) return;
  list.replaceChildren();
  const items = Array.isArray(data.items) ? data.items : [];
  if (!items.length) {
    list.textContent = localizedText('Keine Werte konfiguriert.');
    return;
  }
  items.forEach(item => {
    const row = document.createElement('div');
    row.className = 'check-row ' + (item.state === 'ok' ? 'pass' : item.state === 'error' ? 'fail' : item.state === 'stale' ? 'warn' : 'pending');
    const icon = document.createElement('span');
    icon.className = 'check-icon';
    icon.textContent = item.state === 'ok' ? '✔' : item.state === 'error' ? '✖' : item.state === 'stale' ? '⚠' : '…';
    const label = document.createElement('span');
    const valueText = typeof item.text === 'string' ? item.text : typeof item.value === 'number' ? formatExtValue(item.value) + (item.unit ? ' ' + item.unit : '') : '—';
    label.textContent = item.label + ': ' + valueText + ' · ' + localizedText(EXT_STATE_TEXT[item.state] || item.state) +
      (item.age_s >= 0 ? ' · ' + formatExtAge(item.age_s) : '');
    row.append(icon, label);
    if (item.error) {
      const detail = document.createElement('span');
      detail.className = 'check-detail';
      detail.textContent = item.error;
      row.appendChild(detail);
    }
    list.appendChild(row);
  });
}

async function loadExtLive() {
  try {
    const response = await fetch('/api/ext/values');
    if (response.ok) renderExtLive(await response.json());
  } catch (_) {}
}

async function loadExtConfig() {
  try {
    const response = await fetch('/api/config/ext');
    if (!response.ok) return;
    const config = await response.json();
    const setValue = (id, value) => {
      const element = document.getElementById(id);
      if (element) element.value = value;
    };
    const enabled = document.getElementById('ext-enabled');
    if (enabled) enabled.checked = !!config.enabled;
    setValue('ext-url', config.ha_url || '');
    setValue('ext-poll', config.poll_s || 30);
    const token = document.getElementById('ext-token');
    if (token) token.placeholder = config.token_set ? localizedText('Gesetzt – leer lassen, um es zu behalten') : '';
    renderExtRows(Array.isArray(config.slots) ? config.slots : []);
    const status = document.getElementById('ext-status');
    if (status) {
      status.textContent = !config.mqtt_enabled ? localizedText('MQTT ist ausgeschaltet (Home-Assistant-Integration aktivieren, um MQTT-Werte zu nutzen).')
        : config.mqtt_connected ? localizedText('MQTT-Broker verbunden.') : localizedText('MQTT-Broker nicht verbunden.');
    }
    loadExtLive();
    if (!extLiveTimer) extLiveTimer = setInterval(loadExtLive, 3000);
  } catch (_) {}
}

async function saveExtConfig(event) {
  event.preventDefault();
  let result = { ok: false };
  try {
    const response = await fetch('/api/config/ext', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify(extFormPayload())
    });
    result = await response.json();
  } catch (_) {}
  alert(result.ok ? localizedText('Externe Daten gespeichert.')
    : localizedText('Externe Daten konnten nicht gespeichert werden.') + (result.reason ? '\n' + result.reason : ''));
  if (result.ok) {
    const token = document.getElementById('ext-token');
    if (token) token.value = '';
    const clear = document.getElementById('ext-clear-token');
    if (clear) clear.checked = false;
    loadExtConfig();
    setTimeout(loadExtLive, 3000);
  }
}

async function refreshExtNow() {
  try { await fetch('/api/ext/refresh', { method: 'POST' }); } catch (_) {}
  setTimeout(loadExtLive, 2500);
}

async function fetchExt() {
  const card = document.getElementById('ext-card');
  const grid = document.getElementById('ext-items');
  if (!card || !grid) return;
  try {
    const response = await fetch('/api/ext/values');
    if (!response.ok) return;
    const data = await response.json();
    const items = Array.isArray(data.items) ? data.items : [];
    card.hidden = !(data.enabled && items.length);
    grid.replaceChildren();
    items.forEach(item => {
      const tile = document.createElement('div');
      tile.className = 'ext-tile ext-' + item.state;
      if (item.error) tile.title = item.error;
      const label = document.createElement('div');
      label.className = 'ext-tile-label';
      label.textContent = item.label;
      const value = document.createElement('div');
      value.className = 'ext-tile-value';
      value.textContent = typeof item.text === 'string' ? item.text : typeof item.value === 'number' ? formatExtValue(item.value) + (item.unit ? ' ' + item.unit : '') : '—';
      const role = item.role || (/wallbox/i.test(item.label || '') && /^k?wh$/i.test(item.unit || '') ? 'wb_kwh' : '');
      const todayKey = EXT_TODAY_KEYS[role];
      if (todayKey && typeof item.value === 'number') {
        const unitWh = item.unit === 'Wh' ? 1 : item.unit === 'MWh' ? 1e6 : 1000;
        tile.className += ' ext-big';
        const row = document.createElement('div');
        row.className = 'ext-row';
        const today = document.createElement('div');
        today.className = 'ext-today';
        today.dataset.today = todayKey;
        tile.append(label, row, today);
        if (role === 'pv_kwh') {
          const sav = document.createElement('div');
          sav.className = 'ext-today';
          sav.dataset.saving = 'today_saving';
          tile.appendChild(sav);
        }
        row.append(extIcon(role), extCounter(item.value * unitWh));
        grid.appendChild(tile);
        return;
      }
      if (typeof item.value === 'number' && item.unit === '%') {
        const pct = Math.max(0, Math.min(100, item.value));
        const color = pct < 40 ? 'red' : pct > 60 ? 'green' : 'yellow';
        const on = pct > 0 ? Math.max(1, Math.round(pct / 20)) : 0;
        const isCar = /auto|fahrzeug|\bcar\b|\bev\b/i.test(item.label || '');
        const batt = document.createElement('div');
        batt.className = 'batt';
        const cells = document.createElement('div');
        cells.className = 'batt-cells';
        for (let i = 0; i < 5; i++) {
          const c = document.createElement('i');
          if (i < on) c.className = color;
          cells.appendChild(c);
        }
        const num = document.createElement('span');
        num.className = 'batt-pct';
        num.textContent = Math.round(pct) + ' %';
        const trend = document.createElement('span');
        trend.className = 'batt-trend';
        if (!isCar) trend.id = 'batt-trend';
        batt.append(trend, cells, num);
        const age = document.createElement('div');
        age.className = 'ext-tile-age';
        age.textContent = formatExtAge(item.age_s);
        tile.append(label, batt, age);
        grid.appendChild(tile);
        if (isCar) {
          const st = carTrend[item.label] || (carTrend[item.label] = {prev: null, dir: '', at: 0});
          if (st.prev != null && Math.abs(pct - st.prev) >= 1) { st.dir = pct > st.prev ? 'up' : 'down'; st.at = Date.now(); }
          st.prev = pct;
          if (st.dir && Date.now() - st.at < 300000) {
            trend.className = 'batt-trend ' + st.dir;
            trend.textContent = st.dir === 'up' ? '\u25B2' : '\u25BC';
          }
          const ico = document.createElement('div');
          ico.className = 'batt-car';
          ico.textContent = '\uD83D\uDE97';
          num.prepend(ico);
        } else {
          if (battPrevSoc != null && Math.abs(pct - battPrevSoc) >= 1) { battTrend = pct > battPrevSoc ? 'up' : 'down'; battTrendAt = Date.now(); }
          battPrevSoc = pct;
        }
        return;
      }
      tile.append(label, value);
      if (typeof item.value === 'number' && item.unit === '%' && item.value >= 0 && item.value <= 100) {
        const bar = document.createElement('div');
        bar.className = 'ext-bar';
        const fill = document.createElement('i');
        fill.style.width = item.value + '%';
        bar.appendChild(fill);
        tile.appendChild(bar);
      }
      const age = document.createElement('div');
      age.className = 'ext-tile-age';
      age.textContent = item.state === 'error' ? localizedText('Fehler') : formatExtAge(item.age_s);
      tile.appendChild(age);
      grid.appendChild(tile);
    });
    if (items.some(i => i.role === 'pv_kwh')) {
      const tile = document.createElement('div');
      tile.className = 'ext-tile';
      const label = document.createElement('div');
      label.className = 'ext-tile-label';
      label.textContent = localizedText('Hausverbrauch heute');
      const value = document.createElement('div');
      value.className = 'ext-tile-value';
      value.id = 'ext-house-today';
      value.textContent = '?';
      const row = document.createElement('div');
      row.className = 'ext-row';
      row.append(extIcon('house'), value);
      tile.classList.add('ext-big');
      tile.append(label, row);
      grid.appendChild(tile);
    }
    renderExtToday();
  } catch (_) {}
}

const EXT_ICONS = {
  pv_kwh: ['#b7791f', '#fff1c2', '<circle cx="12" cy="12" r="4.2"/><path d="M12 2.5v3M12 18.5v3M2.5 12h3M18.5 12h3M5.3 5.3l2.1 2.1M16.6 16.6l2.1 2.1M18.7 5.3l-2.1 2.1M7.4 16.6l-2.1 2.1" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round"/>'],
  bat_chg_kwh: ['#1e9e5a', '#dcf7e7', '<path d="M13.5 2 5 13.5h6L10 22l9-12h-6.2z"/>'],
  bat_dis_kwh: ['#e5484d', '#fde4e4', '<path d="M13.5 2 5 13.5h6L10 22l9-12h-6.2z"/>'],
  wb_kwh: ['#6a5acd', '#e9e5fb', '<path d="M8 2v5M16 2v5M6 7h12v4a6 6 0 0 1-12 0zM12 17v5" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"/>'],
  house: ['#2f7de1', '#dfeeff', '<path d="M12 3 2.5 11.5H5V21h5.5v-6h3v6H19v-9.5h2.5z"/>']
};

function extIcon(role) {
  const [fg, bg, shape] = EXT_ICONS[role] || EXT_ICONS.house;
  const el = document.createElement('div');
  el.className = 'ext-ico';
  el.style.background = bg;
  el.style.color = fg;
  el.innerHTML = '<svg viewBox="0 0 24 24" width="24" height="24" fill="currentColor" aria-hidden="true">' + shape + '</svg>';
  return el;
}

function extCounter(valueWh) {
  const wrap = document.createElement('div');
  wrap.className = 'ext-counter';
  const [whole, fraction] = (Number(valueWh) / 1000).toFixed(1).split('.');
  const num = document.createElement('span');
  num.className = 'ext-num';
  num.textContent = whole + '.' + fraction;
  const unit = document.createElement('span');
  unit.className = 'ext-unit';
  unit.textContent = 'kWh';
  wrap.append(num, unit);
  return wrap;
}

const EXT_TODAY_KEYS = {pv_kwh: 'today_pv_kwh', bat_chg_kwh: 'today_bat_chg_kwh', bat_dis_kwh: 'today_bat_dis_kwh', wb_kwh: 'today_wb_kwh'};
let lastStatusData = null;
const powerSamples = [];

function updatePowerBar(netPower, d) {
  const now = Date.now();
  const imp = Math.max(0, netPower);
  const exp = Math.max(0, -netPower);
  const last = powerSamples[powerSamples.length - 1];
  if (!last || now - last[0] >= 5000) powerSamples.push([now, imp, exp]);
  else { last[1] = Math.max(last[1], imp); last[2] = Math.max(last[2], exp); }
  while (powerSamples.length && now - powerSamples[0][0] > 3600000) powerSamples.shift();
  const peakImp = Math.max(0, ...powerSamples.map(s => s[1]));
  const peakExp = Math.max(0, ...powerSamples.map(s => s[2]));
  const mag = Math.abs(netPower);
  const scale = Math.max(6000, peakImp, peakExp);
  const fill = document.getElementById('pbar-fill');
  if (fill) {
    fill.style.width = Math.min(100, 100 * mag / scale) + '%';
    fill.className = 'pbar-fill' + (netPower < 0 ? ' export' : '');
  }
  const max = document.getElementById('pbar-max');
  if (max) max.textContent = (scale / 1000).toFixed(scale % 1000 ? 1 : 0) + ' kW';
  const setText = (id, text) => { const e = document.getElementById(id); if (e) e.textContent = text; };
  setText('peak-imp', (peakImp / 1000).toFixed(2) + ' kW');
  setText('peak-exp', (peakExp / 1000).toFixed(2) + ' kW');
  const amps = [d.has_i_l1 ? d.i_l1_ma : null, d.has_i_l2 ? d.i_l2_ma : null, d.has_i_l3 ? d.i_l3_ma : null];
  setText('phase-cur', amps.some(a => a != null) ? amps.map(a => a == null ? '\u2014' : (a / 1000).toFixed(1)).join(' / ') + ' A' : '\u2014');
}

const carTrend = {};
let battTrend = '', battTrendAt = 0, battPrevSoc = null, battPrevCounters = null;

function updateBattTrend(d) {
  const chg = Number(d.today_bat_chg_kwh), dis = Number(d.today_bat_dis_kwh);
  if (Number.isFinite(chg) && Number.isFinite(dis)) {
    if (battPrevCounters) {
      if (chg > battPrevCounters.chg + 0.0005) { battTrend = 'up'; battTrendAt = Date.now(); }
      else if (dis > battPrevCounters.dis + 0.0005) { battTrend = 'down'; battTrendAt = Date.now(); }
    }
    battPrevCounters = {chg, dis};
  }
  if (battTrend && Date.now() - battTrendAt > 300000) battTrend = '';
  const el = document.getElementById('batt-trend');
  if (el) {
    el.className = 'batt-trend' + (battTrend ? ' ' + battTrend : '');
    el.textContent = battTrend === 'up' ? '\u25B2' : battTrend === 'down' ? '\u25BC' : '';
  }
}

function renderExtToday() {
  const d = lastStatusData;
  if (!d) return;
  updateBattTrend(d);
  document.querySelectorAll('[data-today]').forEach(el => {
    const v = d[el.dataset.today];
    el.replaceChildren(localizedText('Heute') + ' ');
    const b = document.createElement('b');
    b.textContent = typeof v === 'number' ? v.toFixed(2) + ' kWh' : '?';
    el.appendChild(b);
  });
  document.querySelectorAll('[data-saving]').forEach(el => {
    const v = d[el.dataset.saving];
    el.replaceChildren(localizedText('Ersparnis heute') + ' ');
    const b = document.createElement('b');
    b.textContent = typeof v === 'number' ? v.toFixed(2) + ' ' + (d.currency || 'EUR') : '?';
    el.appendChild(b);
  });
  const house = document.getElementById('ext-house-today');
  if (house) {
    const n = k => Number(d[k]) || 0;
    const kwh = n('today_import_kwh') - n('today_export_kwh') + n('today_pv_kwh') - n('today_bat_chg_kwh') + n('today_bat_dis_kwh');
    house.textContent = d.today_import_kwh != null && kwh >= -0.005 ? Math.max(0, kwh).toFixed(2) + ' kWh' : '?';
  }
}

function _startReconnect() {
  if (_offline) return;                  // already in reconnect mode
  if (_pauseUntil && Date.now() < _pauseUntil) return;
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

function kv(id, val) {
  const el = document.getElementById(id);
  if (el) el.textContent = (val == null || val === '') ? '—' : val;
}

function renderNetworkAddresses(element, addresses) {
  element.replaceChildren();
  if (!addresses.length) {
    element.textContent = localizedText('Nicht verfügbar');
    return;
  }
  addresses.forEach((address, index) => {
    if (index) {
      const separator = document.createElement('span');
      separator.className = 'network-ip-separator';
      separator.textContent = ' | ';
      element.appendChild(separator);
    }
    const value = document.createElement('span');
    value.textContent = address;
    element.appendChild(value);
  });
}

function setRegisterValue(id, valueWh, hasValue, digits = 3) {
  const el = document.getElementById(id);
  if (!el) return;
  if (!hasValue || valueWh == null || !Number.isFinite(Number(valueWh))) {
    el.textContent = '—';
    return;
  }
  const [whole, fraction] = (Number(valueWh) / 1000).toFixed(digits).split('.');
  const decimal = document.createElement('span');
  decimal.className = 'register-decimal';
  decimal.textContent = '.' + fraction;
  const padded = whole.padStart(7, '0');
  const lead = padded.match(/^0*/)[0].slice(0, 6);
  const zeros = document.createElement('span');
  zeros.className = 'ext-zero';
  zeros.textContent = lead;
  el.replaceChildren(zeros, padded.slice(lead.length), decimal);
}

function setRowVisible(id, visible) {
  const el = document.getElementById(id);
  if (!el) return;
  const row = el.closest('tr') || el.closest('.kv');
  if (row) row.hidden = !visible;
}

function setConnectionStatus(online) {
  document.querySelectorAll('.sidebar-footer .status').forEach(el => {
    el.classList.toggle('offline', !online);
    const label = el.querySelector('span');
    if (label) label.textContent = online ? 'Verbunden' : 'Getrennt';
  });
}

function updateControlButtons(state) {
  const continuous = state.continuous === true;
  const running = state.job_state === 'Running';
  const singleButton = document.querySelector('button[onclick="doStart()"]');
  const continuousToggle = document.getElementById('continuous-toggle');
  const continuousState = document.getElementById('continuous-state');
  if (singleButton) {
    singleButton.hidden = continuous;
    singleButton.disabled = running || controlRequestPending;
  }
  if (continuousToggle) {
    continuousToggle.checked = continuous;
    continuousToggle.disabled = controlRequestPending || (running && !continuous);
  }
  if (continuousState) continuousState.textContent = localizedText(continuous ? 'Aktiv' : 'Inaktiv');
}

let statsData = null;
let statsView = 'days';

function setStatsView(view) {
  statsView = view;
  renderStats();
}

function statsLabel(view, key) {
  const text = String(key);
  if (view === 'years') return text;
  if (view === 'months') return text.slice(4, 6) + '/' + text.slice(0, 4);
  const date = text.slice(6, 8) + '.' + text.slice(4, 6) + '.' + text.slice(0, 4);
  return view === 'weeks' ? 'KW ab ' + date : date;
}

function renderStats() {
  const table = document.getElementById('stats-table');
  if (!table || !statsData) return;
  const rows = (statsData[statsView] || []).slice().reverse();
  const hasPv = rows.some(r => r.pv > 0 || r.bc > 0 || r.bd > 0);
  const hasWb = rows.some(r => r.wb > 0);
  const kwh = wh => (wh / 1000).toFixed(2);
  const money = m => (m / 1000).toFixed(2);
  const cols = ['Zeitraum', 'Bezug kWh', 'Einspeisung kWh'];
  if (hasPv) cols.push('PV kWh', 'Batterie laden/entladen kWh', 'Haus kWh', 'Eigenanteil %');
  cols.push('Netto ' + (statsData.currency || 'EUR'));
  if (hasPv) cols.push('Ersparnis ' + (statsData.currency || 'EUR'));
  if (hasWb) cols.push('Wallbox kWh', 'Wallbox Ergebnis ' + (statsData.currency || 'EUR'));
  const head = table.tHead;
  head.replaceChildren();
  const hr = head.insertRow();
  cols.forEach((c, i) => { const th = document.createElement('th'); th.textContent = localizedText(c); if (i) th.style.textAlign = 'right'; hr.appendChild(th); });
  const body = table.tBodies[0];
  body.replaceChildren();
  const tot = {i: 0, e: 0, pv: 0, bc: 0, bd: 0, house: 0, grid: 0, c: 0, s: 0, wb: 0, wm: 0};
  rows.forEach(r => {
    const house = Math.max(0, r.i - r.e + r.pv - r.bc + r.bd);
    const pvDirect = Math.max(0, r.pv - r.e - r.bc);
    const grid = Math.max(0, house - pvDirect - r.bd);
    const own = house > 0 ? Math.round(100 * (house - grid) / house) : 0;
    tot.i += r.i; tot.e += r.e; tot.pv += r.pv; tot.bc += r.bc; tot.bd += r.bd; tot.house += house; tot.grid += grid;
    tot.c += r.c; tot.s += r.s; tot.wb += r.wb; tot.wm += r.wm;
    const cells = [statsLabel(statsView, r.k) + (r.p ? ' *' : ''), kwh(r.i), kwh(r.e)];
    if (hasPv) cells.push(kwh(r.pv), kwh(r.bc) + ' / ' + kwh(r.bd), kwh(house), String(own));
    cells.push(money(r.c));
    if (hasPv) cells.push(money(r.s));
    if (hasWb) cells.push(kwh(r.wb), money(r.wm));
    const tr = body.insertRow();
    cells.forEach((c, i) => { const td = tr.insertCell(); td.textContent = c; if (i) td.className = 'value'; });
  });
  if (rows.length) {
    const ownTot = tot.house > 0 ? Math.round(100 * (tot.house - tot.grid) / tot.house) : 0;
    const sum = [localizedText('Summe'), kwh(tot.i), kwh(tot.e)];
    if (hasPv) sum.push(kwh(tot.pv), kwh(tot.bc) + ' / ' + kwh(tot.bd), kwh(tot.house), String(ownTot));
    sum.push(money(tot.c));
    let gainIdx = sum.length - 1;
    if (hasPv) { sum.push(money(tot.s)); gainIdx = sum.length - 1; }
    if (hasWb) sum.push(kwh(tot.wb), money(tot.wm));
    const tr = body.insertRow();
    sum.forEach((c, i) => { const td = tr.insertCell(); td.textContent = c; td.style.fontWeight = '700'; td.style.borderTop = '2px solid var(--border-strong)'; if (i) td.className = 'value'; });
    // Gewinn = Ersparnis - Netto-Kosten (Kosten sind bei Ueberschuss negativ)
    const gain = sum.map(() => '');
    gain[0] = localizedText('Gewinn');
    gain[gainIdx] = money((hasPv ? tot.s : 0) - tot.c);
    const gr = body.insertRow();
    gain.forEach((c, i) => { const td = gr.insertCell(); td.textContent = c; td.style.fontWeight = '700'; if (i) td.className = 'value'; });
  }
  document.querySelectorAll('[data-stats-view]').forEach(b => {
    b.className = 'btn ' + (b.dataset.statsView === statsView ? 'btn-blue' : 'btn-gray');
  });
  const note = document.getElementById('stats-note');
  if (note) note.textContent = localizedText('* laufender Zeitraum. Haus = Bezug - Einspeisung + PV - Batterie laden + Batterie entladen. Eigenanteil = Anteil des Hausverbrauchs aus PV und Batterie. Ersparnis = PV-Eigenverbrauch x Bezugspreis. Kosten gelten mit dem Tarif zum Zeitpunkt des Tagesabschlusses. Gewinn = Ersparnis - Netto-Kosten.');
}

async function loadStats() {
  if (!document.getElementById('stats-table')) return;
  if (document.body.classList.contains('view-energy')) return;
  try {
    const r = await fetch('/api/stats');
    if (!r.ok) return;
    statsData = await r.json();
    renderStats();
  } catch (e) {}
}

async function fetchStatus() {
  // Im Energie-Board pollt nur ehLoop(); parallele Abfragen blockieren den ESP-Webserver.
  if (document.body.classList.contains('view-energy')) return;
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
    kv('power-kilowatt', d.has_power ? (netPower / 1000).toFixed(2) + ' ' + localizedText('kW netto') : null);
    if (d.has_power) updatePowerBar(netPower, d);
    const heroCard = document.querySelector('.hero-card');
    if (heroCard) heroCard.hidden = !d.has_power;
    const meterCard = document.querySelector('.meter-card');
    if (meterCard) meterCard.hidden = !d.has_fwd_active_wh && !d.has_rev_active_wh;
    const kpiGrid = document.querySelector('.kpi-grid');
    if (kpiGrid) kpiGrid.hidden = !d.populated;
    const splitCard = document.querySelector('.split-card');
    if (splitCard) splitCard.hidden = !d.populated;
    kv('today-import', displayNumber(d.today_import_kwh, 2, 'kWh'));
    kv('today-export', displayNumber(d.today_export_kwh, 2, 'kWh'));
    kv('cost-today', displayNumber(d.cost_today, 2, d.currency || 'EUR'));
    const importCost = (Number(d.today_import_kwh) || 0) * (Number(d.price_import_kwh) || 0);
    const exportCredit = (Number(d.today_export_kwh) || 0) * (Number(d.price_export_kwh) || 0);
    kv('cost-breakdown', localizedText('Bezug') + ' ' + importCost.toFixed(2) + ' - ' + localizedText('Einspeisung') + ' ' + exportCredit.toFixed(2) + ' ' + (d.currency || 'EUR'));
    kv('legend-import', displayNumber(d.today_import_kwh, 2, 'kWh'));
    kv('legend-month', displayNumber(d.month_import_kwh, 2, 'kWh'));
    kv('m-mfr',   d.manufacturer);
    kv('m-model', d.model);
    kv('m-fw',    d.fw_version);
    kv('m-ser',   d.serial_bcd);
    kv('m-iser',  d.utility_serial);
    kv('m-time',  d.meter_time);
    lastStatusData = d;
    ehRenderKpis(d);
    renderExtToday();
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
    document.querySelectorAll('[data-tls-link]').forEach(el => { el.hidden = !d.tls_supported; });
    document.querySelectorAll('[data-ext-link]').forEach(el => { el.hidden = !d.ext_supported; });
    if (d.ext_supported) fetchExt();
    const ipv4 = [];
    if (d.ip_sta && d.ip_ap) {
      ipv4.push('STA: ' + d.ip_sta, 'AP: ' + d.ip_ap);
      kv('wifi-ssid', d.ssid_sta + '  |  AP: ' + d.ssid_ap);
    } else if (d.ip_sta) {
      ipv4.push(d.ip_sta); kv('wifi-ssid', d.ssid_sta);
    } else if (d.ip_ap) {
      ipv4.push('AP: ' + d.ip_ap); kv('wifi-ssid', 'AP: ' + d.ssid_ap);
    }
    const ipv4El = document.getElementById('wifi-ip');
    if (ipv4El) renderNetworkAddresses(ipv4El, ipv4);
    const ipv6El = document.getElementById('wifi-ip6');
    if (ipv6El) renderNetworkAddresses(ipv6El, Array.isArray(d.ip6_addresses) ? d.ip6_addresses : []);
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
  if (document.body.classList.contains('view-energy')) return;
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
async function setContinuousReading(enabled) {
  if (controlRequestPending) return;
  controlRequestPending = true;
  updateControlButtons({continuous: !enabled, job_state: ''});
  try {
    const endpoint = enabled ? '/api/start_continuous' : '/api/stop';
    const r = await fetch(endpoint, {method:'POST'});
    const j = await r.json();
    if (!r.ok || !j.ok) {
      if (j.reason === 'already_running') alert(localizedText('Ablesung läuft bereits.'));
      return;
    }
    if (enabled) {
      logSeq = 0;
      await fetchLog();
    }
    await fetchStatus();
  } finally {
    controlRequestPending = false;
    fetchStatus();
  }
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

const PROFILE_URL_DEFAULT = 'https://raw.githubusercontent.com/ip6constructor/SMLEasy/main/profiles.json';

const BUILTIN_METER_PROFILES = {
  mt631_ms2020: {
    name: 'Iskraemeco MT631 / MS2020',
    manufacturer: 'Iskraemeco',
    model: 'MT631/MS2020',
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
    manufacturer: 'Holley',
    model: 'DTZ541',
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
  ed300l: {
    name: 'EMH ED300L',
    manufacturer: 'EMH',
    model: 'ED300L',
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
      voltage_l2_v: '',
      voltage_l3_v: '',
      current_l1_a: '1-0:31.7.0*255',
      current_l2_a: '',
      current_l3_a: '',
      frequency_hz: '1-0:14.7.0*255',
      pf_l1: '1-0:13.7.0*255'
    }
  },
  dzg_dvs74_sml: {
    name: 'DZG DVS74 (SML-Firmware)',
    manufacturer: 'DZG',
    model: 'DVS74',
    pin_required: false,
    login_cmd: '',
    login_wait_ms: 250,
    obis: {
      import_wh: '1-0:1.8.0*255',
      export_wh: '1-0:2.8.0*255',
      power_net_w: '',
      power_import_w: '',
      power_export_w: '',
      voltage_l1_v: '',
      voltage_l2_v: '',
      voltage_l3_v: '',
      current_l1_a: '',
      current_l2_a: '',
      current_l3_a: '',
      frequency_hz: '',
      pf_l1: ''
    }
  },
  edl21_ehz: {
    name: 'EDL21 eHZ',
    manufacturer: 'Generic',
    model: 'EDL21 eHZ',
    pin_required: false,
    login_cmd: '',
    login_wait_ms: 250,
    obis: {
      import_wh: '1-0:1.8.0*255,1-0:1.8.1*255,1-0:1.8.2*255',
      export_wh: '1-0:2.8.0*255,1-0:2.8.1*255',
      power_net_w: '',
      power_import_w: '',
      power_export_w: '',
      voltage_l1_v: '',
      voltage_l2_v: '',
      voltage_l3_v: '',
      current_l1_a: '',
      current_l2_a: '',
      current_l3_a: '',
      frequency_hz: '',
      pf_l1: ''
    }
  },
  emh_ehz_k: {
    name: 'EMH eHZ-K',
    manufacturer: 'EMH',
    model: 'eHZ-K',
    pin_required: false,
    login_cmd: '',
    login_wait_ms: 250,
    obis: {
      import_wh: '1-0:1.8.0*255',
      export_wh: '1-0:2.8.0*255',
      power_net_w: '',
      power_import_w: '',
      power_export_w: '',
      voltage_l1_v: '',
      voltage_l2_v: '',
      voltage_l3_v: '',
      current_l1_a: '',
      current_l2_a: '',
      current_l3_a: '',
      frequency_hz: '',
      pf_l1: ''
    }
  },
  ebz_dd3: {
    name: 'eBZ DD3',
    manufacturer: 'eBZ',
    model: 'DD3',
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
    manufacturer: 'Landis+Gyr',
    model: 'E350',
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
    manufacturer: 'EasyMeter',
    model: 'Q3A/Q3D',
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
  easymeter_q3b: {
    name: 'EasyMeter Q3B',
    manufacturer: 'EasyMeter',
    model: 'Q3B',
    pin_required: false,
    login_cmd: '',
    login_wait_ms: 250,
    obis: {
      import_wh: '1-0:1.8.0*255,1-0:1.8.1*255,1-0:1.8.2*255',
      export_wh: '',
      power_net_w: '',
      power_import_w: '1-0:1.7.0*255',
      power_export_w: '',
      voltage_l1_v: '',
      voltage_l2_v: '',
      voltage_l3_v: '',
      current_l1_a: '',
      current_l2_a: '',
      current_l3_a: '',
      frequency_hz: '',
      pf_l1: ''
    }
  },
  generic_three_phase: {
    name: 'Generisch 3-phasig OBIS',
    manufacturer: 'Generic',
    model: '3-phase SML',
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
    manufacturer: 'Generic',
    model: 'single-phase SML',
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
  iec62056_pin_mode: {
    name: 'IEC62056 Login mit PIN',
    manufacturer: 'Generic',
    model: 'IEC 62056 PIN mode',
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

function sanitizeProfileProfile(entry, fallbackName, profileId) {
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
    id: String(p.id || profileId || ''),
    name: String(p.name || fallbackName || 'Externes Profil'),
    manufacturer: String(p.manufacturer || ''),
    model: String(p.model || ''),
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
      out[id] = sanitizeProfileProfile(e, e.name || id, id);
    }
    return out;
  }

  if (Array.isArray(data.profiles)) {
    for (let i = 0; i < data.profiles.length; i++) {
      const e = data.profiles[i] || {};
      const id = e.id ? String(e.id) : ('profile_' + (i + 1));
      out[id] = sanitizeProfileProfile(e, e.name || id, id);
    }
    return out;
  }

  if (data.profiles && typeof data.profiles === 'object') {
    for (const [id, e] of Object.entries(data.profiles)) {
      out[String(id)] = sanitizeProfileProfile(e, e && e.name ? e.name : String(id), String(id));
    }
    return out;
  }

  if (typeof data === 'object') {
    for (const [id, e] of Object.entries(data)) {
      out[String(id)] = sanitizeProfileProfile(e, e && e.name ? e.name : String(id), String(id));
    }
  }
  return out;
}

async function loadProfilesFromUrl(silent = false) {
  const input = document.getElementById('profile-url-input');
  const status = document.getElementById('profile-url-status');
  if (!input) return;

  const urlRaw = (input.value || '').trim();
  if (!urlRaw) {
    if (!silent) alert(localizedText('Bitte eine URL eintragen.'));
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
    const message = localizedText('Profile catalog could not be loaded: ') + (err && err.message ? err.message : err);
    if (status) status.textContent = message;
    if (!silent) alert(message);
  }
}

function saveCustomProfile(slot, profile) {
  localStorage.setItem('smleasy.custom_profile_' + slot, JSON.stringify(profile));
}

function getAllProfiles() {
  const merged = Object.assign({}, BUILTIN_METER_PROFILES);
  const remote = getRemoteProfiles();
  for (const [rid, rp] of Object.entries(remote)) {
    const profileId = Object.prototype.hasOwnProperty.call(BUILTIN_METER_PROFILES, rid)
      ? rid : 'ext_' + rid;
    merged[profileId] = Object.assign({}, rp, {id: profileId});
  }
  if (previousMeterProfile && previousMeterProfile.obis) {
    merged.previous_profile = sanitizeProfileProfile(
      previousMeterProfile, previousMeterProfile.name || 'Vorheriges Profil', '');
  }
  if (selectedMeterProfileId && !Object.prototype.hasOwnProperty.call(merged, selectedMeterProfileId) &&
      selectedMeterProfileData && selectedMeterProfileData.obis) {
    merged[selectedMeterProfileId] = Object.assign({}, selectedMeterProfileData, {id: selectedMeterProfileId});
  }
  const c1 = getCustomProfile(1);
  const c2 = getCustomProfile(2);
  merged.custom1 = c1 || {
    id: 'custom1', name: 'Custom 1 (leer)', pin_required: false, login_cmd: '', login_wait_ms: 250,
    obis: emptyObisMap()
  };
  merged.custom2 = c2 || {
    id: 'custom2', name: 'Custom 2 (leer)', pin_required: false, login_cmd: '', login_wait_ms: 250,
    obis: emptyObisMap()
  };
  return merged;
}

function renderProfileOptions() {
  const sel = document.getElementById('meter-profile-select');
  if (!sel) return;
  const keep = selectedMeterProfileId || sel.value;
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
    const prefix = id === 'previous_profile' ? localizedText('Vorheriges Profil') + ': ' :
      id.indexOf('ext_') === 0 ? '[GitHub] ' : '';
    o.textContent = prefix + profiles[id].name;
    sel.appendChild(o);
  }
  if (ids.includes(keep)) sel.value = keep;
  if (profiles[selectedMeterProfileId]) selectedMeterProfileData = profiles[selectedMeterProfileId];
}

function applyProfileById(profileId) {
  const f = document.getElementById('meter-form');
  if (!f) return;
  const profiles = getAllProfiles();
  const p = profiles[profileId];
  if (!p) return;
  selectedMeterProfileId = profileId === 'previous_profile' ? (p.id || '') : profileId;
  selectedMeterProfileData = p;

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
    id: 'custom' + slot,
    name: customName,
    pin_required: (f.login_cmd.value || '').indexOf('{PIN}') >= 0,
    login_cmd: (f.login_cmd.value || ''),
    login_wait_ms: parseInt((f.login_wait_ms.value || '250'), 10) || 250,
    obis
  };
  saveCustomProfile(slot, profile);
  renderProfileOptions();
  const sel = document.getElementById('meter-profile-select');
  selectedMeterProfileId = 'custom' + slot;
  selectedMeterProfileData = getAllProfiles()[selectedMeterProfileId] || profile;
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
    profile_id:        selectedMeterProfileId,
    profile_name:      selectedMeterProfileData ? selectedMeterProfileData.name : '',
    manufacturer:      selectedMeterProfileData ? selectedMeterProfileData.manufacturer : '',
    model:              selectedMeterProfileData ? selectedMeterProfileData.model : '',
    obis,
  });
  const pin = (fd.get('meter_pin') || '').toString();
  const payload = JSON.parse(body);
  if (pin.length > 0) payload.meter_pin = pin;
  const r = await fetch('/api/config/meter', {method:'POST',
    headers:{'Content-Type':'application/json'}, body: JSON.stringify(payload)});
  const result = r.ok ? await r.json() : {ok:false};
  if (result.ok) {
    const savedResponse = await fetch('/api/config/meter');
    if (savedResponse.ok) {
      const saved = await savedResponse.json();
      selectedMeterProfileId = saved.profile_id || selectedMeterProfileId;
      previousMeterProfile = saved.previous_profile || null;
      renderProfileOptions();
      const profileSelect = document.getElementById('meter-profile-select');
      if (profileSelect) profileSelect.value = selectedMeterProfileId;
    }
  }
  alert(result.ok ? localizedText('Meter settings saved') : localizedText('Fehler beim Speichern'));
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

async function saveHostname(e) {
  e.preventDefault();
  const hostname = e.target.hostname.value.trim();
  const r = await fetch('/api/config/hostname', {method:'POST',
    headers:{'Content-Type':'application/json'}, body: JSON.stringify({hostname})});
  const result = r.ok ? await r.json() : {ok:false};
  let message = result.ok ? localizedText('Hostname saved.') : localizedText('Fehler beim Speichern');
  if (result.ok && result.resolves === false) message += '\n\n' + result.hint;
  else if (result.ok && result.addresses) message += '\n' + result.addresses.join(', ');
  alert(message);
}

(async () => {
  try {
    const field = document.querySelector('#hostname-form input[name="hostname"]');
    if (!field) return;
    const s = await (await fetch('/api/status')).json();
    field.value = s.hostname || '';
  } catch (_) {}
})();

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
    f.wallbox_refund_kwh.value = d.wallbox_refund_kwh ?? 0.31;
    f.currency.value = d.currency ?? 'EUR';
    for (const k of ['east', 'south', 'west']) { f['pv_modules_' + k].value = d['pv_modules_' + k] ?? 0; f['pv_wp_' + k].value = d['pv_wp_' + k] ?? 485; }
  } catch (_) {}
}

async function saveTariff(e) {
  e.preventDefault();
  const fd = new FormData(e.target);
  const body = JSON.stringify({
    price_import_kwh: Number(fd.get('price_import_kwh')),
    price_export_kwh: Number(fd.get('price_export_kwh')),
    wallbox_refund_kwh: Number(fd.get('wallbox_refund_kwh')),
    currency: fd.get('currency'),
    pv_modules_east: Number(fd.get('pv_modules_east')), pv_wp_east: Number(fd.get('pv_wp_east')),
    pv_modules_south: Number(fd.get('pv_modules_south')), pv_wp_south: Number(fd.get('pv_wp_south')),
    pv_modules_west: Number(fd.get('pv_modules_west')), pv_wp_west: Number(fd.get('pv_wp_west'))
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

async function loadDailyBaseline() {
  const status = document.getElementById('daily-baseline-status');
  const current = document.getElementById('daily-baseline-current');
  try {
    const response = await fetch('/api/config/daily-baseline');
    if (!response.ok) throw new Error('HTTP ' + response.status);
    const data = await response.json();
    const importField = document.getElementById('daily-import-start-kwh');
    const exportField = document.getElementById('daily-export-start-kwh');
    const importWh = data.has_baseline ? data.import_start_wh : data.current_import_wh;
    const exportWh = data.has_baseline ? data.export_start_wh : data.current_export_wh;
    if (importField) {
      importField.value = data.has_reading ? (importWh / 1000).toFixed(3) : '';
      importField.min = data.has_reading ? (Math.max(1, data.current_import_wh - 400000) / 1000).toFixed(3) : '0.001';
      importField.max = data.has_reading ? (Math.min(2147483647, data.current_import_wh + 400000) / 1000).toFixed(3) : '2147483.647';
    }
    if (exportField) {
      exportField.value = data.has_reading ? (exportWh / 1000).toFixed(3) : '';
      exportField.min = data.has_reading ? (Math.max(1, data.current_export_wh - 400000) / 1000).toFixed(3) : '0.001';
      exportField.max = data.has_reading ? (Math.min(2147483647, data.current_export_wh + 400000) / 1000).toFixed(3) : '2147483.647';
    }
    const form = document.getElementById('daily-baseline-form');
    if (form) {
      form.dataset.currentImportWh = data.has_reading ? String(data.current_import_wh) : '';
      form.dataset.currentExportWh = data.has_reading ? String(data.current_export_wh) : '';
    }
    if (current) {
      current.textContent = data.has_reading
        ? localizedText('Aktuelle Zählerstände') + ': ' +
          (data.current_import_wh / 1000).toFixed(3) + ' / ' +
          (data.current_export_wh / 1000).toFixed(3) + ' kWh'
        : localizedText('Noch keine Zählerablesung verfügbar.');
    }
    if (status) status.textContent = data.has_baseline
      ? localizedText('Tagesstartwerte gespeichert.')
      : localizedText('Tagesstart laden');
  } catch (_) {
    if (status) status.textContent = localizedText('Tagesstartwerte konnten nicht gespeichert werden.');
  }
}

async function saveDailyBaseline(event) {
  event.preventDefault();
  const status = document.getElementById('daily-baseline-status');
  const importKwh = Number(document.getElementById('daily-import-start-kwh').value);
  const exportKwh = Number(document.getElementById('daily-export-start-kwh').value);
  const form = document.getElementById('daily-baseline-form');
  const importWh = Math.round(importKwh * 1000);
  const exportWh = Math.round(exportKwh * 1000);
  const currentImportWh = Number(form && form.dataset.currentImportWh);
  const currentExportWh = Number(form && form.dataset.currentExportWh);
  if (!Number.isFinite(importKwh) || !Number.isFinite(exportKwh) || importWh <= 0 || exportWh <= 0 ||
      !Number.isFinite(currentImportWh) || !Number.isFinite(currentExportWh) ||
      Math.abs(importWh - currentImportWh) > 400000 || Math.abs(exportWh - currentExportWh) > 400000) {
    if (status) status.textContent = localizedText('Tagesstart muss positiv sein und innerhalb ±400 kWh des aktuellen Standes liegen.');
    return;
  }
  try {
    const response = await fetch('/api/config/daily-baseline', {
      method: 'POST',
      headers: {'Content-Type':'application/json'},
      body: JSON.stringify({
        import_start_wh: importWh,
        export_start_wh: exportWh
      })
    });
    const result = response.ok ? await response.json() : {ok:false};
    if (!result.ok) throw new Error(result.reason || 'save failed');
    await loadDailyBaseline();
    if (status) status.textContent = localizedText('Tagesstartwerte gespeichert.');
  } catch (_) {
    if (status) status.textContent = localizedText('Tagesstartwerte konnten nicht gespeichert werden.');
  }
}

async function loadConfig() {
  try {
    const r = await fetch('/api/config/meter');
    if (!r.ok) return;
    const d = await r.json();
    selectedMeterProfileId = d.profile_id || 'mt631_ms2020';
    selectedMeterProfileData = {
      id: selectedMeterProfileId,
      name: d.profile_name || 'Iskraemeco MT631 / MS2020',
      manufacturer: d.manufacturer || 'Iskraemeco',
      model: d.model || 'MT631/MS2020',
      obis: d.obis || emptyObisMap()
    };
    previousMeterProfile = d.previous_profile || null;
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
      const savedUrl = getRemoteProfileUrl();
      profileUrlInput.value = savedUrl && savedUrl.indexOf('<user>') < 0
        ? savedUrl : PROFILE_URL_DEFAULT;
      renderProfileOptions();
      await loadProfilesFromUrl(true);
    } else {
      renderProfileOptions();
    }
    const sel = document.getElementById('meter-profile-select');
    if (sel && Array.from(sel.options).some(option => option.value === selectedMeterProfileId))
      sel.value = selectedMeterProfileId;
  } catch(_) {}
}

let githubOtaTimer = null;

async function loadGithubVersions() {
  if (window.GITHUB_OTA_ENABLED === false) return;
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
  if (window.GITHUB_OTA_ENABLED === false) return;
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

const EH_POS_DEFAULT = {pvo: [18.5, 7.3], pvs: [43.6, 7.3], pv: [65.8, 8.3], pvw: [90.6, 25.9], house: [13, 43.4], bat: [13.9, 68.4], grid: [51.9, 60.7], car: [70.6, 45], wb: [91, 43.8]};
const EH_COLORS = {pv: '#fbbf24', gridIn: '#60a5fa', gridOut: '#34d399', bat: '#c084fc', wb: '#2dd4bf'};
let ehTimer = null;
let ehGen = 0;
let ehTiming = {meterAge: -1, readS: 0, haAgeMs: -1, haPollS: 0};
async function ehGet(url) {
  const ctl = new AbortController();
  const t = setTimeout(() => ctl.abort(), 8000);
  try {
    const r = await fetch(url, {signal: ctl.signal, cache: 'no-store'});
    if (!r.ok) throw new Error('HTTP ' + r.status);
    return await r.json();
  } finally { clearTimeout(t); }
}
// Naechste Abfrage 1,5 s nach dem erwarteten Lesen von Zaehler bzw. Home Assistant, nie davor oder parallel.
function ehNextDelay() {
  const t = ehTiming;
  const events = [];
  if (t.meterAge >= 0 && t.readS > 0) events.push(t.readS * 1000 - t.meterAge);
  if (t.haAgeMs >= 0 && t.haPollS > 0) events.push(t.haPollS * 1000 - t.haAgeMs);
  let delay = 5000;
  for (let i = 0; i < 2; i++) {
    for (const e of events) {
      if (delay > e - 1000 && delay < e + 1500) delay = e + 1500;
    }
  }
  return Math.max(2000, Math.min(delay, 40000));
}
async function ehLoop(gen) {
  if (gen !== ehGen) return;
  await ehRefresh();
  ehClock();
  if (gen !== ehGen) return;
  ehTimer = setTimeout(() => ehLoop(gen), ehNextDelay());
}
let ehEdit = false;
function ehLoad(key, fallback) {
  try { const v = JSON.parse(localStorage.getItem(key)); return v === null ? fallback : v; } catch (e) { return fallback; }
}
function ehSave(key, value) {
  try { localStorage.setItem(key, JSON.stringify(value)); return true; } catch (e) { return false; }
}
function ehPos() { return Object.assign({}, EH_POS_DEFAULT, ehLoad('eh_pos7', {})); }
function ehKw(v) { return (Math.abs(v) / 1000).toFixed(2) + ' kW'; }
function ehApplyPhoto() {
  const stage = document.getElementById('eh-stage');
  if (!stage) return;
  const src = ehLoad('eh_photo', '');
  stage.style.backgroundImage = 'url("' + (src ? src.replace(/"/g, '%22') : '/house.jpg') + '")';
    const ar = src ? ehLoad('eh_ar', '16 / 9') : '960 / 540';
  stage.style.aspectRatio = ar;
  const m = /(\d+)\s*\/\s*(\d+)/.exec(ar);
  stage.style.setProperty('--ehr', m ? String(m[1] / m[2]) : '1.7778');
}
function ehPlace() {
  const pos = ehPos();
  document.querySelectorAll('.eh-tile').forEach(t => {
    const p = pos[t.dataset.k] || [50, 50];
    t.style.left = p[0] + '%';
    t.style.top = p[1] + '%';
  });
}
function ehSetPhotoFile(file) {
  const img = new Image();
  const url = URL.createObjectURL(file);
  img.onload = () => {
    const w = Math.min(1280, img.width);
    const c = document.createElement('canvas');
    c.width = w;
    c.height = Math.round(img.height * w / img.width);
    const ctx = c.getContext('2d');
    ctx.drawImage(img, 0, 0, c.width, c.height);
    URL.revokeObjectURL(url);
    // dunkle Zeilen am unteren Rand (Balken) abschneiden
    let h = c.height;
    const minH = Math.round(c.height * 0.4);
    while (h > minH) {
      const row = ctx.getImageData(0, h - 1, c.width, 1).data;
      let sum = 0;
      for (let i = 0; i < row.length; i += 4) sum += row[i] + row[i + 1] + row[i + 2];
      if (sum / (row.length / 4 * 3) > 30) break;
      h--;
    }
    if (h < c.height) {
      const o = document.createElement('canvas');
      o.width = c.width; o.height = h;
      o.getContext('2d').drawImage(c, 0, 0);
      c.width = o.width; c.height = h;
      c.getContext('2d').drawImage(o, 0, 0);
    }
    ehSave('eh_ar', c.width + ' / ' + c.height);
    if (!ehSave('eh_photo', c.toDataURL('image/jpeg', 0.8))) alert(localizedText('Foto zu gro\u00df f\u00fcr den Browser-Speicher'));
    ehApplyPhoto();
  };
  img.src = url;
}
function ehFlow(svg, a, b, w, color, rev) {
  if (!(w > 50) || !ehLoad('eh_photo', '')) return;
  const pos = ehPos();
  const p = pos[rev ? b : a], q = pos[rev ? a : b];
  const l = document.createElementNS('http://www.w3.org/2000/svg', 'line');
  l.setAttribute('x1', p[0] + '%'); l.setAttribute('y1', p[1] + '%');
  l.setAttribute('x2', q[0] + '%'); l.setAttribute('y2', q[1] + '%');
  l.setAttribute('class', 'eh-flow');
  l.setAttribute('stroke', color);
  l.style.color = color;
  l.setAttribute('stroke-width', String(2 + Math.min(5, w / 1500)));
  svg.appendChild(l);
}
function ehSet(k, html) {
  const el = document.querySelector('.eh-tile[data-k="' + k + '"]');
  if (el) el.innerHTML = html;
}
const EH_IC = {
  sun: '<svg class="eh-ic" viewBox="0 0 24 24"><circle cx="12" cy="12" r="4"/><path d="M12 2v3M12 19v3M2 12h3M19 12h3M4.9 4.9l2.1 2.1M17 17l2.1 2.1M4.9 19.1L7 17M17 7l2.1-2.1"/></svg>',
  house: '<svg class="eh-ic" viewBox="0 0 24 24"><path d="M3 11l9-8 9 8"/><path d="M5 10v10h14V10"/><path d="M10 20v-6h4v6"/></svg>',
  pylon: '<svg class="eh-ic" viewBox="0 0 24 24"><path d="M12 2l-5 20M12 2l5 20M4 6h16M6 11h12M8 16h8"/></svg>',
  car: '<svg class="eh-ic" viewBox="0 0 24 24"><path d="M3 13l2-5h14l2 5v5H3z"/><circle cx="7.5" cy="18" r="1.8"/><circle cx="16.5" cy="18" r="1.8"/></svg>',
  battery: '<svg class="eh-ic" viewBox="0 0 24 24"><rect x="2" y="7" width="18" height="10" rx="2"/><path d="M22 11v2M6 10v4M10 10v4M14 10v4"/></svg>',
  plug: '<svg class="eh-ic" viewBox="0 0 24 24"><path d="M9 2v5M15 2v5M6 7h12v4a6 6 0 0 1-12 0zM12 17v5"/></svg>',
  leaf: '<svg class="eh-ic" viewBox="0 0 24 24"><path d="M5 19C5 10 10 5 20 4c0 10-5 15-14 15z"/><path d="M5 19L14 10"/></svg>',
  euro: '<svg class="eh-ic" viewBox="0 0 24 24"><path d="M17 6a7 7 0 1 0 0 12M4 10h9M4 14h9"/></svg>'
};
function ehHd(icon, color, title) { return '<div class="eh-hd" style="color:' + color + '">' + EH_IC[icon] + '<h3 style="color:#cbd5e1">' + title + '</h3></div>'; }
const EH_GRID_PTS = [[0, 0], [10, 10], [50, 25], [100, 35], [500, 55], [1000, 70], [2500, 85], [5000, 100]];
function ehGridPct(w) {
  const a = Math.abs(w);
  if (a >= 5000) return 100;
  for (let i = 1; i < EH_GRID_PTS.length; i++) {
    const p = EH_GRID_PTS[i - 1], q = EH_GRID_PTS[i];
    if (a <= q[0]) return p[1] + (q[1] - p[1]) * (a - p[0]) / (q[0] - p[0]);
  }
  return 100;
}
function ehGauge(prev, w) {
  const a = Math.abs(w), c = w > 0 ? '#ef4444' : '#22c55e';
  const dot = (x, on) => '<circle cx="' + x + '" cy="50" r="4" fill="' + (on ? c : '#334155') + '"/>';
  return '<svg class="eh-gauge" viewBox="0 0 100 100" aria-hidden="true">'
    + '<path class="bg" pathLength="100" d="M50 92A42 42 0 0 1 50 8"/><path class="bg" pathLength="100" d="M50 92A42 42 0 0 0 50 8"/>'
    + '<path class="fl gl" pathLength="100" stroke="#ef4444" style="stroke-dasharray:' + prev.l + ' 100" d="M50 92A42 42 0 0 1 50 8"/>'
    + '<path class="fl gr" pathLength="100" stroke="#22c55e" style="stroke-dasharray:' + prev.r + ' 100" d="M50 92A42 42 0 0 0 50 8"/>'
    + dot(42, a >= 10000) + dot(58, a >= 20000) + '</svg>';
}
function ehTile(icon, color, title, value, rows, sub, extra) {
  let h = '<div class="eh-top" style="color:' + color + '">' + EH_IC[icon] + '<div><h3>' + title + '</h3>' + (sub ? '<small style="color:#94a3b8">' + sub + '</small>' : '') + '<span class="eh-val">' + value + '</span></div></div>';
  if (extra) h += extra;
  if (rows && rows.length) h += '<div class="eh-rows">' + rows.map(r => '<div class="eh-row"' + (r[2] ? ' style="color:' + r[2] + '"' : '') + '><span>' + r[0] + '</span><span>' + r[1] + '</span></div>').join('') + '</div>';
  return h;
}
async function ehRefresh() {
  try {
    const d = await ehGet('/api/status');
    ehRenderKpis(d);
    const e = d.ext_supported ? await ehGet('/api/ext/values').catch(() => ({})) : {};
    const exAges = (Array.isArray(e.items) ? e.items : []).map(i => i.age_s).filter(a => typeof a === 'number' && a >= 0);
    ehTiming = {
      meterAge: typeof d.meter_age_ms === 'number' ? d.meter_age_ms : -1,
      readS: d.read_interval_s || 0,
      haAgeMs: exAges.length ? Math.min(...exAges) * 1000 : -1,
      haPollS: e.poll_s || 0
    };
    const items = Array.isArray(e.items) ? e.items : [];
    const num = it => (it && typeof it.value === 'number') ? it.value * (it.unit === 'kW' ? 1000 : 1) : null;
    const byRole = r => items.find(i => i.role === r);
    const areaW = re => num(items.find(i => /^k?w$/i.test(i.unit || '') && re.test(i.label || '')));
    const pvWest = num(byRole('pv_west')) ?? areaW(/west/i), pvEast = num(byRole('pv_east')) ?? areaW(/\bost\b|east/i), pvSouth = num(byRole('pv_south')) ?? areaW(/s\u00fcd|sued|south/i);
    let pvW = num(byRole('pv'));
    if (pvW === null && (pvWest !== null || pvEast !== null || pvSouth !== null)) pvW = (pvWest || 0) + (pvEast || 0) + (pvSouth || 0);
    let batW = num(byRole('battery'));
    if (batW === null && byRole('battery_neg')) batW = -num(byRole('battery_neg'));
    const soc = items.find(i => i.unit === '%' && !/auto|fahrzeug|\bcar\b|\bev\b/i.test(i.label || ''));
    const car = items.find(i => i.unit === '%' && /auto|fahrzeug|\bcar\b|\bev\b/i.test(i.label || ''));
    const wbP = byRole('wb_power') || items.find(i => /wallbox|keba|\bwb\b/i.test(i.label || '') && /^k?w$/i.test(i.unit || ''));
    const wbW = num(wbP);
    const gridW = (d.fwd_w || 0) - (d.rev_w || 0);
    const houseW = (pvW || 0) - (batW || 0) + gridW - (wbW || 0);
    const T = t => localizedText(t);
    const pk = {e: d.pv_peak_east_w || 0, s: d.pv_peak_south_w || 0, w: d.pv_peak_west_w || 0};
    const pkAll = pk.e + pk.s + pk.w;
    const pbar = (w, peak) => peak > 0 && w !== null ? '<div class="eh-pbar"><i style="width:' + Math.max(0, Math.min(100, w / peak * 100)).toFixed(0) + '%"></i></div>' : '';
    const prow = (w, peak) => peak > 0 ? [[w === null ? '\u2014' : Math.round(Math.max(0, Math.min(100, w / peak * 100))) + ' %', (peak / 1000).toFixed(1) + ' kWp']] : null;
    ehSet('pv', ehTile('sun', '#fbbf24', T('PV'), pvW === null ? '\u2014' : ehKw(pvW), (prow(pvW, pkAll) || []).concat([[T('Heute'), (d.today_pv_kwh || 0).toFixed(1) + ' kWh'], [T('Monat'), (d.month_pv_kwh || 0).toFixed(0) + ' kWh']]), null, pbar(pvW, pkAll)));
    ehSet('pvw', ehTile('sun', '#fbbf24', T('PV West'), pvWest === null ? '\u2014' : ehKw(pvWest), prow(pvWest, pk.w), null, pbar(pvWest, pk.w)));
    ehSet('pvs', ehTile('sun', '#fbbf24', T('PV S\u00fcd'), pvSouth === null ? '\u2014' : ehKw(pvSouth), prow(pvSouth, pk.s), null, pbar(pvSouth, pk.s)));
    ehSet('pvo', ehTile('sun', '#fbbf24', T('PV Ost'), pvEast === null ? '\u2014' : ehKw(pvEast), prow(pvEast, pk.e), null, pbar(pvEast, pk.e)));
    const hc = gridW > 0 ? '#60a5fa' : '#fbbf24';
    ehSet('house', ehTile('house', hc, T('Hausverbrauch'), typeof d.today_house_kwh === 'number' ? d.today_house_kwh.toFixed(1) + ' kWh' : '\u2014', [[T('Leistung'), Math.round(Math.max(0, houseW)) + ' W'], [T('Monat'), typeof d.month_house_kwh === 'number' ? d.month_house_kwh.toFixed(0) + ' kWh' : '?']], T('Heute')));
    const gp = ehGridPct(gridW), gimp = gridW > 0, gact = Math.abs(gridW) >= 1;
    const gcur = {l: gimp && gact ? gp : 0, r: !gimp && gact ? gp : 0};
    const gprev = window.ehGridPrev || {l: 0, r: 0};
    ehSet('grid', ehTile('pylon', gridW < 0 ? '#22c55e' : '#ef4444', T('Stromnetz'), Math.round(Math.abs(gridW)) + ' W', [[T('Bezug') + ' (' + T('Heute') + ')', (d.today_import_kwh || 0).toFixed(2) + ' kWh', '#ef4444'], [T('Einspeisung') + ' (' + T('Heute') + ')', (d.today_export_kwh || 0).toFixed(2) + ' kWh', '#22c55e']], null, ehGauge(gprev, gridW)));
    window.ehGridPrev = gcur;
    setTimeout(() => {
      const gt = document.querySelector('.eh-tile[data-k="grid"]');
      if (!gt) return;
      const gl = gt.querySelector('.gl'), gr = gt.querySelector('.gr');
      if (gl) gl.style.strokeDasharray = gcur.l + ' 100';
      if (gr) gr.style.strokeDasharray = gcur.r + ' 100';
    }, 30);
    const pct = soc ? Math.max(0, Math.min(100, soc.value)) : null;
    const on = pct === null ? 0 : (pct > 0 ? Math.max(1, Math.round(pct / 20)) : 0);
    const col = pct === null ? '' : pct < 40 ? 'red' : pct > 60 ? 'green' : 'yellow';
    let segs = '';
    for (let i = 0; i < 5; i++) segs += '<i class="' + (i < on ? col : '') + '"></i>';
    const bc = pct === null ? '#94a3b8' : pct < 40 ? '#ef4444' : pct > 60 ? '#22c55e' : '#f59e0b';
    ehSet('bat', ehTile('battery', bc, T('Batterie'), pct === null ? (batW === null ? '\u2014' : ehKw(batW)) : Math.round(pct) + ' %', [[T('Geladen') + ' (' + T('Heute') + ')', (d.today_bat_chg_kwh || 0).toFixed(1) + ' kWh'], [T('Entladen') + ' (' + T('Heute') + ')', (d.today_bat_dis_kwh || 0).toFixed(1) + ' kWh']], null, '<div class="eh-soc">' + segs + '</div>'));
    const carText = byRole('car_name');
    const modeItem = byRole('wb_mode');
    const carName = carText && carText.text ? carText.text : car ? (String(car.label || '').replace(/\s*(ladestand|ladezustand|soc|akku|batterie|state of charge)\s*/ig, ' ').trim() || T('E-Auto')) : T('E-Auto');
    const cc = !car ? '#94a3b8' : (car.value < 40 ? '#ef4444' : car.value > 60 ? '#22c55e' : '#f59e0b');
    const cOn = car ? Math.max(car.value > 0 ? 1 : 0, Math.round(car.value / 20)) : 0;
    let cSegs = '';
    for (let i = 0; i < 5; i++) cSegs += '<i class="' + (i < cOn ? (car.value < 40 ? 'red' : car.value > 60 ? 'green' : 'yellow') : '') + '"></i>';
    const sess = byRole('wb_session_kwh') || items.find(i => /ladevorgang|sitzung|session/i.test(i.label || '') && /^k?wh$/i.test(i.unit || ''));
    const modeRaw = modeItem && modeItem.text ? String(modeItem.text) : '';
    const modeKey = /fast|schnell/i.test(modeRaw) ? 'fast' : /both|beide|mix/i.test(modeRaw) ? 'both' : /pv/i.test(modeRaw) ? 'pv' : '';
    const modeLabel = modeKey === 'fast' ? 'Fast' : modeKey === 'both' ? 'Both' : modeKey === 'pv' ? 'PV-Only' : modeRaw;
    const modeColor = modeKey === 'fast' ? '#ef4444' : modeKey === 'both' ? '#f59e0b' : '#22c55e';
    const wbTot = byRole('wb_kwh');
    ehSet('car', ehTile('car', '#a855f7', carName, car ? Math.round(car.value) + ' %' : '\u2014', [[T('Leistung'), (wbW === null ? '\u2014' : ehKw(wbW)) + ' / 11 kW'], [T('Modus'), modeLabel || '?', modeLabel ? modeColor : null], [T('Ladevorgang'), sess && typeof sess.value === 'number' ? sess.value.toFixed(1) + ' kWh' : '?'], [T('Gesamt'), wbTot && typeof wbTot.value === 'number' ? wbTot.value.toFixed(0) + ' kWh' : '?']], T('Ladezustand'), '<div class="eh-soc">' + cSegs + '</div>'));
    ehSet('wb', ehTile('plug', '#2dd4bf', T('Wallbox'), wbW === null ? '\u2014' : ehKw(wbW), [[T('Heute'), typeof d.today_wb_kwh === 'number' ? d.today_wb_kwh.toFixed(1) + ' kWh' : '?'], [T('Woche'), typeof d.week_wb_kwh === 'number' ? d.week_wb_kwh.toFixed(1) + ' kWh' : '?'], [T('Monat'), typeof d.month_wb_kwh === 'number' ? d.month_wb_kwh.toFixed(0) + ' kWh' : '?']]));
    const svg = document.getElementById('eh-flows');
    svg.replaceChildren();
    ehFlow(svg, 'pv', 'house', pvW || 0, EH_COLORS.pv, false);
    ehFlow(svg, 'grid', 'house', Math.abs(gridW), gridW >= 0 ? EH_COLORS.gridIn : EH_COLORS.gridOut, gridW < 0);
    ehFlow(svg, 'bat', 'house', Math.abs(batW || 0), EH_COLORS.bat, (batW || 0) > 0);
    ehFlow(svg, 'wb', 'house', wbW || 0, EH_COLORS.wb, true);
  } catch (err) {
    const kp = document.getElementById('eh-kpis');
    if (kp) kp.dataset.error = localizedText('Verbindungsfehler') + ' — ' + localizedText('Werte möglicherweise veraltet');
    console.warn('Energy board refresh failed', err.name);
  }
}
function ehKpiValues(d) {
  const number = key => typeof d[key] === 'number' && Number.isFinite(d[key]) ? d[key] : null;
  const pv = number('today_pv_kwh'), ex = number('today_export_kwh');
  const fmt = (value, digits, unit) => value === null ? '\u2014' : value.toFixed(digits) + ' ' + unit;
  const currency = d.currency || 'EUR';
  return [
    pv !== null && ex !== null && pv > 0.05 ? Math.max(0, Math.min(100, (pv - ex) / pv * 100)).toFixed(0) + ' %' : '\u2014',
    fmt(pv, 1, 'kWh'), fmt(number('cost_today'), 2, currency), fmt(number('today_saving'), 2, currency)
  ];
}
function ehRenderKpis(d) {
  const kp = document.getElementById('eh-kpis');
  if (!kp) return;
  delete kp.dataset.error;
  const values = ehKpiValues(d);
  kp.replaceChildren();
  ['Eigenverbrauch', 'Gesamterzeugung', 'Netzkosten heute', 'Ersparnis heute'].forEach((label, i) => {
    const row = document.createElement('div'), icon = document.createElement('span');
    icon.style.cssText = 'grid-row:1/3;display:flex;color:' + ['#22c55e', '#fbbf24', '#60a5fa', '#22c55e'][i];
    icon.innerHTML = EH_IC[['leaf', 'sun', 'pylon', 'euro'][i]];
    const title = document.createElement('small'), value = document.createElement('b');
    title.textContent = localizedText(label);
    value.textContent = values[i];
    row.append(icon, title, value);
    kp.appendChild(row);
  });
}
function ehClock() {
  const c = document.getElementById('eh-clock');
  if (c) c.textContent = new Date().toLocaleString();
}
function ehRoute() {
  const on = location.hash === '#energy';
  document.body.classList.toggle('view-energy', on);
  document.body.classList.toggle('eh-full', on);
  document.querySelectorAll('.nav-item[href^="#"]').forEach(a => a.classList.toggle('active', a.getAttribute('href') === location.hash || (!on && a.getAttribute('href') === '#dashboard')));
  clearTimeout(ehTimer);
  ehTimer = null;
  ehGen++;
  if (on) {
    ehPlace(); ehApplyPhoto(); ehClock();
    ehLoop(ehGen);
  }
}
function ehInit() {
  const stage = document.getElementById('eh-stage');
  if (!stage) return;
  localStorage.removeItem('eh_photo'); localStorage.removeItem('eh_ar');
  const exitFs = () => { if (document.fullscreenElement && document.exitFullscreen) document.exitFullscreen().catch(() => {}); };
  const enterFs = () => { const el = document.documentElement; if (!document.fullscreenElement && el.requestFullscreen) el.requestFullscreen().catch(() => {}); };
  document.getElementById('eh-menu').onclick = () => { document.body.classList.remove('eh-full'); exitFs(); };
  document.getElementById('eh-fs').onclick = () => { if (document.fullscreenElement) exitFs(); else enterFs(); };
  document.getElementById('eh-full').onclick = () => { document.body.classList.add('eh-full'); enterFs(); };
  document.addEventListener('keydown', ev => { if (ev.key === 'Escape') document.body.classList.remove('eh-full'); });
  document.getElementById('eh-edit').onclick = ev => {
    ehEdit = !ehEdit;
    document.body.classList.toggle('eh-edit', ehEdit);
    ev.currentTarget.classList.toggle('on', ehEdit);
  };
  let drag = null;
  stage.addEventListener('pointerdown', ev => {
    const t = ev.target.closest('.eh-tile');
    if (!ehEdit || !t) return;
    drag = t;
    t.setPointerCapture(ev.pointerId);
  });
  stage.addEventListener('pointermove', ev => {
    if (!drag) return;
    const r = stage.getBoundingClientRect();
    const x = Math.max(3, Math.min(97, (ev.clientX - r.left) / r.width * 100));
    const y = Math.max(3, Math.min(97, (ev.clientY - r.top) / r.height * 100));
    drag.style.left = x + '%';
    drag.style.top = y + '%';
    drag.dataset.x = x; drag.dataset.y = y;
  });
  const end = () => {
    if (!drag) return;
    if (drag.dataset.x) {
      const pos = ehLoad('eh_pos7', {});
      pos[drag.dataset.k] = [Math.round(drag.dataset.x * 10) / 10, Math.round(drag.dataset.y * 10) / 10];
      ehSave('eh_pos7', pos);
      ehRefresh();
    }
    drag = null;
  };
  stage.addEventListener('pointerup', end);
  stage.addEventListener('pointercancel', end);
  window.addEventListener('hashchange', ehRoute);
  ehRoute();
}
document.addEventListener('DOMContentLoaded', ehInit);
document.addEventListener('DOMContentLoaded', () => {
  initUiLanguage();
  renderProfileOptions();
  const githubOtaPanel = document.getElementById('github-ota-panel');
  if (githubOtaPanel && window.GITHUB_OTA_ENABLED === false) githubOtaPanel.hidden = true;
  if (window.GITHUB_OTA_ENABLED !== false && document.getElementById('github-ota-select')) loadGithubVersions();
  if (document.getElementById('access-networks-form')) loadAccessNetworks();
  if (document.getElementById('tls-form')) loadTlsConfig();
  if (document.getElementById('ext-form')) loadExtConfig();
  const otaForm = document.getElementById('ota-form');
  if (otaForm) {
    otaForm.addEventListener('submit', (e) => {
      e.preventDefault();
      startOta(document.getElementById('ota-file').files[0]);
    });
  }
  if (document.getElementById('meter-form')) loadConfig();
  if (document.getElementById('daily-baseline-form')) loadDailyBaseline();
  if (document.getElementById('tariff-form')) fetchTariffCfg();
  if (document.getElementById('ha-form')) fetchHaCfg();
  fetchStatus();
  fetchLog();
  loadStats();
  setInterval(loadStats, 60000);
  statusTimer = setInterval(fetchStatus, 5000);
  logTimer    = setInterval(fetchLog, 30000);
});
)rawjs";

static const char kIndexHtml[] = R"rawhtml(<!DOCTYPE html>
<html lang="de"><head><meta charset="UTF-8"><meta name="viewport" content="width=device-width,initial-scale=1"><title>SMLEasy Dashboard</title><link rel="stylesheet" href="/style.css"><link rel="icon" href="/favicon.ico" type="image/svg+xml"></head>
<body><div class="app">
<aside class="sidebar"><div class="brand">)rawhtml" R"rawsvg(<svg viewBox="0 0 100 100" xmlns="http://www.w3.org/2000/svg" aria-hidden="true"><defs><linearGradient id="smlGh" x1="0%" y1="0%" x2="100%" y2="100%"><stop offset="0%" stop-color="#22d3ee"/><stop offset="100%" stop-color="#2563eb"/></linearGradient></defs><circle cx="50" cy="50" r="42" fill="none" stroke="#132238" stroke-width="9"/><path d="M50 8 A42 42 0 0 1 90.5 40M8 55 A42 42 0 0 0 40 91.5M12 40 A42 42 0 0 1 30 15" fill="none" stroke="url(#smlGh)" stroke-width="9" stroke-linecap="round"/><path d="M72 30 L58 55 H68 L60 78 L80 50 H69 Z" fill="url(#smlGh)"/></svg>)rawsvg" R"rawhtml(<div>EasySML<small>Smart energy</small></div></div>
<div class="nav-title">Übersicht</div><a class="nav-item active" href="#dashboard"><svg viewBox="0 0 24 24"><rect x="3" y="3" width="7" height="7"/><rect x="14" y="3" width="7" height="7"/><rect x="3" y="14" width="7" height="7"/><rect x="14" y="14" width="7" height="7"/></svg><span>Dashboard</span></a><a class="nav-item" href="#energy"><svg viewBox="0 0 24 24"><path d="M3 11l9-7 9 7"/><path d="M5 10v10h14V10"/><path d="M13 8l-3 5h4l-3 5"/></svg><span>Energiehaus</span></a><div class="nav-title" style="margin-top:24px">Konfiguration</div><a class="nav-item" href="/config#general"><svg viewBox="0 0 24 24"><path d="M12 15.5a3.5 3.5 0 1 0 0-7 3.5 3.5 0 0 0 0 7Z"/><path d="m19.4 15 .1.1a2 2 0 1 1-2.8 2.8l-.1-.1a2 2 0 0 0-3.4 1.4v.3a2 2 0 1 1-4 0v-.2A2 2 0 0 0 5.8 18l-.1.1a2 2 0 1 1-2.8-2.8l.1-.1A2 2 0 0 0 1.6 12H1.5a2 2 0 1 1 0-4h.2A2 2 0 0 0 3 4.6l-.1-.1a2 2 0 1 1 2.8-2.8l.1.1A2 2 0 0 0 9.2.5v-.1a2 2 0 1 1 4 0v.2A2 2 0 0 0 16.6 2l.1-.1a2 2 0 1 1 2.8 2.8l-.1.1A2 2 0 0 0 20.8 8h.2a2 2 0 1 1 0 4h-.2a2 2 0 0 0-1.4 3Z"/></svg><span>Allgemein</span></a><a class="nav-item" href="/config#meter"><svg viewBox="0 0 24 24"><circle cx="12" cy="12" r="9"/><path d="M12 12l4-3"/></svg><span>Zähler</span></a><a class="nav-item" href="/config#wifi"><svg viewBox="0 0 24 24"><path d="M2 9a15 15 0 0 1 20 0M5 12.5a10 10 0 0 1 14 0M8.5 16a5 5 0 0 1 7 0"/><circle cx="12" cy="19" r="1"/></svg><span>WLAN</span></a><a class="nav-item" href="/config#firmware"><svg viewBox="0 0 24 24"><path d="M12 3v12m0 0-4-4m4 4 4-4M4 19h16"/></svg><span>Firmware</span></a><a class="nav-item" href="/tls" data-tls-link hidden><svg viewBox="0 0 24 24"><rect x="4" y="11" width="16" height="10" rx="2"/><path d="M8 11V8a4 4 0 0 1 8 0v3"/></svg><span>HTTPS / Zertifikate</span></a><a class="nav-item" href="/pair" data-tls-link hidden><svg viewBox="0 0 24 24"><rect x="3" y="3" width="7" height="7"/><rect x="14" y="3" width="7" height="7"/><rect x="3" y="14" width="7" height="7"/><path d="M14 14h3v3m4 0v4h-4m-3-1v1"/></svg><span>App koppeln</span></a><a class="nav-item" href="/ext" data-ext-link hidden><svg viewBox="0 0 24 24"><path d="M4 19V5m0 14h16"/><path d="M8 15l3-4 3 2 4-6"/></svg><span>Externe Daten</span></a><div class="sidebar-footer"><div class="status"><i class="status-dot"></i><span>Verbunden</span></div><div style="margin-top:8px">EasySML <span id="fw-ver-footer">v1.0</span></div><div class="side-conn"><div class="kv"><span class="lbl">WLAN</span><span class="val" id="wifi-ssid">—</span></div><div class="kv"><span class="lbl">Uptime</span><span class="val" id="uptime">—</span></div></div></div></aside>
<div class="workspace"><header class="topbar"><div><div class="eyebrow">Smart meter / Übersicht</div><h1>Dashboard</h1></div><div class="topbar-meta"><span id="fw-ver">—</span><span id="status-pill" class="status"><i class="status-dot"></i>Idle</span></div></header>
<main id="dashboard" class="main-content"><div class="page-intro"><div><div class="eyebrow">Live monitoring</div><h2>Verbrauch auf einen Blick</h2><p>Aktuelle Messwerte und Energiefluss deines Zählers.</p></div></div>
<section id="energy-view"><div class="page-intro"><div><div class="eyebrow">Live</div><h2>Mein Energiehaus</h2></div></div><div class="eh-bar"><button id="eh-full" type="button">Vollbild</button><button id="eh-edit" type="button">Kacheln verschieben</button><span class="eh-clock" id="eh-clock"></span></div><div class="eh-stage" id="eh-stage"><button class="eh-menu" id="eh-menu" type="button" aria-label="Menu"><svg viewBox="0 0 24 24"><path d="M4 7h16M4 12h16M4 17h16"/></svg></button><button class="eh-menu" id="eh-fs" type="button" aria-label="Fullscreen"><svg viewBox="0 0 24 24"><path d="M4 9V4h5M20 9V4h-5M4 15v5h5M20 15v5h-5"/></svg></button><svg id="eh-flows" aria-hidden="true"></svg><div class="eh-tile" data-k="pv"></div><div class="eh-tile" data-k="pvs"></div><div class="eh-tile" data-k="pvw"></div><div class="eh-tile" data-k="pvo"></div><div class="eh-tile" data-k="grid"></div><div class="eh-tile" data-k="house"></div><div class="eh-tile" data-k="bat"></div><div class="eh-tile" data-k="car"></div><div class="eh-tile" data-k="wb"></div><div class="eh-kpis" id="eh-kpis"></div></div></section>
<section class="grid">
<article class="card hero-card"><h2>Aktuelle Leistung</h2><div class="metric-value" id="current-power">—<span class="metric-unit">W</span></div><div class="metric-sub" id="power-kilowatt">— kW netto</div><div class="pbar"><div class="pbar-fill" id="pbar-fill"></div></div><div class="pbar-scale"><span>0</span><span id="pbar-max">6 kW</span></div><div class="pstats"><div><span>Spitze Bezug (1 h)</span><b id="peak-imp">—</b></div><div><span>Spitze Einspeisung (1 h)</span><b id="peak-exp">—</b></div><div><span>Strom je Phase</span><b id="phase-cur">—</b></div></div></article>
<article class="card meter-card register-card"><div class="register-heading"><h2>Zählerstände</h2><span class="register-live">Zähler Live</span></div><div class="register-subtitle">Bezug und Einspeisung als Zählwerke</div><div class="register-meter"><div class="register-brand"><span>ZÄHLWERKE · OBIS 1.8.0 / 2.8.0</span></div><div class="register-displays"><div class="register-window"><div class="register-label">1.8.0 / BEZUG GESAMT</div><div class="register-row"><div class="ext-ico" style="background:#dfeeff;color:#2f7de1"><svg viewBox="0 0 24 24" width="24" height="24" fill="none" stroke="currentColor" stroke-width="1.8" stroke-linecap="round" stroke-linejoin="round" aria-hidden="true"><path d="M12 2 6.5 22M12 2l5.5 20M4 7h16M6 13h12M7.5 7l9 6M16.5 7l-9 6"/></svg></div><span class="register-reading" id="m-fwh">—</span></div><div class="register-unit">kWh</div></div><div class="register-window"><div class="register-label">2.8.0 / EINSPEISUNG GESAMT</div><div class="register-row"><div class="ext-ico" style="background:#dcf7e7;color:#1e9e5a"><svg viewBox="0 0 24 24" width="24" height="24" fill="none" stroke="currentColor" stroke-width="1.8" stroke-linecap="round" stroke-linejoin="round" aria-hidden="true"><path d="M12 2 6.5 22M12 2l5.5 20M4 7h16M6 13h12M7.5 7l9 6M16.5 7l-9 6"/></svg></div><span class="register-reading" id="m-rwh">—</span></div><div class="register-unit">kWh</div></div></div></div><div class="register-time"><span>Zählerzeit</span><span class="val" id="m-time">—</span></div></article>
<div class="kpi-grid"><article class="card kpi-card"><div class="label">Verbrauch heute</div><div class="kpi-value" id="today-import">—</div><div class="kpi-change positive">Importierte Energie</div></article><article class="card kpi-card"><div class="label">Einspeisung heute</div><div class="kpi-value" id="today-export">—</div><div class="kpi-change">Exportierte Energie</div></article><article class="card kpi-card"><div class="label">Nettokosten heute</div><div class="kpi-value" id="cost-today">—</div><div class="kpi-change" id="cost-breakdown">Bezug minus Einspeisevergütung</div></article></div>
<article class="card ext-card" id="ext-card" hidden><h2>Haus & Energie</h2><div class="ext-grid" id="ext-items"></div></article>

<article class="card split-card"><h2>Verbrauchsaufteilung</h2><div class="donut" aria-label="Verbrauchsaufteilung"></div><div class="legend"><span><b><i style="background:var(--primary)"></i>Bezug heute</b><strong id="legend-import">—</strong></span><span><b><i style="background:#8bdcf4"></i>Monat gesamt</b><strong id="legend-month">—</strong></span></div></article>

<article class="card system-card"><h2>Systemstatus</h2><div class="system-list"><div class="kv"><span class="lbl">Hersteller</span><span class="val" id="m-mfr">—</span></div><div class="kv"><span class="lbl">Modell</span><span class="val" id="m-model">—</span></div><div class="kv"><span class="lbl">Firmware</span><span class="val" id="m-fw">—</span></div><div class="kv"><span class="lbl">Seriennr.</span><span class="val" id="m-ser">—</span></div><div class="kv"><span class="lbl">Login</span><span class="val" id="last-login-ok">—</span></div><div class="kv"><span class="lbl">Alarme</span><span class="val" id="m-alarm">Keine</span></div></div></article>
<article class="card stats-card" style="grid-column:1/-1"><h2>Statistik <span class="stats-tabs"><button class="btn btn-gray" data-stats-view="days" onclick="setStatsView('days')">Tage</button> <button class="btn btn-gray" data-stats-view="weeks" onclick="setStatsView('weeks')">Wochen</button> <button class="btn btn-gray" data-stats-view="months" onclick="setStatsView('months')">Monate</button> <button class="btn btn-gray" data-stats-view="years" onclick="setStatsView('years')">Jahre</button></span></h2><div style="overflow-x:auto"><table class="data-table" id="stats-table"><thead></thead><tbody></tbody></table></div><div class="metric-sub" id="stats-note" style="margin-top:10px"></div></article>
<article class="card control-card"><h2>Steuerung</h2><div class="card-inner"><div class="btn-row"><button class="btn btn-blue" onclick="doStart()">&#9654; Einmal auslesen</button><label class="continuous-control"><span>Dauerlesen</span><span class="continuous-switch"><input id="continuous-toggle" type="checkbox" role="switch" checked onchange="setContinuousReading(this.checked)"><span class="continuous-switch-track"></span></span></label><button class="btn btn-gray" onclick="doResetCounters()">Zähler reset</button><button class="btn btn-gray" onclick="doReboot()">Neustart</button></div><div class="control-meta"><div class="kv"><span class="lbl">Firmware</span><span class="val" id="fw-ver-ctrl">—</span></div><a href="/config" class="btn btn-gray">Konfiguration</a></div></div></article>
</section><footer class="overview-info">Copyright Michael Kreutzer 2026 <span>·</span> GNU AGPLv3: Weitergabe und Änderung erlaubt <a href="https://github.com/ip6constructor/SMLEasy/blob/v3.6.0/LICENSE">Lizenztext</a> <span>·</span> <a href="https://github.com/ip6constructor/SMLEasy/tree/v3.6.0">Quellcode</a> <span>·</span> Keine Gewährleistung <span>·</span> Version <strong id="app-version">—</strong></footer><div class="overview-info network-addresses" aria-label="Netzwerkadressen"><div class="network-address-row"><span class="lbl">IP-V4: </span><span class="network-ip-list" id="wifi-ip">—</span></div><div class="network-address-row"><span class="lbl">IP-V6: </span><span class="network-ip-list" id="wifi-ip6">—</span></div></div></main></div></div><script src="/app.js"></script></body></html>
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
    <a href="https://github.com/ip6constructor/SMLEasy/tree/v3.6.0" class="btn btn-gray" style="text-decoration:none">Quellcode</a>
    <a href="https://github.com/ip6constructor/SMLEasy/blob/v3.6.0/LICENSE" class="btn btn-gray" style="text-decoration:none">AGPLv3</a>
    <a href="#general" class="btn btn-gray" data-cfg-tab="general" style="text-decoration:none">Allgemein</a><a href="#meter" class="btn btn-gray" data-cfg-tab="meter" style="text-decoration:none">Zähler</a><a href="#wifi" class="btn btn-gray" data-cfg-tab="wifi" style="text-decoration:none">WLAN</a><a href="#firmware" class="btn btn-gray" data-cfg-tab="firmware" style="text-decoration:none">Firmware</a>
    <a href="/tls" class="btn btn-gray" data-tls-link hidden style="text-decoration:none">HTTPS / Zertifikate</a>
    <a href="/pair" class="btn btn-gray" data-tls-link hidden style="text-decoration:none">App koppeln</a>
    <a href="/ext" class="btn btn-gray" data-ext-link hidden style="text-decoration:none">Externe Daten</a>
    <a href="/" class="btn btn-gray" style="text-decoration:none">← Dashboard</a>
  </div>
</header>
<p class="overview-info" style="padding:8px 16px">GNU AGPLv3: Weitergabe und Änderung erlaubt. Keine Gewährleistung.</p>
<main>
  <div class="card peach" data-section="general">
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

  <div class="card peach" data-section="general">
    <h2 class="tan">Zugriff auf das Zähler-Frontend</h2>
    <div class="card-inner">
      <form id="access-networks-form" onsubmit="saveAccessNetworks(event)">
        <label for="allowed-networks">Zusätzliche erlaubte CIDR-Netze</label>
        <textarea id="allowed-networks" rows="4" maxlength="512" placeholder="192.168.1.0/24&#10;2a00:6020:a105:c900::/64"></textarea>
        <p style="margin-top:8px;color:var(--lc-dim);font-size:.78rem">IPv4- oder IPv6-CIDR-Netze pro Zeile. Standardmäßig erlaubt: private IPv4-Netze, IPv6-ULA (fd95…), Link-Local (fe80::/10) und das eigene globale IPv6-/64-Präfix des ESP, auch wenn der Router es erst später zuweist. Andere globale Netze müssen hier explizit freigegeben werden.</p>
        <p style="margin-top:4px;color:var(--lc-dim);font-size:.74rem">Beispiele: 192.168.1.0/24, 2a00:6020:a105:c900::/64</p>
        <div class="save-row"><button class="btn btn-blue" type="submit">Netzwerke speichern</button></div>
      </form>
    </div>
  </div>

  <!-- Meter config -->
  <div class="card peach" data-section="meter">
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
          <input type="url" id="profile-url-input" placeholder="https://raw.githubusercontent.com/ip6constructor/SMLEasy/main/profiles.json">
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

  <div class="card peach" data-section="meter">
    <h2 class="tan">Tagesstart-Zählerstände</h2>
    <div class="card-inner">
      <p style="margin-bottom:10px;color:var(--lc-dim);font-size:.82rem">Tageswerte werden aus diesen Ständen berechnet und im Gerätespeicher gesichert.</p>
      <div id="daily-baseline-current" class="kv" style="margin-bottom:10px">Aktuelle Zählerstände werden geladen …</div>
      <form id="daily-baseline-form" onsubmit="saveDailyBaseline(event)">
        <label for="daily-import-start-kwh">Bezug zum Tagesstart (kWh)</label>
        <input id="daily-import-start-kwh" type="number" min="0.001" max="2147483.647" step="0.001" required>
        <label for="daily-export-start-kwh">Einspeisung zum Tagesstart (kWh)</label>
        <input id="daily-export-start-kwh" type="number" min="0.001" max="2147483.647" step="0.001" required>
        <div id="daily-baseline-status" class="kv" style="margin-top:8px">—</div>
        <div class="save-row" style="display:flex;gap:8px">
          <button class="btn btn-gray" type="button" onclick="loadDailyBaseline()">Tagesstart laden</button>
          <button class="btn btn-blue" type="submit">Tagesstart im Flash speichern</button>
        </div>
      </form>
    </div>
  </div>

  <!-- WiFi config -->
  <div class="card peach" data-section="wifi">
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
    <form id="hostname-form" onsubmit="saveHostname(event)" style="margin-top:16px" data-tls-section>
      <label>Hostname (FQDN, dauerhaft; Basis für QR-Code und Zertifikat)</label>
      <input type="text" name="hostname" maxlength="253" placeholder="smarthome.example.de">
      <div class="save-row"><button class="btn btn-blue" type="submit">Hostname speichern</button></div>
    </form>
    </div>
  </div>

  <!-- Tariff config -->
  <div class="card peach" data-section="meter">
    <h2 class="tan">Tarife</h2>
    <div class="card-inner">
    <form id="tariff-form" onsubmit="saveTariff(event)">
      <label>Bezugspreis (je kWh)</label>
      <input type="number" name="price_import_kwh" min="0" max="10" step="0.0001" value="0.30">
      <label>Einspeisevergütung (je kWh)</label>
      <input type="number" name="price_export_kwh" min="0" max="10" step="0.0001" value="0.0664">
      <label>Wallbox-Erstattung (je kWh)</label>
      <input type="number" name="wallbox_refund_kwh" min="0" max="10" step="0.0001" value="0.31">
      <label>Währung</label>
      <input type="text" name="currency" maxlength="4" value="EUR">
      <label>PV Ost: Module / Wp je Modul</label>
      <input type="number" name="pv_modules_east" min="0" max="500" step="1" value="0">
      <input type="number" name="pv_wp_east" min="0" max="2000" step="1" value="485">
      <label>PV Süd: Module / Wp je Modul</label>
      <input type="number" name="pv_modules_south" min="0" max="500" step="1" value="0">
      <input type="number" name="pv_wp_south" min="0" max="2000" step="1" value="485">
      <label>PV West: Module / Wp je Modul</label>
      <input type="number" name="pv_modules_west" min="0" max="500" step="1" value="0">
      <input type="number" name="pv_wp_west" min="0" max="2000" step="1" value="485">
      <div class="save-row"><button class="btn btn-blue" type="submit">Tarife speichern</button></div>
    </form>
    </div>
  </div>
    </form>
    </div>
  </div>

  <!-- OTA -->
  <div class="card orange" style="grid-column:1/-1" data-section="firmware">
    <h2 class="orange">OTA-Update (Firmware)</h2>
    <div class="card-inner">
    <div id="github-ota-panel">
      <label for="github-ota-select">Verfügbare GitHub-Versionen</label>
      <div style="display:flex;gap:8px;align-items:end;flex-wrap:wrap">
        <select id="github-ota-select" style="flex:1;min-width:220px"></select>
        <button class="btn btn-gray" id="github-ota-load" type="button" onclick="loadGithubVersions()">GitHub-Versionen laden</button>
        <button class="btn btn-blue" id="github-ota-install" type="button" onclick="installGithubVersion()">Ausgewählte Version installieren</button>
      </div>
      <div id="github-ota-result" style="margin-top:8px;font-size:.82rem;color:var(--primary-dark)"></div>
    </div>
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
<script>(function(){const names=['general','meter','wifi','firmware'];function show(){let s=(location.hash||'').slice(1);if(names.indexOf(s)<0)s='general';document.querySelectorAll('[data-section]').forEach(function(c){c.style.display=c.dataset.section===s?'':'none';});document.querySelectorAll('[data-cfg-tab]').forEach(function(a){a.classList.toggle('active',a.dataset.cfgTab===s);});}window.addEventListener('hashchange',show);show();})();</script>
<script src="/app.js"></script>
</body>
</html>
)rawhtml";
static const char kTlsHtml[] = R"rawhtml(<!DOCTYPE html>
<html lang="de">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>SMLEasy — HTTPS / Zertifikate</title>
<link rel="stylesheet" href="/style.css">
<link rel="icon" href="/favicon.ico" type="image/svg+xml">
</head>
<body>
<header>
  <div class="lcars-header-elbow sml-logo">)rawhtml" R"rawsvg(<svg viewBox="0 0 100 100" xmlns="http://www.w3.org/2000/svg" role="img" aria-label="SMLEasy"><defs><linearGradient id="smlGt" x1="0%" y1="0%" x2="100%" y2="100%"><stop offset="0%" stop-color="#22d3ee"/><stop offset="100%" stop-color="#2563eb"/></linearGradient></defs><circle cx="50" cy="50" r="42" fill="none" stroke="#0b1220" stroke-width="9"/><path d="M50 8 A42 42 0 0 1 90.5 40" fill="none" stroke="url(#smlGt)" stroke-width="9" stroke-linecap="round"/><path d="M8 55 A42 42 0 0 0 40 91.5" fill="none" stroke="url(#smlGt)" stroke-width="9" stroke-linecap="round"/><path d="M12 40 A42 42 0 0 1 30 15" fill="none" stroke="url(#smlGt)" stroke-width="9" stroke-linecap="round" opacity=".85"/><rect x="34" y="55" width="7" height="16" rx="1.5" fill="url(#smlGt)"/><rect x="44" y="48" width="7" height="23" rx="1.5" fill="url(#smlGt)"/><rect x="54" y="40" width="7" height="31" rx="1.5" fill="url(#smlGt)"/><path d="M72 30 L58 55 H68 L60 78 L80 50 H69 Z" fill="url(#smlGt)"/></svg>)rawsvg" R"rawhtml(</div>
  <div class="lcars-header-bar">
    <h1>SMLEasy &mdash; HTTPS / Zertifikate</h1>
    <a href="/config" class="btn btn-gray" style="text-decoration:none">Konfiguration</a>
    <a href="/" class="btn btn-gray" style="text-decoration:none">← Dashboard</a>
  </div>
</header>
<p class="overview-info" style="padding:8px 16px">GNU AGPLv3: Weitergabe und Änderung erlaubt. Keine Gewährleistung.</p>
<main>
  <div class="card peach" id="tls-card">
    <h2 class="tan">HTTPS / Zertifikate</h2>
    <div class="card-inner">
      <form id="tls-form" onsubmit="saveTlsConfig(event)">
        <p id="tls-status" style="color:var(--lc-dim);font-size:.82rem">—</p>
        <label for="tls-mode">Zertifikatsquelle</label>
        <select id="tls-mode" onchange="tlsModeChanged()">
          <option value="manual">Manuell / Self-Signed</option>
          <option value="letsencrypt">Let's Encrypt (ACME HTTP-01)</option>
        </select>
        <label for="tls-fqdn">Hostname (FQDN) – unter Konfiguration › WLAN änderbar</label>
        <input type="text" id="tls-fqdn" maxlength="253" readonly oninput="updateAcmeGate()">

        <div id="tls-acme-fields" hidden>
          <div class="acme-section">
            <h3>1. Nutzungsbedingungen</h3>
            <p style="color:var(--lc-dim);font-size:.78rem">Mit dem Anfordern eines Zertifikats stimmst du den Nutzungsbedingungen von Let's Encrypt zu. Das Gerät legt dafür ein Konto bei Let's Encrypt an.</p>
            <p style="margin:6px 0"><a id="acme-terms-link" href="https://letsencrypt.org/repository/" target="_blank" rel="noopener noreferrer">Nutzungsbedingungen lesen</a></p>
            <label style="display:flex;align-items:center;gap:8px;text-transform:none;letter-spacing:0"><input type="checkbox" id="tls-terms" style="width:auto" onchange="updateAcmeGate()"> Nutzungsbedingungen von Let's Encrypt akzeptieren</label>
          </div>

          <div class="acme-section">
            <h3>2. Angaben</h3>
            <label for="tls-email">E-Mail für Let's Encrypt</label>
            <input type="text" id="tls-email" maxlength="254" placeholder="name@example.de" oninput="updateAcmeGate()">
            <label style="display:flex;align-items:center;gap:8px;text-transform:none;letter-spacing:0"><input type="checkbox" id="tls-staging" style="width:auto"> Staging-Umgebung (Test, Zertifikat nicht vertrauenswürdig)</label>
            <label for="tls-interval">Erneuerung nach (Tagen)</label>
            <input type="number" id="tls-interval" min="1" max="60" value="60">
          </div>

          <div class="acme-section">
            <h3>3. Tests</h3>
            <p style="color:var(--lc-dim);font-size:.78rem">Vor der Anforderung prüft das Gerät Internetzugang, DNS und die Erreichbarkeit von außen. Die Erreichbarkeit wird mit einer Testvalidierung in der Staging-Umgebung von Let's Encrypt bewiesen; dabei wird kein Zertifikat ausgestellt.</p>
            <div id="acme-checks" class="check-list"></div>
            <div class="save-row"><button class="btn btn-gray" type="button" id="acme-check-btn" onclick="runAcmeChecks()" disabled>Tests starten</button></div>
          </div>

          <div class="acme-section">
            <h3>4. Zertifikat anfordern</h3>
            <div class="save-row"><button class="btn btn-blue" type="button" id="acme-request-btn" onclick="requestAcmeCertificate()" disabled>Zertifikat jetzt anfordern</button></div>
            <p id="acme-gate-hint" style="margin-top:8px;color:var(--lc-dim);font-size:.78rem"></p>
            <p id="acme-status" style="margin-top:8px;color:var(--lc-dim);font-size:.82rem"></p>
            <pre id="acme-log" hidden style="max-height:220px;overflow:auto;margin-top:6px;padding:10px;background:#f8fbfe;border:1px solid var(--border);border-radius:8px;font:.74rem/1.5 'Cascadia Code','JetBrains Mono',monospace;white-space:pre-wrap;word-break:break-word"></pre>
          </div>
        </div>

        <div id="tls-manual-fields">
          <label for="tls-cert">Zertifikat (PEM, optional)</label>
          <textarea id="tls-cert" rows="4" maxlength="3800" placeholder="-----BEGIN CERTIFICATE-----"></textarea>
          <label for="tls-key">Privater Schlüssel (PEM, optional)</label>
          <textarea id="tls-key" rows="4" maxlength="3800" placeholder="-----BEGIN PRIVATE KEY-----"></textarea>
          <label style="display:flex;align-items:center;gap:8px;text-transform:none;letter-spacing:0"><input type="checkbox" id="tls-clear" style="width:auto"> Eigenes Zertifikat entfernen (Self-Signed verwenden)</label>
          <p style="margin-top:8px;color:var(--lc-dim);font-size:.78rem">HTTPS läuft parallel zu HTTP auf Port 443. Ohne eigenes Zertifikat erzeugt das Gerät ein Self-Signed-Zertifikat; der Browser zeigt dafür eine Warnung. Zertifikat und privater Schlüssel müssen zusammenpassen.</p>
        </div>
        <div class="save-row"><button class="btn btn-blue" type="submit">HTTPS speichern</button></div>
      </form>
    </div>
  </div>
</main>
<script src="/app.js"></script>
</body>
</html>
)rawhtml";

static const char kPairHtml[] = R"rawhtml(<!DOCTYPE html>
<html lang="de">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>SMLEasy — App koppeln</title>
<link rel="stylesheet" href="/style.css">
<link rel="icon" href="/favicon.ico" type="image/svg+xml">
</head>
<body>
<header>
  <div class="lcars-header-bar">
    <h1>SMLEasy &mdash; App koppeln</h1>
    <a href="/config" class="btn btn-gray" style="text-decoration:none">Konfiguration</a>
    <a href="/" class="btn btn-gray" style="text-decoration:none">← Dashboard</a>
  </div>
</header>
<main>
  <div class="card peach">
    <h2 class="tan">App koppeln</h2>
    <div class="card-inner">
      <p style="color:var(--lc-dim);font-size:.82rem">Koppelmodus starten, dann in der App „Gerät suchen“ wählen. Der lokale TLS-Dienst auf TCP-Port 7384 ist maximal 5 Minuten aktiv; kein QR-Code, kein DNS und kein öffentliches Zertifikat nötig. Vor dem Bestätigen die sechsstellige Nummer mit der App vergleichen! Erst danach erhält genau diese Verbindung ein eigenes Zugriffstoken und den Zertifikatsfingerprint. Nur im vertrauenswürdigen lokalen Netz verwenden.</p>
      <p id="pair-msg" style="margin:8px 0;font-size:.82rem">—</p>
      <div id="pair-request" hidden style="margin-top:10px">
        <p id="pair-request-name"></p>
        <p>Vergleichsnummer: <strong id="pair-request-code" style="font-size:1.5rem"></strong></p>
        <div class="save-row">
          <button class="btn btn-blue" id="pair-approve" type="button">Nummer stimmt — App freigeben</button>
          <button class="btn btn-gray" id="pair-reject" type="button">Ablehnen</button>
        </div>
      </div>
      <div class="save-row">
        <button class="btn btn-blue" id="pair-open" type="button" onclick="pairToggle()">Koppelmodus starten (5 Minuten)</button>
      </div>
      <h3 style="margin-top:14px">Gekoppelte Geräte / Apps</h3>
      <p id="pair-capacity"></p>
      <p style="font-size:.82rem">Widerrufen sperrt das App-Token ab der nächsten Anfrage. Namen können geändert werden; Geheimnisse werden nie angezeigt.</p>
      <div id="pair-apps" style="font-size:.85rem"></div>
    </div>
  </div>
</main>
<script>
let pairTimer = null;
let pairIsOpen = false;
function pairRender(d) {
  const msg = document.getElementById('pair-msg');
  const open = !!d.open;
  pairIsOpen = open;
  document.getElementById('pair-open').textContent = open ? 'Koppelmodus beenden' : 'Koppelmodus starten (5 Minuten)';
  document.getElementById('pair-request').hidden = !open || !d.request_id;
  document.getElementById('pair-request-name').textContent = 'Anfrage von: ' + (d.request_name || '');
  document.getElementById('pair-request-code').textContent = d.code || '';
  document.getElementById('pair-approve').onclick = function () { pairDecide(d.request_id, true); };
  document.getElementById('pair-reject').onclick = function () { pairDecide(d.request_id, false); };
  msg.textContent = (open ? 'Koppelmodus aktiv, noch ' + d.remaining_s + ' s.' : 'Koppelmodus geschlossen.') +
    (d.error ? ' Fehler: ' + d.error : '') +
    (d.key_provider === 'development' ? ' Entwicklungsschlüssel — nicht produktiv.' : '');
  clearTimeout(pairTimer);
  const list = document.getElementById('pair-apps');
  list.textContent = '';
  document.getElementById('pair-capacity').textContent = (d.apps || []).length + ' von 4 App-Plätzen belegt.';
  (d.apps || []).forEach(function (a) {
    const row = document.createElement('div');
    row.textContent = a.name + ' (Platz ' + (a.slot + 1) + ')' +
      (a.created_at ? ' — gekoppelt ' + new Date(a.created_at * 1000).toLocaleString() : '') + ' ';
    const rename = document.createElement('button');
    rename.className = 'btn btn-gray';
    rename.textContent = 'Umbenennen';
    rename.onclick = function () {
      const name = prompt('Neuer App-/Gerätename (max. 40 UTF-8-Bytes)', a.name);
      if (name !== null) pairCall('/api/pairing/rename', 'POST', { slot: a.slot, name: name.trim() });
    };
    row.appendChild(rename);
    const b = document.createElement('button');
    b.className = 'btn btn-gray';
    b.type = 'button';
    b.textContent = 'Widerrufen';
    b.onclick = function () { pairRevoke(a.slot); };
    row.appendChild(b);
    list.appendChild(row);
  });
  pairTimer = setTimeout(pairPoll, open ? 1000 : 5000);
}
async function pairRevoke(slot) {
  if (!confirm('Diese App wirklich widerrufen?')) return;
  await pairCall('/api/pairing/revoke', 'POST', { slot: slot });
}
async function pairCall(path, method, body) {
  clearTimeout(pairTimer);
  try {
    const r = await fetch(path, { method: method, headers: { 'Content-Type': 'application/json' },
      body: body ? JSON.stringify(body) : undefined, cache: 'no-store' });
    if (!r.ok) { document.getElementById('pair-msg').textContent = 'Fehler: ' + (await r.text()); return; }
    pairRender(await r.json());
  } catch (e) {
    document.getElementById('pair-msg').textContent = 'Gerät nicht erreichbar.';
  }
}
function pairPoll() { pairCall('/api/pairing', 'GET'); }
function pairDecide(id, approve) { pairCall('/api/pairing/decide', 'POST', { request_id: id, approve: approve }); }
function pairToggle() { if (pairIsOpen) pairClose(); else pairOpen(); }
function pairOpen() { pairCall('/api/pairing/open', 'POST'); }
function pairClose() { pairCall('/api/pairing/close', 'POST'); }
pairPoll();
</script>
</body>
</html>
)rawhtml";

static const char kExtHtml[] = R"rawhtml(<!DOCTYPE html>
<html lang="de">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>SMLEasy — Externe Daten</title>
<link rel="stylesheet" href="/style.css">
<link rel="icon" href="/favicon.ico" type="image/svg+xml">
</head>
<body>
<header>
  <div class="lcars-header-elbow sml-logo">)rawhtml" R"rawsvg(<svg viewBox="0 0 100 100" xmlns="http://www.w3.org/2000/svg" role="img" aria-label="SMLEasy"><defs><linearGradient id="smlGe" x1="0%" y1="0%" x2="100%" y2="100%"><stop offset="0%" stop-color="#22d3ee"/><stop offset="100%" stop-color="#2563eb"/></linearGradient></defs><circle cx="50" cy="50" r="42" fill="none" stroke="#0b1220" stroke-width="9"/><path d="M50 8 A42 42 0 0 1 90.5 40" fill="none" stroke="url(#smlGe)" stroke-width="9" stroke-linecap="round"/><path d="M8 55 A42 42 0 0 0 40 91.5" fill="none" stroke="url(#smlGe)" stroke-width="9" stroke-linecap="round"/><path d="M12 40 A42 42 0 0 1 30 15" fill="none" stroke="url(#smlGe)" stroke-width="9" stroke-linecap="round" opacity=".85"/><rect x="34" y="55" width="7" height="16" rx="1.5" fill="url(#smlGe)"/><rect x="44" y="48" width="7" height="23" rx="1.5" fill="url(#smlGe)"/><rect x="54" y="40" width="7" height="31" rx="1.5" fill="url(#smlGe)"/><path d="M72 30 L58 55 H68 L60 78 L80 50 H69 Z" fill="url(#smlGe)"/></svg>)rawsvg" R"rawhtml(</div>
  <div class="lcars-header-bar">
    <h1>SMLEasy &mdash; Externe Daten</h1>
    <a href="/config" class="btn btn-gray" style="text-decoration:none">Konfiguration</a>
    <a href="/" class="btn btn-gray" style="text-decoration:none">← Dashboard</a>
  </div>
</header>
<p class="overview-info" style="padding:8px 16px">GNU AGPLv3: Weitergabe und Änderung erlaubt. Keine Gewährleistung.</p>
<main>
  <div class="card peach" id="ext-card-settings">
    <h2 class="tan">Externe Daten (Home Assistant)</h2>
    <div class="card-inner">
      <form id="ext-form" onsubmit="saveExtConfig(event)">
        <p id="ext-status" style="color:var(--lc-dim);font-size:.82rem">—</p>
        <label style="display:flex;align-items:center;gap:8px;text-transform:none;letter-spacing:0"><input type="checkbox" id="ext-enabled" style="width:auto"> Werte aus Home Assistant anzeigen</label>

        <div class="acme-section">
          <h3>REST-Abfrage</h3>
          <label for="ext-url">Home-Assistant-URL</label>
          <input type="text" id="ext-url" maxlength="200" placeholder="http://homeassistant.local:8123">
          <label for="ext-token">Zugriffstoken (Long-Lived Access Token)</label>
          <input type="password" id="ext-token" maxlength="512" autocomplete="off" placeholder="Leer lassen, um das gespeicherte Token zu behalten">
          <label style="display:flex;align-items:center;gap:8px;text-transform:none;letter-spacing:0"><input type="checkbox" id="ext-clear-token" style="width:auto"> Gespeichertes Token entfernen</label>
          <label for="ext-poll">Abfrageintervall (Sekunden)</label>
          <input type="number" id="ext-poll" min="10" max="600" value="30">
        </div>

        <div class="acme-section">
          <h3>Anzuzeigende Werte (bis zu 8)</h3>
          <div class="ext-row ext-head"><span>Bezeichnung</span><span>Quelle</span><span>Kurzname / Entity-ID / Topic</span><span>Einheit</span><span>JSON-Schlüssel (MQTT)</span><span>Rolle für Statistik</span></div>
          <div id="ext-rows"></div>
          <p style="margin-top:8px;color:var(--lc-dim);font-size:.78rem">REST: Das Gerät fragt Home Assistant über die REST-API ab. Ein Token erzeugst du in Home Assistant unter Profil → Sicherheit → Langlebige Zugriffstoken. MQTT: Das Gerät abonniert ein Topic am Broker der Home-Assistant-Integration (Einstellungen → Home Assistant); eine Automation in Home Assistant muss den Wert dort veröffentlichen. Bei JSON-Nachrichten gibt der Schlüssel das Feld an, zum Beispiel battery.soc. Textwerte (Auto-Name, Lademodus PV-Only/Both/Fast) werden unver&auml;ndert angezeigt. Rolle f&uuml;r Statistik: Werte mit der Rolle PV oder Batterie (Leistung in W oder kW) werden über die Zeit zu Wh aufsummiert und fließen in die Tages-, Wochen- und Monatsstatistik ein (Hausverbrauch, Eigenanteil, Ersparnis).</p>
        </div>

        <div class="save-row" style="display:flex;gap:8px;flex-wrap:wrap">
          <button class="btn btn-blue" type="submit">Externe Daten speichern</button>
          <button class="btn btn-gray" type="button" onclick="refreshExtNow()">Jetzt abrufen</button>
        </div>
      </form>
    </div>
  </div>

  <div class="card peach">
    <h2 class="tan">Aktuelle Werte</h2>
    <div class="card-inner">
      <div id="ext-live" class="check-list"></div>
    </div>
  </div>
</main>
<script src="/app.js"></script>
</body>
</html>
)rawhtml";

} // namespace app
