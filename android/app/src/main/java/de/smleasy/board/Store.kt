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
    /** Optionaler Hostname fuer HTTPS; lokale Kopplung benoetigt keinen DNS-Namen. */
    var fqdn: String?
        get() = p.getString("fqdn", null)
        set(v) = p.edit().putString("fqdn", v).apply()
    var httpsPort: Int
        get() = p.getInt("https_port", 443)
        set(v) = p.edit().putInt("https_port", v).apply()
    /** Feste IPv6-Adresse als Rueckfall, wenn der Hostname nicht aufgeloest wird (z. B. DNS-Rebind-Schutz). */
    var host6: String?
        get() = p.getString("host6", null)
        set(v) = p.edit().putString("host6", v).apply()

    var dashboardMode: Boolean
        get() = p.getBoolean("dashboard_mode", false)
        set(v) = p.edit().putBoolean("dashboard_mode", v).apply()

    /** Helligkeit: automatisch per Lichtsensor oder fest in Prozent (1-100). */
    var brightnessAuto: Boolean
        get() = p.getBoolean("bright_auto", true)
        set(v) = p.edit().putBoolean("bright_auto", v).apply()
    var brightnessPct: Int
        get() = p.getInt("bright_pct", 60).coerceIn(1, 100)
        set(v) = p.edit().putInt("bright_pct", v.coerceIn(1, 100)).apply()
    /** Nachtabsenkung: begrenzt die Helligkeit zwischen Beginn und Ende (Minuten ab Mitternacht). */
    var nightEnabled: Boolean
        get() = p.getBoolean("night_on", true)
        set(v) = p.edit().putBoolean("night_on", v).apply()
    var nightPct: Int
        get() = p.getInt("night_pct", 5).coerceIn(1, 100)
        set(v) = p.edit().putInt("night_pct", v.coerceIn(1, 100)).apply()
    var nightFromMin: Int
        get() = p.getInt("night_from", 22 * 60).coerceIn(0, 1439)
        set(v) = p.edit().putInt("night_from", v.coerceIn(0, 1439)).apply()
    var nightToMin: Int
        get() = p.getInt("night_to", 6 * 60).coerceIn(0, 1439)
        set(v) = p.edit().putInt("night_to", v.coerceIn(0, 1439)).apply()

    var fingerprint: String?
        get() = p.getString("fingerprint", null)
        set(v) = p.edit().putString("fingerprint", v).apply()

    fun savePairing(host: String, instance: String, result: PairingResult): Boolean =
        p.edit().putString("host", result.ipv4 ?: host.takeUnless { it.contains(':') })
            .putString("host6", result.ipv6.firstOrNull { it.startsWith("2") || it.startsWith("3") }
                ?: result.ipv6.firstOrNull() ?: host.takeIf { it.contains(':') }?.removeSurrounding("[", "]"))
            .putInt("port", 80)
            .putString("instance", instance).putString("fqdn", result.fqdn)
            .putInt("https_port", result.httpsPort).putString("token", result.token)
            .putString("fingerprint", result.fingerprint).putString("device_id", result.deviceId)
            .remove("pass").commit()

    fun clearPairing() {
        p.edit().remove("token").remove("fingerprint").remove("device_id").remove("fqdn").apply()
    }

    fun baseUrl(): String? = (host ?: host6)?.let { "http://${PairingInfo.urlHost(it)}:$port" }

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
