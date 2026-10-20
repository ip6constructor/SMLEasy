package de.smleasy.board

import android.content.Context
import android.graphics.Bitmap
import android.os.Handler
import android.os.Looper
import org.json.JSONObject
import java.util.concurrent.Executors

/** Fragt das gespeicherte Geraet zyklisch ab und fuellt das Board (Activity und Bildschirmschoner). */
class BoardController(
    private val context: Context,
    private val store: Store,
    private val board: BoardView,
    private val listener: Listener,
    private val dashboard: DashboardView? = null,
    private val dashboardActive: () -> Boolean = { false },
) {
    interface Listener {
        fun onStatus(msg: String?)
        fun onAuthRequired()
        /** Kein erreichbares Geraet mehr bekannt: neu suchen. */
        fun onLost()
    }

    private val main = Handler(Looper.getMainLooper())
    private val io = Executors.newSingleThreadExecutor()
    private var generation = 0
    private var failures = 0
    private var pausedUntil = 0L
    private var lastStatsMs = 0L
    private var apiCache: Pair<List<Any?>, Api>? = null

    fun start() {
        failures = 0
        val g = ++generation
        poll(g)
    }

    fun stop() {
        generation++
    }

    fun release() {
        stop()
        io.shutdownNow()
    }

    /** Neustart im Hintergrund; Ergebnis kommt auf dem Hauptthread. */
    fun rebootDevice(done: (String) -> Unit) {
        val api = apiFor(store.host)
        Thread {
            val msg = try {
                api.reboot()
                "Geraet startet neu ..."
            } catch (_: AuthRequired) {
                "Neustart braucht die Passwort-Anmeldung (Menue: Zugangsdaten)"
            } catch (e: Exception) {
                "Neustart fehlgeschlagen: ${e.message}"
            }
            main.post { done(msg) }
        }.start()
    }

    private fun apiFor(host: String?): Api {
        val key = listOf(host, store.port, store.fqdn, store.httpsPort, store.user, store.password, store.token, store.host6, store.fingerprint)
        apiCache?.let { if (it.first == key) return it.second }
        return Api(host, store.port, store.fqdn, store.httpsPort, store.user, store.password, store.token, store.host6, store.fingerprint)
            .also { apiCache = Pair(key, it) }
    }

    private fun poll(g: Int) {
        val host = store.host
        if (host == null && store.host6 == null && store.fqdn == null) {
            listener.onLost()
            return
        }
        val api = apiFor(host)
        val key = store.instanceName ?: store.fqdn ?: host ?: "device"
        io.execute {
            var err: String? = null
            var auth = false
            var pauseS = 0
            var tiles: Map<String, TileData>? = null
            var kpis: List<BoardKpi>? = null
            var image: Bitmap? = null
            var st0: JSONObject? = null
            var ext0: JSONObject? = null
            var stats0: JSONObject? = null
            try {
                val st = JSONObject(api.getJson("/api/status"))
                pauseS = st.optInt("pause_s", 0)
                if (pauseS == 0) {
                    val ext = runCatching { JSONObject(api.getJson("/api/ext/values")) }.getOrNull()
                    st0 = st; ext0 = ext
                    if (dashboard != null && dashboardActive() && System.currentTimeMillis() - lastStatsMs > 60_000) {
                        stats0 = runCatching { JSONObject(api.getJson("/api/stats")) }.getOrNull()
                        if (stats0 != null) lastStatsMs = System.currentTimeMillis()
                    }
                    tiles = BoardModel.build(st, ext)
                    kpis = BoardModel.kpis(st)
                    image = api.cachedHouse(context, key)
                }
            } catch (e: AuthRequired) {
                auth = true
            } catch (e: Exception) {
                err = e.message ?: e.javaClass.simpleName
            }
            main.post {
                if (g != generation) return@post
                if (pauseS > 0) {
                    pausedUntil = System.currentTimeMillis() + pauseS * 1000L
                } else if (tiles != null) {
                    pausedUntil = 0
                }
                val paused = System.currentTimeMillis() < pausedUntil
                if (paused) {
                    failures = 0
                    listener.onStatus("Das Ger\u00e4t pausiert kurz (Zertifikatspr\u00fcfung) und ist gleich wieder da.")
                } else if (tiles != null) {
                    failures = 0
                    listener.onStatus(
                        if (api.paired && !api.usesHttps) "HTTPS nicht erreichbar â€“ unverschl\u00fcsseltes HTTP aktiv" else null,
                    )
                    image?.let { board.setImage(it) }
                    board.setData(tiles)
                    if (dashboard != null && dashboardActive()) dashboard.setData(st0, ext0, stats0)
                    kpis?.let { board.setKpis(it) }
                } else if (auth) {
                    listener.onStatus("Anmeldung erforderlich")
                    listener.onAuthRequired()
                } else {
                    failures++
                    listener.onStatus("Keine Verbindung ($err). Neuer Versuch ...")
                    // Gespeicherter Name, neue IP: erneut per mDNS suchen.
                    if (failures >= 3 && store.instanceName != null && store.host != null) {
                        store.host = null
                        listener.onLost()
                        return@post
                    }
                }
                main.postDelayed({ if (g == generation) poll(g) }, if (paused) 10_000 else if (failures > 0) 10_000 else 5_000)
            }
        }
    }
}
