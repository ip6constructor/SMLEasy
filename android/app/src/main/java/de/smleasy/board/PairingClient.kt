package de.smleasy.board

import org.json.JSONObject
import java.io.DataInputStream
import java.io.DataOutputStream
import java.io.IOException
import java.net.InetSocketAddress
import java.net.Socket
import java.security.MessageDigest
import java.security.SecureRandom
import java.security.cert.X509Certificate
import javax.net.ssl.SSLContext
import javax.net.ssl.SSLSocket
import javax.net.ssl.X509TrustManager

object DeviceTls {
    fun fingerprint(cert: X509Certificate): String =
        MessageDigest.getInstance("SHA-256").digest(cert.encoded).joinToString("") { "%02x".format(it) }

    // Trust is established only by comparing the connection-bound code in the web UI.
    fun context(pin: String?): SSLContext = SSLContext.getInstance("TLS").apply {
        init(null, arrayOf(object : X509TrustManager {
            override fun getAcceptedIssuers(): Array<X509Certificate> = emptyArray()
            override fun checkClientTrusted(chain: Array<X509Certificate>, authType: String) {
                throw java.security.cert.CertificateException("Client certificates not supported")
            }
            override fun checkServerTrusted(chain: Array<X509Certificate>, authType: String) {
                if (chain.isEmpty()) throw java.security.cert.CertificateException("Missing device certificate")
                if (pin != null && !MessageDigest.isEqual(
                        fingerprint(chain[0]).toByteArray(Charsets.US_ASCII), pin.toByteArray(Charsets.US_ASCII),
                    )) throw java.security.cert.CertificateException("Geraetefingerprint stimmt nicht; erneut koppeln")
            }
        }), SecureRandom())
    }
}

data class PairingResult(
    val token: String, val fingerprint: String, val deviceId: String, val fqdn: String?, val httpsPort: Int,
    val ipv4: String?, val ipv6: List<String>,
)

object PairingClient {
    private const val MAX_FRAME = 2048

    fun confirmationCode(fingerprint: String, nonce: String, id: String): String {
        val hash = MessageDigest.getInstance("SHA-256")
            .digest("smleasy-pair-v1:$fingerprint:$nonce:$id".toByteArray(Charsets.US_ASCII))
        val number = ((hash[0].toLong() and 255) shl 24) or ((hash[1].toLong() and 255) shl 16) or
            ((hash[2].toLong() and 255) shl 8) or (hash[3].toLong() and 255)
        return (number % 1_000_000).toString().padStart(6, '0')
    }

    fun pair(host: String, name: String, onCode: (String) -> Unit, cancelled: () -> Boolean): PairingResult {
        val raw = Socket()
        try {
            raw.connect(InetSocketAddress(host.removeSurrounding("[", "]"), 7384), 4000)
            raw.soTimeout = 6000
            val socket = DeviceTls.context(null).socketFactory.createSocket(raw, host, 7384, true) as SSLSocket
            socket.use {
                it.soTimeout = 6000
                it.startHandshake()
                val fingerprint = DeviceTls.fingerprint(it.session.peerCertificates[0] as X509Certificate)
                val input = DataInputStream(it.inputStream)
                val output = DataOutputStream(it.outputStream)
                fun send(json: JSONObject) {
                    val bytes = json.toString().toByteArray(Charsets.UTF_8)
                    if (bytes.size !in 1..MAX_FRAME) throw IOException("Kopplungsnachricht zu gross")
                    output.writeShort(bytes.size)
                    output.write(bytes)
                    output.flush()
                }
                fun receive(): JSONObject {
                    val size = input.readUnsignedShort()
                    if (size !in 1..MAX_FRAME) throw IOException("Ungueltige Kopplungsnachricht")
                    val bytes = ByteArray(size)
                    input.readFully(bytes)
                    return JSONObject(String(bytes, Charsets.UTF_8))
                }
                val nonce = ByteArray(32).also { SecureRandom().nextBytes(it) }
                    .joinToString("") { "%02x".format(it) }
                send(JSONObject().put("op", "pair").put("nonce", nonce).put("name", name.take(20)))
                val pending = receive()
                if (pending.optString("status") != "pending") throw IOException(pending.optString("error", "Koppeln nicht aktiv"))
                val id = pending.getString("request_id")
                if (!Regex("[0-9a-f]{32}").matches(id)) throw IOException("Ungueltige Anfragekennung")
                onCode(confirmationCode(fingerprint, nonce, id))
                val deadline = android.os.SystemClock.elapsedRealtime() + 300_000
                while (!cancelled() && android.os.SystemClock.elapsedRealtime() < deadline) {
                    send(JSONObject().put("op", "poll").put("request_id", id))
                    val response = receive()
                    when (response.getString("status")) {
                        "approved" -> {
                            val token = response.getString("token")
                            val pin = response.getString("fingerprint")
                            val port = response.getInt("https_port")
                            if (!Regex("[0-9a-f]{64}").matches(token) || pin != fingerprint || port !in 1..65535)
                                throw IOException("Ungueltige Kopplungsantwort")
                            val ipv4 = response.optString("ipv4").takeIf { PairingInfo.isValidIpv4(it) }
                            val ipv6 = response.optJSONArray("ipv6")?.let { a ->
                                (0 until a.length()).map { a.getString(it) }.filter { PairingInfo.isValidIpv6(it) }
                            }.orEmpty()
                            return PairingResult(token, pin, response.getString("device_id"),
                                response.optString("fqdn").ifBlank { null }, port, ipv4, ipv6)
                        }
                        "rejected" -> throw IOException("Kopplung im Webfrontend abgelehnt")
                        "error" -> throw IOException(response.optString("error", "Kopplungsfehler"))
                        "waiting" -> Thread.sleep(1000)
                        else -> throw IOException("Unbekannte Kopplungsantwort")
                    }
                }
                throw IOException("Kopplung abgebrochen oder abgelaufen")
            }
        } finally {
            raw.close()
        }
    }
}
