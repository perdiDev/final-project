cmake_minimum_required(VERSION 3.10)
project(app)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

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

# --- Parser custom untuk output EfficientNMS_TRT ---
# Library ini sengaja terpisah dari parser YOLO baseline agar kedua jalur
# inference dapat dipilih secara independen melalui file konfigurasi nvinfer.
add_library(efficientnms_parser SHARED src/efficientnms_parser.cpp)
target_include_directories(efficientnms_parser PRIVATE
    /opt/nvidia/deepstream/deepstream/sources/includes
)
file(GLOB CUDA_INCLUDE_CANDIDATES
    "/usr/local/cuda*/include"
    "/usr/local/cuda*/targets/*/include"
)
find_path(CUDA_RUNTIME_INCLUDE_DIR
    cuda_runtime_api.h
    HINTS ${CUDA_INCLUDE_CANDIDATES}
)
if(NOT CUDA_RUNTIME_INCLUDE_DIR)
    message(FATAL_ERROR
        "cuda_runtime_api.h tidak ditemukan; CUDA Toolkit diperlukan untuk parser EfficientNMS")
endif()
target_include_directories(efficientnms_parser PRIVATE
    ${CUDA_RUNTIME_INCLUDE_DIR}
)
set_target_properties(efficientnms_parser PROPERTIES
    OUTPUT_NAME "nvdsinfer_custom_impl_EfficientNMS"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/lib"
)
