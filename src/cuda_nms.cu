/**
 * cuda_nms.cu
 * Parallel CUDA NMS for DeepStream 7.1 – Jetson Orin Nano (sm_87)
 *
 * Algorithm:
 *   1. k_iou_upper  – each thread computes one (i,j) IoU pair for the
 *                     upper triangle of the N×N matrix, class-aware.
 *   2. Greedy NMS   – runs on CPU reading from cudaMallocManaged buffer.
 *                     On Jetson (shared DRAM) there is NO explicit DtoH copy.
 */

#include "cuda_nms.h"

#include <string.h>   /* memset */
#include <stdio.h>    /* fprintf */

/* ────────────────────────── Error macro ────────────────────────── */
#define CUDA_CHECK(call)                                                   \
    do {                                                                   \
        cudaError_t _e = (call);                                           \
        if (_e != cudaSuccess) {                                           \
            fprintf(stderr, "[cuda_nms] CUDA error %s:%d  %s\n",         \
                    __FILE__, __LINE__, cudaGetErrorString(_e));           \
            return _e;                                                     \
        }                                                                  \
    } while (0)

/* ────────────────────────── Kernel ─────────────────────────────── */
/**
 * k_iou_upper
 * Thread (blockIdx.y, blockIdx.x*blockDim.x + threadIdx.x) computes IoU
 * for the pair (i, j) where i = row, j = col, j > i.
 *
 * Grid launch:  grid.y = N (one block-row per detection)
 *               grid.x = ceil(N / 32)
 *               block  = (32, 1)
 *
 * Class-aware: cross-class pairs receive IoU = 0 (no suppression).
 */
__global__ void k_iou_upper(
        const NmsBox* __restrict__ B,
        float*        __restrict__ M,   /* N×N unified-memory matrix */
        int N)
{
    int i = blockIdx.y;                              /* row  */
    int j = blockIdx.x * blockDim.x + threadIdx.x;  /* col  */

    /* Only upper triangle, skip diagonal */
    if (i >= N || j >= N || j <= i) return;

    /* Different classes → IoU = 0, boxes cannot suppress each other */
    if (B[i].class_id != B[j].class_id) {
        M[i * N + j] = 0.f;
        return;
    }

    /* Intersection */
    float ix1 = fmaxf(B[i].x1, B[j].x1);
    float iy1 = fmaxf(B[i].y1, B[j].y1);
    float ix2 = fminf(B[i].x2, B[j].x2);
    float iy2 = fminf(B[i].y2, B[j].y2);
    float inter = fmaxf(0.f, ix2 - ix1) * fmaxf(0.f, iy2 - iy1);

    /* Union */
    float ai  = (B[i].x2 - B[i].x1) * (B[i].y2 - B[i].y1);
    float aj  = (B[j].x2 - B[j].x1) * (B[j].y2 - B[j].y1);
    float uni = ai + aj - inter;

    M[i * N + j] = (uni > 1e-6f) ? (inter / uni) : 0.f;
}

/* ────────────────────────── Public API ─────────────────────────── */
cudaError_t cuda_nms(
        const NmsBox*  d_boxes,
        int            n,
        float          iou_thresh,
        unsigned char* h_keep,          /* host output */
        cudaStream_t   stream)
{
    if (n <= 0) {
        return cudaSuccess;
    }
    if (n > NMS_MAX_DETECTIONS) {
        n = NMS_MAX_DETECTIONS;
    }

    /* ── Allocate IoU matrix in unified memory ──────────────────────
     * On Jetson (iGPU, shared DRAM), cudaMallocManaged avoids the
     * explicit DtoH copy after the kernel — the CPU reads directly
     * from the same physical pages the GPU wrote.
     * ────────────────────────────────────────────────────────────── */
    float* iou = nullptr;
    CUDA_CHECK(cudaMallocManaged(&iou, (size_t)n * n * sizeof(float)));

    /* Zero-init (diagonal + lower triangle are never written by kernel) */
    CUDA_CHECK(cudaMemsetAsync(iou, 0, (size_t)n * n * sizeof(float), stream));

    /* ── Phase 1: GPU parallel IoU matrix ─────────────────────────── */
    dim3 block(32, 1);
    dim3 grid((unsigned int)((n + 31) / 32), (unsigned int)n);
    k_iou_upper<<<grid, block, 0, stream>>>(d_boxes, iou, n);

    /* Sync before CPU reads the unified buffer */
    CUDA_CHECK(cudaStreamSynchronize(stream));

    /* ── Phase 2: CPU greedy suppression ──────────────────────────── */
    /* Initialise: keep everything */
    memset(h_keep, 1, (size_t)n);

    /*
     * Classic greedy NMS:
     *   For each box i (highest score first), suppress all lower-scored
     *   boxes j that overlap it by more than iou_thresh (same class
     *   already enforced in the kernel — cross-class IoU = 0).
     */
    for (int i = 0; i < n; i++) {
        if (!h_keep[i]) continue;
        for (int j = i + 1; j < n; j++) {
            if (h_keep[j] && iou[i * n + j] > iou_thresh) {
                h_keep[j] = 0;
            }
        }
    }

    CUDA_CHECK(cudaFree(iou));
    return cudaGetLastError();
}
