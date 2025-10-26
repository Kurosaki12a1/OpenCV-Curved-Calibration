cmake_minimum_required(VERSION 3.22.1)
project("generate_chessboard")

# Determine project root relative to this CMake file
get_filename_component(PROJECT_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../../../.." ABSOLUTE)

# Construct the OpenCV SDK path relative to project root
set(OpenCV_DIR "${PROJECT_ROOT}/opencv/sdk/native/jni")

# Find OpenCV package
find_package(OpenCV REQUIRED NO_DEFAULT_PATH)

# Include OpenCV headers
include_directories(${OpenCV_INCLUDE_DIRS})

message(STATUS "✅ OpenCV found at: ${OpenCV_DIR}")
message(STATUS "✅ Include path: ${OpenCV_INCLUDE_DIRS}")
message(STATUS "✅ Libraries: ${OpenCV_LIBS}")

add_library(${CMAKE_PROJECT_NAME}
        SHARED
        # List C/C++ source files with relative paths to this CMakeLists.txt.
        native-lib.cpp
        chessboard.cpp)

#add_library(opencv_java4 SHARED IMPORTED)
#set_target_properties(opencv_java4 PROPERTIES
#        IMPORTED_LOCATION
#        ${CMAKE_SOURCE_DIR}/../jniLibs/${ANDROID_ABI}/libopencv_java4.so
#)

target_link_libraries(${CMAKE_PROJECT_NAME}
        # List libraries link to the target library
        android
        jnigraphics
        ${OpenCV_LIBS}
        log)