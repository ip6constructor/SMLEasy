package de.smleasy.board

import org.json.JSONObject
import org.junit.Assert.*
import org.junit.Test

class BoardAndAddressTest {
    @Test fun ipv6LiteralsAndInterfaceScopesArePreserved() {
        for (address in listOf("2001:db8::42", "fd00::1", "fe80::1%wlan0", "fe80::1%7", "::1")) {
            assertTrue(address, PairingInfo.isValidIpv6(address))
            assertEquals("[$address]", PairingInfo.urlHost(address))
            assertEquals("[$address]", PairingInfo.urlHost("[$address]"))
        }
        for (invalid in listOf("not-a-host", "2001:::1", "12345::1", "fe80::1%", "fe80::1%a%b", "2001:db8::1/path")) {
            assertFalse(invalid, PairingInfo.isValidIpv6(invalid))
        }
        assertEquals("192.168.1.5", PairingInfo.urlHost("192.168.1.5"))
    }

    @Test fun footerShowsAllFourValuesAndUsesTheStatusCurrency() {
        val data = JSONObject().put("today_pv_kwh", 12.5).put("today_export_kwh", 2.5)
            .put("cost_today", -1.23).put("today_saving", 3.45).put("currency", "CHF")
        assertEquals(listOf("80 %", "12,5 kWh", "-1,23 CHF", "3,45 CHF"), BoardModel.kpis(data).map { it.value })
        data.put("today_pv_kwh", 20.0).put("today_export_kwh", 10.0)
        assertEquals("50 %", BoardModel.kpis(data)[0].value)
    }

    @Test fun footerDistinguishesMissingFromRealZeroValues() {
        assertEquals(List(4) { "\u2014" }, BoardModel.kpis(JSONObject()).map { it.value })
        val zeros = JSONObject().put("today_pv_kwh", 0).put("today_export_kwh", 0)
            .put("cost_today", 0).put("today_saving", 0)
        assertEquals(listOf("\u2014", "0,0 kWh", "0,00 EUR", "0,00 EUR"), BoardModel.kpis(zeros).map { it.value })
        zeros.put("today_pv_kwh", JSONObject.NULL)
        assertEquals("\u2014", BoardModel.kpis(zeros)[1].value)
    }
}
