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

    private fun apiFor(host: String?): Api {
        val key = listOf(host, store.port, store.fqdn, store.httpsPort, store.user, store.password, store.token)
        apiCache?.let { if (it.first == key) return it.second }
        return Api(host, store.port, store.fqdn, store.httpsPort, store.user, store.password, store.token)
            .also { apiCache = Pair(key, it) }
    }

    private fun poll(g: Int) {
        val host = store.host
        if (host == null && store.fqdn == null) {
            listener.onLost()
            return
        }
        val api = apiFor(host)
        val key = store.instanceName ?: store.fqdn ?: host ?: "device"
        io.execute {
            var err: String? = null
            var auth = false
            var tiles: Map<String, TileData>? = null
            var image: Bitmap? = null
            try {
                val st = JSONObject(api.getJson("/api/status"))
                val ext = runCatching { JSONObject(api.getJson("/api/ext/values")) }.getOrNull()
                tiles = BoardModel.build(st, ext)
                image = api.cachedHouse(context, key)
            } catch (e: AuthRequired) {
                auth = true
            } catch (e: Exception) {
                err = e.message ?: e.javaClass.simpleName
            }
            main.post {
                if (g != generation) return@post
                if (tiles != null) {
                    failures = 0
                    listener.onStatus(
                        if (api.paired && !api.usesHttps) "HTTPS nicht erreichbar – unverschl\u00fcsseltes HTTP aktiv" else null,
                    )
                    image?.let { board.setImage(it) }
                    board.setData(tiles)
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
                main.postDelayed({ if (g == generation) poll(g) }, if (failures > 0) 10_000 else 5_000)
            }
        }
    }
}
