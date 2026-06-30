/**
 * nvds_postprocess_nms.cpp
 *
 * GStreamer pad-probe implementation that:
 *   • reads raw YOLOv8n output tensor from NvDsInferTensorMeta
 *   • decodes bounding boxes (CPU, ~235 KB tensor)
 *   • applies custom CUDA parallel NMS (GPU IoU matrix + CPU greedy suppress)
 *   • rebuilds NvDsObjectMeta with the final kept detections
 *
 * Required pgie changes:
 *   output-tensor-meta=1   → nvinfer attaches raw tensors as user metadata
 *   cluster-mode=2         → DeepStream NMS is disabled; we own post-processing
 *
 * Thread safety: this probe is called from a GStreamer streaming thread.
 * All state (d_boxes, g_cfg) is module-global and must not be modified
 * while the pipeline is running.
 */

#include "nvds_postprocess_nms.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <vector>
#include <algorithm>

#include <cuda_runtime.h>

/* ── DeepStream / GStreamer headers ───────────────────────────────── */
#include "gstnvdsmeta.h"               /* gst_buffer_get_nvds_batch_meta   */
#include "nvdsmeta.h"                  /* NvDsBatchMeta, NvDsFrameMeta …   */
#include "gstnvdsinfer.h"              /* NvDsInferTensorMeta, NvDsInferLayerInfo,
                                          NVDSINFER_TENSOR_OUTPUT_META       */
#include "nvbufsurface.h"              /* NvBufSurface (for completeness)   */

/* ── Module state ──────────────────────────────────────────────────── */
static NmsPostprocConfig g_cfg;
static NmsBox*           d_boxes = nullptr;   /* device buffer for boxes  */
static bool              g_init  = false;

/* ════════════════════════════════════════════════════════════════════
 *  Init / Deinit
 * ════════════════════════════════════════════════════════════════════ */
int nms_postproc_init(const NmsPostprocConfig* cfg)
{
    if (g_init) return 1;

    g_cfg = *cfg;

    if (cudaMalloc(&d_boxes, NMS_MAX_DETECTIONS * sizeof(NmsBox)) != cudaSuccess) {
        fprintf(stderr, "[NmsPostproc] ERROR: cudaMalloc failed\n");
        return 0;
    }

    g_init = true;

    printf("[NmsPostproc] Initialized\n"
           "  gie_uid=%d  net=%dx%d  classes=%d  anchors=%d\n"
           "  conf_thresh=%.3f  nms_thresh=%.3f  apply_sigmoid=%d\n",
           g_cfg.gie_uid,
           g_cfg.net_w, g_cfg.net_h,
           g_cfg.num_classes, g_cfg.num_anchors,
           g_cfg.conf_thresh, g_cfg.nms_thresh,
           g_cfg.apply_sigmoid);

    return 1;
}

void nms_postproc_deinit(void)
{
    if (!g_init) return;
    cudaFree(d_boxes);
    d_boxes = nullptr;
    g_init  = false;
}

/* ════════════════════════════════════════════════════════════════════
 *  YOLOv8 tensor decoder  (CPU)
 *
 *  Tensor layout (standard ONNX export, channel-first, batch removed):
 *    h_tensor[ channel * num_anchors + anchor_idx ]
 *    channels: 0-3 = cx,cy,w,h (pixel coords in network input space)
 *              4..4+nc-1 = raw class logits (sigmoid applied here if needed)
 *
 *  Output: candidate NmsBox list in original frame coordinates,
 *          sorted by score DESC (required for greedy NMS).
 * ════════════════════════════════════════════════════════════════════ */
static std::vector<NmsBox> decode_yolov8_tensor(
        const float* h_tensor,   /* CPU pointer, size = (4+nc) * na    */
        int          na,         /* num_anchors  (e.g. 8400)           */
        int          nc,         /* num_classes  (e.g. 3)              */
        float        conf_thresh,
        float        src_w,      /* original frame width               */
        float        src_h,      /* original frame height              */
        float        net_w,      /* TRT engine input width  (e.g. 640) */
        float        net_h,      /* TRT engine input height (e.g. 640) */
        int          apply_sig)  /* 1 = apply sigmoid to class scores  */
{
    /* ── Letterbox geometry ── */
    float scale = std::min(net_w / src_w, net_h / src_h);
    float pad_x = (net_w - src_w * scale) * 0.5f;   /* horizontal pad */
    float pad_y = (net_h - src_h * scale) * 0.5f;   /* vertical pad   */

    std::vector<NmsBox> dets;
    dets.reserve(512);

    for (int i = 0; i < na; i++) {

        /* ── Box coordinates (channel-first layout) ── */
        float cx = h_tensor[0 * na + i];
        float cy = h_tensor[1 * na + i];
        float bw = h_tensor[2 * na + i];
        float bh = h_tensor[3 * na + i];

        /* ── Best class score ── */
        float best_score = -1e9f;
        int   best_cls   = 0;

        for (int c = 0; c < nc; c++) {
            float v = h_tensor[(4 + c) * na + i];
            if (apply_sig) {
                v = 1.f / (1.f + std::exp(-v));
            }
            if (v > best_score) {
                best_score = v;
                best_cls   = c;
            }
        }

        if (best_score < conf_thresh) continue;

        /* ── Network coords → original frame coords ── */
        /*  Remove letterbox padding, then un-scale                     */
        float x1 = ((cx - bw * 0.5f) - pad_x) / scale;
        float y1 = ((cy - bh * 0.5f) - pad_y) / scale;
        float x2 = ((cx + bw * 0.5f) - pad_x) / scale;
        float y2 = ((cy + bh * 0.5f) - pad_y) / scale;

        /* Clamp to frame boundaries */
        x1 = std::max(0.f, std::min(x1, src_w - 1.f));
        y1 = std::max(0.f, std::min(y1, src_h - 1.f));
        x2 = std::max(0.f, std::min(x2, src_w - 1.f));
        y2 = std::max(0.f, std::min(y2, src_h - 1.f));

        /* Discard degenerate boxes */
        if (x2 <= x1 || y2 <= y1) continue;

        dets.push_back({x1, y1, x2, y2, best_score, best_cls});
    }

    /* ── Sort by score DESC (greedy NMS requires this ordering) ── */
    std::sort(dets.begin(), dets.end(),
              [](const NmsBox& a, const NmsBox& b) {
                  return a.score > b.score;
              });

    return dets;
}

/* ════════════════════════════════════════════════════════════════════
 *  GStreamer pad probe callback
 * ════════════════════════════════════════════════════════════════════ */
GstPadProbeReturn nms_postproc_probe(
        GstPad*          pad,
        GstPadProbeInfo* info,
        gpointer         user_data)
{
    (void)pad;
    (void)user_data;

    if (!g_init) return GST_PAD_PROBE_OK;

    GstBuffer* buf = GST_PAD_PROBE_INFO_BUFFER(info);
    if (!buf) return GST_PAD_PROBE_OK;

    NvDsBatchMeta* bmeta = gst_buffer_get_nvds_batch_meta(buf);
    if (!bmeta) return GST_PAD_PROBE_OK;

    /* ── Process each frame in the batch ── */
    for (NvDsMetaList* fl = bmeta->frame_meta_list; fl; fl = fl->next) {

        NvDsFrameMeta* fm = (NvDsFrameMeta*)fl->data;
        if (!fm) continue;

        float src_w = (float)fm->source_frame_width;
        float src_h = (float)fm->source_frame_height;

        /* ── 1. Find NvDsInferTensorMeta from our GIE ───────────── */
        NvDsInferTensorMeta* tmeta = nullptr;

        for (NvDsMetaList* ul = fm->frame_user_meta_list; ul; ul = ul->next) {
            NvDsUserMeta* um = (NvDsUserMeta*)ul->data;
            if (!um) continue;
            if (um->base_meta.meta_type != NVDSINFER_TENSOR_OUTPUT_META) continue;

            auto* t = (NvDsInferTensorMeta*)um->user_meta_data;
            if ((int)t->unique_id == g_cfg.gie_uid) {

                tmeta = t;
                break;
            }
        }

        if (!tmeta || tmeta->num_output_layers == 0) {
            continue;   /* no tensor meta – inference didn't run this frame */
        }

        /* ── 2. Get the raw output tensor pointer ────────────────── */
        /*
         * YOLOv8n ONNX output is layer 0 ("output0").
         * We prefer the host pointer (populated on Jetson unified memory);
         * fall back to an explicit DtoH copy if only the device ptr exists.
         */
        size_t tensor_elems = (size_t)(4 + g_cfg.num_classes) * g_cfg.num_anchors;
        std::vector<float> h_tensor_buf;
        const float* h_tensor = nullptr;

        if (tmeta->out_buf_ptrs_host && tmeta->out_buf_ptrs_host[0]) {
            /* Unified / mapped memory – CPU can read directly (Jetson) */
            h_tensor = (const float*)tmeta->out_buf_ptrs_host[0];
        } else {
            /* Discrete GPU or fallback: explicit DtoH copy (~235 KB) */
            h_tensor_buf.resize(tensor_elems);
            if (cudaMemcpy(h_tensor_buf.data(),
                           tmeta->out_buf_ptrs_dev[0],
                           tensor_elems * sizeof(float),
                           cudaMemcpyDeviceToHost) != cudaSuccess) {
                fprintf(stderr, "[NmsPostproc] cudaMemcpy DtoH failed\n");
                continue;
            }
            h_tensor = h_tensor_buf.data();
        }

        /* ── 3. Decode YOLOv8 tensor → candidate boxes ──────────── */
        std::vector<NmsBox> dets = decode_yolov8_tensor(
            h_tensor,
            g_cfg.num_anchors, g_cfg.num_classes,
            g_cfg.conf_thresh,
            src_w, src_h,
            (float)g_cfg.net_w, (float)g_cfg.net_h,
            g_cfg.apply_sigmoid);

        /* ── 4. Clear any objects nvinfer's parser may have added ── */
        /*
         * Even with cluster-mode=2 (NMS disabled), the custom parse func
         * inside libnvdsinfer_custom_impl_Yolo.so may still populate
         * NvDsObjectMeta. We replace all of them with our NMS output.
         */
        {
            std::vector<NvDsObjectMeta*> to_del;
            to_del.reserve(64);

            for (NvDsMetaList* ol = fm->obj_meta_list; ol; ol = ol->next) {
                auto* obj = (NvDsObjectMeta*)ol->data;
                if (obj && (int)obj->unique_component_id == g_cfg.gie_uid)
                    to_del.push_back(obj);
            }
            for (auto* obj : to_del)
                nvds_remove_obj_meta_from_frame(fm, obj);
        }

        if (dets.empty()) continue;

        int n = (int)std::min((int)dets.size(), NMS_MAX_DETECTIONS);

        /* ── 5. Upload sorted boxes to GPU ──────────────────────── */
        cudaStream_t stream = 0;   /* default stream */

        if (cudaMemcpyAsync(d_boxes,
                            dets.data(),
                            (size_t)n * sizeof(NmsBox),
                            cudaMemcpyHostToDevice,
                            stream) != cudaSuccess) {
            fprintf(stderr, "[NmsPostproc] cudaMemcpyAsync HtoD failed\n");
            continue;
        }

        /* ── 6. CUDA parallel NMS ────────────────────────────────── */
        /*
         * cuda_nms():
         *   Phase 1 (GPU)  – compute N×N class-aware IoU matrix in parallel
         *   Phase 2 (CPU)  – greedy suppression reading from unified memory
         * Returns h_keep[i] = 1 to keep box i, 0 to suppress.
         */
        unsigned char h_keep[NMS_MAX_DETECTIONS];
        cudaError_t nms_err = cuda_nms(d_boxes, n, g_cfg.nms_thresh, h_keep, stream);

        if (nms_err != cudaSuccess) {
            fprintf(stderr, "[NmsPostproc] cuda_nms failed: %s\n",
                    cudaGetErrorString(nms_err));
            continue;
        }

        /* ── 7. Create NvDsObjectMeta for each kept detection ────── */
        int kept = 0;
        for (int k = 0; k < n; k++) {
            if (!h_keep[k]) continue;

            const NmsBox& b = dets[k];

            NvDsObjectMeta* obj = nvds_acquire_obj_meta_from_pool(bmeta);
            if (!obj) {
                fprintf(stderr, "[NmsPostproc] Object meta pool exhausted\n");
                break;
            }

            /* Identity */
            obj->unique_component_id = (guint)g_cfg.gie_uid;
            obj->confidence          = b.score;
            obj->class_id            = b.class_id;
            obj->object_id           = UNTRACKED_OBJECT_ID;

            /* Bounding box (NvOSD_RectParams uses unsigned int) */
            NvOSD_RectParams& r = obj->rect_params;
            r.left         = (unsigned int)std::max(0.f, b.x1);
            r.top          = (unsigned int)std::max(0.f, b.y1);
            r.width        = (unsigned int)(b.x2 - b.x1);
            r.height       = (unsigned int)(b.y2 - b.y1);
            r.border_width = 2;
            r.border_color = {1.0, 0.45, 0.0, 1.0};   /* orange – R,G,B,A */
            r.has_bg_color = 0;

            /* Text label */
            obj->text_params.display_text =
                g_strdup_printf("cls%d  %.2f", b.class_id, b.score);
            obj->text_params.x_offset = r.left;
            obj->text_params.y_offset = (r.top > 15u) ? r.top - 15u : 0u;
            obj->text_params.font_params.font_name  = (char*)"Serif";
            obj->text_params.font_params.font_size  = 10;
            obj->text_params.font_params.font_color = {1., 1., 1., 1.};
            obj->text_params.set_bg_clr  = 1;
            obj->text_params.text_bg_clr = {0., 0., 0., 0.5};

            nvds_add_obj_meta_to_frame(fm, obj, nullptr);
            kept++;
        }

        (void)kept;   /* suppress unused-variable warning if logging is off */
    }

    return GST_PAD_PROBE_OK;
}
