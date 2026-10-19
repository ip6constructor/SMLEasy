package de.smleasy.board

import android.content.Context
import android.net.nsd.NsdManager
import android.net.nsd.NsdServiceInfo
import android.net.wifi.WifiManager
import android.os.Handler
import android.os.Looper
import java.net.HttpURLConnection
import java.net.URL
import java.util.concurrent.Executors

data class Device(val instanceName: String, val host: String, val port: Int)

/**
 * Sucht _http._tcp-Dienste und akzeptiert nur Geraete, deren /api/status als
 * SMLEasy erkennbar ist (JSON oder Basic-Realm "EasySML"). Der Instanzname
 * stammt vom im Geraet konfigurierten Namen. Ist ein Name gespeichert, wird
 * nur dieses Geraet akzeptiert.
 */
class Discovery(
    context: Context,
    private val preferredName: String?,
    private val onFound: (Device) -> Unit,
) {
    private val nsd = context.getSystemService(Context.NSD_SERVICE) as NsdManager
    private val wifi = context.applicationContext.getSystemService(Context.WIFI_SERVICE) as WifiManager
    private val main = Handler(Looper.getMainLooper())
    private val io = Executors.newSingleThreadExecutor()
    private var multicastLock: WifiManager.MulticastLock? = null
    private var listener: NsdManager.DiscoveryListener? = null
    private var done = true
    private val pending = ArrayDeque<NsdServiceInfo>()
    private var resolving = false

    fun start() {
        stop()
        done = false
        multicastLock = wifi.createMulticastLock("smleasy-mdns").apply {
            setReferenceCounted(false)
            acquire()
        }
        val l = object : NsdManager.DiscoveryListener {
            override fun onDiscoveryStarted(serviceType: String) {}
            override fun onDiscoveryStopped(serviceType: String) {}
            override fun onStartDiscoveryFailed(serviceType: String, errorCode: Int) {}
            override fun onStopDiscoveryFailed(serviceType: String, errorCode: Int) {}
            override fun onServiceLost(info: NsdServiceInfo) {}
            override fun onServiceFound(info: NsdServiceInfo) {
                main.post {
                    if (done) return@post
                    if (preferredName != null && info.serviceName != preferredName) return@post
                    pending.addLast(info)
                    resolveNext()
                }
            }
        }
        listener = l
        try {
            nsd.discoverServices("_http._tcp.", NsdManager.PROTOCOL_DNS_SD, l)
        } catch (_: Exception) {
            listener = null
        }
    }

    fun stop() {
        done = true
        pending.clear()
        resolving = false
        listener?.let { runCatching { nsd.stopServiceDiscovery(it) } }
        listener = null
        multicastLock?.let { if (it.isHeld) it.release() }
        multicastLock = null
    }

    // Vor API 34 loest NsdManager nur einen Dienst gleichzeitig auf.
    private fun resolveNext() {
        if (resolving || done) return
        val info = pending.removeFirstOrNull() ?: return
        resolving = true
        @Suppress("DEPRECATION")
        nsd.resolveService(info, object : NsdManager.ResolveListener {
            override fun onResolveFailed(serviceInfo: NsdServiceInfo, errorCode: Int) {
                main.post { resolving = false; resolveNext() }
            }

            @Suppress("DEPRECATION")
            override fun onServiceResolved(serviceInfo: NsdServiceInfo) {
                val addr = serviceInfo.host?.hostAddress
                val port = serviceInfo.port
                val name = serviceInfo.serviceName
                main.post { resolving = false; resolveNext() }
                if (addr == null) return
                val host = if (addr.contains(':')) "[${addr.substringBefore('%')}]" else addr
                io.execute {
                    if (done || !isSmlEasy(host, port)) return@execute
                    main.post {
                        if (!done) {
                            stop()
                            onFound(Device(name, host, port))
                        }
                    }
                }
            }
        })
    }

    private fun isSmlEasy(host: String, port: Int): Boolean = try {
        val c = URL("http://$host:$port/api/status").openConnection() as HttpURLConnection
        c.connectTimeout = 3000
        c.readTimeout = 3000
        try {
            when (c.responseCode) {
                200 -> c.inputStream.bufferedReader().use { it.readText().trimStart().startsWith("{") }
                401 -> c.getHeaderField("WWW-Authenticate")?.contains("EasySML") == true
                else -> false
            }
        } finally {
            c.disconnect()
        }
    } catch (_: Exception) {
        false
    }
}
