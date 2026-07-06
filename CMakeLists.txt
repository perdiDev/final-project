cmake_minimum_required(VERSION 3.10)
project(DeepStreamZedyoloRTSP)

set(CMAKE_CXX_STANDARD 17)

# Find GStreamer and glib
find_package(PkgConfig REQUIRED)
pkg_check_modules(GST REQUIRED gstreamer-1.0 gstreamer-rtsp-server-1.0 glib-2.0)

add_executable(${PROJECT_NAME} src/main.cpp)

# Include directories and link libraries
target_include_directories(${PROJECT_NAME} PUBLIC ${GST_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PUBLIC ${GST_LIBRARIES})
