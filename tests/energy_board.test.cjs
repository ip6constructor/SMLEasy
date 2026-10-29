const { test } = require('node:test');
const assert = require('node:assert/strict');
const fs = require('node:fs');
const path = require('node:path');
const vm = require('node:vm');

const assets = fs.readFileSync(path.join(__dirname, '..', 'src', 'app', 'include', 'web_assets.hpp'), 'utf8');
const valuesSource = assets.slice(assets.indexOf('function ehKpiValues('), assets.indexOf('function ehRenderKpis('));
const context = vm.createContext({});
vm.runInContext(valuesSource, context);
const values = data => Array.from(context.ehKpiValues(data));

test('footer renders all four real status values and updates on the next sample', () => {
    assert.deepEqual(values({
        today_pv_kwh: 12.5, today_house_kwh: 10, today_import_kwh: 2, cost_today: 1.23, today_saving: 3.45, currency: 'EUR',
    }), ['80 %', '12.5 kWh', '1.23 EUR', '3.45 EUR']);
    assert.deepEqual(values({
        today_pv_kwh: 20, today_house_kwh: 20, today_import_kwh: 10, cost_today: -2, today_saving: 5, currency: 'CHF',
    }), ['50 %', '20.0 kWh', '-2.00 CHF', '5.00 CHF']);
});

test('missing, null and invalid values never become plausible zero readings', () => {
    assert.deepEqual(values({}), ['—', '—', '—', '—']);
    assert.deepEqual(values({ today_pv_kwh: null, cost_today: NaN, today_saving: '3.0' }), ['—', '—', '—', '—']);
    assert.deepEqual(values({
        today_pv_kwh: 0, today_house_kwh: 0, today_import_kwh: 0, today_export_kwh: 0, cost_today: 0, today_saving: 0,
    }), ['—', '0.0 kWh', '0.00 EUR', '0.00 EUR']);
    assert.equal(values({ today_pv_kwh: 10 })[0], '—');
    assert.equal(values({ today_pv_kwh: 1, today_house_kwh: 1, today_import_kwh: 2 })[0], '0 %');
});

test('external-data failure cannot prevent a fresh status footer', async () => {
    const rendered = [];
    const footer = { dataset: {} };
    const sandbox = vm.createContext({
        ehGet: async url => {
            if (url === '/api/status') return { cost_today: 8.5, ext_supported: true };
            throw new Error('External data unavailable');
        },
        ehRenderKpis: data => rendered.push(data),
        document: { getElementById: () => footer },
        localizedText: text => text,
        console: { warn() {} },
    });
    const source = assets.slice(assets.indexOf('async function ehRefresh('), assets.indexOf('function ehKpiValues('));
    vm.runInContext(source, sandbox);
    await sandbox.ehRefresh();
    assert.equal(rendered.length, 1);
    assert.equal(rendered[0].cost_today, 8.5);
    assert.match(footer.dataset.error, /veraltet/);
});

test('pairing page has web approval and management but no QR bootstrap controls', () => {
    const page = assets.slice(assets.indexOf('static const char kPairHtml[]'), assets.indexOf('static const char kExtHtml[]'));
    assert.match(page, /Koppelmodus starten \(5 Minuten\)/);
    assert.match(page, /\/api\/pairing\/decide/);
    assert.match(page, /\/api\/pairing\/rename/);
    assert.match(page, /\/api\/pairing\/revoke/);
    assert.doesNotMatch(page, /id="pair-qr"|smleasy:\/\/pair|pairCopy/);
    const script = page.match(/<script>([\s\S]*?)<\/script>/)[1];
    new vm.Script(script);
});
