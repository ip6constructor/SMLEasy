package de.smleasy.board

import android.net.Uri

/** Inhalt des Pairing-QR: smleasy://pair?v=2&fqdn=HOST&port=443&s=EINMAL-GEHEIMNIS (nur im Pairing-Fenster gueltig). */
data class PairingInfo(val fqdn: String, val httpsPort: Int, val secret: String) {
    companion object {
        private val HOST = Regex("^(?=.{1,253}$)([A-Za-z0-9]([A-Za-z0-9-]{0,61}[A-Za-z0-9])?\\.)+[A-Za-z]{2,63}$")
        private val SECRET = Regex("^[0-9a-f]{32}$")

        fun isValidFqdn(s: String) = HOST.matches(s)

        fun parse(text: String): PairingInfo? {
            val u = runCatching { Uri.parse(text.trim()) }.getOrNull() ?: return null
            if (u.scheme != "smleasy" || u.host != "pair" || u.getQueryParameter("v") != "2") return null
            val fqdn = u.getQueryParameter("fqdn")?.lowercase() ?: return null
            if (!isValidFqdn(fqdn)) return null
            val port = u.getQueryParameter("port")?.toIntOrNull() ?: 443
            if (port !in 1..65535) return null
            val secret = u.getQueryParameter("s") ?: return null
            if (!SECRET.matches(secret)) return null
            return PairingInfo(fqdn, port, secret)
        }
    }
}
