package de.smleasy.board

import android.app.Activity
import android.app.AlertDialog
import android.content.Intent
import android.graphics.Color
import android.os.Build
import android.os.Bundle
import android.os.Handler
import android.os.Looper
import android.text.InputType
import android.view.Gravity
import android.view.View
import android.view.ViewGroup
import android.view.WindowManager
import android.widget.Button
import android.widget.CheckBox
import android.widget.EditText
import android.widget.FrameLayout
import android.widget.LinearLayout
import android.widget.SeekBar
import android.widget.TextView
import java.util.concurrent.Executors

class MainActivity : Activity() {
    private lateinit var store: Store
    private lateinit var board: BoardView
    private lateinit var dashboard: DashboardView
    private lateinit var status: TextView
    private lateinit var editBar: LinearLayout
    private lateinit var controller: BoardController
    private lateinit var care: ScreenCare
    private lateinit var modeButton: Button
    private val main = Handler(Looper.getMainLooper())
    private val io = Executors.newSingleThreadExecutor()
    private var discovery: Discovery? = null
    private var pairingDiscovery: Discovery? = null
    private var pairingDialog: AlertDialog? = null
    @Volatile private var pairingCancelled = true
    @Volatile private var pairingInFlight = false
    private var resumed = false
    private var authDialogOpen = false
    private var generation = 0

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        window.addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON)
        store = Store(this)

        val root = FrameLayout(this)
        board = BoardView(this, store) { key, l -> store.saveLayout(key, l) }
        board.dashboardMode = store.dashboardMode
        root.addView(board, ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.MATCH_PARENT)
        dashboard = DashboardView(this)
        root.addView(dashboard, ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.MATCH_PARENT)
        applyModeVisibility()

        status = TextView(this).apply {
            setTextColor(Color.WHITE)
            setBackgroundColor(Color.argb(170, 0, 0, 0))
            setPadding(24, 12, 24, 12)
            textSize = 14f
            visibility = View.GONE
        }
        root.addView(status, FrameLayout.LayoutParams(ViewGroup.LayoutParams.WRAP_CONTENT, ViewGroup.LayoutParams.WRAP_CONTENT, Gravity.TOP or Gravity.CENTER_HORIZONTAL))

        editBar = LinearLayout(this).apply {
            orientation = LinearLayout.HORIZONTAL
            setBackgroundColor(Color.argb(200, 0, 0, 0))
            visibility = View.GONE
            fun btn(t: String, a: () -> Unit) = addView(Button(this@MainActivity).apply {
                text = t
                setOnClickListener { a() }
            })
            btn("\u2212") { board.scaleSelected(0.9f) }
            btn("+") { board.scaleSelected(1.1f) }
            btn("Fertig") { setEdit(false) }
        }
        root.addView(editBar, FrameLayout.LayoutParams(ViewGroup.LayoutParams.WRAP_CONTENT, ViewGroup.LayoutParams.WRAP_CONTENT, Gravity.BOTTOM or Gravity.CENTER_HORIZONTAL))

        modeButton = Button(this).apply {
            alpha = 0.6f
            setOnClickListener { toggleDashboardMode(this) }
        }
        updateModeButton(modeButton)
        root.addView(modeButton, FrameLayout.LayoutParams(ViewGroup.LayoutParams.WRAP_CONTENT, ViewGroup.LayoutParams.WRAP_CONTENT, Gravity.TOP or Gravity.END))

        // Langer Druck auf den Hintergrund oeffnet das Menue (Kacheln beim Bearbeiten reagieren selbst).
        board.setOnLongClickListener { showMenu(); true }
        setContentView(root)

        controller = BoardController(this, store, board, object : BoardController.Listener {
            override fun onStatus(msg: String?) = showStatus(msg)
            override fun onAuthRequired() {
                if (store.token != null) {
                    controller.stop()
                    showStatus("App-Token ung\u00fcltig oder widerrufen \u2013 bitte neu koppeln")
                } else askCredentials()
            }
            override fun onLost() = startDiscovery()
        }, dashboard) { board.dashboardMode }
        care = ScreenCare(this, board, store) { window }
    }

    override fun onNewIntent(intent: Intent) {
        super.onNewIntent(intent)
        setIntent(intent)
    }

    private var pairingMenuAutoShown = false

    override fun onResume() {
        super.onResume()
        resumed = true
        hideSystemUi()
        generation++
        care.start()
        if (store.baseUrl() == null && store.fqdn == null) startDiscovery() else startPolling()
        if (store.token == null && !pairingMenuAutoShown) {
            pairingMenuAutoShown = true
            showPairingMenu()
        }
    }

    override fun onPause() {
        super.onPause()
        resumed = false
        generation++
        controller.stop()
        care.stop()
        discovery?.release()
        discovery = null
        pairingCancelled = true
        pairingDiscovery?.release()
        pairingDiscovery = null
        pairingDialog?.dismiss()
        pairingDialog = null
    }

    override fun onDestroy() {
        super.onDestroy()
        controller.release()
        io.shutdownNow()
    }

    override fun onWindowFocusChanged(hasFocus: Boolean) {
        super.onWindowFocusChanged(hasFocus)
        if (hasFocus) hideSystemUi()
    }

    @Suppress("DEPRECATION")
    private fun hideSystemUi() {
        window.decorView.systemUiVisibility = View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY or
            View.SYSTEM_UI_FLAG_FULLSCREEN or View.SYSTEM_UI_FLAG_HIDE_NAVIGATION or
            View.SYSTEM_UI_FLAG_LAYOUT_STABLE or View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN or
            View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
    }

    private fun setEdit(on: Boolean) {
        board.editMode = on
        editBar.visibility = if (on) View.VISIBLE else View.GONE
    }

    private fun updateModeButton(b: Button) {
        b.text = if (board.dashboardMode) "Foto" else "Dashboard"
    }

    private fun toggleDashboardMode(b: Button) {
        board.dashboardMode = !board.dashboardMode
        store.dashboardMode = board.dashboardMode
        if (board.dashboardMode) setEdit(false)
        updateModeButton(b)
        applyModeVisibility()
        controller.start()
    }

    private fun applyModeVisibility() {
        dashboard.visibility = if (board.dashboardMode) View.VISIBLE else View.GONE
        board.visibility = if (board.dashboardMode) View.GONE else View.VISIBLE
    }

    private fun showStatus(msg: String?) {
        status.visibility = if (msg == null) View.GONE else View.VISIBLE
        status.text = msg
    }

    private fun showMenu() {
        val items = arrayOf("Layout bearbeiten", "Layout zuruecksetzen", "Geraet neu suchen", "Zugangsdaten", "App koppeln", "Geraet neu starten", "Helligkeit")
        AlertDialog.Builder(this).setItems(items) { _, i ->
            when (i) {
                0 -> {
                    if (board.dashboardMode) toggleDashboardMode(modeButton)
                    setEdit(true)
                }
                1 -> { store.resetLayout(); board.reloadLayouts() }
                2 -> { store.host = null; store.instanceName = null; startDiscovery() }
                3 -> askCredentials()
                4 -> showPairingMenu()
                5 -> AlertDialog.Builder(this)
                    .setMessage("SMLEasy-Geraet jetzt neu starten?")
                    .setPositiveButton("Neu starten") { _, _ -> controller.rebootDevice { showStatus(it) } }
                    .setNegativeButton("Abbrechen", null)
                    .show()
                6 -> showBrightness()
            }
        }.show()
    }

    private fun showBrightness() {
        fun clock(min: Int) = String.format(java.util.Locale.GERMANY, "%02d:%02d", min / 60, min % 60)
        fun seek(value: Int, onChange: (Int) -> Unit) = SeekBar(this).apply {
            max = 99
            progress = value - 1
            setOnSeekBarChangeListener(object : SeekBar.OnSeekBarChangeListener {
                override fun onProgressChanged(s: SeekBar?, p: Int, fromUser: Boolean) {
                    if (fromUser) onChange(p + 1)
                }
                override fun onStartTrackingTouch(s: SeekBar?) = Unit
                override fun onStopTrackingTouch(s: SeekBar?) = Unit
            })
        }
        fun label(t: String) = TextView(this).apply { text = t; setPadding(0, 24, 0, 4) }

        val dayLabel = label("")
        val nightLabel = label("")
        fun refreshLabels() {
            dayLabel.text = "Helligkeit tagsueber: ${store.brightnessPct} %"
            nightLabel.text = "Helligkeit nachts: ${store.nightPct} %"
        }
        val auto = CheckBox(this).apply {
            text = "Automatisch (Lichtsensor, falls vorhanden)"
            isChecked = store.brightnessAuto
        }
        val daySeek = seek(store.brightnessPct) {
            store.brightnessPct = it; refreshLabels(); care.apply()
        }
        val night = CheckBox(this).apply {
            text = "Nachtabsenkung"
            isChecked = store.nightEnabled
        }
        val nightSeek = seek(store.nightPct) {
            store.nightPct = it; refreshLabels(); care.apply()
        }
        val from = Button(this)
        val to = Button(this)
        fun refreshTimes() {
            from.text = "Von ${clock(store.nightFromMin)}"
            to.text = "Bis ${clock(store.nightToMin)}"
        }
        fun pick(current: Int, set: (Int) -> Unit) =
            android.app.TimePickerDialog(this, { _, h, m ->
                set(h * 60 + m); refreshTimes(); care.apply()
            }, current / 60, current % 60, true).show()
        from.setOnClickListener { pick(store.nightFromMin) { store.nightFromMin = it } }
        to.setOnClickListener { pick(store.nightToMin) { store.nightToMin = it } }
        auto.setOnCheckedChangeListener { _, c ->
            store.brightnessAuto = c; daySeek.isEnabled = !c; care.apply()
        }
        night.setOnCheckedChangeListener { _, c ->
            store.nightEnabled = c; nightSeek.isEnabled = c; from.isEnabled = c; to.isEnabled = c
            care.apply()
        }
        daySeek.isEnabled = !store.brightnessAuto
        nightSeek.isEnabled = store.nightEnabled
        from.isEnabled = store.nightEnabled
        to.isEnabled = store.nightEnabled
        refreshLabels(); refreshTimes()

        val box = LinearLayout(this).apply {
            orientation = LinearLayout.VERTICAL
            setPadding(48, 16, 48, 0)
            addView(auto); addView(dayLabel); addView(daySeek)
            addView(night); addView(nightLabel); addView(nightSeek)
            addView(LinearLayout(this@MainActivity).apply { addView(from); addView(to) })
        }
        AlertDialog.Builder(this).setTitle("Helligkeit")
            .setView(android.widget.ScrollView(this).apply { addView(box) })
            .setPositiveButton("Fertig", null)
            .show()
    }

    private fun startDiscovery() {
        controller.stop()
        val g = ++generation
        showStatus("Suche SMLEasy im lokalen Netz ...")
        discovery?.release()
        discovery = Discovery(this, store.instanceName, onFound = { dev ->
            if (g != generation) return@Discovery
            store.instanceName = dev.instanceName
            store.host = dev.host
            store.port = dev.port
            dev.hosts.firstOrNull { it.contains(':') }?.let {
                store.host6 = it.removeSurrounding("[", "]")
            }
            startPolling()
        }, onError = { showStatus(it) }).also { it.start() }
        // Ohne Treffer erneut versuchen; nie blockieren oder neu starten.
        main.postDelayed({
            if (g == generation && store.baseUrl() == null) startDiscovery()
        }, 30_000)
    }

    private fun startPolling() {
        controller.start()
    }

    private fun askManual() {
        fun field(hintText: String, value: String?) = EditText(this).apply {
            hint = hintText
            setText(value ?: "")
            setSingleLine()
        }
        val fqdn = field("Hostname, z. B. smarthome.example.de", store.fqdn)
        val ip4 = field("IPv4 (RÃ¼ckfall), z. B. 192.168.1.36", store.host)
        val ip6 = field("IPv6 (RÃ¼ckfall), z. B. 2001:db8::1", store.host6)
        val box = LinearLayout(this).apply {
            orientation = LinearLayout.VERTICAL
            setPadding(48, 16, 48, 0)
            addView(fqdn); addView(ip4); addView(ip6)
        }
        AlertDialog.Builder(this).setTitle("Manuelle Konfiguration").setView(box)
            .setPositiveButton("Speichern") { _, _ ->
                val h = fqdn.text.toString().trim().lowercase()
                val v4 = ip4.text.toString().trim()
                val v6 = ip6.text.toString().trim().removePrefix("[").removeSuffix("]")
                val error = when {
                    h.isNotEmpty() && !PairingInfo.isValidFqdn(h) -> "Ung\u00fcltiger Hostname"
                    v4.isNotEmpty() && !PairingInfo.isValidIpv4(v4) -> "Ung\u00fcltige IPv4-Adresse"
                    v6.isNotEmpty() && !PairingInfo.isValidIpv6(v6) -> "Ung\u00fcltige IPv6-Adresse"
                    else -> null
                }
                if (error != null) {
                    showStatus(error)
                } else {
                    store.fqdn = h.ifEmpty { null }
                    store.host = v4.ifEmpty { null }
                    store.host6 = v6.ifEmpty { null }
                    store.token = null
                    store.fingerprint = null
                    showStatus("Gespeichert (Passwort-Anmeldung; fuer ein App-Token lokal koppeln)")
                    startPolling()
                }
            }
            .setNegativeButton("Abbrechen", null)
            .show()
    }

    private fun missingNetworkPermissions(): Array<String> {
        val wanted = buildList {
            if (Build.VERSION.SDK_INT >= 33) add("android.permission.NEARBY_WIFI_DEVICES")
            if (Build.VERSION.SDK_INT >= 36) add("android.permission.ACCESS_LOCAL_NETWORK")
        }
        return wanted.filter { checkSelfPermission(it) != android.content.pm.PackageManager.PERMISSION_GRANTED }.toTypedArray()
    }

    private var permissionAsked = false

    @Deprecated("Activity-API")
    override fun onRequestPermissionsResult(requestCode: Int, permissions: Array<out String>, grantResults: IntArray) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults)
        if (requestCode == 71 && resumed) searchPairing()
    }

    private fun searchPairing() {
        val missing = missingNetworkPermissions()
        if (missing.isNotEmpty() && !permissionAsked) {
            permissionAsked = true
            requestPermissions(missing, 71)
            return
        }
        if (pairingInFlight) {
            showStatus("Vorherige Kopplung wird noch beendet. Bitte kurz warten.")
            return
        }
        controller.stop()
        discovery?.release()
        discovery = null
        pairingCancelled = false
        pairingDiscovery?.release()
        pairingDialog = AlertDialog.Builder(this).setTitle("Geraet suchen")
            .setMessage("Im ESP-Webfrontend den Koppelmodus starten. Suche im lokalen Netz ...")
            .setNegativeButton("Abbrechen") { _, _ -> cancelPairing() }
            .setOnCancelListener { cancelPairing() }.show()
        pairingDiscovery = Discovery(this, null, onFound = { dev ->
            pairingDiscovery?.release()
            pairingDiscovery = null
            connectPairing(dev)
        }, pairing = true, onError = { showStatus(it) }).also { it.start() }
        main.postDelayed({
            if (pairingDiscovery != null) {
                val diag = pairingDiscovery?.summary().orEmpty()
                cancelPairing()
                pairingDialog?.dismiss()
                showStatus("Kein Geraet im Koppelmodus gefunden ($diag). Gleiches WLAN, Berechtigung \u201eGeraete in der Naehe\u201c und Lokales Netzwerk pruefen oder IP eingeben.")
            }
        }, 30_000)
    }

    private fun cancelPairing() {
        pairingCancelled = true
        pairingDiscovery?.release()
        pairingDiscovery = null
    }

    private fun connectPairing(dev: Device) {
        if (pairingInFlight) {
            showStatus("Vorherige Kopplung wird noch beendet. Bitte kurz warten.")
            return
        }
        pairingInFlight = true
        pairingCancelled = false
        pairingDialog?.dismiss()
        pairingDialog = AlertDialog.Builder(this).setTitle("Koppeln: ${dev.instanceName}")
            .setMessage("Verbindung zu ${dev.host}:7384 ...")
            .setNegativeButton("Abbrechen") { _, _ -> cancelPairing() }
            .setOnCancelListener { cancelPairing() }.show()
        io.execute {
            var success = false
            val message = try {
                val result = PairingClient.pair(dev.host, Build.MODEL, onCode = { code ->
                    main.post {
                        if (!pairingCancelled) pairingDialog?.setMessage(
                            "Vergleichsnummer: $code\n\nNur wenn dieselbe Nummer im ESP-Webfrontend steht, dort diese App freigeben.",
                        )
                    }
                }, cancelled = { pairingCancelled })
                if (pairingCancelled) throw java.io.IOException("Kopplung abgebrochen")
                if (!store.savePairing(dev.host, dev.instanceName, result))
                    throw java.io.IOException("Kopplung konnte nicht gespeichert werden; App im ESP widerrufen und erneut versuchen")
                success = true
                "Gekoppelt mit ${dev.instanceName}"
            } catch (e: Exception) {
                "Kopplung fehlgeschlagen: ${e.message ?: e.javaClass.simpleName}"
            }
            main.post {
                pairingInFlight = false
                pairingDialog?.dismiss()
                pairingDialog = null
                if (!resumed) return@post
                showStatus(message)
                if (success) startPolling()
            }
        }
    }

    private fun pairingByIp() {
        val input = EditText(this).apply { hint = "IPv4 oder IPv6"; setSingleLine() }
        AlertDialog.Builder(this).setTitle("Geraet ueber IP koppeln").setView(input)
            .setPositiveButton("Verbinden") { _, _ ->
                val ip = input.text.toString().trim().removeSurrounding("[", "]")
                if (PairingInfo.isValidIpv4(ip) || PairingInfo.isValidIpv6(ip)) {
                    controller.stop()
                    connectPairing(Device(ip, if (ip.contains(':')) "[$ip]" else ip, 7384))
                } else showStatus("Ungueltige IP-Adresse")
            }.setNegativeButton("Abbrechen", null).show()
    }

    private fun showPairingMenu() {
        val items = arrayOf("Geraet suchen (Koppelmodus)", "Ueber IP koppeln", "Manuell (Passwort-Zugang)", "Kopplung loeschen")
        AlertDialog.Builder(this).setTitle("Koppeln").setItems(items) { _, i ->
            when (i) {
                0 -> searchPairing()
                1 -> pairingByIp()
                2 -> askManual()
                3 -> { store.clearPairing(); startPolling() }
            }
        }.show()
    }

    private fun askCredentials() {
        if (authDialogOpen) return
        authDialogOpen = true
        val user = EditText(this).apply { hint = "Benutzer"; setText(store.user); setSingleLine() }
        val pass = EditText(this).apply {
            hint = "Passwort"
            inputType = InputType.TYPE_CLASS_TEXT or InputType.TYPE_TEXT_VARIATION_PASSWORD
        }
        val box = LinearLayout(this).apply {
            orientation = LinearLayout.VERTICAL
            setPadding(48, 16, 48, 0)
            addView(user)
            addView(pass)
        }
        AlertDialog.Builder(this).setTitle("Anmeldung").setView(box)
            .setPositiveButton("Speichern") { _, _ ->
                store.user = user.text.toString().ifBlank { "admin" }
                store.password = pass.text.toString()
                authDialogOpen = false
                startPolling()
            }
            .setNegativeButton("Abbrechen") { _, _ -> authDialogOpen = false }
            .setOnCancelListener { authDialogOpen = false }
            .show()
    }
}
