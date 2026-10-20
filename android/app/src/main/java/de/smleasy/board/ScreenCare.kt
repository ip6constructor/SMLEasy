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
    private val store: Store,
    private val window: () -> Window?,
) : SensorEventListener {
    private val main = Handler(Looper.getMainLooper())
    private val sensors = context.getSystemService(Context.SENSOR_SERVICE) as SensorManager
    private var lastApplied = -2f
    private var lastUpdateMs = 0L
    private var luxBrightness = -1f
    private var running = false

    private val shiftCheck = object : Runnable {
        override fun run() {
            applyDailyShift()
            if (running) main.postDelayed(this, 15 * 60 * 1000L)
        }
    }

    // Prueft jede Minute, ob die Nachtabsenkung beginnt oder endet.
    private val nightCheck = object : Runnable {
        override fun run() {
            apply()
            if (running) main.postDelayed(this, 60_000L)
        }
    }

    fun start() {
        running = true
        shiftCheck.run()
        nightCheck.run()
        sensors.getDefaultSensor(Sensor.TYPE_LIGHT)?.let {
            sensors.registerListener(this, it, SensorManager.SENSOR_DELAY_NORMAL)
        }
    }

    fun stop() {
        running = false
        main.removeCallbacks(shiftCheck)
        main.removeCallbacks(nightCheck)
        sensors.unregisterListener(this)
        setWindowBrightness(android.view.WindowManager.LayoutParams.BRIGHTNESS_OVERRIDE_NONE)
        lastApplied = -2f
    }

    /** Wendet geaenderte Einstellungen sofort an. */
    fun apply() {
        val b = target()
        if (b == lastApplied) return
        lastApplied = b
        setWindowBrightness(
            if (b < 0f) android.view.WindowManager.LayoutParams.BRIGHTNESS_OVERRIDE_NONE else b,
        )
    }

    private fun setWindowBrightness(value: Float) {
        window()?.let { w ->
            val p = w.attributes
            p.screenBrightness = value
            w.attributes = p
        }
    }

    // -1 = Systemhelligkeit unveraendert lassen (kein Sensor, Automatik, keine Nacht).
    private fun target(): Float {
        var b = when {
            !store.brightnessAuto -> store.brightnessPct / 100f
            else -> luxBrightness
        }
        if (store.nightEnabled && isNight()) {
            val n = store.nightPct / 100f
            b = if (b < 0f) n else min(b, n)
        }
        return if (b < 0f) -1f else b.coerceIn(0.01f, 1f)
    }

    private fun isNight(): Boolean {
        val cal = java.util.Calendar.getInstance()
        val now = cal.get(java.util.Calendar.HOUR_OF_DAY) * 60 + cal.get(java.util.Calendar.MINUTE)
        val from = store.nightFromMin
        val to = store.nightToMin
        return when {
            from == to -> false
            from < to -> now in from until to
            else -> now >= from || now < to
        }
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
        if (luxBrightness >= 0 && abs(b - luxBrightness) < 0.05f) return
        lastUpdateMs = now
        luxBrightness = b
        apply()
    }

    override fun onAccuracyChanged(sensor: Sensor?, accuracy: Int) = Unit

    private companion object {
        const val MIN_BRIGHTNESS = 0.05f
        val SHIFTS = listOf(
            0 to 0, 1 to 0, 1 to 1, 0 to 1, -1 to 1, -1 to 0, -1 to -1, 0 to -1, 1 to -1,
        )
    }
}
