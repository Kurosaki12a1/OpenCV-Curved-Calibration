#include <jni.h>
#include <string>
#include <jni.h>
#include <opencv2/opencv.hpp>
#include <android/bitmap.h>

using namespace cv;
using namespace std;
/**
 * @brief Generates a chessboard pattern image and returns it as a Bitmap.
 *
 * @param env JNI environment pointer.
 * @param instance Java object reference (unused).
 * @param width Output image width in pixels.
 * @param height Output image height in pixels.
 * @param cols Number of chessboard columns.
 * @param rows Number of chessboard rows.
 * @param startX X-coordinate (top-left) where chessboard starts.
 * @param startY Y-coordinate (top-left) where chessboard starts.
 * @return Android Bitmap containing the generated chessboard.
 */
extern "C"
JNIEXPORT jobject JNICALL
Java_com_kuro_android_opencv_ChessBoardManager_generateChessBoard(
        JNIEnv *env,
        jobject instance,
        jint width,
        jint height,
        jint cols, jint rows,
        jint startX,
        jint startY
) {
    // Create a white canvas
    Mat chessboard(height, width, CV_8UC3, Scalar(255, 255, 255));


    // Use the smaller side to ensure perfect squares (optional)
    double cellWidth  = static_cast<double>(width  - startX) / cols;
    double cellHeight = static_cast<double>(height - startY) / rows;

    // Draw the pattern
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            if ((i + j) % 2 == 0) {
                int x0 = static_cast<int>(startX + j * cellWidth);
                int y0 = static_cast<int>(startY + i * cellHeight);
                int x1 = static_cast<int>(startX + (j + 1) * cellWidth);
                int y1 = static_cast<int>(startY + (i + 1) * cellHeight);

                rectangle(chessboard, Point(x0, y0), Point(x1, y1),
                          Scalar(0, 0, 0), FILLED);
            }
        }
    }

    // Convert to RGBA for Android Bitmap
    Mat rgba;
    cvtColor(chessboard, rgba, COLOR_BGR2RGBA);

    // ==== Create Android Bitmap ====
    jclass bitmapCls = env->FindClass("android/graphics/Bitmap");
    jmethodID createBitmapMID = env->GetStaticMethodID(
            bitmapCls,
            "createBitmap",
            "(IILandroid/graphics/Bitmap$Config;)Landroid/graphics/Bitmap;"
    );

    jclass bitmapConfigCls = env->FindClass("android/graphics/Bitmap$Config");
    jfieldID argb8888FID = env->GetStaticFieldID(
            bitmapConfigCls,
            "ARGB_8888",
            "Landroid/graphics/Bitmap$Config;"
    );

    jobject argb8888Obj = env->GetStaticObjectField(bitmapConfigCls, argb8888FID);
    jobject bitmap = env->CallStaticObjectMethod(
            bitmapCls,
            createBitmapMID,
            width,
            height,
            argb8888Obj
    );

    // ==== Copy pixels to Bitmap ====
    AndroidBitmapInfo info;
    void *pixels = nullptr;
    AndroidBitmap_getInfo(env, bitmap, &info);
    AndroidBitmap_lockPixels(env, bitmap, &pixels);

    memcpy(pixels, rgba.data, width * height * 4);
    AndroidBitmap_unlockPixels(env, bitmap);

    return bitmap;
}


/**
 * Generate a chessboard segment for a given cabinet group.
 * Each group knows its offset in the global layout.
 *
 * @param totalWidth  total combined width of all groups
 * @param totalHeight total height (should be same for all groups)
 * @param groupXOffset offset of this group (in pixels)
 * @param groupWidth  width of this group
 * @param groupHeight height of this group
 * @param cols total number of chessboard columns (across all groups)
 * @param rows total number of chessboard rows
 */
extern "C"
JNIEXPORT jobject JNICALL
Java_com_kuro_android_opencv_ChessBoardManager_generateChessBoardGroup(
        JNIEnv *env,
        jobject instance,
        jint totalWidth,
        jint totalHeight,
        jint groupXOffset,
        jint groupWidth,
        jint groupHeight,
        jint cols,
        jint rows
) {
    // Create white canvas for this group
    Mat chessboard(groupHeight, groupWidth, CV_8UC3, Scalar(255, 255, 255));

    // Compute global cell sizes
    double cellWidth = static_cast<double>(totalWidth) / cols;
    double cellHeight = static_cast<double>(totalHeight) / rows;

    // For each visible cell that overlaps this group
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            if ((i + j) % 2 == 0) {
                // Compute global coordinates
                double gx0 = j * cellWidth;
                double gx1 = (j + 1) * cellWidth;
                double gy0 = i * cellHeight;
                double gy1 = (i + 1) * cellHeight;

                // Intersection region with this group
                double localX0 = std::max(0.0, gx0 - groupXOffset);
                double localX1 = std::min(static_cast<double>(groupWidth), gx1 - groupXOffset);

                // Only draw if the cell intersects this group horizontally
                if (localX1 > 0 && localX0 < groupWidth) {
                    rectangle(chessboard,
                              Point(static_cast<int>(localX0), static_cast<int>(gy0)),
                              Point(static_cast<int>(localX1), static_cast<int>(gy1)),
                              Scalar(0, 0, 0), FILLED);
                }
            }
        }
    }

    // Convert to RGBA
    Mat rgba;
    cvtColor(chessboard, rgba, COLOR_BGR2RGBA);

    // ==== Create Android Bitmap ====
    jclass bitmapCls = env->FindClass("android/graphics/Bitmap");
    jmethodID createBitmapMID = env->GetStaticMethodID(
            bitmapCls,
            "createBitmap",
            "(IILandroid/graphics/Bitmap$Config;)Landroid/graphics/Bitmap;"
    );

    jclass bitmapConfigCls = env->FindClass("android/graphics/Bitmap$Config");
    jfieldID argb8888FID = env->GetStaticFieldID(
            bitmapConfigCls,
            "ARGB_8888",
            "Landroid/graphics/Bitmap$Config;"
    );
    jobject argb8888Obj = env->GetStaticObjectField(bitmapConfigCls, argb8888FID);

    jobject bitmap = env->CallStaticObjectMethod(
            bitmapCls, createBitmapMID, groupWidth, groupHeight, argb8888Obj
    );

    // Copy pixels
    AndroidBitmapInfo info;
    void *pixels = nullptr;
    AndroidBitmap_getInfo(env, bitmap, &info);
    AndroidBitmap_lockPixels(env, bitmap, &pixels);
    memcpy(pixels, rgba.data, groupWidth * groupHeight * 4);
    AndroidBitmap_unlockPixels(env, bitmap);

    return bitmap;
}

/**
 * Generates a chessboard pattern for one LED group with black background.
 *
 * The generated bitmap represents a single group (cabinet cluster) within the
 * global LED wall layout. Only the active region (the physical LED area) is
 * filled with the chessboard pattern; all other areas remain black.
 *
 * @param env              JNI environment pointer.
 * @param instance         Java object calling this native method.
 * @param totalWidth       Total layout width (sum of all groups in global layout).
 * @param totalHeight      Total layout height (global layout height).
 * @param groupXOffset     Horizontal offset of this group in the global layout (pixels).
 * @param groupYOffset     Vertical offset of this group in the global layout (pixels).
 * @param groupWidth       Full canvas width of this group (pixels).
 * @param groupHeight      Full canvas height of this group (pixels).
 * @param activeXOffset    X offset (inside this group) where LED panels actually exist.
 * @param activeYOffset    Y offset (inside this group) where LED panels actually exist.
 * @param activeWidth      Width of the active LED region within the group.
 * @param activeHeight     Height of the active LED region within the group.
 * @param cols             Number of chessboard columns across the entire layout.
 * @param rows             Number of chessboard rows across the entire layout.
 * @return                 A Java Bitmap (ARGB_8888) containing the generated pattern.
 */
extern "C"
JNIEXPORT jobject JNICALL
Java_com_kuro_android_opencv_ChessBoardManager_generateChessBoardGroupWithBlackPad(
        JNIEnv *env,
        jobject instance,
        jint totalWidth,
        jint totalHeight,
        jint groupXOffset,
        jint groupYOffset,
        jint groupWidth,
        jint groupHeight,
        jint activeXOffset,
        jint activeYOffset,
        jint activeWidth,
        jint activeHeight,
        jint cols,
        jint rows
) {
    // --- 1️⃣ Create full black canvas for group
    Mat chessboard(groupHeight, groupWidth, CV_8UC3, Scalar(0, 0, 0));

    // --- 2️⃣ Global cell size
    double cellWidth  = static_cast<double>(totalWidth) / cols;
    double cellHeight = static_cast<double>(totalHeight) / rows;

    // --- 3️⃣ Draw only inside active region
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            if ((i + j) % 2 == 0) {
                double gx0 = j * cellWidth;
                double gx1 = (j + 1) * cellWidth;
                double gy0 = i * cellHeight;
                double gy1 = (i + 1) * cellHeight;

                // Compute local coordinates (relative to group)
                double localX0 = gx0 - groupXOffset;
                double localX1 = gx1 - groupXOffset;
                double localY0 = gy0 - groupYOffset;
                double localY1 = gy1 - groupYOffset;

                // Clamp to active region inside group
                double drawX0 = std::max(localX0, (double)activeXOffset);
                double drawX1 = std::min(localX1, (double)(activeXOffset + activeWidth));
                double drawY0 = std::max(localY0, (double)activeYOffset);
                double drawY1 = std::min(localY1, (double)(activeYOffset + activeHeight));

                // Only draw if overlap with active region
                if (drawX1 > drawX0 && drawY1 > drawY0) {
                    rectangle(
                            chessboard,
                            Point(static_cast<int>(drawX0), static_cast<int>(drawY0)),
                            Point(static_cast<int>(drawX1), static_cast<int>(drawY1)),
                            Scalar(255, 255, 255),
                            FILLED
                    );
                }
            }
        }
    }

    // --- 4️⃣ Convert to RGBA
    Mat rgba;
    cvtColor(chessboard, rgba, COLOR_BGR2RGBA);

    // --- 5️⃣ Create Android Bitmap
    jclass bitmapCls = env->FindClass("android/graphics/Bitmap");
    jmethodID createBitmapMID = env->GetStaticMethodID(
            bitmapCls,
            "createBitmap",
            "(IILandroid/graphics/Bitmap$Config;)Landroid/graphics/Bitmap;"
    );

    jclass bitmapConfigCls = env->FindClass("android/graphics/Bitmap$Config");
    jfieldID argb8888FID = env->GetStaticFieldID(
            bitmapConfigCls,
            "ARGB_8888",
            "Landroid/graphics/Bitmap$Config;"
    );
    jobject argb8888Obj = env->GetStaticObjectField(bitmapConfigCls, argb8888FID);

    jobject bitmap = env->CallStaticObjectMethod(
            bitmapCls, createBitmapMID,
            groupWidth, groupHeight, argb8888Obj
    );

    // --- 6️⃣ Copy pixels
    AndroidBitmapInfo info;
    void *pixels = nullptr;
    AndroidBitmap_getInfo(env, bitmap, &info);
    AndroidBitmap_lockPixels(env, bitmap, &pixels);
    memcpy(pixels, rgba.data, groupWidth * groupHeight * 4);
    AndroidBitmap_unlockPixels(env, bitmap);

    return bitmap;
}
