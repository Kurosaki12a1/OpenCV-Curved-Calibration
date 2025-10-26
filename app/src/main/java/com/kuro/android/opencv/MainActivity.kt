package com.kuro.android.opencv

import android.os.Bundle
import androidx.appcompat.app.AppCompatActivity
import androidx.lifecycle.lifecycleScope
import com.kuro.android.opencv.databinding.ActivityMainBinding
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.launch
import kotlinx.coroutines.withContext

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
            withContext(Dispatchers.Main) {
                binding.zoomLayout.setBitmaps(listOf(group1, group2))
            }
        }
    }
}