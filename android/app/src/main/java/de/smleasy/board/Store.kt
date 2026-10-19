package de.smleasy.board

import android.content.Context
import org.json.JSONObject

data class TileLayout(val x: Float, val y: Float, val scale: Float)

/** Persistente App-Einstellungen: Geraet, Zugangsdaten und Kachel-Layout (Prozent-Position, Skalierung). */
class Store(context: Context) {
    private val p = context.getSharedPreferences("board", Context.MODE_PRIVATE)

    var instanceName: String?
        get() = p.getString("instance", null)
        set(v) = p.edit().putString("instance", v).apply()
    var host: String?
        get() = p.getString("host", null)
        set(v) = p.edit().putString("host", v).apply()
    var port: Int
        get() = p.getInt("port", 80)
        set(v) = p.edit().putInt("port", v).apply()
    var user: String
        get() = p.getString("user", "admin") ?: "admin"
        set(v) = p.edit().putString("user", v).apply()
    var password: String?
        get() = p.getString("pass", null)
        set(v) = p.edit().putString("pass", v).apply()
    /** Per Pairing erhaltenes App-Token (ersetzt das Web-Passwort). */
    var token: String?
        get() = p.getString("token", null)
        set(v) = p.edit().putString("token", v).apply()
    /** Aus dem Pairing-QR bzw. manuell: Hostname fuer HTTPS. */
    var fqdn: String?
        get() = p.getString("fqdn", null)
        set(v) = p.edit().putString("fqdn", v).apply()
    var httpsPort: Int
        get() = p.getInt("https_port", 443)
        set(v) = p.edit().putInt("https_port", v).apply()

    fun baseUrl(): String? = host?.let { "http://$it:$port" }

    fun layout(key: String): TileLayout {
        val d = DEFAULTS[key] ?: floatArrayOf(50f, 50f)
        val o = runCatching { JSONObject(p.getString("layout", "{}")!!).optJSONObject(key) }.getOrNull()
            ?: return TileLayout(d[0], d[1], 1f)
        return TileLayout(
            o.optDouble("x", d[0].toDouble()).toFloat(),
            o.optDouble("y", d[1].toDouble()).toFloat(),
            o.optDouble("s", 1.0).toFloat().coerceIn(MIN_SCALE, MAX_SCALE),
        )
    }

    fun saveLayout(key: String, l: TileLayout) {
        val all = runCatching { JSONObject(p.getString("layout", "{}")!!) }.getOrDefault(JSONObject())
        all.put(key, JSONObject().put("x", l.x.toDouble()).put("y", l.y.toDouble()).put("s", l.scale.toDouble()))
        p.edit().putString("layout", all.toString()).apply()
    }

    fun resetLayout() = p.edit().remove("layout").apply()

    companion object {
        const val MIN_SCALE = 0.5f
        const val MAX_SCALE = 2.5f

        // Gleiche Standardpositionen wie EH_POS_DEFAULT im Web-Dashboard.
        val DEFAULTS = mapOf(
            "pvo" to floatArrayOf(18.5f, 7.3f), "pvs" to floatArrayOf(43.6f, 7.3f),
            "pv" to floatArrayOf(65.8f, 8.3f), "pvw" to floatArrayOf(90.6f, 25.9f),
            "house" to floatArrayOf(13f, 43.4f), "bat" to floatArrayOf(13.9f, 68.4f),
            "grid" to floatArrayOf(51.9f, 60.7f), "car" to floatArrayOf(70.6f, 45f),
            "wb" to floatArrayOf(91f, 43.8f),
        )
    }
}
