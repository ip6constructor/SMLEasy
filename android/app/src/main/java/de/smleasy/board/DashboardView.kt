package de.smleasy.board

import android.content.Context
import android.graphics.Canvas
import android.graphics.Color
import android.graphics.Paint
import android.graphics.RectF
import android.graphics.Typeface
import android.graphics.drawable.GradientDrawable
import android.view.Gravity
import android.view.View
import android.view.ViewGroup
import android.widget.Button
import android.widget.HorizontalScrollView
import android.widget.LinearLayout
import android.widget.ScrollView
import android.widget.TableLayout
import android.widget.TableRow
import android.widget.TextView
import org.json.JSONArray
import org.json.JSONObject
import java.util.Locale

/**
 * Native Dashboard-Ansicht (Gegenstueck zur Dashboard-Seite im ESP-Web): Leistung, Zaehlerstaende,
 * Tageswerte, Haus & Energie, Verbrauchsaufteilung, Systemstatus und Statistik je Tag/Woche/Monat.
 * Rendert nur aus der REST-API.
 */
class DashboardView(context: Context) : ScrollView(context) {
    private val bg = Color.parseColor("#eef5fc")
    private val ink = Color.parseColor("#0f1f3a")
    private val muted = Color.parseColor("#64748b")
    private val primary = Color.parseColor("#1e90ff")
    private val green = Color.parseColor("#16a34a")
    private val tileBg = Color.parseColor("#eaf3ff")

    private val box = LinearLayout(context).apply {
        orientation = LinearLayout.VERTICAL
        val p = dp(14)
        setPadding(p, dp(52), p, p)
    }
    private var status: JSONObject? = null
    private var ext: JSONObject? = null
    private var stats: JSONObject? = null
    private var statsView = "days"
    private val samples = ArrayDeque<Triple<Long, Double, Double>>()

    init {
        setBackgroundColor(bg)
        addView(box, ViewGroup.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.WRAP_CONTENT))
    }

    fun setData(st: JSONObject?, extValues: JSONObject?, statsValues: JSONObject?) {
        st?.let {
            status = it
            val net = (num(it, "fwd_w") ?: 0.0) - (num(it, "rev_w") ?: 0.0)
            val now = System.currentTimeMillis()
            samples.addLast(Triple(now, maxOf(0.0, net), maxOf(0.0, -net)))
            while (samples.isNotEmpty() && now - samples.first().first > 3_600_000) samples.removeFirst()
        }
        extValues?.let { ext = it }
        statsValues?.let { stats = it }
        render()
    }

    private fun dp(v: Int) = (v * resources.displayMetrics.density).toInt()

    private fun columns(maxCols: Int): Int {
        val wDp = resources.displayMetrics.widthPixels / resources.displayMetrics.density
        return minOf(maxCols, if (wDp < 560) 1 else if (wDp < 900) 2 else 3)
    }

    private fun text(s: String, sp: Float, color: Int = ink, bold: Boolean = false, mono: Boolean = false) = TextView(context).apply {
        text = s
        textSize = sp
        setTextColor(color)
        if (mono) typeface = Typeface.create(Typeface.MONOSPACE, if (bold) Typeface.BOLD else Typeface.NORMAL)
        else if (bold) typeface = Typeface.DEFAULT_BOLD
    }

    private fun label(s: String) = text(s.uppercase(Locale.GERMANY), 10.5f, muted, true).apply { letterSpacing = 0.08f }

    private fun panel(fill: Int, stroke: Int, radius: Int = 14) = GradientDrawable().apply {
        cornerRadius = dp(radius).toFloat()
        setColor(fill)
        setStroke(dp(1), stroke)
    }

    private fun card(title: String?, fill: Int = Color.WHITE, stroke: Int = Color.parseColor("#dbe7f5")): LinearLayout =
        LinearLayout(context).apply {
            orientation = LinearLayout.VERTICAL
            background = panel(fill, stroke)
            setPadding(dp(14), dp(12), dp(14), dp(12))
            title?.let { addView(label(it)) }
        }

    private fun tile(title: String) = card(title, tileBg, Color.parseColor("#c9dcf3"))

    private fun kv(parent: LinearLayout, k: String, v: String) {
        val r = LinearLayout(context).apply { orientation = LinearLayout.HORIZONTAL; setPadding(0, dp(6), 0, dp(2)) }
        r.addView(text(k, 12f, muted), LinearLayout.LayoutParams(0, ViewGroup.LayoutParams.WRAP_CONTENT, 1f))
        r.addView(text(v, 12.5f, ink, true, true))
        parent.addView(r)
    }

    private fun gridRows(views: List<View>, cols: Int, gap: Int, rowGap: Int): LinearLayout {
        val out = LinearLayout(context).apply { orientation = LinearLayout.VERTICAL }
        views.chunked(cols).forEachIndexed { n, chunk ->
            val row = LinearLayout(context).apply { orientation = LinearLayout.HORIZONTAL }
            for (i in 0 until cols) {
                row.addView(chunk.getOrNull(i) ?: View(context), LinearLayout.LayoutParams(0, ViewGroup.LayoutParams.MATCH_PARENT, 1f).apply {
                    if (i < cols - 1) rightMargin = dp(gap)
                })
            }
            out.addView(row, LinearLayout.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.WRAP_CONTENT).apply {
                if (n > 0) topMargin = dp(rowGap)
            })
        }
        return out
    }

    private fun grid(views: List<View>, maxCols: Int = 3) {
        if (views.isEmpty()) return
        box.addView(gridRows(views, columns(maxCols), 10, 10), lp())
    }

    private fun lp() = LinearLayout.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.WRAP_CONTENT).apply { bottomMargin = dp(12) }

    private fun num(o: JSONObject, key: String): Double? =
        if (o.has(key) && !o.isNull(key) && o.opt(key) is Number) o.optDouble(key).takeIf { it.isFinite() } else null

    private fun fmt(v: Double?, digits: Int, unit: String) =
        if (v == null) "?" else String.format(Locale.GERMANY, "%.${digits}f", v) + if (unit.isEmpty()) "" else " $unit"

    private class Bar(context: Context, val fraction: Float, val color: Int) : View(context) {
        private val p = Paint(Paint.ANTI_ALIAS_FLAG)
        override fun onDraw(c: Canvas) {
            val r = height / 2f
            p.color = Color.parseColor("#dbe3ee")
            c.drawRoundRect(0f, 0f, width.toFloat(), height.toFloat(), r, r, p)
            p.color = color
            c.drawRoundRect(0f, 0f, width * fraction.coerceIn(0f, 1f), height.toFloat(), r, r, p)
        }
    }

    private class Donut(context: Context, val a: Float, val b: Float) : View(context) {
        private val p = Paint(Paint.ANTI_ALIAS_FLAG).apply { style = Paint.Style.STROKE }
        override fun onDraw(c: Canvas) {
            val d = minOf(width, height).toFloat()
            val w = d * 0.18f
            p.strokeWidth = w
            val left = (width - d) / 2f
            val r = RectF(left + w / 2, w / 2, left + d - w / 2, d - w / 2)
            p.color = Color.parseColor("#dbe3ee")
            c.drawArc(r, 0f, 360f, false, p)
            val total = a + b
            if (total <= 0f) return
            p.color = Color.parseColor("#8bdcf4")
            c.drawArc(r, -90f, 360f, false, p)
            p.color = Color.parseColor("#1e90ff")
            c.drawArc(r, -90f, 360f * a / total, false, p)
        }
    }

    private fun render() {
        val y = scrollY
        box.removeAllViews()
        val st = status
        if (st == null) {
            box.addView(text("Warte auf Daten ...", 15f, muted))
            return
        }
        val currency = st.optString("currency", "EUR").ifEmpty { "EUR" }

        box.addView(label("Live-\u00dcberwachung"))
        box.addView(text("Verbrauch auf einen Blick", 22f, ink, true))
        box.addView(text("Aktuelle Messwerte und Energiefluss deines Z\u00e4hlers.", 12.5f, muted).apply { setPadding(0, 0, 0, dp(12)) })

        val top = mutableListOf<View>()
        if (st.optBoolean("has_power")) top += heroCard(st)
        if (st.optBoolean("has_fwd_active_wh") || st.optBoolean("has_rev_active_wh")) top += counterCard(st)
        grid(top, 2)

        if (st.optBoolean("populated")) {
            val imp = num(st, "today_import_kwh")
            val exp = num(st, "today_export_kwh")
            fun kpi(t: String, v: String, sub: String, col: Int) = card(t).apply {
                addView(text(v, 22f, ink, true, true))
                addView(text(sub, 11.5f, col))
            }
            val impCost = imp?.times(num(st, "price_import_kwh") ?: 0.0)
            val expCost = exp?.times(num(st, "price_export_kwh") ?: 0.0)
            grid(listOf(
                kpi("Verbrauch heute", fmt(imp, 2, "kWh"), "Importierte Energie", green),
                kpi("Einspeisung heute", fmt(exp, 2, "kWh"), "Exportierte Energie", muted),
                kpi("Nettokosten heute", fmt(num(st, "cost_today"), 2, currency), "Bezug ${fmt(impCost, 2, "")} \u2212 Einspeisung ${fmt(expCost, 2, currency)}", muted),
            ))
        }

        energyCard(st, currency)?.let { box.addView(it, lp()) }

        val side = mutableListOf<View>()
        if (st.optBoolean("populated")) side += splitCard(st)
        side += systemCard(st)
        grid(side, 2)

        renderStats(currency)
        post { scrollTo(0, y) }
    }

    private fun heroCard(st: JSONObject): View {
        val net = (num(st, "fwd_w") ?: 0.0) - (num(st, "rev_w") ?: 0.0)
        val peakImp = samples.maxOfOrNull { it.second } ?: 0.0
        val peakExp = samples.maxOfOrNull { it.third } ?: 0.0
        val scale = maxOf(6000.0, peakImp, peakExp)
        return card("Aktuelle Leistung").apply {
            addView(text(String.format(Locale.GERMANY, "%s%.0f W", if (net < 0) "\u2212" else "", Math.abs(net)), 38f, ink, true, true))
            addView(text(String.format(Locale.GERMANY, "%.2f kW netto", net / 1000), 12f, muted))
            addView(Bar(context, (Math.abs(net) / scale).toFloat(), if (net < 0) green else primary), LinearLayout.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT, dp(10)).apply { topMargin = dp(10) })
            val sc = LinearLayout(context).apply { orientation = LinearLayout.HORIZONTAL }
            sc.addView(text("0", 11f, muted), LinearLayout.LayoutParams(0, ViewGroup.LayoutParams.WRAP_CONTENT, 1f))
            sc.addView(text(String.format(Locale.GERMANY, "%.0f kW", scale / 1000), 11f, muted))
            addView(sc)
            val amps = listOf("i_l1", "i_l2", "i_l3").map { if (st.optBoolean("has_$it")) num(st, it + "_ma") else null }
            kv(this, "Spitze Bezug (1 h)", String.format(Locale.GERMANY, "%.2f kW", peakImp / 1000))
            kv(this, "Spitze Einspeisung (1 h)", String.format(Locale.GERMANY, "%.2f kW", peakExp / 1000))
            kv(this, "Strom je Phase", if (amps.all { it == null }) "\u2014" else amps.joinToString(" / ") { a -> a?.let { String.format(Locale.GERMANY, "%.1f", it / 1000) } ?: "\u2014" } + " A")
        }
    }

    private fun counterCard(st: JSONObject): View {
        fun counter(title: String, wh: Double?, col: Int) = tile(title).apply {
            val v = wh?.let { String.format(Locale.US, "%010.3f", it / 1000) } ?: "\u2014"
            addView(text(v, 16f, col, true, true).apply { setPadding(0, dp(8), 0, 0) })
            addView(text("kWh", 11f, muted))
        }
        val pair = gridRows(listOf(counter("1.8.0 / Bezug gesamt", num(st, "fwd_active_wh"), primary), counter("2.8.0 / Einspeisung", num(st, "rev_active_wh"), green)), 2, 8, 8)
        return card("Z\u00e4hlerst\u00e4nde").apply {
            addView(text("Bezug und Einspeisung als Z\u00e4hlwerke", 11.5f, muted).apply { setPadding(0, dp(4), 0, dp(8)) })
            addView(pair)
            kv(this, "Z\u00e4hlerzeit", st.optString("meter_time").ifEmpty { "\u2014" })
        }
    }

    private fun energyCard(st: JSONObject, currency: String): View? {
        val items = ext?.optJSONArray("items") ?: return null
        if (items.length() == 0) return null
        val tiles = mutableListOf<View>()
        val todayKey = mapOf("pv_kwh" to "today_pv_kwh", "bat_chg_kwh" to "today_bat_chg_kwh", "bat_dis_kwh" to "today_bat_dis_kwh", "wb_kwh" to "today_wb_kwh")
        for (i in 0 until items.length()) {
            val it = items.optJSONObject(i) ?: continue
            val unit = it.optString("unit")
            val role = it.optString("role")
            val v = num(it, "value")
            val t = tile(it.optString("label"))
            if (unit == "%" && v != null) {
                val col = if (v < 40) Color.parseColor("#ef4444") else if (v > 60) green else Color.parseColor("#f59e0b")
                t.addView(text(String.format(Locale.GERMANY, "%.0f %%", v), 18f, ink, true, true).apply { setPadding(0, dp(6), 0, 0) })
                t.addView(Bar(context, (v / 100).toFloat(), col), LinearLayout.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT, dp(10)).apply { topMargin = dp(6) })
            } else {
                val s = it.optString("text").ifEmpty { null }
                    ?: v?.let { x -> String.format(Locale.GERMANY, if (x % 1.0 == 0.0 || unit == "W") "%.0f" else "%.1f", x) + (if (unit.isEmpty()) "" else " $unit") }
                    ?: "\u2014"
                t.addView(text(s, 18f, ink, true, true).apply { setPadding(0, dp(6), 0, 0) })
            }
            todayKey[role]?.let { k -> kv(t, "Heute", fmt(num(st, k), 2, "kWh")) }
            if (role == "pv_kwh") kv(t, "Ersparnis heute", fmt(num(st, "today_saving"), 2, currency))
            tiles += t
        }
        if (num(st, "today_import_kwh") != null) {
            fun n(k: String) = num(st, k) ?: 0.0
            val house = n("today_import_kwh") - n("today_export_kwh") + n("today_pv_kwh") - n("today_bat_chg_kwh") + n("today_bat_dis_kwh")
            tiles += tile("Hausverbrauch heute").apply { addView(text(fmt(maxOf(0.0, house), 2, "kWh"), 18f, ink, true, true).apply { setPadding(0, dp(6), 0, 0) }) }
        }
        return card("Haus & Energie").apply {
            addView(gridRows(tiles, columns(3), 8, 8), LinearLayout.LayoutParams(ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.WRAP_CONTENT).apply { topMargin = dp(8) })
        }
    }

    private fun splitCard(st: JSONObject): View {
        val today = num(st, "today_import_kwh") ?: 0.0
        val month = num(st, "month_import_kwh") ?: 0.0
        return card("Verbrauchsaufteilung").apply {
            addView(Donut(context, today.toFloat(), maxOf(0.0, month - today).toFloat()), LinearLayout.LayoutParams(dp(150), dp(150)).apply { gravity = Gravity.CENTER_HORIZONTAL; topMargin = dp(8) })
            kv(this, "\u25cf Bezug heute", fmt(today, 2, "kWh"))
            kv(this, "\u25cf Monat gesamt", fmt(month, 2, "kWh"))
        }
    }

    private fun systemCard(st: JSONObject): View = card("Systemstatus").apply {
        st.optString("manufacturer").takeIf { it.isNotEmpty() }?.let { kv(this, "Hersteller", it) }
        st.optString("model").takeIf { it.isNotEmpty() }?.let { kv(this, "Modell", it) }
        kv(this, "Alarme", if (st.optBoolean("has_alarms")) st.optString("alarm_list").ifEmpty { "!" } else "Keine")
        st.optString("app_version").takeIf { it.isNotEmpty() }?.let { kv(this, "Firmware", "v$it") }
        if (st.has("uptime_s")) kv(this, "Laufzeit", uptime(st.optLong("uptime_s")))
    }

    private fun uptime(sec: Long): String {
        val d = sec / 86400
        val h = sec % 86400 / 3600
        val m = sec % 3600 / 60
        return if (d > 0) "${d}T ${h}h ${m}min" else "${h}h ${m}min"
    }

    private fun renderStats(currency: String) {
        val data = stats ?: return
        val c = card(null)
        val tabs = LinearLayout(context).apply { orientation = LinearLayout.HORIZONTAL; gravity = Gravity.CENTER_VERTICAL }
        tabs.addView(label("Statistik").apply { setPadding(0, 0, dp(10), 0) })
        listOf("days" to "Tage", "weeks" to "Wochen", "months" to "Monate").forEach { (key, name) ->
            val sel = key == statsView
            tabs.addView(Button(context).apply {
                text = name
                isAllCaps = false
                textSize = 12f
                minHeight = 0
                minimumHeight = 0
                setPadding(dp(12), dp(6), dp(12), dp(6))
                setTextColor(if (sel) Color.WHITE else ink)
                background = panel(if (sel) primary else Color.WHITE, if (sel) primary else Color.parseColor("#c9dcf3"), 8)
                setOnClickListener { statsView = key; render() }
            }, LinearLayout.LayoutParams(ViewGroup.LayoutParams.WRAP_CONTENT, ViewGroup.LayoutParams.WRAP_CONTENT).apply { rightMargin = dp(6) })
        }
        c.addView(HorizontalScrollView(context).apply { addView(tabs) })
        val rows: JSONArray = data.optJSONArray(statsView) ?: JSONArray()
        val list = (0 until rows.length()).mapNotNull { rows.optJSONObject(it) }.reversed()
        val hasPv = list.any { it.optDouble("pv") > 0 || it.optDouble("bc") > 0 || it.optDouble("bd") > 0 }
        val hasWb = list.any { it.optDouble("wb") > 0 }
        val cols = mutableListOf("Zeitraum", "Bezug kWh", "Einspeisung kWh")
        if (hasPv) cols += listOf("PV kWh", "Batterie laden/entladen kWh", "Haus kWh", "Eigenanteil %")
        cols += "Netto $currency"
        if (hasPv) cols += "Ersparnis $currency"
        if (hasWb) cols += listOf("Wallbox kWh", "Wallbox $currency")

        val table = TableLayout(context)
        fun cell(s: String, header: Boolean, first: Boolean) = (if (header) label(s) else text(s, 12.5f, ink, false, true)).apply {
            setPadding(dp(8), dp(7), dp(8), dp(7))
            gravity = if (first) Gravity.START else Gravity.END
        }
        table.addView(TableRow(context).also { r -> cols.forEachIndexed { i, s -> r.addView(cell(s, true, i == 0)) } })
        val kwh = { wh: Double -> String.format(Locale.GERMANY, "%.2f", wh / 1000) }
        val money = { m: Double -> String.format(Locale.GERMANY, "%.2f", m / 1000) }
        for (r in list) {
            val i = r.optDouble("i"); val e = r.optDouble("e"); val pv = r.optDouble("pv")
            val bc = r.optDouble("bc"); val bd = r.optDouble("bd")
            val house = maxOf(0.0, i - e + pv - bc + bd)
            val pvDirect = maxOf(0.0, pv - e - bc)
            val grid = maxOf(0.0, house - pvDirect - bd)
            val own = if (house > 0) Math.round(100 * (house - grid) / house) else 0L
            val cells = mutableListOf(periodLabel(r.optLong("k")) + if (r.has("p")) " *" else "", kwh(i), kwh(e))
            if (hasPv) cells += listOf(kwh(pv), kwh(bc) + " / " + kwh(bd), kwh(house), own.toString())
            cells += money(r.optDouble("c"))
            if (hasPv) cells += money(r.optDouble("s"))
            if (hasWb) cells += listOf(kwh(r.optDouble("wb")), money(r.optDouble("wm")))
            table.addView(TableRow(context).also { tr ->
                cells.forEachIndexed { n, s -> tr.addView(cell(s, false, n == 0)) }
            })
        }
        c.addView(HorizontalScrollView(context).apply { addView(table) })
        c.addView(text("* laufender Zeitraum. Haus = Bezug \u2212 Einspeisung + PV \u2212 Batterie laden + Batterie entladen. Eigenanteil = Anteil des Hausverbrauchs aus PV und Batterie. Ersparnis = PV-Eigenverbrauch \u00d7 Bezugspreis.", 11f, muted).apply { setPadding(0, dp(8), 0, 0) })
        box.addView(c, lp())
    }

    private fun periodLabel(key: Long): String {
        val s = key.toString()
        if (s.length < 6) return s
        if (statsView == "months") return s.substring(4, 6) + "/" + s.substring(0, 4)
        val date = s.substring(6, 8) + "." + s.substring(4, 6) + "." + s.substring(0, 4)
        return if (statsView == "weeks") "KW ab $date" else date
    }
}
