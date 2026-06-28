/**
 * nvds_postprocess_nms.h
 * GStreamer pad-probe that replaces DeepStream's built-in NMS with a
 * custom CUDA parallel NMS for DeepStream 7.1 – Jetson Orin Nano.
 *
 * How to integrate in your existing C/C++ DeepStream app:
 * ─────────────────────────────────────────────────────────
 *   #include "nvds_postprocess_nms.h"
 *
 *   // 1. Configure (match your pgie_yolov8n.txt values)
 *   NmsPostprocConfig cfg;
 *   cfg.gie_uid       = 1;      // gie-unique-id in pgie config
 *   cfg.net_w         = 640;    // TRT engine input width
 *   cfg.net_h         = 640;    // TRT engine input height
 *   cfg.num_classes   = 3;
 *   cfg.num_anchors   = 8400;   // 80*80+40*40+20*20 for 640x640
 *   cfg.conf_thresh   = 0.25f;
 *   cfg.nms_thresh    = 0.45f;
 *   cfg.apply_sigmoid = 1;      // 1 for standard YOLOv8 ONNX export
 *
 *   // 2. Init
 *   nms_postproc_init(&cfg);
 *
 *   // 3. Attach probe to nvinfer src pad (AFTER pipeline is built)
 *   GstElement *pgie = gst_bin_get_by_name(GST_BIN(pipeline), "primary-inference");
 *   GstPad     *pad  = gst_element_get_static_pad(pgie, "src");
 *   gst_pad_add_probe(pad, GST_PAD_PROBE_TYPE_BUFFER,
 *                     nms_postproc_probe, NULL, NULL);
 *   gst_object_unref(pad);
 *
 *   // 4. Cleanup on exit
 *   nms_postproc_deinit();
 *
 * Required pgie_yolov8n.txt changes (see config/ for full file):
 *   output-tensor-meta=1    ← enables raw tensor output to metadata
 *   cluster-mode=2          ← disables DeepStream's built-in NMS
 */

#pragma once

#include <gst/gst.h>
#include "cuda_nms.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Configuration for the NMS post-processing probe.
 * All values must match the pgie config and the exported ONNX model.
 */
typedef struct {
    int   gie_uid;        /**< Must match gie-unique-id in pgie config (default: 1) */

    /* Network / model parameters */
    int   net_w;          /**< TRT engine input width  (e.g. 640) */
    int   net_h;          /**< TRT engine input height (e.g. 640) */
    int   num_classes;    /**< Number of detection classes (e.g. 3) */
    int   num_anchors;    /**< Total anchor count (8400 for YOLOv8n 640x640) */

    /* Thresholds */
    float conf_thresh;    /**< Confidence threshold (e.g. 0.25) */
    float nms_thresh;     /**< IoU suppression threshold (e.g. 0.45) */

    /**
     * Whether to apply sigmoid to raw class logits from the ONNX tensor.
     * Set to 1 for standard `yolo.export(format='onnx', simplify=True)`.
     * Set to 0 if the model already includes sigmoid in the graph.
     */
    int   apply_sigmoid;
} NmsPostprocConfig;

/**
 * nms_postproc_init()
 * Allocates GPU resources. Must be called before attaching the probe.
 *
 * @return 1 on success, 0 on failure.
 */
int  nms_postproc_init  (const NmsPostprocConfig* cfg);

/**
 * nms_postproc_deinit()
 * Frees GPU resources. Call on application exit.
 */
void nms_postproc_deinit(void);

/**
 * nms_postproc_probe()
 * GStreamer GST_PAD_PROBE_TYPE_BUFFER callback.
 *
 * On each buffer it:
 *   1. Reads NvDsInferTensorMeta (raw YOLOv8 output) from frame user meta.
 *   2. Decodes the tensor → candidate boxes with confidence filter.
 *   3. Clears any objects DeepStream's parser already placed in frame meta.
 *   4. Uploads sorted boxes to GPU → runs parallel CUDA IoU matrix.
 *   5. Applies greedy NMS on CPU (unified memory, no DtoH copy on Jetson).
 *   6. Creates fresh NvDsObjectMeta for each kept detection.
 */
GstPadProbeReturn nms_postproc_probe(
    GstPad*          pad,
    GstPadProbeInfo* info,
    gpointer         user_data
);

#ifdef __cplusplus
}
#endif
