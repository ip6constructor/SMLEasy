package de.smleasy.board

import org.json.JSONObject
import org.junit.Assert.*
import org.junit.Test
import java.io.DataInputStream
import java.io.DataOutputStream
import java.net.InetAddress
import java.nio.file.Files
import java.security.KeyStore
import java.security.SecureRandom
import java.security.cert.X509Certificate
import java.util.concurrent.Executors
import java.util.concurrent.TimeUnit
import javax.net.ssl.KeyManagerFactory
import javax.net.ssl.SSLContext
import javax.net.ssl.SSLServerSocket
import javax.net.ssl.SSLSocket

class PairingProtocolTest {
    private fun randomHex(size: Int) = ByteArray(size).also { SecureRandom().nextBytes(it) }
        .joinToString("") { "%02x".format(it) }

    private fun serverContext(): SSLContext {
        val dir = Files.createTempDirectory("smleasy-pair-test")
        val keystore = dir.resolve("identity.p12")
        val password = randomHex(16)
        try {
            val tool = java.nio.file.Paths.get(System.getProperty("java.home"), "bin", "keytool").toString()
            val process = ProcessBuilder(tool, "-genkeypair", "-alias", "test", "-keyalg", "EC",
                "-groupname", "secp256r1", "-dname", "CN=localhost", "-validity", "1",
                "-storetype", "PKCS12", "-keystore", keystore.toString(), "-storepass", password,
                "-noprompt").redirectErrorStream(true).start()
            process.inputStream.use { it.copyTo(java.io.ByteArrayOutputStream()) }
            check(process.waitFor(30, TimeUnit.SECONDS) && process.exitValue() == 0)
            val store = KeyStore.getInstance("PKCS12")
            Files.newInputStream(keystore).use { store.load(it, password.toCharArray()) }
            val managers = KeyManagerFactory.getInstance(KeyManagerFactory.getDefaultAlgorithm())
            managers.init(store, password.toCharArray())
            return SSLContext.getInstance("TLS").apply { init(managers.keyManagers, null, SecureRandom()) }
        } finally {
            Files.deleteIfExists(keystore)
            Files.deleteIfExists(dir)
        }
    }

    @Test fun tlsPairingOverIpv6DeliversTokenOnlyAfterApproval() {
        val context = serverContext()
        val server = context.serverSocketFactory.createServerSocket(7384, 1, InetAddress.getByName("::1")) as SSLServerSocket
        val worker = Executors.newSingleThreadExecutor()
        val token = randomHex(32)
        var displayedCode: String? = null
        try {
            val exchange = worker.submit<String> {
                (server.accept() as SSLSocket).use { socket ->
                    socket.soTimeout = 6000
                    socket.startHandshake()
                    val fingerprint = DeviceTls.fingerprint(socket.session.localCertificates[0] as X509Certificate)
                    val input = DataInputStream(socket.inputStream)
                    val output = DataOutputStream(socket.outputStream)
                    fun read(): JSONObject {
                        val bytes = ByteArray(input.readUnsignedShort())
                        input.readFully(bytes)
                        return JSONObject(String(bytes, Charsets.UTF_8))
                    }
                    fun write(json: JSONObject) {
                        val bytes = json.toString().toByteArray(Charsets.UTF_8)
                        output.writeShort(bytes.size); output.write(bytes); output.flush()
                    }
                    val request = read()
                    assertEquals("pair", request.getString("op"))
                    val id = randomHex(16)
                    val code = PairingClient.confirmationCode(fingerprint, request.getString("nonce"), id)
                    write(JSONObject().put("status", "pending").put("request_id", id))
                    assertEquals(id, read().getString("request_id"))
                    write(JSONObject().put("status", "waiting"))
                    assertEquals(id, read().getString("request_id"))
                    write(JSONObject().put("status", "approved").put("token", token)
                        .put("fingerprint", fingerprint).put("device_id", "test-device")
                        .put("https_port", 443).put("fqdn", ""))
                    code
                }
            }
            val result = PairingClient.pair("[::1]", "Test app", { displayedCode = it }, { false })
            assertEquals(token, result.token)
            assertEquals(exchange.get(10, TimeUnit.SECONDS), displayedCode)
            assertEquals(64, result.fingerprint.length)
            assertNull(result.fqdn)
        } finally {
            server.close()
            worker.shutdownNow()
        }
    }

    @Test fun confirmationCodeIsFixedWidthAndBindsEveryInput() {
        val fingerprint = "0".repeat(64)
        val nonce = "1".repeat(64)
        val id = "2".repeat(32)
        val code = PairingClient.confirmationCode(fingerprint, nonce, id)
        assertTrue(Regex("[0-9]{6}").matches(code))
        assertNotEquals(code, PairingClient.confirmationCode("3".repeat(64), nonce, id))
        assertNotEquals(code, PairingClient.confirmationCode(fingerprint, "4".repeat(64), id))
        assertNotEquals(code, PairingClient.confirmationCode(fingerprint, nonce, "5".repeat(32)))
    }
}
