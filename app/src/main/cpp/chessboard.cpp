#include <jni.h>
#include <string>
#include <jni.h>
#include <opencv2/opencv.hpp>
#include <android/bitmap.h>
#include <android/log.h>
#include <vector>
#include <cmath>

using namespace cv;
using namespace std;

#define LOG_TAG "ChessboardDetector"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

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


/**
 * Detects the geometric curvature (bending) of a displayed chessboard pattern
 * within an image represented by a cv::Mat.
 *
 * Steps:
 *   1. Convert the image to grayscale.
 *   2. Detect chessboard corners (findChessboardCorners).
 *   3. Refine the corners for subpixel precision.
 *   4. Fit a 2nd-degree polynomial (y = ax² + bx + c) to each row of corners.
 *   5. Compute the curvature radius from the fitted "a" coefficient.
 *
 * @param matPtr Address mat
 * @param cols   Number of chessboard inner corners horizontally.
 * @param rows   Number of chessboard inner corners vertically.
 * @param debug  If true, saves a debug image with drawn corners to /sdcard/Download/.
 * @return       Mean curvature radius in pixels (positive float). -1.0f if failed.
 */
extern "C"
JNIEXPORT jfloat JNICALL
Java_com_kuro_android_opencv_ChessBoardManager_detectCurvatureFromMat(
        JNIEnv *env,
        jobject instance,
        jlong matPtr,
        int cols,
        int rows,
        jboolean debug
) {
    cv::Mat &img = *(cv::Mat *)matPtr;
    if (img.empty()) {
        LOGE("Input Mat is empty!");
        return -1.0f;
    }

    // 1️⃣ Convert to grayscale
    Mat gray;
    if (img.channels() == 3)
        cvtColor(img, gray, COLOR_BGR2GRAY);
    else if (img.channels() == 4)
        cvtColor(img, gray, COLOR_RGBA2GRAY);
    else
        gray = img.clone();

    // 2️⃣ Find chessboard corners
    Size patternSize(cols, rows);
    vector<Point2f> corners;
    bool found = findChessboardCorners(gray, patternSize, corners,
                                       CALIB_CB_ADAPTIVE_THRESH + CALIB_CB_NORMALIZE_IMAGE);

    if (!found) {
        LOGE("Chessboard not found in image.");
        return -1.0f;
    }

    // 3️⃣ Refine detected corners
    cornerSubPix(gray, corners, Size(11, 11), Size(-1, -1),
                 TermCriteria(TermCriteria::EPS + TermCriteria::MAX_ITER, 30, 0.1));

    // 4️⃣ (Optional) Debug visualization
    if (debug) {
        Mat vis = img.clone();
        drawChessboardCorners(vis, patternSize, corners, found);
        imwrite("/sdcard/Download/debug_chessboard_detected.jpg", vis);
        LOGE("Saved debug chessboard overlay.");
    }

    // 5️⃣ Compute curvature along each row
    vector<double> curvatures;
    for (int r = 0; r < rows; ++r) {
        vector<Point2f> rowPts;
        for (int c = 0; c < cols; ++c)
            rowPts.push_back(corners[r * cols + c]);

        // Fit y = ax² + bx + c
        int n = (int) rowPts.size();
        if (n < 3) continue;

        Mat A(n, 3, CV_64F);
        Mat Y(n, 1, CV_64F);
        for (int i = 0; i < n; ++i) {
            A.at<double>(i, 0) = rowPts[i].x * rowPts[i].x;
            A.at<double>(i, 1) = rowPts[i].x;
            A.at<double>(i, 2) = 1.0;
            Y.at<double>(i, 0) = rowPts[i].y;
        }

        Mat coef;
        solve(A, Y, coef, DECOMP_NORMAL);

        double a = coef.at<double>(0);
        if (fabs(a) > 1e-9) {
            double radius = 1.0 / (2.0 * fabs(a));
            curvatures.push_back(radius);
        }
    }

    if (curvatures.empty()) {
        LOGE("No valid curvature rows detected.");
        return -1.0f;
    }

    // 6️⃣ Compute mean curvature radius
    double meanRadius = 0.0;
    for (double r: curvatures) meanRadius += r;
    meanRadius /= curvatures.size();

    LOGE("Mean curvature radius = %.2f px", meanRadius);
    return static_cast<float>(meanRadius);
}

extern "C"
JNIEXPORT jfloat JNICALL
Java_com_kuro_android_opencv_ChessBoardManager_pixelRadiusToMeters(
        JNIEnv* env,
        jobject /*thiz*/,
        jfloat radiusPx,
        jfloat pixelPitchMM
) {
    /**
     * Converts a curvature radius from pixels to meters.
     *
     * @param radiusPx       Radius of curvature in pixels.
     * @param pixelPitchMM   Physical pixel pitch in millimeters.
     * @return               Radius of curvature in meters.
     */
    if (radiusPx <= 0.0f) return -1.0f;
    float radiusMeters = (radiusPx * pixelPitchMM) / 1000.0f;
    return radiusMeters;
}

extern "C"
JNIEXPORT jfloatArray JNICALL
Java_com_kuro_android_opencv_ChessBoardManager_generateCurvatureProfile(
        JNIEnv* env,
        jobject /*thiz*/,
        jint width,
        jfloat radiusPx
) {
    /**
     * Generates curvature profile (height deviation along x-axis).
     *
     * @param width      Image width in pixels.
     * @param radiusPx   Radius of curvature (pixels).
     * @return           Java float[] of z(x) values.
     */
    std::vector<float> profile(width);
    float half = width / 2.0f;
    for (int i = 0; i < width; ++i) {
        float x = (i - half);
        profile[i] = radiusPx - sqrtf(radiusPx * radiusPx - x * x);
    }

    jfloatArray jProfile = env->NewFloatArray(width);
    env->SetFloatArrayRegion(jProfile, 0, width, profile.data());
    return jProfile;
}

extern "C"
JNIEXPORT jlong JNICALL
Java_com_kuro_android_opencv_ChessBoardManager_generateCurvatureMap(
        JNIEnv* env,
        jobject /*thiz*/,
        jint width,
        jint height,
        jfloat radiusPx
) {
    /**
     * Generates a 2D curvature height map (CV_32F Mat).
     * Each pixel represents z(x) deviation based on curvature radius.
     *
     * @param width     Image width.
     * @param height    Image height.
     * @param radiusPx  Curvature radius in pixels.
     * @return          Native pointer (jlong) to cv::Mat curvature map.
     */
    cv::Mat map(height, width, CV_32F);
    float half = width / 2.0f;

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            float dx = (x - half);
            float z = radiusPx - sqrtf(radiusPx * radiusPx - dx * dx);
            map.at<float>(y, x) = z;
        }
    }

    cv::normalize(map, map, 0, 1, cv::NORM_MINMAX);
    cv::Mat *matPtr = new cv::Mat(map);
    return reinterpret_cast<jlong>(matPtr);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_kuro_android_opencv_ChessBoardManager_warpCurvedToFlat(
        JNIEnv* env,
        jobject /*thiz*/,
        jlong matPtr,
        jfloat radiusPx
) {
    /**
     * Warps a curved image into a flat projection using sinusoidal remap.
     *
     * @param matPtr    Pointer to input cv::Mat (curved image).
     * @param radiusPx  Radius of curvature (pixels).
     * @return          Pointer to new cv::Mat (flattened image).
     */
    cv::Mat &mat = *(cv::Mat *) matPtr;
    int width = mat.cols;
    int height = mat.rows;
    float half = width / 2.0f;

    cv::Mat mapX(height, width, CV_32F);
    cv::Mat mapY(height, width, CV_32F);

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            float theta = (x - half) / radiusPx;
            mapX.at<float>(y, x) = radiusPx * sinf(theta) + half;
            mapY.at<float>(y, x) = y;
        }
    }

    cv::remap(mat.clone(), mat, mapX, mapY, cv::INTER_LINEAR);
}

/**
 * Warps a curved image to a flat projection, modifying the original Mat in-place.
 *
 * This function assumes the screen curvature follows a circular arc (1D curvature along width).
 * It generates a mapping based on the provided radius, then remaps the input image so that
 * curved pixels are repositioned as if the screen were flat.
 *
 * @param env      JNI environment.
 * @param thiz     Java instance (unused).
 * @param matAddr  Native address of cv::Mat to warp (modified in-place).
 * @param radiusPx Radius of curvature in pixels. Larger → less curvature.
 */
extern "C"
JNIEXPORT void JNICALL
Java_com_kuro_android_opencv_ChessBoardManager_warpCurvedToFlatInPlace(
        JNIEnv *env,
        jobject instance,
        jlong matAddr,
        jfloat radiusPx
) {
    // --- Validate inputs ---
    cv::Mat &mat = *(cv::Mat *) matAddr;
    if (mat.empty()) {
        LOGE("Input Mat is empty!");
        return;
    }
    if (radiusPx <= 0.0f) {
        LOGE("Invalid radius: %.2f", radiusPx);
        return;
    }

    const int width = mat.cols;
    const int height = mat.rows;
    const float cx = width / 2.0f;  // optical center (horizontal middle)

    LOGI("Warping image %dx%d with radius = %.2f px", width, height, radiusPx);

    // --- 1️⃣ Generate remap matrices (mapX, mapY) ---
    cv::Mat mapX(height, width, CV_32F);
    cv::Mat mapY(height, width, CV_32F);

    // Optional exaggeration factor for debugging visualization
    const float visualScale = 1.0f; // < 1.0 to exaggerate curvature visually (e.g. 0.1f)

    for (int y = 0; y < height; ++y) {
        float *ptrX = mapX.ptr<float>(y);
        float *ptrY = mapY.ptr<float>(y);

        for (int x = 0; x < width; ++x) {
            // Convert pixel to angular displacement θ along curved surface
            float theta = (x - cx) / (radiusPx * visualScale);

            // Compute new flat-space X coordinate using cylindrical projection
            float flatX = radiusPx * sinf(theta) + cx;
            ptrX[x] = flatX;
            ptrY[x] = (float) y; // No vertical distortion assumed
        }
    }

    // --- 2️⃣ Remap curved image to flat projection ---
    cv::Mat srcClone = mat.clone();
    cv::remap(srcClone, mat, mapX, mapY, cv::INTER_LINEAR, BORDER_CONSTANT, Scalar(0, 0, 0));

    LOGI("Warp completed successfully.");
}
