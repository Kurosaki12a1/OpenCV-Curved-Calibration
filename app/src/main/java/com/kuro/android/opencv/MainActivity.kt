package com.kuro.android.opencv

import android.content.Context
import android.graphics.BitmapFactory
import android.os.Bundle
import androidx.appcompat.app.AppCompatActivity
import androidx.core.graphics.createBitmap
import androidx.lifecycle.lifecycleScope
import com.kuro.android.opencv.ChessBoardManager.warpCurvedToFlatInPlace
import com.kuro.android.opencv.databinding.ActivityMainBinding
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.launch
import kotlinx.coroutines.withContext
import org.opencv.android.Utils
import org.opencv.core.Mat

class MainActivity : AppCompatActivity() {

    private lateinit var binding: ActivityMainBinding

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)

        binding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)


        lifecycleScope.launch(Dispatchers.IO) {
            val totalWidth = 3840 + 1920// Group1 + Group2
            val totalHeight = 2160
            val maxGroupWidth = 1920
            val maxGroupHeight = 1080
            val cols = 34
            val rows = 12

            val group1 = ChessBoardManager.generateChessBoardGroupWithBlackPad(
                totalWidth = totalWidth,
                totalHeight = totalHeight,
                groupXOffset = 0,
                groupYOffset = 0,
                groupWidth = 3840,
                groupHeight = 2160,
                activeXOffset = 1920, // vùng thật nằm bên phải
                activeYOffset = 0,    // top
                activeWidth = 1920,
                activeHeight = 1080,
                cols = cols,
                rows = rows
            )

            val group2 = ChessBoardManager.generateChessBoardGroupWithBlackPad(
                totalWidth = totalWidth,
                totalHeight = totalHeight,
                groupXOffset = 3840,      // group này nằm bên phải phần dưới
                groupYOffset = 0,      // bắt đầu từ nửa dưới màn hình
                groupWidth = 1920,
                groupHeight = 1080,
                activeXOffset = 0,        // full active
                activeYOffset = 0,
                activeWidth = 960,
                activeHeight = 1080,
                cols = cols,
                rows = rows
            )
            val mat = loadMatFromAssets(this@MainActivity, "chessboard_3.png")
            val radius = ChessBoardManager.detectCurvatureFromMat(
                matPtr = mat.nativeObjAddr,
                cols = 8,
                rows = 10
            )

            val mat1 = loadMatFromAssets(this@MainActivity, "chessboard_3.png")


         //   warpCurvedToFlatInPlace(mat.nativeObjAddr, radius)
        //    warpCurvedToFlatInPlace(mat.nativeObjAddr, 1000f)
            val bitmap = createBitmap(mat.cols(), mat.rows())
            Utils.matToBitmap(mat, bitmap)

            val bitmap1 = createBitmap(mat1.cols(), mat1.rows())
            Utils.matToBitmap(mat1, bitmap1)

            withContext(Dispatchers.Main) {
                binding.zoomLayout.setBitmaps(listOf(bitmap1, bitmap))
            }
        }
    }

    private
            /**
             * Loads an image from assets folder and converts it to an OpenCV Mat.
             *
             * @param context  The Android Context
             * @param fileName The file name inside assets folder (e.g. "chessboard.png")
             * @return         Mat representation of the image (BGR color order)
             */
    fun loadMatFromAssets(context: Context, fileName: String): Mat {
        val assetManager = context.assets
        assetManager.open(fileName).use { inputStream ->
            val bitmap = BitmapFactory.decodeStream(inputStream)
            val mat = Mat()
            Utils.bitmapToMat(bitmap, mat)
            // Convert from RGBA (Bitmap) → BGR (OpenCV default)
            org.opencv.imgproc.Imgproc.cvtColor(mat, mat, org.opencv.imgproc.Imgproc.COLOR_RGBA2BGR)
            return mat
        }
    }
}