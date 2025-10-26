package com.kuro.android.opencv

import android.content.Context
import android.content.res.AssetManager
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


    external fun detectCurvatureFromMat(
        matPtr: Long,
        cols: Int,
        rows: Int,
        isDebug : Boolean = true
    ): Float


    external fun pixelRadiusToMeters(radiusPx: Float, pixelPitchMM: Float): Float
    external fun generateCurvatureProfile(width: Int, radiusPx: Float): FloatArray
    external fun generateCurvatureMap(width: Int, height: Int, radiusPx: Float): Long
    external fun warpCurvedToFlat(matPtr: Long, radiusPx: Float)

    external fun warpCurvedToFlatInPlace(matPtr: Long, radiusPx: Float)

}