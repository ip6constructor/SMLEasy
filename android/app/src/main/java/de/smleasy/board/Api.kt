package de.smleasy.board

import android.content.Context
import android.graphics.Bitmap
import android.graphics.BitmapFactory
import android.util.Base64
import org.json.JSONObject
import java.io.File
import java.io.IOException
import java.net.HttpURLConnection
import java.net.Inet6Address
import java.net.InetAddress
import java.net.UnknownHostException
import java.net.Socket
import java.net.URL
import javax.net.ssl.HttpsURLConnection
import javax.net.ssl.SSLSocketFactory

class AuthRequired : Exception()

/**
 * REST-Client fuer die Geraete-Endpunkte (nur GET).
 *
 * Der FQDN stammt aus dem Pairing (QR oder manuelle Eingabe), nicht aus einer Abfrage am
 * Geraet. Ist er gesetzt, wird https://FQDN verwendet. Loest er im LAN nicht auf, verbindet
 * die App zur mDNS-IP, setzt aber SNI und Zertifikatspruefung auf den FQDN; die normale
 * Zertifikatspruefung bleibt aktiv. Neue Kopplungen pruefen stattdessen den bestaetigten
 * Fingerprint. Mit Token/Pin gibt es niemals einen HTTP-Fallback.
 */
class Api(
    private val host: String?,
    private val port: Int,
    private val fqdn: String?,
    private val httpsPort: Int,
    private val user: String,
    private val pass: String?,
    private val token: String? = null,
    private val host6: String? = null,
    private val fingerprint: String? = null,
) {
    private var base = ""
    private var sniHost: String? = null
    private var resolved = false
    private var lastConnectionError: IOException? = null

    val usesHttps: Boolean get() = base.startsWith("https://")
    val paired: Boolean get() = fqdn != null || fingerprint != null

    /** Hostname (inkl. AAAA), danach direkte IPv6, zuletzt IPv4; niemals HTTP mit App-Token. */
    private fun httpsCandidates(): List<Triple<String, String?, String?>> {
        val suffix = if (httpsPort == 443) "" else ":$httpsPort"
        val dns = if (fqdn == null) emptyList() else try {
            InetAddress.getAllByName(fqdn).toList()
        } catch (e: UnknownHostException) {
            lastConnectionError = e
            emptyList()
        }
        return buildList {
            for (address in dns.filterIsInstance<Inet6Address>()) {
                add(Triple("https://${PairingInfo.urlHost(address.hostAddress ?: continue)}$suffix", fqdn, fqdn))
            }
            if (!host6.isNullOrBlank() && paired) add(Triple("https://${PairingInfo.urlHost(host6)}$suffix", fqdn, fqdn))
            for (address in dns.filterNot { it is Inet6Address }) {
                add(Triple("https://${PairingInfo.urlHost(address.hostAddress ?: continue)}$suffix", fqdn, fqdn))
            }
            if (host != null && paired) add(Triple("https://${PairingInfo.urlHost(host)}$suffix", fqdn, fqdn))
        }.distinct()
    }

    /** Einmalig (oder nach Fehlern erneut) die beste Basisadresse bestimmen. */
    fun ensureBase() {
        if (resolved) return
        lastConnectionError = null
        val httpBase = (host6 ?: host)?.let { "http://${PairingInfo.urlHost(it)}:$port" }
        base = if (token == null && fingerprint == null) httpBase ?: "" else ""
        sniHost = null
        if (paired) {
            for ((url, sni, name) in httpsCandidates()) {
                if (probe(url, sni, name)) {
                    base = url
                    sniHost = sni
                    break
                }
            }
        }
        if (base.isEmpty()) throw IOException(
            "HTTPS nicht erreichbar: ${lastConnectionError?.message ?: "keine Adresse"}; Zertifikat/IPv6-Route pruefen",
            lastConnectionError,
        )
        resolved = true
    }

    fun invalidate() {
        resolved = false
    }

    private fun probe(url: String, sni: String?, name: String?): Boolean = try {
        rawGet("$url/api/status", sni, name, authorize = true)
        true
    } catch (_: AuthRequired) {
        true
    } catch (e: IOException) {
        lastConnectionError = e
        false
    }

    private fun connect(url: String, sni: String?, name: String?): HttpURLConnection {
        val c = URL(url).openConnection() as HttpURLConnection
        c.connectTimeout = 4000
        c.readTimeout = 6000
        c.instanceFollowRedirects = false
        if (c is HttpsURLConnection && fingerprint != null) {
            val factory = DeviceTls.context(fingerprint).socketFactory
            c.sslSocketFactory = if (sni != null) SniFactory(factory, sni) else factory
            // Authentication is by the pinned certificate rather than by DNS/IP.
            c.hostnameVerifier = javax.net.ssl.HostnameVerifier { _, _ -> true }
        } else if (c is HttpsURLConnection && sni != null) {
            val default = HttpsURLConnection.getDefaultSSLSocketFactory()
            c.sslSocketFactory = SniFactory(default, sni)
            val verifier = HttpsURLConnection.getDefaultHostnameVerifier()
            c.hostnameVerifier = javax.net.ssl.HostnameVerifier { _, session -> verifier.verify(name ?: sni, session) }
        }
        return c
    }

    private fun applyAuth(c: HttpURLConnection) {
        if (token != null) {
            c.setRequestProperty("Authorization", "Bearer $token")
        } else if (pass != null) {
            val b = Base64.encodeToString("$user:$pass".toByteArray(), Base64.NO_WRAP)
            c.setRequestProperty("Authorization", "Basic $b")
        }
    }

    private fun rawGet(url: String, sni: String?, name: String?, authorize: Boolean): String {
        val c = connect(url, sni, name)
        try {
            if (authorize) applyAuth(c)
            val code = c.responseCode
            if (code == 401 || (code == 403 && token != null)) throw AuthRequired()
            if (code != 200) throw IOException("HTTP $code")
            return c.inputStream.bufferedReader().use { it.readText() }
        } finally {
            c.disconnect()
        }
    }

    fun getJson(path: String): String {
        ensureBase()
        try {
            return rawGet(base + path, sniHost, sniHost, authorize = true)
        } catch (e: AuthRequired) {
            throw e
        } catch (e: IOException) {
            invalidate()
            throw e
        }
    }

    /** Loest einen Geraete-Neustart aus; benoetigt die Passwort-Anmeldung (App-Token sind nur lesend). */
    fun reboot() {
        if (pass == null) throw AuthRequired()
        ensureBase()
        val c = connect(base + "/api/reboot", sniHost, sniHost)
        try {
            c.requestMethod = "POST"
            c.doOutput = true
            c.setFixedLengthStreamingMode(0)
            val b = Base64.encodeToString("$user:$pass".toByteArray(), Base64.NO_WRAP)
            c.setRequestProperty("Authorization", "Basic $b")
            val code = c.responseCode
            if (code == 401 || code == 403) throw AuthRequired()
            if (code != 200) throw IOException("HTTP $code")
        } catch (e: IOException) {
            invalidate()
            throw e
        } finally {
            c.disconnect()
        }
    }

    /**
     * Laedt /house.jpg in den App-Cache (je Geraet). Bei Netzfehlern bleibt die alte
     * Datei erhalten; leert Android den Cache, wird sie beim naechsten Abruf neu geladen.
     */
    fun cachedHouse(context: Context, deviceKey: String, maxAgeMs: Long = 24L * 3600 * 1000): Bitmap? {
        val f = File(context.cacheDir, "house-" + deviceKey.replace(Regex("[^A-Za-z0-9._-]"), "_") + ".jpg")
        val fresh = f.exists() && System.currentTimeMillis() - f.lastModified() < maxAgeMs
        if (!fresh) {
            try {
                ensureBase()
                val c = connect(base + "/house.jpg", sniHost, sniHost)
                try {
                    applyAuth(c)
                    if (c.responseCode == 401) throw AuthRequired()
                    if (c.responseCode == 200) {
                        val tmp = File(f.path + ".tmp")
                        c.inputStream.use { i -> tmp.outputStream().use { o -> i.copyTo(o) } }
                        if (BitmapFactory.decodeFile(tmp.path) != null) {
                            f.delete()
                            tmp.renameTo(f)
                        } else {
                            tmp.delete()
                        }
                    }
                } finally {
                    c.disconnect()
                }
            } catch (e: AuthRequired) {
                throw e
            } catch (_: Exception) {
            }
        }
        return if (f.exists()) BitmapFactory.decodeFile(f.path) else null
    }

    /** Erzeugt TLS-Sockets mit dem FQDN als SNI-Namen, auch wenn zur IP verbunden wird. */
    private class SniFactory(private val d: SSLSocketFactory, private val sni: String) : SSLSocketFactory() {
        override fun getDefaultCipherSuites(): Array<String> = d.defaultCipherSuites
        override fun getSupportedCipherSuites(): Array<String> = d.supportedCipherSuites
        override fun createSocket(s: Socket, host: String, port: Int, autoClose: Boolean): Socket =
            d.createSocket(s, sni, port, autoClose)
        override fun createSocket(host: String, port: Int): Socket = d.createSocket(sni, port)
        override fun createSocket(host: String, port: Int, local: java.net.InetAddress, lp: Int): Socket =
            d.createSocket(sni, port, local, lp)
        override fun createSocket(host: java.net.InetAddress, port: Int): Socket = d.createSocket(host, port)
        override fun createSocket(a: java.net.InetAddress, port: Int, l: java.net.InetAddress, lp: Int): Socket =
            d.createSocket(a, port, l, lp)
    }
}
