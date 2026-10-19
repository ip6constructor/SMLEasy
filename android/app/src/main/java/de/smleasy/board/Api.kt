package de.smleasy.board

import android.content.Context
import android.graphics.Bitmap
import android.graphics.BitmapFactory
import android.util.Base64
import org.json.JSONObject
import java.io.File
import java.io.IOException
import java.net.HttpURLConnection
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
 * Zertifikatspruefung bleibt aktiv. Ohne Pairing oder bei HTTPS-Fehlern bleibt HTTP
 * (sichtbar ueber [usesHttps]).
 */
class Api(
    private val host: String?,
    private val port: Int,
    private val fqdn: String?,
    private val httpsPort: Int,
    private val user: String,
    private val pass: String?,
    private val token: String? = null,
) {
    private var base = ""
    private var sniHost: String? = null
    private var resolved = false

    val usesHttps: Boolean get() = base.startsWith("https://")
    val paired: Boolean get() = fqdn != null

    /** Einmalig (oder nach Fehlern erneut) die beste Basisadresse bestimmen. */
    fun ensureBase() {
        if (resolved) return
        val httpBase = host?.let { "http://$it:$port" }
        base = httpBase ?: ""
        sniHost = null
        if (fqdn != null) {
            val suffix = if (httpsPort == 443) "" else ":$httpsPort"
            val candidates = buildList {
                add(Triple("https://$fqdn$suffix", null as String?, fqdn))
                if (host != null) add(Triple("https://$host$suffix", fqdn, fqdn))
            }
            for ((url, sni, name) in candidates) {
                if (probe(url, sni, name)) {
                    base = url
                    sniHost = sni
                    break
                }
            }
        }
        if (base.isEmpty()) throw IOException("Kein Geraet erreichbar")
        resolved = true
    }

    fun invalidate() {
        resolved = false
    }

    private fun probe(url: String, sni: String?, name: String): Boolean = try {
        rawGet("$url/api/status", sni, name, authorize = true)
        true
    } catch (_: AuthRequired) {
        true
    } catch (_: Exception) {
        false
    }

    private fun connect(url: String, sni: String?, name: String?): HttpURLConnection {
        val c = URL(url).openConnection() as HttpURLConnection
        c.connectTimeout = 4000
        c.readTimeout = 6000
        if (c is HttpsURLConnection && sni != null) {
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

    /** Tauscht das Einmal-Geheimnis aus dem QR gegen ein App-Token (HTTPS, ohne Passwort). */
    fun claim(secret: String, appName: String): String {
        val suffix = if (httpsPort == 443) "" else ":$httpsPort"
        val candidates = buildList {
            add(Triple("https://$fqdn$suffix", null as String?, fqdn))
            if (host != null) add(Triple("https://$host$suffix", fqdn, fqdn))
        }
        var last: Exception = IOException("Kein Geraet erreichbar")
        for ((url, sni, name) in candidates) {
            val c = connect("$url/api/pairing/claim", sni, name)
            try {
                c.requestMethod = "POST"
                c.doOutput = true
                c.setRequestProperty("Content-Type", "application/json")
                val body = JSONObject().put("secret", secret).put("name", appName).toString()
                c.outputStream.use { it.write(body.toByteArray()) }
                val code = c.responseCode
                if (code == 403) throw IOException("QR-Code abgelaufen oder ung\u00fcltig")
                if (code == 507) throw IOException("Keine freien App-Pl\u00e4tze (max. 4)")
                if (code != 200) throw IOException("HTTP $code")
                return JSONObject(c.inputStream.bufferedReader().use { it.readText() }).getString("token")
            } catch (e: IOException) {
                last = e
                if (e.message?.startsWith("QR-Code") == true || e.message?.startsWith("Keine freien") == true) throw e
            } finally {
                c.disconnect()
            }
        }
        throw last
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
