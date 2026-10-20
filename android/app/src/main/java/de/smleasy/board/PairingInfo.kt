package de.smleasy.board

import java.net.Inet6Address
import java.net.InetAddress

object PairingInfo {
    private val HOST = Regex("^(?=.{1,253}$)([A-Za-z0-9]([A-Za-z0-9-]{0,61}[A-Za-z0-9])?\\.)+[A-Za-z]{2,63}$")
    private val IPV4 = Regex("^(\\d{1,3})\\.(\\d{1,3})\\.(\\d{1,3})\\.(\\d{1,3})$")

    fun isValidFqdn(s: String) = HOST.matches(s)
    fun isValidIpv4(s: String) = IPV4.matchEntire(s)?.groupValues?.drop(1)?.all { it.toInt() in 0..255 } == true
    fun isValidIpv6(s: String): Boolean {
        val ip = s.removeSurrounding("[", "]")
        val address = ip.substringBefore('%')
        if (!address.contains(':') || !Regex("[0-9A-Fa-f:.]+").matches(address)) return false
        if (ip.contains('%') && !Regex("[A-Za-z0-9_.-]{1,32}").matches(ip.substringAfter('%'))) return false
        return try {
            InetAddress.getByName(address) is Inet6Address
        } catch (_: java.net.UnknownHostException) {
            false
        }
    }

    fun urlHost(host: String): String =
        if (host.contains(':')) "[${host.removeSurrounding("[", "]")}]" else host
}
