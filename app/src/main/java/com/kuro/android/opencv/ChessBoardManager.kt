package com.kuro.android.opencv

import android.graphics.Bitmap

object ChessBoardManager {
    init {
        System.loadLibrary("opencv_java4")
        System.loadLibrary("generate_chessboard")
    }

    external fun generateChessBoard(
        width: Int,
        height: Int,
        cols: Int,
        rows: Int,
        startX: Int,
        startY: Int
    ): Bitmap

    external fun generateChessBoardGroup(
        totalWidth: Int,
        totalHeight: Int,
        groupXOffset: Int,
        groupWidth: Int,
        groupHeight: Int,
        cols: Int,
        rows: Int
    ): Bitmap

    external fun generateChessBoardGroupWithBlackPad(
        totalWidth: Int,
        totalHeight: Int,
        groupXOffset: Int,
        groupYOffset: Int,
        groupWidth: Int,
        groupHeight: Int,
        activeXOffset: Int,
        activeYOffset: Int,
        activeWidth: Int,
        activeHeight: Int,
        cols: Int,
        rows: Int
    ): Bitmap
}