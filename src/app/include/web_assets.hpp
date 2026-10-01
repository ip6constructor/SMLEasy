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
:root{--bg-page:#f4f8fc;--bg-sidebar:rgba(255,255,255,.9);--bg-card:rgba(255,255,255,.9);--bg-card-hover:#fff;--text-primary:#132238;--text-secondary:#60758f;--text-muted:#8ca0b8;--primary:#168cff;--primary-light:#31c7ff;--primary-dark:#135bd8;--success:#24bf6b;--warning:#f4a928;--danger:#e85858;--border:#dce7f2;--border-strong:#c7d7e8;--shadow-card:0 10px 30px rgba(40,76,120,.06);--sidebar-width:240px;--header-height:72px;--radius-sm:8px;--radius-md:12px;--radius-lg:16px}
html{scroll-behavior:smooth}body{font-family:"Segoe UI",system-ui,sans-serif;color:var(--text-primary);background:radial-gradient(circle at 70% -10%,rgba(49,199,255,.12),transparent 30%),var(--bg-page);min-height:100vh}button,input,select{font:inherit}button{cursor:pointer}
.app{min-height:100vh;display:grid;grid-template-columns:var(--sidebar-width) 1fr}.sidebar{position:sticky;top:0;height:100vh;padding:24px 16px;background:var(--bg-sidebar);border-right:1px solid var(--border);backdrop-filter:blur(18px);display:flex;flex-direction:column;z-index:2}.brand{display:flex;align-items:center;gap:10px;margin:0 8px 34px;color:var(--text-primary);font-weight:750;font-size:1.05rem}.brand svg{width:38px;height:38px}.brand small{display:block;color:var(--text-muted);font-size:.66rem;font-weight:600;letter-spacing:.1em;text-transform:uppercase}.nav-title{padding:0 12px 8px;color:var(--text-muted);font-size:.67rem;font-weight:700;letter-spacing:.1em;text-transform:uppercase}.nav-item{min-height:44px;display:flex;align-items:center;gap:12px;padding:0 14px;border-radius:var(--radius-md);color:var(--text-secondary);text-decoration:none;font-size:.9rem;transition:background .16s,color .16s}.nav-item:hover{background:rgba(22,140,255,.06);color:var(--primary)}.nav-item.active{color:var(--primary);background:linear-gradient(90deg,rgba(22,140,255,.12),rgba(49,199,255,.04));font-weight:650}.nav-item svg{width:18px;height:18px;stroke:currentColor;fill:none;stroke-width:1.8}.sidebar-footer{margin-top:auto;padding:14px 12px;border-top:1px solid var(--border);color:var(--text-muted);font-size:.76rem}.status{display:inline-flex;align-items:center;gap:7px;color:var(--success);font-weight:650}.status-dot{width:8px;height:8px;border-radius:50%;background:currentColor;box-shadow:0 0 0 4px rgba(36,191,107,.1)}
.workspace{min-width:0}.topbar{height:var(--header-height);display:flex;align-items:center;justify-content:space-between;padding:0 32px;border-bottom:1px solid var(--border);background:rgba(255,255,255,.55);backdrop-filter:blur(14px)}.topbar h1{font-size:1.28rem;font-weight:700}.topbar-meta{display:flex;align-items:center;gap:18px;color:var(--text-muted);font-size:.78rem}.main-content{max-width:1500px;margin:0 auto;padding:28px 32px 40px}.eyebrow,.label{font-size:.7rem;font-weight:700;letter-spacing:.09em;text-transform:uppercase;color:var(--text-secondary)}.page-intro{display:flex;align-items:flex-end;justify-content:space-between;margin-bottom:22px}.page-intro h2{font-size:1.65rem;margin-top:5px}.page-intro p{color:var(--text-secondary);font-size:.88rem;margin-top:5px}.grid{display:grid;grid-template-columns:repeat(12,minmax(0,1fr));gap:16px}.card{position:relative;min-width:0;padding:20px;background:var(--bg-card);border:1px solid var(--border);border-radius:var(--radius-lg);box-shadow:var(--shadow-card);backdrop-filter:blur(14px);overflow:hidden}.card:before{content:"";position:absolute;inset:0 auto auto 0;width:80px;height:2px;background:linear-gradient(90deg,var(--primary-light),transparent)}.card h2{font-size:.78rem;font-weight:700;letter-spacing:.08em;text-transform:uppercase;color:var(--text-secondary);margin:0 0 16px}.card-inner{padding:0}.hero-card{grid-column:span 5;min-height:210px}.meter-card{grid-column:span 4}..connection-card{grid-column:span 3}.kpi-grid{grid-column:1/-1;display:grid;grid-template-columns:repeat(4,minmax(0,1fr));gap:16px}.kpi-card{padding:18px 20px}.metric-value,.kpi-value,.mono,.data-table td.value{font-family:"Cascadia Code","JetBrains Mono",monospace;font-variant-numeric:tabular-nums}.metric-value{font-size:clamp(2.5rem,4vw,4rem);font-weight:650;line-height:1;color:var(--text-primary);margin:20px 0 7px}.metric-unit{font-size:1rem;color:var(--text-secondary);margin-left:6px}.metric-sub{color:var(--text-secondary);font-size:.8rem}.direction-row{display:flex;gap:28px;margin-top:22px}.direction-row strong{display:block;font-family:"Cascadia Code","JetBrains Mono",monospace;font-size:1rem;margin-top:5px}.import{color:var(--primary)}.export{color:var(--success)}.kpi-value{font-size:1.55rem;font-weight:650;margin-top:10px}.kpi-change{font-size:.77rem;color:var(--text-secondary);margin-top:8px}.kpi-change.positive{color:var(--success)}.chart-card{grid-column:span 8;min-height:300px}.split-card{grid-column:span 4}.table-card{grid-column:span 9}.system-card{grid-column:span 3}.chart-wrap{height:220px;margin-top:8px}.chart-wrap svg{width:100%;height:100%;overflow:visible}.chart-grid{stroke:var(--border);stroke-width:1}.chart-line{fill:none;stroke:var(--primary);stroke-width:3;stroke-linecap:round;stroke-linejoin:round}.chart-area{fill:rgba(49,199,255,.13)}.chart-empty{fill:var(--text-muted);font-size:12px}.donut{width:150px;height:150px;margin:12px auto 18px;border-radius:50%;background:conic-gradient(var(--primary) 0 68%,#8bdcf4 68% 85%,var(--border) 85%);display:grid;place-items:center}.donut:after{content:"";width:92px;height:92px;border-radius:50%;background:#fff}.legend{display:grid;gap:10px;color:var(--text-secondary);font-size:.8rem}.legend span{display:flex;justify-content:space-between}.legend i{width:9px;height:9px;border-radius:50%;display:inline-block;margin-right:8px}.data-table{width:100%;border-collapse:collapse}.data-table th{padding:0 8px 10px;text-align:left;font-size:.68rem;font-weight:700;color:var(--text-muted);letter-spacing:.06em;text-transform:uppercase;border-bottom:1px solid var(--border)}.data-table td{padding:10px 8px;border-bottom:1px solid var(--border);font-size:.83rem}.data-table td.value{text-align:right;color:var(--text-primary)}.system-list{display:grid;gap:0}.system-list .kv{padding:10px 0}.kv{display:flex;justify-content:space-between;align-items:baseline;gap:12px;border-bottom:1px solid var(--border);font-size:.8rem}.kv .lbl{color:var(--text-secondary)}.kv .val{color:var(--text-primary);font-family:"Cascadia Code","JetBrains Mono",monospace;font-size:.8rem;font-weight:650;text-align:right}.btn{min-height:40px;padding:0 16px;border:1px solid transparent;border-radius:var(--radius-sm);font-size:.82rem;font-weight:650;transition:filter .15s,transform .15s}.btn:hover{filter:brightness(.97);transform:translateY(-1px)}.btn-green,.btn-blue{color:#fff;background:linear-gradient(135deg,var(--primary-dark),var(--primary-light));box-shadow:0 4px 12px rgba(22,140,255,.18)}.btn-red{color:#fff;background:var(--danger)}.btn-gray{color:var(--text-secondary);background:#fff;border-color:var(--border-strong)}.btn-row{display:flex;gap:8px;flex-wrap:wrap;margin-bottom:14px}.control-card{grid-column:1/-1}.control-card .card-inner{display:flex;align-items:center;justify-content:space-between;gap:16px}.control-meta{display:flex;gap:20px;flex-wrap:wrap}.control-meta .kv{border:0;display:grid;gap:3px}.control-meta .val{text-align:left}.card.peach,.card.orange{grid-column:span 6}.card.orange[style],.card.peach[style]{grid-column:1/-1}.card h2.orange,.card h2.tan{background:none;color:var(--text-secondary);padding:0}.card h2.orange{color:var(--primary-dark)}label{display:block;font-size:.72rem;font-weight:650;color:var(--text-secondary);margin:12px 0 5px;text-transform:uppercase;letter-spacing:.06em}input[type=text],input[type=password],input[type=number],input[type=url],select{width:100%;min-height:42px;padding:0 12px;background:#fff;border:1px solid var(--border-strong);border-radius:var(--radius-sm);color:var(--text-primary);font-size:.9rem}input:focus,select:focus{outline:none;border-color:var(--primary);box-shadow:0 0 0 3px rgba(22,140,255,.1)}.save-row{margin-top:14px}.lcars-bar{height:1px;background:var(--border);margin:18px 0}.progress{height:7px;background:var(--border);border-radius:8px;overflow:hidden;margin-top:6px}.progress-bar{height:100%;background:linear-gradient(90deg,var(--primary),var(--primary-light));width:0;transition:width .3s}#ota-result{margin-top:8px;font-size:.82rem;color:var(--primary-dark)}#log{background:#f8fbfe;border-radius:var(--radius-sm);padding:10px;height:300px;overflow-y:auto;font-size:.78rem;font-family:monospace;border:1px solid var(--border)}.log-I{color:var(--primary-dark)}.log-W{color:var(--warning)}.log-E{color:var(--danger)}.log-D{color:var(--text-muted)}footer{color:var(--text-muted)!important;border-top:1px solid var(--border)!important}footer a,footer span{color:var(--text-secondary)!important}
@media(max-width:1100px){.hero-card,.meter-card,.connection-card{grid-column:span 6}.chart-card,.table-card{grid-column:span 8}.split-card,.system-card{grid-column:span 4}.sidebar{width:72px;padding:24px 10px}.app{grid-template-columns:72px 1fr}.brand{justify-content:center;margin:0 0 34px}.brand>div,.nav-title,.nav-item span,.sidebar-footer div:not(.status){display:none}.nav-item{justify-content:center;padding:0}.sidebar-footer{padding:14px 0;text-align:center}.main-content{padding:24px}}
@media(max-width:760px){.app{display:block}.sidebar{display:none}.topbar{padding:0 18px}.topbar-meta{gap:8px}.main-content{padding:20px 14px 32px}.page-intro{display:block}.grid{grid-template-columns:1fr}.hero-card,.meter-card,.connection-card,.chart-card,.split-card,.table-card,.system-card,.card.peach,.card.orange{grid-column:1/-1}.kpi-grid{grid-template-columns:repeat(2,minmax(0,1fr));gap:12px}.control-card .card-inner{display:block}.control-meta{margin-top:14px}.data-table{min-width:560px}.table-card{overflow-x:auto}}
@media(max-width:430px){.kpi-grid{grid-template-columns:1fr}.metric-value{font-size:2.7rem}}.connection-card{grid-column:span 3}@media(max-width:1100px){.connection-card{grid-column:span 6}}@media(max-width:760px){.connection-card{grid-column:1/-1}}header{display:flex;align-items:center;min-height:var(--header-height);padding:0 24px;border-bottom:1px solid var(--border);background:rgba(255,255,255,.72)}.lcars-header-elbow{display:flex;align-items:center;justify-content:center;width:52px;height:52px;flex:0 0 52px}.lcars-header-elbow.sml-logo svg{width:38px;height:38px;display:block}.lcars-header-bar{display:flex;align-items:center;gap:16px;flex:1;margin-left:14px}.lcars-header-bar h1{font-size:1.15rem;color:var(--text-primary);font-weight:700}.lcars-header-bar .btn{margin-left:auto}.overview-info{display:flex;justify-content:center;align-items:center;gap:8px;padding:20px 8px;color:var(--text-muted);font-size:.76rem;text-align:center}.overview-info strong{color:var(--text-secondary);font-family:"Cascadia Code","JetBrains Mono",monospace}
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
      banner.textContent = '\u26A0 Gerät nicht erreichbar. Bitte Seite manuell neu laden.';
      banner.style.background = '#CC0000';
      banner.style.color = '#fff';
      return;
    }
    banner.textContent = '\u21BB Verbindung unterbrochen \u2014 Wiederverbindung in ' + remaining + ' s …';
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
        banner2.textContent = '\u21BB Verbindung unterbrochen \u2014 Wiederverbindung in ' + remaining + ' s …';
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
  btn.textContent = logPaused ? '\u25B6 Weiter' : '\u23F8 Pause';
  btn.style.background = logPaused ? '#0f9d58' : '';
}

function copyLog() {
  const lines = document.getElementById('log').innerText;
  navigator.clipboard.writeText(lines).then(
    () => alert('Log in Zwischenablage kopiert'),
    () => alert('Kopieren fehlgeschlagen')
  );
}

function pill(state) {
  const p = document.getElementById('status-pill');
  if (!p) return;
  p.innerHTML = '<i class="status-dot"></i>' + state;
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
    kv('m-fwh',   d.fwd_active_wh   != null ? d.fwd_active_wh  + ' Wh' : null);
    kv('m-rwh',   d.rev_active_wh   != null ? d.rev_active_wh  + ' Wh' : null);
    kv('m-irh',   d.import_react_varh != null ? d.import_react_varh + ' varh' : null);
    kv('m-erh',   d.export_react_varh != null ? d.export_react_varh + ' varh' : null);
    kv('m-fw-w',  d.fwd_w != null ? (d.fwd_w/1000).toFixed(3) + ' kW' : null);
    kv('m-rv-w',  d.rev_w != null ? (d.rev_w/1000).toFixed(3) + ' kW' : null);
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
    kv('wifi-ip', d.ip); kv('wifi-ipv6', d.ipv6 || 'Nicht verfügbar'); kv('wifi-ssid', d.ssid);
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
    alert('Ablesung läuft bereits.');
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
    alert('Ablesung läuft bereits.');
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
  if (!confirm('Gerät neu starten?')) return;
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
    alert('Bitte eine URL eintragen.');
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
    if (status) status.textContent = count + ' externe Profile geladen';
  } catch (err) {
    if (status) status.textContent = 'Fehler beim Laden';
    alert('Profilkatalog konnte nicht geladen werden: ' + (err && err.message ? err.message : err));
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
  alert('Custom-Profil ' + slot + ' gespeichert.');
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
        alert('Ungueltiges OBIS-Format bei ' + key + ': ' + p + '\nErwartet: 1-0:1.8.0*255');
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
  alert(r.ok ? 'Zähler-Konfiguration gespeichert' : 'Fehler beim Speichern');
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
    ? 'WLAN gespeichert und verbunden.'
    : result.ok ? 'WLAN gespeichert, Verbindung fehlgeschlagen. AP bleibt aktiv.' : 'Fehler beim Speichern');
}

async function saveWebPassword(e) {
  e.preventDefault();
  const password = e.target.password.value;
  const confirmation = e.target.password_confirm.value;
  if (password.length < 8 || password !== confirmation) {
    alert('Passwörter müssen übereinstimmen und mindestens 8 Zeichen lang sein.');
    return;
  }
  const r = await fetch('/api/config/auth', {method:'POST', headers:{'Content-Type':'application/json'}, body:JSON.stringify({password})});
  if (r.ok && (await r.json()).ok) {
    e.target.reset();
    alert('Passwort gespeichert. Beim nächsten Zugriff wird das neue Passwort verwendet.');
  } else alert('Passwort konnte nicht gespeichert werden.');
}

async function scanWifi() {
  const button = document.getElementById('wifi-scan-btn');
  const select = document.getElementById('wifi-network-select');
  const status = document.getElementById('wifi-scan-status');
  if (!button || !select) return;
  button.disabled = true;
  if (status) status.textContent = 'Suche nach Netzwerken ...';
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
    if (status) status.textContent = (data.networks || []).length + ' Netzwerke gefunden';
  } catch (_) {
    if (status) status.textContent = 'Scan fehlgeschlagen';
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
  alert(r.ok && (await r.json()).ok ? 'Tarife gespeichert.' : 'Tarife konnten nicht gespeichert werden.');
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
    if (pill) pill.textContent = d.connected ? '● Verbunden' : '○ Getrennt';
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
  alert(j && j.ok ? 'HA-Integration gespeichert.' : 'Fehler beim Speichern');
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
    kv('meter-interval', (d.interval_s ?? 30) + ' s');
    f.uart_debug.checked      = d.uart_debug      ?? false;
    f.login_cmd.value = d.login_cmd ?? '';
    f.login_wait_ms.value = d.login_wait_ms ?? 250;
    f.clear_meter_pin.checked = false;
    const pinInfo = document.getElementById('meter-pin-info');
    if (pinInfo) pinInfo.textContent = (d.has_meter_pin ? 'PIN ist gesetzt' : 'PIN ist nicht gesetzt');
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

// OTA upload
function startOta(file) {
  if (!file) return;
  const bar = document.getElementById('ota-bar');
  const res = document.getElementById('ota-result');
  const btn = document.getElementById('ota-btn');
  btn.disabled = true;
  bar.style.width = '0%';
  res.textContent = 'Übertrage …';
  const xhr = new XMLHttpRequest();
  xhr.upload.addEventListener('progress', ev => {
    if (ev.lengthComputable)
      bar.style.width = (ev.loaded / ev.total * 100) + '%';
  });
  xhr.onload = () => {
    bar.style.width = '100%';
    btn.disabled = false;
    res.textContent = xhr.status === 200
      ? 'Update erfolgreich! Gerät startet neu …'
      : 'Fehler: ' + xhr.responseText;
  };
  xhr.onerror = () => { btn.disabled = false; res.textContent = 'Verbindungsfehler'; };
  xhr.open('POST', '/api/ota');
  xhr.send(file);
}
document.addEventListener('DOMContentLoaded', () => {
  renderProfileOptions();
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
<article class="card hero-card"><h2>Aktuelle Leistung</h2><div class="metric-value" id="current-power">—<span class="metric-unit">W</span></div><div class="metric-sub" id="power-kilowatt">— kW netto</div><div class="direction-row"><div class="import"><span class="label">Bezug</span><strong id="m-fw-w">—</strong></div><div class="export"><span class="label">Einspeisung</span><strong id="m-rv-w">—</strong></div></div></article>
<article class="card meter-card"><h2>Zählerstand</h2><div class="kpi-value" id="m-fwh">—</div><div class="label" style="margin-top:7px">Bezug gesamt</div><div class="kv" style="margin-top:20px"><span class="lbl">Einspeisung gesamt</span><span class="val" id="m-rwh">—</span></div><div class="kv"><span class="lbl">Zählerzeit</span><span class="val" id="m-time">—</span></div></article>
<article class="card connection-card"><h2>Verbindung</h2><div class="status"><i class="status-dot"></i><span>—</span></div><div class="kv" style="margin-top:20px"><span class="lbl">IPv4</span><span class="val" id="wifi-ip">—</span></div><div class="kv"><span class="lbl">IPv6</span><span class="val" id="wifi-ipv6">—</span></div><div class="kv"><span class="lbl">WLAN</span><span class="val" id="wifi-ssid">—</span></div><div class="kv"><span class="lbl">Uptime</span><span class="val" id="uptime">—</span></div></article>
<div class="kpi-grid"><article class="card kpi-card"><div class="label">Verbrauch heute</div><div class="kpi-value" id="today-import">—</div><div class="kpi-change positive">Importierte Energie</div></article><article class="card kpi-card"><div class="label">Einspeisung heute</div><div class="kpi-value" id="today-export">—</div><div class="kpi-change">Exportierte Energie</div></article><article class="card kpi-card"><div class="label">Nettokosten heute</div><div class="kpi-value" id="cost-today">—</div><div class="kpi-change" id="cost-breakdown">Bezug minus Einspeisevergütung</div></article><article class="card kpi-card"><div class="label">Abfrageintervall</div><div class="kpi-value" id="meter-interval">—</div><div class="kpi-change">Automatische Aktualisierung</div></article></div>
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

  <!-- OTA -->
  <div class="card orange" style="grid-column:1/-1">
    <h2 class="orange">OTA-Update (Firmware)</h2>
    <div class="card-inner">
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
