cmake_minimum_required(VERSION 3.10)
project(app)

set(CMAKE_CXX_STANDARD 17)

link_directories(
    /opt/nvidia/deepstream/deepstream/lib
)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    /opt/nvidia/deepstream/deepstream/sources/includes
    /usr/include/gstreamer-1.0
    /usr/include/glib-2.0
    /usr/lib/aarch64-linux-gnu/glib-2.0/include
)

# Find GStreamer and glib
find_package(PkgConfig REQUIRED)
pkg_check_modules(GST REQUIRED gstreamer-1.0 gstreamer-rtsp-server-1.0 glib-2.0)

add_executable(${PROJECT_NAME} src/main.cpp)

# Include directories and link libraries
target_include_directories(${PROJECT_NAME} PUBLIC ${GST_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PUBLIC 
    ${GST_LIBRARIES}
    nvdsgst_meta
    nvds_meta
)

# --- Build aplikasi Parser Log terpisah ---
add_executable(parser src/log_parser.cpp)
# LogParser hanya menggunakan library standar C++ bawaan, tidak perlu link GStreamer
