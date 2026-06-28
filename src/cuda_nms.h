/**
 * cuda_nms.h
 * Custom parallel CUDA NMS for DeepStream 7.1 – Jetson Orin Nano (sm_87)
 *
 * Design:
 *   Phase 1 – GPU: compute upper-triangular class-aware IoU matrix in parallel
 *   Phase 2 – CPU: greedy suppression on the precomputed matrix
 *             (On Jetson unified memory, Phase 2 reads directly from GPU-written
 *              memory without an explicit DtoH transfer.)
 */

#pragma once

#include <cuda_runtime.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Maximum detections handled per frame (after confidence filter) */
#define NMS_MAX_DETECTIONS 1024

/**
 * A single detection box in x1/y1/x2/y2 format (original frame coordinates).
 * Must be sorted by score DESC before passing to cuda_nms().
 */
typedef struct {
    float x1, y1, x2, y2;
    float score;
    int   class_id;
} NmsBox;

/**
 * cuda_nms()
 *
 * Computes class-aware greedy NMS.
 * The IoU matrix is computed on the GPU (parallel); greedy suppression runs
 * on the CPU using unified memory – optimal on Jetson's shared DRAM.
 *
 * @param d_boxes    Device pointer to NmsBox array, sorted score DESC.
 *                   Length must be exactly `n` boxes.
 * @param n          Number of input boxes (clamped to NMS_MAX_DETECTIONS).
 * @param iou_thresh IoU threshold for suppression (e.g. 0.45).
 * @param h_keep     Host output array of length n.
 *                   On return: 1 = keep, 0 = suppressed.
 *                   Caller must allocate at least n bytes.
 * @param stream     CUDA stream (use 0 for default stream).
 *
 * @return cudaSuccess, or a CUDA error code.
 */
cudaError_t cuda_nms(
    const NmsBox*  d_boxes,
    int            n,
    float          iou_thresh,
    unsigned char* h_keep,
    cudaStream_t   stream
);

#ifdef __cplusplus
}
#endif
