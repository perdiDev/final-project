#include <gst/gst.h>
#include <gst/rtsp-server/rtsp-server.h>
#include <glib.h>
#include <iostream>

int main(int argc, char *argv[]) {
    gst_init(&argc, &argv);
    GMainLoop *loop = g_main_loop_new(NULL, FALSE);

    GstElement *pipeline = gst_pipeline_new("zed-yolo-rtsp-pipeline");
    
    // ZED Source
    GstElement *source = gst_element_factory_make("zedsrc", "zed-source");
    g_object_set(G_OBJECT(source), "stream-type", 0, NULL);

    // Bridge ZED to NVMM
    GstElement *vidconv = gst_element_factory_make("videoconvert", "cpu-vidconv");
    GstElement *caps_yuy2 = gst_element_factory_make("capsfilter", "caps-yuy2");
    GstCaps *yuy2_caps = gst_caps_from_string("video/x-raw, format=(string)YUY2");
    g_object_set(G_OBJECT(caps_yuy2), "caps", yuy2_caps, NULL);
    gst_caps_unref(yuy2_caps);

    GstElement *nvvidconv1 = gst_element_factory_make("nvvideoconvert", "nvvidconv1");
    GstElement *caps_nvmm = gst_element_factory_make("capsfilter", "caps-nvmm");
    GstCaps *nvmm_caps = gst_caps_from_string("video/x-raw(memory:NVMM), format=(string)NV12, width=(int)1280, height=(int)720");
    g_object_set(G_OBJECT(caps_nvmm), "caps", nvmm_caps, NULL);
    gst_caps_unref(nvmm_caps);

    // DeepStream elements
    GstElement *streammux = gst_element_factory_make("nvstreammux", "stream-muxer");
    g_object_set(G_OBJECT(streammux), "batch-size", 1, "width", 1280, "height", 720, "live-source", TRUE, "batched-push-timeout", 40000, NULL);

    GstElement *pgie = gst_element_factory_make("nvinfer", "primary-inference");
    // Ensure this path points to where your config actually lives
    g_object_set(G_OBJECT(pgie), "config-file-path", "/home/aimp/FinalProject/config/pgie_coco.txt", NULL);

    GstElement *nvvidconv2 = gst_element_factory_make("nvvideoconvert", "nvvidconv2");
    GstElement *nvosd = gst_element_factory_make("nvdsosd", "nv-onscreendisplay");
    
    // Convert back from NVMM to CPU memory for software encoding
    GstElement *nvvidconv3 = gst_element_factory_make("nvvideoconvert", "nvvidconv3");
    GstElement *caps_cpu = gst_element_factory_make("capsfilter", "caps-cpu");
    GstCaps *cpu_caps = gst_caps_from_string("video/x-raw, format=(string)I420");
    g_object_set(G_OBJECT(caps_cpu), "caps", cpu_caps, NULL);
    gst_caps_unref(cpu_caps);

    // CPU H.264 Encoder (since Orin Nano has no NVENC)
    GstElement *encoder = gst_element_factory_make("x264enc", "h264-encoder");
    // Tune for speed: 1 = ultrafast, 4 = zerolatency (prevents CPU bottleneck)
    g_object_set(G_OBJECT(encoder), "bitrate", 4000, "speed-preset", 1, "tune", 4, NULL);

    GstElement *rtppay = gst_element_factory_make("rtph264pay", "rtp-payer");
    
    // UDP Sink
    GstElement *sink = gst_element_factory_make("udpsink", "udp-sink");
    g_object_set(G_OBJECT(sink), "host", "127.0.0.1", "port", 5400, "async", FALSE, "sync", FALSE, NULL);

    if (!pipeline || !source || !vidconv || !caps_yuy2 || !nvvidconv1 || !caps_nvmm || !streammux || !pgie || !nvvidconv2 || !nvosd || !nvvidconv3 || !caps_cpu || !encoder || !rtppay || !sink) {
        g_printerr("Not all elements could be created.\n");
        return -1;
    }

    // Build the pipeline
    gst_bin_add_many(GST_BIN(pipeline), source, vidconv, caps_yuy2, nvvidconv1, caps_nvmm, streammux, pgie, nvvidconv2, nvosd, nvvidconv3, caps_cpu, encoder, rtppay, sink, NULL);

    // Link up to streammux
    gst_element_link_many(source, vidconv, caps_yuy2, nvvidconv1, caps_nvmm, NULL);

    // Modern syntax for requesting pad (fixes your compilation warning)
    GstPad *sinkpad = gst_element_request_pad_simple(streammux, "sink_0");
    GstPad *srcpad = gst_element_get_static_pad(caps_nvmm, "src");
    if (gst_pad_link(srcpad, sinkpad) != GST_PAD_LINK_OK) {
        g_printerr("Failed to link caps to streammux.\n");
        return -1;
    }
    gst_object_unref(sinkpad);
    gst_object_unref(srcpad);

    // Link the rest of the pipeline
    gst_element_link_many(streammux, pgie, nvvidconv2, nvosd, nvvidconv3, caps_cpu, encoder, rtppay, sink, NULL);

    // Setup RTSP Server
    GstRTSPServer *server = gst_rtsp_server_new();
    g_object_set(server, "service", "8554", NULL);
    GstRTSPMountPoints *mounts = gst_rtsp_server_get_mount_points(server);
    GstRTSPMediaFactory *factory = gst_rtsp_media_factory_new();

    gst_rtsp_media_factory_set_launch(factory,
        "( udpsrc name=pay0 port=5400 buffer-size=524288 caps=\"application/x-rtp, media=video, clock-rate=90000, encoding-name=(string)H264, payload=96 \" )");
    gst_rtsp_media_factory_set_shared(factory, TRUE);
    gst_rtsp_mount_points_add_factory(mounts, "/ds-test", factory);
    g_object_unref(mounts);
    gst_rtsp_server_attach(server, NULL);

    g_print("\n*** RTSP Stream is READY at rtsp://<Jetson-IP>:8554/ds-test ***\n\n");

    gst_element_set_state(pipeline, GST_STATE_PLAYING);
    g_main_loop_run(loop);

    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_object_unref(pipeline);
    g_main_loop_unref(loop);

    return 0;
}
