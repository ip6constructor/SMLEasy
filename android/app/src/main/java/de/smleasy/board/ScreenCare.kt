package de.smleasy.board

import android.content.Context
import android.hardware.Sensor
import android.hardware.SensorEvent
import android.hardware.SensorEventListener
import android.hardware.SensorManager
import android.os.Handler
import android.os.Looper
import android.view.View
import android.view.Window
import java.util.TimeZone
import kotlin.math.abs
import kotlin.math.log10
import kotlin.math.max
import kotlin.math.min

/**
 * Einbrennschutz und Helligkeit fuer Dauerbetrieb:
 * - der Inhalt wird einmal pro Kalendertag um wenige Pixel verschoben,
 * - die Fensterhelligkeit folgt dem Umgebungslichtsensor (ohne Sensor bleibt sie unveraendert).
 */
class ScreenCare(
    private val context: Context,
    private val content: View,
    private val window: () -> Window?,
) : SensorEventListener {
    private val main = Handler(Looper.getMainLooper())
    private val sensors = context.getSystemService(Context.SENSOR_SERVICE) as SensorManager
    private var lastApplied = -1f
    private var lastUpdateMs = 0L
    private var running = false

    private val shiftCheck = object : Runnable {
        override fun run() {
            applyDailyShift()
            if (running) main.postDelayed(this, 15 * 60 * 1000L)
        }
    }

    fun start() {
        running = true
        shiftCheck.run()
        sensors.getDefaultSensor(Sensor.TYPE_LIGHT)?.let {
            sensors.registerListener(this, it, SensorManager.SENSOR_DELAY_NORMAL)
        }
    }

    fun stop() {
        running = false
        main.removeCallbacks(shiftCheck)
        sensors.unregisterListener(this)
        window()?.let { w ->
            val p = w.attributes
            p.screenBrightness = android.view.WindowManager.LayoutParams.BRIGHTNESS_OVERRIDE_NONE
            w.attributes = p
        }
        lastApplied = -1f
    }

    private fun applyDailyShift() {
        val now = System.currentTimeMillis()
        val day = (now + TimeZone.getDefault().getOffset(now)) / 86_400_000L
        val (sx, sy) = SHIFTS[(day % SHIFTS.size).toInt()]
        val step = context.resources.displayMetrics.density * 8f
        content.translationX = sx * step
        content.translationY = sy * step
    }

    override fun onSensorChanged(e: SensorEvent) {
        val now = System.currentTimeMillis()
        if (now - lastUpdateMs < 3000) return
        val lux = max(0f, e.values[0])
        // Logarithmische Kennlinie: dunkel ~5 %, Tageslicht (>= 1000 lx) 100 %.
        val b = min(1f, max(MIN_BRIGHTNESS, MIN_BRIGHTNESS + log10(1f + lux) / 3f * (1f - MIN_BRIGHTNESS)))
        if (lastApplied >= 0 && abs(b - lastApplied) < 0.05f) return
        lastUpdateMs = now
        lastApplied = b
        window()?.let { w ->
            val p = w.attributes
            p.screenBrightness = b
            w.attributes = p
        }
    }

    override fun onAccuracyChanged(sensor: Sensor?, accuracy: Int) = Unit

    private companion object {
        const val MIN_BRIGHTNESS = 0.05f
        val SHIFTS = listOf(
            0 to 0, 1 to 0, 1 to 1, 0 to 1, -1 to 1, -1 to 0, -1 to -1, 0 to -1, 1 to -1,
        )
    }
}
