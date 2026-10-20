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

data class Device(val instanceName: String, val host: String, val port: Int, val hosts: List<String> = listOf(host))

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
    private val pairing: Boolean = false,
    private val onError: (String) -> Unit = {},
) {
    private val nsd = context.getSystemService(Context.NSD_SERVICE) as NsdManager
    private val wifi = context.applicationContext.getSystemService(Context.WIFI_SERVICE) as WifiManager
    private val main = Handler(Looper.getMainLooper())
    private val io = Executors.newSingleThreadExecutor()
    private var multicastLock: WifiManager.MulticastLock? = null
    private val listeners = mutableListOf<NsdManager.DiscoveryListener>()
    @Volatile private var done = true
    private val pending = ArrayDeque<NsdServiceInfo>()
    private var resolving = false
    @Volatile private var foundCount = 0
    @Volatile private var resolveFailures = 0
    @Volatile private var lastCode: Int? = null

    /** Kurzdiagnose fuer die Fehlersuche: was NSD gemeldet hat. */
    fun summary(): String = "NSD: $foundCount Dienste gefunden, $resolveFailures Aufloesungsfehler, Code ${lastCode ?: "-"}"

    fun start() {
        stop()
        done = false
        multicastLock = wifi.createMulticastLock("smleasy-mdns").apply {
            setReferenceCounted(false)
            acquire()
        }
        // Im Koppelmodus zusaetzlich _http: findet das Geraet auch, wenn der Pairing-Dienst nicht aufgeloest wird.
        val types = if (pairing) listOf("_smleasy-pair._tcp.", "_http._tcp.") else listOf("_http._tcp.")
        for (type in types) {
            val l = object : NsdManager.DiscoveryListener {
                override fun onDiscoveryStarted(serviceType: String) {}
                override fun onDiscoveryStopped(serviceType: String) {}
                override fun onStartDiscoveryFailed(serviceType: String, errorCode: Int) {
                    android.util.Log.w("Discovery", "Start $serviceType failed: $errorCode")
                    lastCode = errorCode
                    if (type == types.first()) main.post { if (!done) onError("Netzwerksuche fehlgeschlagen ($errorCode)") }
                }
                override fun onStopDiscoveryFailed(serviceType: String, errorCode: Int) {
                    android.util.Log.w("Discovery", "Stopping discovery failed: $errorCode")
                }
                override fun onServiceLost(info: NsdServiceInfo) {}
                override fun onServiceFound(info: NsdServiceInfo) {
                    foundCount++
                    main.post {
                        if (done) return@post
                        if (preferredName != null && info.serviceName != preferredName) return@post
                        pending.addLast(info)
                        resolveNext()
                    }
                }
            }
            try {
                nsd.discoverServices(type, NsdManager.PROTOCOL_DNS_SD, l)
                listeners.add(l)
            } catch (e: RuntimeException) {
                onError("Netzwerksuche nicht verfuegbar: ${e.message}")
            }
        }
    }

    fun release() {
        stop()
        io.shutdownNow()
    }

    fun stop() {
        done = true
        pending.clear()
        resolving = false
        listeners.forEach { l -> runCatching { nsd.stopServiceDiscovery(l) } }
        listeners.clear()
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
                resolveFailures++
                lastCode = errorCode
                main.post { resolving = false; resolveNext() }
            }

            @Suppress("DEPRECATION")
            override fun onServiceResolved(serviceInfo: NsdServiceInfo) {
                val addresses = if (android.os.Build.VERSION.SDK_INT >= 34) serviceInfo.hostAddresses
                    else listOfNotNull(serviceInfo.host)
                val hosts = addresses.sortedWith(compareBy<java.net.InetAddress>(
                    { if (it is java.net.Inet6Address) 0 else 1 },
                    { if (it.isLinkLocalAddress) 1 else 0 },
                )).mapNotNull { it.hostAddress }.map { PairingInfo.urlHost(it) }.distinct()
                val port = serviceInfo.port
                val name = serviceInfo.serviceName
                val pairService = serviceInfo.serviceType?.contains("smleasy-pair") == true
                main.post { resolving = false; resolveNext() }
                if (done) return
                io.execute {
                    if (done) return@execute
                    val host = when {
                        !pairing -> hosts.firstOrNull { isSmlEasy(it, port) }
                        pairService -> if (port == 7384) hosts.firstOrNull() else null
                        else -> hosts.firstOrNull { canConnect(it, 7384) }
                    }
                    if (host == null) return@execute
                    main.post {
                        if (!done) {
                            stop()
                            onFound(Device(name, host, if (pairing) 7384 else port, hosts))
                        }
                    }
                }
            }
        })
    }

    private fun canConnect(host: String, port: Int): Boolean = try {
        java.net.Socket().use { it.connect(java.net.InetSocketAddress(host.removeSurrounding("[", "]"), port), 1500) }
        true
    } catch (_: Exception) {
        false
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
