#include <gst/gst.h>
#include <gst/rtsp-server/rtsp-server.h>
#include <glib.h>
#include <signal.h>
#include <iostream>
#include <string>
#include <stdlib.h>

// Variabel Global
GMainLoop *loop = NULL;
GstElement *pipeline = NULL;

// Handler Ctrl+C
static void signal_handler(int sig) {
    g_print("\nCtrl+C ditekan. Mengirim sinyal EOS untuk menyimpan MP4 dengan aman...\n");
    if (pipeline) {
        // Mengirim sinyal End of Stream ke pipeline
        gst_element_send_event(pipeline, gst_event_new_eos());
    }
}

// Callback untuk uridecodebin (jika input dari File)
// Tugasnya: Menyambungkan output dari file video ke dalam streammuxer
static void cb_newpad(GstElement *decodebin, GstPad *pad, gpointer data) {
    GstElement *streammux = (GstElement *)data;
    GstCaps *caps = gst_pad_query_caps(pad, NULL);
    const GstStructure *str = gst_caps_get_structure(caps, 0);
    const gchar *name = gst_structure_get_name(str);

    // Jika pad yang keluar adalah video, sambungkan ke streammux
    if (g_str_has_prefix(name, "video/x-raw")) {
        GstPad *sinkpad = gst_element_request_pad_simple(streammux, "sink_0");
        if (gst_pad_link(pad, sinkpad) != GST_PAD_LINK_OK) {
            g_printerr("Gagal menyambungkan decodebin ke streammux.\n");
        }
        gst_object_unref(sinkpad);
    }
    gst_caps_unref(caps);
}

// Callback untuk mendengarkan pesan dari Bus GStreamer
static gboolean bus_call(GstBus *bus, GstMessage *msg, gpointer data) {
    GMainLoop *loop = (GMainLoop *)data;
    switch (GST_MESSAGE_TYPE(msg)) {
        case GST_MESSAGE_EOS:
            g_print("End of stream (EOS) tercapai. Menutup pipeline dengan aman...\n");
            g_main_loop_quit(loop); // Matikan program HANYA setelah file MP4 selesai ditulis
            break;
        case GST_MESSAGE_ERROR: {
            gchar *debug;
            GError *error;
            gst_message_parse_error(msg, &error, &debug);
            g_printerr("Error pada pipeline: %s\n", error->message);
            g_error_free(error);
            g_free(debug);
            g_main_loop_quit(loop);
            break;
        }
        default:
            break;
    }
    return TRUE;
}

int main(int argc, char *argv[]) {
    gst_init(&argc, &argv);

    // 1. Konfigurasi Default
    std::string config_path = "config/pgie_yolov8n.txt";
    std::string input_type = "zed";     // Opsi: "zed", "file"
    std::string input_file = "";        // Contoh: "file:///home/aimp/video.mp4"
    std::string output_type = "rtsp";   // Opsi: "rtsp", "monitor", "file"
    std::string output_file = "output.mp4";

    // 2. Parsing Argumen Dinamis
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        if (arg == "--config" && i + 1 < argc) config_path = argv[++i];
        else if (arg == "--input" && i + 1 < argc) input_type = argv[++i];
        else if (arg == "--input-file" && i + 1 < argc) input_file = argv[++i];
        else if (arg == "--output" && i + 1 < argc) output_type = argv[++i];
        else if (arg == "--output-file" && i + 1 < argc) output_file = argv[++i];
        else if (arg == "--help" || arg == "-h") {
            g_print("\nPenggunaan:\n");
            g_print("  --config <path>        : Path file config txt (Default: config/pgie_yolov8n.txt)\n");
            g_print("  --input <zed|file>     : Sumber video (Default: zed)\n");
            g_print("  --input-file <path>    : Path video jika input=file (Gunakan format URI: file:///path/ke/video.mp4)\n");
            g_print("  --output <rtsp|monitor|file> : Jenis output (Default: rtsp)\n");
            g_print("  --output-file <path>   : Nama file simpan jika output=file (Default: output.mp4)\n\n");
            return 0;
        }
    }

    g_print("=== KONFIGURASI PIPELINE ===\n");
    g_print("Config   : %s\n", config_path.c_str());
    g_print("Input    : %s %s\n", input_type.c_str(), (input_type == "file" ? input_file.c_str() : ""));
    g_print("Output   : %s %s\n", output_type.c_str(), (output_type == "file" ? output_file.c_str() : ""));
    g_print("============================\n");

    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    loop = g_main_loop_new(NULL, FALSE);

    // 3. Buat Pipeline dan Elemen Utama (Core Inference)
    pipeline = gst_pipeline_new("dynamic-yolo-pipeline");

    // -- Tambahkan kode Bus Watcher ini --
    GstBus *bus = gst_pipeline_get_bus(GST_PIPELINE(pipeline));
    guint bus_watch_id = gst_bus_add_watch(bus, bus_call, loop);
    gst_object_unref(bus);

    GstElement *streammux = gst_element_factory_make("nvstreammux", "stream-muxer");
    g_object_set(G_OBJECT(streammux), "batch-size", 1, "width", 1280, "height", 720, "batched-push-timeout", 40000, NULL);
    
    // Jika input file, bukan live source. Jika ZED, live source = TRUE.
    g_object_set(G_OBJECT(streammux), "live-source", (input_type == "zed" ? TRUE : FALSE), NULL);

    GstElement *pgie = gst_element_factory_make("nvinfer", "primary-inference");
    g_object_set(G_OBJECT(pgie), "config-file-path", config_path.c_str(), NULL);

    GstElement *nvvidconv2 = gst_element_factory_make("nvvideoconvert", "nvvidconv2");
    GstElement *nvosd = gst_element_factory_make("nvdsosd", "nv-onscreendisplay");
    GstElement *nvvidconv_out = gst_element_factory_make("nvvideoconvert", "nvvidconv_out");

    gst_bin_add_many(GST_BIN(pipeline), streammux, pgie, nvvidconv2, nvosd, nvvidconv_out, NULL);
    gst_element_link_many(streammux, pgie, nvvidconv2, nvosd, nvvidconv_out, NULL);

    // 4. Konfigurasi Blok INPUT (Source)
    if (input_type == "zed") {
        GstElement *source = gst_element_factory_make("zedsrc", "zed-source");
        g_object_set(G_OBJECT(source), "stream-type", 0, NULL);
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

        gst_bin_add_many(GST_BIN(pipeline), source, vidconv, caps_yuy2, nvvidconv1, caps_nvmm, NULL);
        gst_element_link_many(source, vidconv, caps_yuy2, nvvidconv1, caps_nvmm, NULL);

        GstPad *sinkpad = gst_element_request_pad_simple(streammux, "sink_0");
        GstPad *srcpad = gst_element_get_static_pad(caps_nvmm, "src");
        gst_pad_link(srcpad, sinkpad);
        gst_object_unref(sinkpad);
        gst_object_unref(srcpad);
    } 
    else if (input_type == "file") {
        if (input_file.empty()) {
            g_printerr("Input file path harus diisi dengan --input-file\n");
            return -1;
        }
	// --- FITUR AUTO-FIX JALUR RELATIF ---
        // Jika user tidak memasukkan "://" (misal lupa file://), ubah otomatis
        if (input_file.find("://") == std::string::npos) {
            char *abs_path = realpath(input_file.c_str(), NULL);
            if (abs_path != NULL) {
                input_file = "file://" + std::string(abs_path);
                free(abs_path);
            } else {
                g_printerr("ERROR: File video tidak ditemukan di: %s\n", input_file.c_str());
                return -1;
            }
            g_print("Auto-koreksi URI menjadi: %s\n", input_file.c_str());
        }
        // ------------------------------------
	
        GstElement *source = gst_element_factory_make("uridecodebin", "uri-decode-bin");
        g_object_set(G_OBJECT(source), "uri", input_file.c_str(), NULL);
        gst_bin_add(GST_BIN(pipeline), source);
        // Hubungkan callback agar stream video otomatis masuk ke streammux
        g_signal_connect(source, "pad-added", G_CALLBACK(cb_newpad), streammux);
    }

    // 5. Konfigurasi Blok OUTPUT (Sink)
    if (output_type == "rtsp") {
        GstElement *caps_cpu = gst_element_factory_make("capsfilter", "caps-cpu");
        GstCaps *cpu_caps = gst_caps_from_string("video/x-raw, format=(string)I420");
        g_object_set(G_OBJECT(caps_cpu), "caps", cpu_caps, NULL);
        gst_caps_unref(cpu_caps);

        GstElement *encoder = gst_element_factory_make("x264enc", "h264-encoder");
        g_object_set(G_OBJECT(encoder), "bitrate", 4000, "speed-preset", 1, "tune", 4, NULL);
        GstElement *rtppay = gst_element_factory_make("rtph264pay", "rtp-payer");
        GstElement *sink = gst_element_factory_make("udpsink", "udp-sink");
        g_object_set(G_OBJECT(sink), "host", "127.0.0.1", "port", 5400, "async", FALSE, "sync", FALSE, NULL);

        gst_bin_add_many(GST_BIN(pipeline), caps_cpu, encoder, rtppay, sink, NULL);
        gst_element_link_many(nvvidconv_out, caps_cpu, encoder, rtppay, sink, NULL);

        GstRTSPServer *server = gst_rtsp_server_new();
        g_object_set(server, "service", "8554", NULL);
        GstRTSPMountPoints *mounts = gst_rtsp_server_get_mount_points(server);
        GstRTSPMediaFactory *factory = gst_rtsp_media_factory_new();
        gst_rtsp_media_factory_set_launch(factory, "( udpsrc name=pay0 port=5400 buffer-size=524288 caps=\"application/x-rtp, media=video, clock-rate=90000, encoding-name=(string)H264, payload=96 \" )");
        gst_rtsp_media_factory_set_shared(factory, TRUE);
        gst_rtsp_mount_points_add_factory(mounts, "/ds-test", factory);
        g_object_unref(mounts);
        gst_rtsp_server_attach(server, NULL);
        g_print("*** RTSP Stream READY at rtsp://<Jetson-IP>:8554/ds-test ***\n");
    } 
    else if (output_type == "monitor") {
        GstElement *sink = gst_element_factory_make("nv3dsink", "nv-3d-sink");
        // Memastikan sync dimatikan agar tidak ada delay jika input dari kamera
        g_object_set(G_OBJECT(sink), "sync", FALSE, NULL);
        gst_bin_add(GST_BIN(pipeline), sink);
        gst_element_link(nvvidconv_out, sink);
        g_print("*** Output akan ditampilkan di Monitor ***\n");
    } 
    else if (output_type == "file") {
        GstElement *caps_cpu = gst_element_factory_make("capsfilter", "caps-cpu");
        GstCaps *cpu_caps = gst_caps_from_string("video/x-raw, format=(string)I420");
        g_object_set(G_OBJECT(caps_cpu), "caps", cpu_caps, NULL);
        gst_caps_unref(cpu_caps);

        GstElement *encoder = gst_element_factory_make("x264enc", "h264-encoder");
        g_object_set(G_OBJECT(encoder), "bitrate", 4000, "speed-preset", 1, NULL); // Tune dihilangkan untuk kualitas file
        GstElement *parse = gst_element_factory_make("h264parse", "h264-parser");
        GstElement *mux = gst_element_factory_make("qtmux", "mp4-muxer");
        GstElement *sink = gst_element_factory_make("filesink", "file-sink");
        g_object_set(G_OBJECT(sink), "location", output_file.c_str(), NULL);

        gst_bin_add_many(GST_BIN(pipeline), caps_cpu, encoder, parse, mux, sink, NULL);
        gst_element_link_many(nvvidconv_out, caps_cpu, encoder, parse, mux, sink, NULL);
        g_print("*** Output akan disimpan ke: %s ***\n", output_file.c_str());
    }

    // 6. Jalankan Pipeline
    gst_element_set_state(pipeline, GST_STATE_PLAYING);
    g_main_loop_run(loop);

    // 7. Bersihkan Resource saat program dimatikan
    g_print("Membersihkan resource...\n");
    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_object_unref(pipeline);
    g_main_loop_unref(loop);

    return 0;
}
