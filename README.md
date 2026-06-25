# Real-Time Spatio-Temporal Object Detection using YOLOv8 and DeepStream on Jetson Orin Nano

This repository contains the source code, configurations, and implementation for my Informatics Final Project. The system utilizes an **NVIDIA Jetson Orin Nano (4GB)** to process stereo video feeds from a **Stereolabs ZED Camera**, perform high-speed AI inference using a custom-optimized **YOLOv8** model via TensorRT, and stream the processed analytical metadata over **RTSP** in a headless network environment.

---

## 📌 Project Overview
* **Hardware Platform:** NVIDIA Jetson Orin Nano Developer Kit (4GB RAM, Headless Architecture via Ethernet)
* **AI Framework:** NVIDIA DeepStream 7.1 & TensorRT
* **Model Architecture:** YOLOv8 (Nano variant, optimized to FP16)
* **Camera Input:** Stereolabs ZED Stereo Camera (handled via GStreamer V4L2 drivers)
* **Streaming Protocol:** RTSP (Real-Time Streaming Protocol) encoded in H.264 for low-latency network broadcasting

---

## 📂 Project Structure

```text
FinalProject/
├── config/                  # All configuration files (.txt, .ini)
│   ├── deepstream_app.txt   # Main headless orchestration config
│   ├── labels.txt           # Class names (one per line)
│   ├── pgie_yolov8_config.txt # TensorRT inference config for YOLOv8
│   └── sgie_custom_nms.txt  # (Future) Config for Custom NMS handling
│
├── models/                  # Raw weights and compiled engines
│   ├── yolov8n.onnx         # Original exported ONNX model
│   └── yolov8n.engine       # Compiled TensorRT Engine for Orin Nano (FP16)
│
├── src/                     # Custom C++ source code and plugins
│   └── nvdsinfer_custom_impl_Yolo/ # Bounding box custom parser repo
│       ├── Makefile
│       └── nvdsinfer_yolo_engine.cpp
│
├── lib/                     # Compiled binaries (.so files)
│   ├── libnvdsinfer_custom_impl_Yolo.so # Your custom bounding box parser
│   └── libnvds_custom_nms.so            # (Future) Custom NMS library
│
├── data/                    # Test media for evaluation & debugging
│   ├── input_sample.mp4
│   └── output_recorded.mp4
│
├── scripts/                 # Automation & maintenance shell scripts
│   ├── clean_cache.sh       # Hard reset for hung V4L2 drivers and RAM cache
│   └── setup_swap.sh        # Automates 8GB SWAP space initialization
│
└── README.md                # Project documentation
```


🚀 Prerequisites & Installation
1. Hardware Environment Setup

Since the Jetson Orin Nano 4GB variant has highly constrained physical RAM, you must setup an 8GB SWAP partition to prevent TensorRT compilation and DeepStream initialization from causing Out-Of-Memory (OOM) Segmentation Faults.

Run the automation helper script to setup memory management:
```Bash
chmod +x scripts/clean_cache.sh
./scripts/clean_cache.sh
```

2. Compile Custom YOLOv8 Parser

The custom bounding box parser needs to be compiled natively on DeepStream 7.1's underlying environment:
```Bash
cd src/nvdsinfer_custom_impl_Yolo/
make clean
CUDA_VER=12.6 make
cp libnvdsinfer_custom_impl_Yolo.so ../../lib/
```

🏃 How To Run
1. Launch DeepStream Pipeline on Jetson (Headless)

Execute the application using the main centralized configuration profile. It will automatically load the YOLOv8 TensorRT engine, process the ZED camera input, and spin up an internal RTSP server.
```Bash
cd ~/FinalProject
deepstream-app -c config/deepstream_app.txt
```

2. Consume the Live Stream on Client Machine (e.g., Arch Linux)

To watch the low-latency analytical stream remotely via the ethernet bridge, use ffplay with zero-buffering flags:
```Bash

ffplay -f rtsp -rtsp_transport udp -fflags nobuffer -flags low_delay -framedrop rtsp://<JETSON_IP_ADDRESS>:8554/ds-test
```

🛠️ Maintenance & Troubleshooting

If the GStreamer pipeline crashes unexpectedly during prototyping, the kernel V4L2 camera driver may stay locked in a frozen state. Run the maintenance script to safely force-release system resources without needing a physical hardware reboot:
Bash

./scripts/clean_cache.sh

👨‍💻 Author

    Name: Perdi

    Student ID: D121221015

    Department: Informatics Engineering
