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
import android.widget.EditText
import android.widget.FrameLayout
import android.widget.LinearLayout
import android.widget.TextView
import com.google.mlkit.vision.barcode.common.Barcode
import com.google.mlkit.vision.codescanner.GmsBarcodeScannerOptions
import com.google.mlkit.vision.codescanner.GmsBarcodeScanning
import java.util.concurrent.Executors

class MainActivity : Activity() {
    private companion object {
        const val REQ_PHOTO = 41
    }
    private lateinit var store: Store
    private lateinit var board: BoardView
    private lateinit var status: TextView
    private lateinit var editBar: LinearLayout
    private lateinit var controller: BoardController
    private lateinit var care: ScreenCare
    private val main = Handler(Looper.getMainLooper())
    private val io = Executors.newSingleThreadExecutor()
    private var discovery: Discovery? = null
    private var authDialogOpen = false
    private var generation = 0

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        window.addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON)
        store = Store(this)

        val root = FrameLayout(this)
        board = BoardView(this, store) { key, l -> store.saveLayout(key, l) }
        root.addView(board, ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.MATCH_PARENT)

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

        // Langer Druck auf den Hintergrund oeffnet das Menue (Kacheln beim Bearbeiten reagieren selbst).
        board.setOnLongClickListener { showMenu(); true }
        setContentView(root)

        controller = BoardController(this, store, board, object : BoardController.Listener {
            override fun onStatus(msg: String?) = showStatus(msg)
            override fun onAuthRequired() {
                if (store.token != null) {
                    store.token = null
                    showStatus("App-Token ung\u00fcltig oder widerrufen \u2013 bitte neu koppeln")
                } else askCredentials()
            }
            override fun onLost() = startDiscovery()
        })
        care = ScreenCare(this, board) { window }
    }

    override fun onResume() {
        super.onResume()
        hideSystemUi()
        generation++
        care.start()
        if (store.baseUrl() == null && store.fqdn == null) startDiscovery() else startPolling()
    }

    override fun onPause() {
        super.onPause()
        generation++
        controller.stop()
        care.stop()
        discovery?.stop()
        discovery = null
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

    private fun showStatus(msg: String?) {
        status.visibility = if (msg == null) View.GONE else View.VISIBLE
        status.text = msg
    }

    private fun showMenu() {
        val items = arrayOf("Layout bearbeiten", "Layout zuruecksetzen", "Geraet neu suchen", "Zugangsdaten", "Koppeln (QR / FQDN)")
        AlertDialog.Builder(this).setItems(items) { _, i ->
            when (i) {
                0 -> setEdit(true)
                1 -> { store.resetLayout(); board.reloadLayouts() }
                2 -> { store.host = null; store.instanceName = null; startDiscovery() }
                3 -> askCredentials()
                4 -> showPairingMenu()
            }
        }.show()
    }

    private fun startDiscovery() {
        controller.stop()
        val g = ++generation
        showStatus("Suche SMLEasy im lokalen Netz ...")
        discovery?.stop()
        discovery = Discovery(this, store.instanceName) { dev ->
            if (g != generation) return@Discovery
            store.instanceName = dev.instanceName
            store.host = dev.host
            store.port = dev.port
            startPolling()
        }.also { it.start() }
        // Ohne Treffer erneut versuchen; nie blockieren oder neu starten.
        main.postDelayed({
            if (g == generation && store.baseUrl() == null) startDiscovery()
        }, 30_000)
    }

    private fun startPolling() {
        controller.start()
    }

    // Pairing: QR per Kamera (Google Code Scanner), aus einem Foto oder manuelle Eingabe.
    private fun scanWithCamera() {
        val options = GmsBarcodeScannerOptions.Builder()
            .setBarcodeFormats(Barcode.FORMAT_QR_CODE)
            .build()
        GmsBarcodeScanning.getClient(this, options).startScan()
            .addOnSuccessListener { applyPairing(it.rawValue) }
            .addOnFailureListener {
                showStatus("Kamera-Scan nicht verf\u00fcgbar (${it.message ?: "Fehler"}). Foto oder manuelle Eingabe nutzen.")
            }
    }

    private fun pickPhoto() {
        val i = Intent(Intent.ACTION_GET_CONTENT).apply {
            type = "image/*"
            addCategory(Intent.CATEGORY_OPENABLE)
        }
        @Suppress("DEPRECATION")
        startActivityForResult(i, REQ_PHOTO)
    }

    @Deprecated("Deprecated in Java")
    override fun onActivityResult(requestCode: Int, resultCode: Int, data: Intent?) {
        @Suppress("DEPRECATION")
        super.onActivityResult(requestCode, resultCode, data)
        val uri = data?.data
        if (requestCode != REQ_PHOTO || resultCode != RESULT_OK || uri == null) return
        io.execute {
            val text = runCatching { QrImage.decode(this, uri) }.getOrNull()
            main.post { applyPairing(text) }
        }
    }

    private fun askFqdn() {
        val input = EditText(this).apply {
            hint = "smarthome.example.de"
            setText(store.fqdn ?: "")
            setSingleLine()
        }
        AlertDialog.Builder(this).setTitle("Hostname (FQDN)").setView(input)
            .setPositiveButton("Speichern") { _, _ ->
                val v = input.text.toString().trim().lowercase()
                if (v.isEmpty()) {
                    store.fqdn = null
                    startPolling()
                } else {
                    store.fqdn = v
                    store.token = null
                    showStatus("Hostname gespeichert (ohne Token: Passwort-Anmeldung)")
                    startPolling()
                }
            }
            .setNegativeButton("Abbrechen", null)
            .show()
    }

    private fun applyPairing(text: String?) {
        val info = text?.let { PairingInfo.parse(it) }
        if (info == null) {
            showStatus("Kein g\u00fcltiger SMLEasy-Pairing-QR-Code (v2)")
            return
        }
        showStatus("Kopplung l\u00e4uft\u2026")
        io.execute {
            val msg = try {
                val api = Api(store.host, store.port, info.fqdn, info.httpsPort, store.user, null)
                val token = api.claim(info.secret, Build.MODEL ?: "Android")
                store.fqdn = info.fqdn
                store.httpsPort = info.httpsPort
                store.token = token
                "Gekoppelt mit ${info.fqdn}"
            } catch (e: Exception) {
                "Kopplung fehlgeschlagen: ${e.message ?: e.javaClass.simpleName}"
            }
            main.post {
                showStatus(msg)
                if (store.token != null) startPolling()
            }
        }
    }

    private fun showPairingMenu() {
        val items = arrayOf("QR scannen (Kamera)", "QR aus Foto", "Hostname manuell", "Kopplung l\u00f6schen")
        AlertDialog.Builder(this).setTitle("Koppeln").setItems(items) { _, i ->
            when (i) {
                0 -> scanWithCamera()
                1 -> pickPhoto()
                2 -> askFqdn()
                3 -> { store.fqdn = null; store.token = null; startPolling() }
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
