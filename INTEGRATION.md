────────────────────────────────────────────────────────────────────────
INTEGRATION GUIDE – Custom CUDA NMS for DeepStream 7.1 / YOLOv8n
Jetson Orin Nano
────────────────────────────────────────────────────────────────────────

PROJECT FILE STRUCTURE
──────────────────────
Copy the new files into your existing project layout:

  your_project/
  ├── config/
  │   ├── deepstream_app.txt          ← unchanged
  │   └── pgie_yolov8n.txt            ← REPLACE with config/pgie_yolov8n.txt
  ├── lib/
  │   ├── libnvdsinfer_custom_impl_Yolo.so   ← unchanged (existing)
  │   └── libnvds_custom_nms.so              ← NEW (built by Makefile)
  ├── models/  …                      ← unchanged
  ├── scripts/ …                      ← unchanged
  └── src/                            ← NEW directory
      ├── cuda_nms.h
      ├── cuda_nms.cu
      ├── nvds_postprocess_nms.h
      └── nvds_postprocess_nms.cpp
  Makefile                            ← NEW (place at project root, or adjust paths)


STEP 1 – Build the shared library
──────────────────────────────────
  cd your_project/
  make

This produces  lib/libnvds_custom_nms.so


STEP 2 – Replace the pgie config
─────────────────────────────────
The new config/pgie_yolov8n.txt adds two lines compared to the original:

  output-tensor-meta=1   # exposes raw ONNX output as NvDsInferTensorMeta
  cluster-mode=4         # NONE — probe handles all post-processing

Everything else (engine path, FP16 mode, custom lib, label file) is unchanged.


STEP 3A – Integrate into a C/C++ DeepStream app
────────────────────────────────────────────────
In your existing app file (e.g. deepstream_app.cpp):

  /* 1. Include the probe header */
  #include "nvds_postprocess_nms.h"

  /* 2. Before g_main_loop_run(), configure and initialise: */
  NmsPostprocConfig nms_cfg;
  nms_cfg.gie_uid       = 1;      /* must match gie-unique-id in pgie config */
  nms_cfg.net_w         = 640;    /* YOLOv8n default input width             */
  nms_cfg.net_h         = 640;    /* YOLOv8n default input height            */
  nms_cfg.num_classes   = 3;      /* your label count                        */
  nms_cfg.num_anchors   = 8400;   /* 80²+40²+20² for 640×640 input          */
  nms_cfg.conf_thresh   = 0.25f;
  nms_cfg.nms_thresh    = 0.45f;
  nms_cfg.apply_sigmoid = 1;      /* 1 for standard YOLOv8 ONNX export       */
  nms_postproc_init(&nms_cfg);

  /* 3. Get the nvinfer element and attach the probe to its src pad: */
  GstElement *pgie = gst_bin_get_by_name(GST_BIN(pipeline), "primary-inference");
  GstPad     *pad  = gst_element_get_static_pad(pgie, "src");
  gst_pad_add_probe(pad,
                    GST_PAD_PROBE_TYPE_BUFFER,
                    nms_postproc_probe,   /* from nvds_postprocess_nms.h */
                    NULL,
                    NULL);
  gst_object_unref(pad);
  gst_object_unref(pgie);

  /* 4. Run the pipeline normally */
  g_main_loop_run(loop);

  /* 5. On exit */
  nms_postproc_deinit();

Note: the element name "primary-inference" is what deepstream-app gives to
the primary GIE. If you built the pipeline manually, use whatever name you
assigned: gst_element_factory_make("nvinfer", "my-pgie").


STEP 3B – Integrate into a Python DeepStream app
─────────────────────────────────────────────────
If your app is Python (using pyds / deepstream-python-apps), the probe is
implemented in C but callable via ctypes:

  import ctypes, sys

  # Load the shared library
  nms_lib = ctypes.CDLL("lib/libnvds_custom_nms.so")

  # Define the C structs
  class NmsPostprocConfig(ctypes.Structure):
      _fields_ = [
          ("gie_uid",       ctypes.c_int),
          ("net_w",         ctypes.c_int),
          ("net_h",         ctypes.c_int),
          ("num_classes",   ctypes.c_int),
          ("num_anchors",   ctypes.c_int),
          ("conf_thresh",   ctypes.c_float),
          ("nms_thresh",    ctypes.c_float),
          ("apply_sigmoid", ctypes.c_int),
      ]

  cfg = NmsPostprocConfig(
      gie_uid=1, net_w=640, net_h=640,
      num_classes=3, num_anchors=8400,
      conf_thresh=0.25, nms_thresh=0.45,
      apply_sigmoid=1)
  nms_lib.nms_postproc_init(ctypes.byref(cfg))

  # Then use pyds to attach the native probe:
  import pyds
  pgie = pipeline.get_by_name("primary-inference")
  pgie_src_pad = pgie.get_static_pad("src")

  # Wrap the C probe as a Python callback using ctypes function pointer
  PROBE_CB = ctypes.CFUNCTYPE(ctypes.c_int,
                               ctypes.c_void_p,
                               ctypes.c_void_p,
                               ctypes.c_void_p)
  probe_fn = PROBE_CB(nms_lib.nms_postproc_probe)
  pgie_src_pad.add_probe(Gst.PadProbeType.BUFFER, probe_fn, 0)

  # On exit
  nms_lib.nms_postproc_deinit()

Alternatively, port decode_yolov8_tensor() to Python/pyds and call
cuda_nms() via ctypes for just the GPU NMS step.


STEP 4 – Verify
───────────────
Run with CUDA_LAUNCH_BLOCKING=1 on first test to catch any kernel errors:

  CUDA_LAUNCH_BLOCKING=1 ./your_app -c config/deepstream_app.txt

Expected console output:
  [NmsPostproc] Initialized
    gie_uid=1  net=640x640  classes=3  anchors=8400
    conf_thresh=0.250  nms_thresh=0.450  apply_sigmoid=1

Profile the NMS kernel:
  nsys profile --trace=cuda,nvtx ./your_app -c config/deepstream_app.txt


TUNABLES
────────
conf_thresh   Lower = more candidate boxes → more NMS work, higher recall.
              Raise to 0.35–0.5 for speed if false positives are acceptable.

nms_thresh    Lower = stricter suppression (fewer boxes).
              0.45 is standard for vehicle detection.

apply_sigmoid Set to 0 if your ONNX was exported with
              `model.export(..., simplify=False)` and already embeds sigmoid.
              Tip: inspect with `netron your_model.onnx` — look for a Sigmoid
              node on the output branch.

num_anchors   For a different YOLOv8 input size:
                640×640  → 8400   (80²+40²+20²)
                1280×1280→ 33600  (160²+80²+40²)
              Update net_w, net_h, and num_anchors accordingly and rebuild
              the TRT engine (delete yolov8n.engine to force re-export).


DATA FLOW SUMMARY
─────────────────
  Camera (2560×720, ZED2)
      │
      ▼  [nvstreammux]  batch-size=1, 2560×720
      │
      ▼  [nvinfer / pgie]
      │     • TensorRT FP16 inference  (YOLOv8n engine)
      │     • NvDsInferParseYolo decodes tensor → NvDsObjectMeta (no NMS)
      │     • output-tensor-meta=1 → raw [7×8400] tensor in user-meta
      │     • cluster-mode=4 (NONE) → no additional clustering
      │
      ▼  [nms_postproc_probe  ←  attached to nvinfer src pad]
      │     • reads NvDsInferTensorMeta (raw tensor)
      │     • clears NvDsObjectMeta written by parse function
      │     • CPU: decodes [7×8400] → candidate NmsBox list (conf filter)
      │     • CPU→GPU: upload sorted boxes
      │     • GPU: compute N×N class-aware IoU matrix in parallel (k_iou_upper)
      │     • CPU: greedy NMS on unified-memory IoU matrix (no DtoH copy)
      │     • rebuilds NvDsObjectMeta with final kept detections
      │
      ▼  [nvdsosd]   draw boxes / labels
      │
      ▼  [RTSP sink / file sink]
