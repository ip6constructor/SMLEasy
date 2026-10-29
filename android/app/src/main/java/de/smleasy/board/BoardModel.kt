package de.smleasy.board

import android.graphics.Color
import org.json.JSONObject
import java.util.Locale
import kotlin.math.abs
import kotlin.math.max
import kotlin.math.min
import kotlin.math.roundToInt

data class TileData(
    val title: String,
    val value: String,
    val color: Int,
    val rows: List<Pair<String, String>> = emptyList(),
)

data class BoardKpi(val title: String, val value: String, val icon: String, val color: Int)

/** Bildet /api/status und /api/ext/values auf dieselben Kacheln ab wie ehRefresh() im Web-Dashboard (vereinfacht, ohne Balkengrafiken). */
object BoardModel {
    private val AMBER = Color.parseColor("#fbbf24")
    private val BLUE = Color.parseColor("#60a5fa")
    private val RED = Color.parseColor("#ef4444")
    private val GREEN = Color.parseColor("#22c55e")
    private val PURPLE = Color.parseColor("#a855f7")
    private val TEAL = Color.parseColor("#2dd4bf")
    private val GRAY = Color.parseColor("#94a3b8")
    private val ORANGE = Color.parseColor("#f59e0b")

    private fun f(v: Double, d: Int) = String.format(Locale.GERMANY, "%.${d}f", v)
    private fun kw(w: Double) = f(abs(w) / 1000.0, 2) + " kW"
    private val dash = "\u2014"

    private class Item(val label: String, val unit: String, val role: String, val value: Double?, val text: String?)

    fun kpis(status: JSONObject): List<BoardKpi> {
        fun number(key: String): Double? = (status.opt(key) as? Number)?.toDouble()?.takeIf { it.isFinite() }
        fun value(key: String, digits: Int, unit: String) = number(key)?.let { f(it, digits) + " " + unit } ?: dash
        // Autarkiegrad (Anteil des Hausverbrauchs ohne Netzbezug), nicht PV-Eigenverbrauchsquote.
        val house = number("today_house_kwh")
        val gridImport = number("today_import_kwh")
        val share = if (house != null && gridImport != null && house > 0.05)
            f(((house - gridImport) / house * 100).coerceIn(0.0, 100.0), 0) + " %" else dash
        val currency = status.optString("currency", "EUR")
        return listOf(
            BoardKpi("Eigenverbrauch", share, "\u25c7", GREEN),
            BoardKpi("Gesamterzeugung", value("today_pv_kwh", 1, "kWh"), "\u2600", AMBER),
            BoardKpi("Netzkosten heute", value("cost_today", 2, currency), "\u26a1", BLUE),
            BoardKpi("Ersparnis heute", value("today_saving", 2, currency), "\u20ac", GREEN),
        )
    }

    fun build(status: JSONObject, ext: JSONObject?): Map<String, TileData> {
        val items = mutableListOf<Item>()
        ext?.optJSONArray("items")?.let { a ->
            for (i in 0 until a.length()) {
                val o = a.optJSONObject(i) ?: continue
                items += Item(
                    o.optString("label"), o.optString("unit"), o.optString("role"),
                    if (o.has("value") && !o.isNull("value") && o.opt("value") is Number) o.optDouble("value") else null,
                    o.optString("text").ifEmpty { null },
                )
            }
        }
        fun watts(it: Item?): Double? = it?.value?.let { v -> if (it.unit == "kW") v * 1000 else v }
        fun role(r: String) = items.firstOrNull { it.role == r }
        fun power(it: Item) = it.unit.matches(Regex("(?i)^k?w$"))
        fun area(re: Regex) = watts(items.firstOrNull { power(it) && re.containsMatchIn(it.label) })

        val pvWest = watts(role("pv_west")) ?: area(Regex("(?i)west"))
        val pvEast = watts(role("pv_east")) ?: area(Regex("(?i)\\bost\\b|east"))
        val pvSouth = watts(role("pv_south")) ?: area(Regex("(?i)s\u00fcd|sued|south"))
        var pvW = watts(role("pv"))
        if (pvW == null && (pvWest != null || pvEast != null || pvSouth != null)) {
            pvW = (pvWest ?: 0.0) + (pvEast ?: 0.0) + (pvSouth ?: 0.0)
        }
        var batW = watts(role("battery"))
        if (batW == null) role("battery_neg")?.let { n -> watts(n)?.let { batW = -it } }
        val carRe = Regex("(?i)auto|fahrzeug|\\bcar\\b|\\bev\\b")
        val soc = items.firstOrNull { it.unit == "%" && !carRe.containsMatchIn(it.label) && it.value != null }
        val car = items.firstOrNull { it.unit == "%" && carRe.containsMatchIn(it.label) && it.value != null }
        val wbItem = role("wb_power")
            ?: items.firstOrNull { Regex("(?i)wallbox|keba|\\bwb\\b").containsMatchIn(it.label) && power(it) }
        val wbW = watts(wbItem)

        val gridW = status.optDouble("fwd_w", 0.0) - status.optDouble("rev_w", 0.0)
        val houseW = (pvW ?: 0.0) - (batW ?: 0.0) + gridW - (wbW ?: 0.0)
        fun num(k: String): Double? = if (status.has(k) && status.opt(k) is Number) status.optDouble(k) else null
        fun kwh(k: String, d: Int = 1, fallback: String = "?") = num(k)?.let { f(it, d) + " kWh" } ?: fallback

        val pkE = status.optDouble("pv_peak_east_w", 0.0)
        val pkS = status.optDouble("pv_peak_south_w", 0.0)
        val pkW = status.optDouble("pv_peak_west_w", 0.0)
        fun pvRows(w: Double?, peak: Double) =
            if (peak > 0) listOf(
                (if (w == null) dash else "${max(0.0, min(100.0, w / peak * 100)).roundToInt()} %") to f(peak / 1000, 1) + " kWp",
            ) else emptyList()

        val t = LinkedHashMap<String, TileData>()
        t["pv"] = TileData(
            "PV", pvW?.let { kw(it) } ?: dash, AMBER,
            pvRows(pvW, pkE + pkS + pkW) + listOf("Heute" to kwh("today_pv_kwh"), "Monat" to kwh("month_pv_kwh", 0)),
        )
        t["pvw"] = TileData("PV West", pvWest?.let { kw(it) } ?: dash, AMBER, pvRows(pvWest, pkW))
        t["pvs"] = TileData("PV S\u00fcd", pvSouth?.let { kw(it) } ?: dash, AMBER, pvRows(pvSouth, pkS))
        t["pvo"] = TileData("PV Ost", pvEast?.let { kw(it) } ?: dash, AMBER, pvRows(pvEast, pkE))
        t["house"] = TileData(
            "Hausverbrauch", kwh("today_house_kwh", 1, dash), if (gridW > 0) BLUE else AMBER,
            listOf("Leistung" to "${max(0.0, houseW).roundToInt()} W", "Monat" to kwh("month_house_kwh", 0)),
        )
        t["grid"] = TileData(
            "Stromnetz", "${abs(gridW).roundToInt()} W", RED,
            listOf(
                "Bezug (Heute)" to kwh("today_import_kwh", 2, "0,00 kWh"),
                "Einspeisung (Heute)" to kwh("today_export_kwh", 2, "0,00 kWh"),
            ),
        )
        val pct = soc?.value?.let { max(0.0, min(100.0, it)) }
        t["bat"] = TileData(
            "Batterie", pct?.let { "${it.roundToInt()} %" } ?: (batW?.let { kw(it) } ?: dash),
            when { pct == null -> GRAY; pct < 40 -> RED; pct > 60 -> GREEN; else -> ORANGE },
            listOf("Geladen (Heute)" to kwh("today_bat_chg_kwh"), "Entladen (Heute)" to kwh("today_bat_dis_kwh")),
        )
        val carName = role("car_name")?.text
            ?: car?.label?.replace(Regex("(?i)\\s*(ladestand|ladezustand|soc|akku|batterie|state of charge)\\s*"), " ")?.trim()
                ?.ifEmpty { null }
            ?: "E-Auto"
        val sess = role("wb_session_kwh")
            ?: items.firstOrNull { Regex("(?i)ladevorgang|sitzung|session").containsMatchIn(it.label) && it.unit.matches(Regex("(?i)^k?wh$")) }
        val mode = role("wb_mode")?.text.orEmpty()
        val modeLabel = when {
            Regex("(?i)fast|schnell").containsMatchIn(mode) -> "Fast"
            Regex("(?i)both|beide|mix").containsMatchIn(mode) -> "Both"
            Regex("(?i)pv").containsMatchIn(mode) -> "PV-Only"
            else -> mode
        }
        t["car"] = TileData(
            carName, car?.value?.let { "${it.roundToInt()} %" } ?: dash, PURPLE,
            listOf(
                "Leistung" to (wbW?.let { kw(it) } ?: dash) + " / 11 kW",
                "Modus" to modeLabel.ifEmpty { "?" },
                "Ladevorgang" to (sess?.value?.let { f(it, 1) + " kWh" } ?: "?"),
                "Gesamt" to (role("wb_kwh")?.value?.let { f(it, 0) + " kWh" } ?: "?"),
            ),
        )
        t["wb"] = TileData(
            "Wallbox", wbW?.let { kw(it) } ?: dash, TEAL,
            listOf("Heute" to kwh("today_wb_kwh"), "Woche" to kwh("week_wb_kwh"), "Monat" to kwh("month_wb_kwh", 0)),
        )
        return t
    }
}
