package de.smleasy.board

import android.content.Context
import android.graphics.Bitmap
import android.graphics.Canvas
import android.graphics.Color
import android.graphics.Paint
import android.graphics.RectF
import android.graphics.Typeface
import android.graphics.drawable.GradientDrawable
import android.view.Gravity
import android.view.MotionEvent
import android.view.View
import android.view.ViewGroup
import android.widget.FrameLayout
import android.widget.LinearLayout
import android.widget.TextView
import kotlin.math.max
import kotlin.math.min

/**
 * Zeigt das Hintergrundbild mit Kacheln. Position (Prozent der Bildflaeche) und
 * Groesse (Skalierung) je Kachel werden ueber [onLayoutChanged] gespeichert.
 */
class BoardView(
    context: Context,
    private val store: Store,
    private val onLayoutChanged: (String, TileLayout) -> Unit,
) : FrameLayout(context) {

    private class Tile(val key: String, val box: LinearLayout, var layout: TileLayout)

    private val tiles = LinkedHashMap<String, Tile>()
    private var bitmap: Bitmap? = null
    private val imageRect = RectF()
    private val paint = Paint(Paint.FILTER_BITMAP_FLAG)
    private var data: Map<String, TileData> = emptyMap()
    var editMode = false
        set(v) {
            field = v
            tiles.values.forEach { applyEditStyle(it) }
        }
    var selectedKey: String? = null
        private set

    init {
        setBackgroundColor(Color.BLACK)
        setWillNotDraw(false)
        for (key in Store.DEFAULTS.keys) {
            val box = LinearLayout(context).apply {
                orientation = LinearLayout.VERTICAL
                visibility = View.GONE
            }
            val tile = Tile(key, box, store.layout(key))
            box.setOnTouchListener(dragListener(tile))
            tiles[key] = tile
            addView(box, LayoutParams(ViewGroup.LayoutParams.WRAP_CONTENT, ViewGroup.LayoutParams.WRAP_CONTENT))
        }
    }

    fun setImage(b: Bitmap?) {
        bitmap = b
        computeImageRect(width, height)
        invalidate()
        requestLayout()
    }

    fun setData(d: Map<String, TileData>) {
        data = d
        render()
    }

    fun scaleSelected(factor: Float) {
        val t = selectedKey?.let { tiles[it] } ?: return
        t.layout = t.layout.copy(scale = (t.layout.scale * factor).coerceIn(Store.MIN_SCALE, Store.MAX_SCALE))
        onLayoutChanged(t.key, t.layout)
        place(t)
    }

    fun reloadLayouts() {
        tiles.values.forEach { it.layout = store.layout(it.key) }
        requestLayout()
    }

    private fun computeImageRect(w: Int, h: Int) {
        val b = bitmap
        if (b == null || w == 0 || h == 0) {
            imageRect.set(0f, 0f, w.toFloat(), h.toFloat())
            return
        }
        val s = min(w.toFloat() / b.width, h.toFloat() / b.height)
        val iw = b.width * s
        val ih = b.height * s
        imageRect.set((w - iw) / 2, (h - ih) / 2, (w + iw) / 2, (h + ih) / 2)
    }

    override fun onDraw(canvas: Canvas) {
        bitmap?.let { canvas.drawBitmap(it, null, imageRect, paint) }
    }

    private fun render() {
        val textPx = max(11f, imageRect.width() * 0.0125f)
        val pad = (textPx * 0.7f).toInt()
        for ((key, tile) in tiles) {
            val d = data[key]
            val box = tile.box
            box.removeAllViews()
            if (d == null) {
                box.visibility = View.GONE
                continue
            }
            box.visibility = View.VISIBLE
            box.setPadding(pad, pad / 2 + 2, pad, pad / 2 + 2)
            box.addView(text(d.title, textPx * 0.95f, Color.parseColor("#cbd5e1"), false))
            box.addView(text(d.value, textPx * 1.9f, d.color, true))
            for ((k, v) in d.rows) {
                val row = LinearLayout(context).apply { orientation = LinearLayout.HORIZONTAL }
                row.addView(text(k, textPx * 0.85f, Color.parseColor("#94a3b8"), false),
                    LinearLayout.LayoutParams(0, ViewGroup.LayoutParams.WRAP_CONTENT, 1f))
                row.addView(text(v, textPx * 0.85f, Color.parseColor("#e2e8f0"), false).apply { setPadding(pad, 0, 0, 0) })
                box.addView(row)
            }
            box.minimumWidth = (textPx * 9).toInt()
            applyEditStyle(tile)
        }
        requestLayout()
    }

    private fun text(s: String, px: Float, color: Int, bold: Boolean) = TextView(context).apply {
        text = s
        setTextSize(android.util.TypedValue.COMPLEX_UNIT_PX, px)
        setTextColor(color)
        setSingleLine(true)
        if (bold) typeface = Typeface.DEFAULT_BOLD
    }

    private fun applyEditStyle(t: Tile) {
        val selected = editMode && t.key == selectedKey
        t.box.background = GradientDrawable().apply {
            cornerRadius = 24f
            setColor(Color.argb(200, 8, 15, 30))
            setStroke(if (editMode) 4 else 2,
                if (selected) Color.parseColor("#facc15")
                else if (editMode) Color.parseColor("#38bdf8") else Color.argb(90, 148, 163, 184))
        }
    }

    override fun onLayout(changed: Boolean, l: Int, t: Int, r: Int, b: Int) {
        computeImageRect(r - l, b - t)
        tiles.values.forEach { place(it) }
    }

    // Kachelmitte liegt bei x%/y% der Bildflaeche; die Skalierung wirkt um die Mitte.
    private fun place(t: Tile) {
        val box = t.box
        if (box.visibility == View.GONE) return
        box.measure(MeasureSpec.makeMeasureSpec(0, MeasureSpec.UNSPECIFIED), MeasureSpec.makeMeasureSpec(0, MeasureSpec.UNSPECIFIED))
        val w = box.measuredWidth
        val h = box.measuredHeight
        val cx = imageRect.left + imageRect.width() * t.layout.x / 100f
        val cy = imageRect.top + imageRect.height() * t.layout.y / 100f
        box.layout((cx - w / 2).toInt(), (cy - h / 2).toInt(), (cx + w / 2).toInt(), (cy + h / 2).toInt())
        box.pivotX = w / 2f
        box.pivotY = h / 2f
        box.scaleX = t.layout.scale
        box.scaleY = t.layout.scale
    }

    private fun dragListener(t: Tile) = object : OnTouchListener {
        var lastX = 0f
        var lastY = 0f
        override fun onTouch(v: View, e: MotionEvent): Boolean {
            if (!editMode) return false
            when (e.actionMasked) {
                MotionEvent.ACTION_DOWN -> {
                    lastX = e.rawX; lastY = e.rawY
                    val old = selectedKey?.let { tiles[it] }
                    selectedKey = t.key
                    old?.let { applyEditStyle(it) }
                    applyEditStyle(t)
                    parent?.requestDisallowInterceptTouchEvent(true)
                }
                MotionEvent.ACTION_MOVE -> {
                    val dx = e.rawX - lastX
                    val dy = e.rawY - lastY
                    lastX = e.rawX; lastY = e.rawY
                    t.layout = t.layout.copy(
                        x = (t.layout.x + dx / imageRect.width() * 100f).coerceIn(0f, 100f),
                        y = (t.layout.y + dy / imageRect.height() * 100f).coerceIn(0f, 100f),
                    )
                    place(t)
                }
                MotionEvent.ACTION_UP, MotionEvent.ACTION_CANCEL -> onLayoutChanged(t.key, t.layout)
            }
            return true
        }
    }
}
