package de.smleasy.board

import android.graphics.Color
import android.service.dreams.DreamService
import android.view.Gravity
import android.view.ViewGroup
import android.widget.FrameLayout
import android.widget.TextView

/** Bildschirmschoner (Daydream): zeigt das Energie-Board mit den in der App gespeicherten Einstellungen. */
class BoardDream : DreamService() {
    private var controller: BoardController? = null
    private var care: ScreenCare? = null
    private lateinit var status: TextView

    override fun onAttachedToWindow() {
        super.onAttachedToWindow()
        isInteractive = false
        isFullscreen = true
        isScreenBright = true

        val store = Store(this)
        val board = BoardView(this, store) { _, _ -> }
        board.dashboardMode = store.dashboardMode
        status = TextView(this).apply {
            setTextColor(Color.WHITE)
            setBackgroundColor(Color.argb(170, 0, 0, 0))
            setPadding(24, 12, 24, 12)
            textSize = 14f
            visibility = android.view.View.GONE
        }
        val root = FrameLayout(this)
        root.addView(board, ViewGroup.LayoutParams.MATCH_PARENT, ViewGroup.LayoutParams.MATCH_PARENT)
        root.addView(
            status,
            FrameLayout.LayoutParams(
                ViewGroup.LayoutParams.WRAP_CONTENT, ViewGroup.LayoutParams.WRAP_CONTENT,
                Gravity.TOP or Gravity.CENTER_HORIZONTAL,
            ),
        )
        setContentView(root)

        controller = BoardController(this, store, board, object : BoardController.Listener {
            override fun onStatus(msg: String?) = show(msg)
            override fun onAuthRequired() = show("Anmeldung erforderlich: SMLEasy-App \u00f6ffnen")
            override fun onLost() = show("Kein Ger\u00e4t gekoppelt: SMLEasy-App \u00f6ffnen")
        })
        care = ScreenCare(this, board, store) { window }
    }

    override fun onDreamingStarted() {
        super.onDreamingStarted()
        care?.start()
        controller?.start()
    }

    override fun onDreamingStopped() {
        controller?.stop()
        care?.stop()
        super.onDreamingStopped()
    }

    override fun onDetachedFromWindow() {
        controller?.release()
        controller = null
        super.onDetachedFromWindow()
    }

    private fun show(msg: String?) {
        status.visibility = if (msg == null) android.view.View.GONE else android.view.View.VISIBLE
        status.text = msg
    }
}
