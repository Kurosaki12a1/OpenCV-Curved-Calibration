package com.kuro.android.opencv

import android.content.Context
import android.graphics.Bitmap
import android.graphics.Canvas
import android.graphics.Color
import android.graphics.Matrix
import android.graphics.Paint
import android.graphics.RectF
import android.util.AttributeSet
import android.view.MotionEvent
import android.view.ScaleGestureDetector
import android.view.View
import kotlin.math.max
import kotlin.math.min

class ZoomableConcatView @JvmOverloads constructor(
    context: Context,
    attrs: AttributeSet? = null
) : View(context, attrs), ScaleGestureDetector.OnScaleGestureListener {

    private val bitmaps = mutableListOf<Bitmap>()
    private val paint = Paint(Paint.ANTI_ALIAS_FLAG)
    private val borderPaint = Paint(Paint.ANTI_ALIAS_FLAG).apply {
        color = Color.RED
        style = Paint.Style.STROKE
        strokeWidth = resources.displayMetrics.density // ~0.2dp scaled
    }

    private val drawMatrix = Matrix()
    private val scaleDetector = ScaleGestureDetector(context, this)
    private var scaleFactor = 1f
    private var translateX = 0f
    private var translateY = 0f

    private var lastTouchX = 0f
    private var lastTouchY = 0f
    private var isDragging = false

    /** Thêm nhiều bitmap vào view */
    fun setBitmaps(list: List<Bitmap>) {
        bitmaps.clear()
        bitmaps.addAll(list)
        invalidate()
    }

    override fun onDraw(canvas: Canvas) {
        super.onDraw(canvas)
        canvas.save()
        canvas.concat(drawMatrix)

        var currentX = 0f
        for (bmp in bitmaps) {
            canvas.drawBitmap(bmp, currentX, 0f, null)

            // Draw red border
            val rect = RectF(
                currentX,
                0f,
                currentX + bmp.width.toFloat(),
                bmp.height.toFloat()
            )
            canvas.drawRect(rect, borderPaint)

            currentX += bmp.width
        }

        canvas.restore()
    }

    // ================================
    // 🔍 Xử lý Zoom & Pan
    // ================================
    override fun onTouchEvent(event: MotionEvent): Boolean {
        scaleDetector.onTouchEvent(event)

        when (event.actionMasked) {
            MotionEvent.ACTION_DOWN -> {
                lastTouchX = event.x
                lastTouchY = event.y
                isDragging = true
            }
            MotionEvent.ACTION_MOVE -> {
                if (!scaleDetector.isInProgress && isDragging) {
                    val dx = event.x - lastTouchX
                    val dy = event.y - lastTouchY
                    translateX += dx
                    translateY += dy
                    updateMatrix()
                    invalidate()
                    lastTouchX = event.x
                    lastTouchY = event.y
                }
            }
            MotionEvent.ACTION_UP, MotionEvent.ACTION_CANCEL -> {
                isDragging = false
            }
        }

        return true
    }

    private fun updateMatrix() {
        drawMatrix.reset()
        drawMatrix.postTranslate(translateX, translateY)
        drawMatrix.postScale(scaleFactor, scaleFactor, width / 2f, height / 2f)
    }

    // ================================
    // 🔎 ScaleGestureDetector callbacks
    // ================================
    override fun onScale(detector: ScaleGestureDetector): Boolean {
        scaleFactor *= detector.scaleFactor
        scaleFactor = max(0.3f, min(scaleFactor, 5.0f))
        updateMatrix()
        invalidate()
        return true
    }

    override fun onScaleBegin(detector: ScaleGestureDetector): Boolean = true
    override fun onScaleEnd(detector: ScaleGestureDetector) {}
}