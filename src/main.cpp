#include <gst/gst.h>
#include <gst/rtsp-server/rtsp-server.h>
#include <glib-unix.h>
#include <glib.h>

#include <nvdsmeta.h>
#include <gstnvdsmeta.h>
#include <nvll_osd_struct.h>

#include <algorithm>
#include <array>
#include <chrono>
#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <exception>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <initializer_list>
#include <memory>
#include <new>
#include <string>
#include <thread>
#include <unordered_map>
#include <utility>
#include <vector>
#include <mutex>

namespace {

using SteadyClock = std::chrono::steady_clock;

constexpr guint kStreamWidth = 1280;
constexpr guint kStreamHeight = 720;
constexpr guint kStreamMuxBatchSize = 1;
constexpr guint kStreamMuxTimeoutUs = 40000;
constexpr guint kDefaultZedCameraFps = 30;
constexpr guint kUdpPort = 5400;
constexpr guint kRtspPort = 8554;
constexpr guint kEncoderBitrateBps = 4'000'000;
constexpr guint kSoftwareEncoderBitrateKbps = 4000;
constexpr guint kTrackerWidth = 640;
constexpr guint kTrackerHeight = 384;
constexpr guint kEosShutdownTimeoutSeconds = 10;
constexpr guint kBenchmarkFlushIntervalRecords = 256;
constexpr char kRtspMountPoint[] = "/ds-test";
constexpr char kTrackerLibraryPath[] =
    "/opt/nvidia/deepstream/deepstream/lib/libnvds_nvmultiobjecttracker.so";
constexpr char kTrackerConfigDirectory[] = "config";
constexpr char kTrackerConfigPrefix[] = "tracker_";

struct BenchmarkData {
    guint64 frameNumber{0};
    double mediaPtsMs{-1.0};
    double elapsedMs{0.0};
    double fps{0.0};
    double latencyMs{0.0};
    double latencyPreMuxMs{0.0};
    double latencyMuxMs{0.0};
    double latencyInferMs{0.0};
    double latencyTrackerMs{0.0};
    double latencyPreOsdMs{0.0};
    double latencyOsdMs{0.0};
    double latencyOutputMs{0.0};
    gint64 wallClockUs{0};
};

enum class InputMode {
    Zed,
    File,
};

enum class OutputMode {
    Rtsp,
    Monitor,
    File,
};

struct AppConfig {
    std::string inferenceConfigPath{"config/pgie_yolov8n.txt"};
    std::string trackerConfigPath;
    InputMode inputMode{InputMode::Zed};
    guint zedCameraFps{kDefaultZedCameraFps};
    std::string inputFile;
    OutputMode outputMode{OutputMode::Rtsp};
    std::string outputFile{"output.mp4"};
    bool benchmarkEnabled{false};
    std::string benchmarkFile{"benchmark_result.txt"};
};

enum class ParseResult {
    Run,
    Help,
    Error,
};

const char *toString(InputMode mode) {
    return mode == InputMode::Zed ? "zed" : "file";
}

const char *toString(OutputMode mode) {
    switch (mode) {
        case OutputMode::Rtsp:
            return "rtsp";
        case OutputMode::Monitor:
            return "monitor";
        case OutputMode::File:
            return "file";
    }
    return "unknown";
}

using TrackerConfigEntry = std::pair<std::string, std::string>;

std::vector<TrackerConfigEntry> listTrackerConfigs() {
    namespace fs = std::filesystem;
    std::vector<TrackerConfigEntry> configs;
    std::error_code error;
    const fs::path directory(kTrackerConfigDirectory);
    if (!fs::is_directory(directory, error)) {
        return configs;
    }

    for (const fs::directory_entry &entry : fs::directory_iterator(directory, error)) {
        if (error || !entry.is_regular_file(error)) {
            continue;
        }

        const fs::path path = entry.path();
        const std::string filename = path.filename().string();
        const bool isYaml = path.extension() == ".yml" || path.extension() == ".yaml";
        if (!isYaml || filename.rfind(kTrackerConfigPrefix, 0) != 0) {
            continue;
        }

        const std::string name = path.stem().string().substr(std::strlen(kTrackerConfigPrefix));
        if (!name.empty()) {
            configs.emplace_back(name, path.string());
        }
    }

    std::sort(configs.begin(), configs.end(),
              [](const TrackerConfigEntry &left, const TrackerConfigEntry &right) {
                  return left.first < right.first;
              });
    return configs;
}



bool resolveTrackerConfig(AppConfig &config) {
    namespace fs = std::filesystem;
    const std::vector<TrackerConfigEntry> configs = listTrackerConfigs();

    if (!config.trackerConfigPath.empty()) {
        std::error_code error;
        if (!fs::is_regular_file(fs::path(config.trackerConfigPath), error)) {
            g_printerr("Config tracker tidak ditemukan: %s\n", config.trackerConfigPath.c_str());
            return false;
        }
        return true;
    }

    if (configs.empty()) {
        g_printerr("Tidak ada config tracker di %s/tracker_*.yml atau *.yaml.\n",
                   kTrackerConfigDirectory);
        return false;
    }

    const auto defaultConfig = std::find_if(
        configs.begin(), configs.end(),
        [](const TrackerConfigEntry &entry) { return entry.first == "nvdcf"; });
    const TrackerConfigEntry &selected =
        defaultConfig != configs.end() ? *defaultConfig : configs.front();
    config.trackerConfigPath = selected.second;
    return true;
}

void printUsage(const char *programName) {
    const std::vector<TrackerConfigEntry> configs = listTrackerConfigs();
    g_print("\nPenggunaan: %s [opsi]\n", programName);
    g_print("  --config <path>              : Config nvinfer/YOLO (default: config/pgie_yolov8n.txt)\n");
    g_print("  --tracker <path>              : Path file YAML tracker (default dipilih dari config/tracker_*.yml)\n");
    g_print("  --input <zed|file>           : Sumber video (default: zed)\n");
    g_print("  --camera-fps <fps>           : FPS kamera ZED: 15, 30, 60, 100, 120 (default: 30)\n");
    g_print("  --input-file <path|URI>      : Video jika input=file\n");
    g_print("  --output <rtsp|monitor|file> : Jenis output (default: rtsp)\n");
    g_print("  --output-file <path>         : File MP4 jika output=file (default: output.mp4)\n");
    g_print("  --benchmark [path]           : Aktifkan CSV benchmark (default: benchmark_result.txt)\n");
    g_print("  --help, -h                   : Tampilkan bantuan ini\n\n");
}

bool readRequiredValue(int argc, char *argv[], int &index, const std::string &option,
                       std::string &value) {
    if (index + 1 >= argc || argv[index + 1][0] == '-') {
        g_printerr("Opsi %s membutuhkan sebuah nilai.\n", option.c_str());
        return false;
    }

    value = argv[++index];
    return true;
}

ParseResult parseArguments(int argc, char *argv[], AppConfig &config) {
    for (int i = 1; i < argc; ++i) {
        const std::string argument = argv[i];

        if (argument == "--help" || argument == "-h") {
            printUsage(argv[0]);
            return ParseResult::Help;
        }

        if (argument == "--benchmark") {
            config.benchmarkEnabled = true;
            if (i + 1 < argc && argv[i + 1][0] != '-') {
                config.benchmarkFile = argv[++i];
            }
            continue;
        }

        std::string value;
        if (argument == "--config") {
            if (!readRequiredValue(argc, argv, i, argument, value)) {
                return ParseResult::Error;
            }
            config.inferenceConfigPath = std::move(value);
        } else if (argument == "--tracker") {
            if (!readRequiredValue(argc, argv, i, argument, value)) {
                return ParseResult::Error;
            }
            config.trackerConfigPath = std::move(value);
        } else if (argument == "--input") {
            if (!readRequiredValue(argc, argv, i, argument, value)) {
                return ParseResult::Error;
            }
            if (value == "zed") {
                config.inputMode = InputMode::Zed;
            } else if (value == "file") {
                config.inputMode = InputMode::File;
            } else {
                g_printerr("Input tidak dikenal: %s (gunakan zed atau file).\n", value.c_str());
                return ParseResult::Error;
            }
        } else if (argument == "--camera-fps") {
            if (!readRequiredValue(argc, argv, i, argument, value)) {
                return ParseResult::Error;
            }
            if (value == "15") {
                config.zedCameraFps = 15;
            } else if (value == "30") {
                config.zedCameraFps = 30;
            } else if (value == "60") {
                config.zedCameraFps = 60;
            } else if (value == "100") {
                config.zedCameraFps = 100;
            } else if (value == "120") {
                config.zedCameraFps = 120;
            } else {
                g_printerr("FPS kamera ZED tidak didukung: %s (gunakan 15, 30, 60, 100, atau 120).\n",
                           value.c_str());
                return ParseResult::Error;
            }
        } else if (argument == "--input-file") {
            if (!readRequiredValue(argc, argv, i, argument, config.inputFile)) {
                return ParseResult::Error;
            }
        } else if (argument == "--output") {
            if (!readRequiredValue(argc, argv, i, argument, value)) {
                return ParseResult::Error;
            }
            if (value == "rtsp") {
                config.outputMode = OutputMode::Rtsp;
            } else if (value == "monitor") {
                config.outputMode = OutputMode::Monitor;
            } else if (value == "file") {
                config.outputMode = OutputMode::File;
            } else {
                g_printerr("Output tidak dikenal: %s (gunakan rtsp, monitor, atau file).\n",
                           value.c_str());
                return ParseResult::Error;
            }
        } else if (argument == "--output-file") {
            if (!readRequiredValue(argc, argv, i, argument, config.outputFile)) {
                return ParseResult::Error;
            }
        } else {
            g_printerr("Opsi tidak dikenal: %s\n", argument.c_str());
            return ParseResult::Error;
        }
    }

    if (config.inputMode == InputMode::File && config.inputFile.empty()) {
        g_printerr("Input file harus diisi dengan --input-file.\n");
        return ParseResult::Error;
    }

    if (!resolveTrackerConfig(config)) {
        return ParseResult::Error;
    }

    return ParseResult::Run;
}

class DeepStreamApplication {
public:
    explicit DeepStreamApplication(AppConfig config) : config_(std::move(config)) {}

    ~DeepStreamApplication() {
        cleanup();
    }

    DeepStreamApplication(const DeepStreamApplication &) = delete;
    DeepStreamApplication &operator=(const DeepStreamApplication &) = delete;

    int run() {
        printConfiguration();

        mainLoop_ = g_main_loop_new(nullptr, FALSE);
        if (mainLoop_ == nullptr) {
            g_printerr("Gagal membuat GLib main loop.\n");
            return EXIT_FAILURE;
        }

        if (!buildPipeline() || !installEventSources() || !startBenchmarkLogger()) {
            return EXIT_FAILURE;
        }

        resetMetrics();
        const GstStateChangeReturn stateResult =
            gst_element_set_state(pipeline_, GST_STATE_PLAYING);
        if (stateResult == GST_STATE_CHANGE_FAILURE) {
            g_printerr("Gagal menjalankan pipeline.\n");
            return EXIT_FAILURE;
        }

        g_print("Menjalankan pipeline...\n");
        g_main_loop_run(mainLoop_);
        return pipelineError_ ? EXIT_FAILURE : EXIT_SUCCESS;
    }

private:
    struct ComponentTimestamps {
        SteadyClock::time_point muxIn;
        SteadyClock::time_point inferIn;
        SteadyClock::time_point trackerIn;
        SteadyClock::time_point preOsdIn;
        SteadyClock::time_point osdIn;
        SteadyClock::time_point outputIn;
        SteadyClock::time_point outputOut;
    };

    struct MetricsState {
        SteadyClock::time_point fpsWindowStart{SteadyClock::now()};
        SteadyClock::time_point benchmarkStart{SteadyClock::now()};
        guint frameCount{0};
        double currentFps{0.0};
    };

    void printConfiguration() const {
        g_print("=== KONFIGURASI PIPELINE ===\n");
        g_print("Config   : %s\n", config_.inferenceConfigPath.c_str());
        g_print("Input    : %s %s\n", toString(config_.inputMode),
                config_.inputMode == InputMode::File ? config_.inputFile.c_str() : "");
        if (config_.inputMode == InputMode::Zed) {
            g_print("Camera FPS: %u\n", config_.zedCameraFps);
        }
        g_print("Output   : %s %s\n", toString(config_.outputMode),
                config_.outputMode == OutputMode::File ? config_.outputFile.c_str() : "");
        g_print("Tracker  : %s\n", config_.trackerConfigPath.c_str());
        g_print("============================\n");
    }

    GstElement *createElement(const char *factoryName, const char *elementName) {
        GstElement *element = gst_element_factory_make(factoryName, elementName);
        if (element == nullptr) {
            g_printerr("Gagal membuat elemen GStreamer '%s' (factory: %s).\n", elementName,
                       factoryName);
            return nullptr;
        }

        if (!gst_bin_add(GST_BIN(pipeline_), element)) {
            g_printerr("Gagal menambahkan elemen '%s' ke pipeline.\n", elementName);
            gst_object_unref(element);
            return nullptr;
        }

        return element;
    }

    bool addCreatedElement(GstElement *element, const char *factoryName) {
        if (element == nullptr) {
            return false;
        }

        if (!gst_bin_add(GST_BIN(pipeline_), element)) {
            g_printerr("Gagal menambahkan elemen dari factory '%s' ke pipeline.\n", factoryName);
            gst_object_unref(element);
            return false;
        }
        return true;
    }

    bool linkElements(std::initializer_list<GstElement *> elements) const {
        GstElement *previous = nullptr;
        for (GstElement *element : elements) {
            if (element == nullptr) {
                return false;
            }
            if (previous != nullptr && !gst_element_link(previous, element)) {
                g_printerr("Gagal menghubungkan '%s' ke '%s'.\n", GST_ELEMENT_NAME(previous),
                           GST_ELEMENT_NAME(element));
                return false;
            }
            previous = element;
        }
        return true;
    }

    bool setCaps(GstElement *capsFilter, const char *capsDescription) const {
        GstCaps *caps = gst_caps_from_string(capsDescription);
        if (caps == nullptr) {
            g_printerr("Caps GStreamer tidak valid: %s\n", capsDescription);
            return false;
        }

        g_object_set(G_OBJECT(capsFilter), "caps", caps, nullptr);
        gst_caps_unref(caps);
        return true;
    }

    bool buildPipeline() {
        pipeline_ = gst_pipeline_new("dynamic-yolo-pipeline");
        if (pipeline_ == nullptr) {
            g_printerr("Gagal membuat pipeline GStreamer.\n");
            return false;
        }

        GstBus *bus = gst_pipeline_get_bus(GST_PIPELINE(pipeline_));
        if (bus == nullptr) {
            g_printerr("Gagal mengambil bus GStreamer.\n");
            return false;
        }
        busWatchId_ = gst_bus_add_watch(bus, &DeepStreamApplication::onBusMessage, this);
        gst_object_unref(bus);
        if (busWatchId_ == 0) {
            g_printerr("Gagal memasang GStreamer bus watch.\n");
            return false;
        }

        GstElement *streamMux = createElement("nvstreammux", "stream-muxer");
        GstElement *primaryInference = createElement("nvinfer", "primary-inference");

	GstElement *queueInfer = createElement("queue", "queue-infer");
        GstElement *queueTracker = createElement("queue", "queue-tracker");
        GstElement *queueOsd = createElement("queue", "queue-osd");
        GstElement *tracker = createElement("nvtracker", "object-tracker");
        GstElement *preOsdConverter =
            createElement("nvvideoconvert", "pre-osd-converter");
        GstElement *osd = createElement("nvdsosd", "nv-onscreendisplay");
        GstElement *outputConverter =
            createElement("nvvideoconvert", "output-converter");
        if (streamMux == nullptr || primaryInference == nullptr || tracker == nullptr ||
            preOsdConverter == nullptr || osd == nullptr || outputConverter == nullptr || queueInfer == nullptr ||
	    queueTracker == nullptr || queueOsd == nullptr) {
            return false;
        }

        streamMux_ = streamMux;
        g_object_set(G_OBJECT(streamMux_), "batch-size", kStreamMuxBatchSize, "width",
                     kStreamWidth, "height", kStreamHeight, "batched-push-timeout",
                     kStreamMuxTimeoutUs, "live-source",
                     config_.inputMode == InputMode::Zed ? TRUE : FALSE, nullptr);

        // The DeepStream nvinfer configuration still owns the YOLO parser, custom library,
        // model, labels, and TensorRT engine settings.
        g_object_set(G_OBJECT(primaryInference), "config-file-path",
                     config_.inferenceConfigPath.c_str(), nullptr);
        g_object_set(G_OBJECT(tracker), "tracker-width", kTrackerWidth, "tracker-height",
                     kTrackerHeight, "gpu-id", 0U, "ll-lib-file", kTrackerLibraryPath,
                     "ll-config-file", config_.trackerConfigPath.c_str(),
                     "display-tracking-id", TRUE, nullptr);

        if (!linkElements(
                {streamMux_, primaryInference, queueInfer, tracker, queueTracker, preOsdConverter, osd, queueOsd, outputConverter})) {
            return false;
        }
        if (!buildInput()) {
            return false;
        }
        if (!buildOutput(outputConverter)) {
            return false;
        }
        return installProbes(primaryInference, tracker, preOsdConverter, osd, outputConverter);
    }

    bool requestStreamMuxPad() {
        streamMuxSinkPad_ = gst_element_request_pad_simple(streamMux_, "sink_0");
        if (streamMuxSinkPad_ == nullptr) {
            g_printerr("Gagal meminta pad sink_0 dari nvstreammux.\n");
            return false;
        }
        return true;
    }

    bool buildInput() {
        if (!requestStreamMuxPad()) {
            return false;
        }

        if (config_.inputMode == InputMode::Zed) {
            return buildZedInput();
        }
        return buildFileInput();
    }

    bool buildZedInput() {
        GstElement *source = createElement("zedsrc", "zed-source");
        GstElement *cpuConverter = createElement("videoconvert", "zed-cpu-converter");
        GstElement *yuy2Caps = createElement("capsfilter", "zed-yuy2-caps");
        GstElement *nvmmConverter =
            createElement("nvvideoconvert", "zed-nvmm-converter");
        GstElement *nvmmCaps = createElement("capsfilter", "zed-nvmm-caps");
        if (source == nullptr || cpuConverter == nullptr || yuy2Caps == nullptr ||
            nvmmConverter == nullptr || nvmmCaps == nullptr) {
            return false;
        }

        g_object_set(G_OBJECT(source), "stream-type", 0, "camera-fps",
                     static_cast<gint>(config_.zedCameraFps), nullptr);
        if (!setCaps(yuy2Caps, "video/x-raw, format=(string)YUY2") ||
            !setCaps(nvmmCaps,
                     "video/x-raw(memory:NVMM), format=(string)NV12, width=(int)1280, "
                     "height=(int)720") ||
            !linkElements({source, cpuConverter, yuy2Caps, nvmmConverter, nvmmCaps})) {
            return false;
        }

        GstPad *sourcePad = gst_element_get_static_pad(nvmmCaps, "src");
        if (sourcePad == nullptr) {
            g_printerr("Gagal mengambil source pad dari input ZED.\n");
            return false;
        }

        const GstPadLinkReturn linkResult = gst_pad_link(sourcePad, streamMuxSinkPad_);
        gst_object_unref(sourcePad);
        if (linkResult != GST_PAD_LINK_OK) {
            g_printerr("Gagal menghubungkan input ZED ke nvstreammux: %s.\n",
                       gst_pad_link_get_name(linkResult));
            return false;
        }
        return true;
    }

    bool normalizeInputUri() {
        if (config_.inputFile.find("://") != std::string::npos) {
            return true;
        }

        std::unique_ptr<char, decltype(&std::free)> absolutePath(
            realpath(config_.inputFile.c_str(), nullptr), &std::free);
        if (!absolutePath) {
            g_printerr("File video tidak ditemukan: %s\n", config_.inputFile.c_str());
            return false;
        }

        GError *error = nullptr;
        gchar *uri = gst_filename_to_uri(absolutePath.get(), &error);
        if (uri == nullptr) {
            g_printerr("Gagal mengubah path video menjadi URI: %s\n",
                       error != nullptr ? error->message : "unknown error");
            g_clear_error(&error);
            return false;
        }

        config_.inputFile = uri;
        g_free(uri);
        g_print("Auto-koreksi URI menjadi: %s\n", config_.inputFile.c_str());
        return true;
    }

    bool buildFileInput() {
        if (!normalizeInputUri()) {
            return false;
        }

        GstElement *source = createElement("uridecodebin", "uri-decode-bin");
        GstElement *converter =
            createElement("nvvideoconvert", "file-input-nvmm-converter");
        GstElement *nvmmCaps = createElement("capsfilter", "file-input-nvmm-caps");
        if (source == nullptr || converter == nullptr || nvmmCaps == nullptr) {
            return false;
        }

        if (!setCaps(nvmmCaps, "video/x-raw(memory:NVMM), format=(string)NV12") ||
            !linkElements({converter, nvmmCaps})) {
            return false;
        }

        fileDecodeSinkPad_ = gst_element_get_static_pad(converter, "sink");
        GstPad *sourcePad = gst_element_get_static_pad(nvmmCaps, "src");
        if (fileDecodeSinkPad_ == nullptr || sourcePad == nullptr) {
            g_printerr("Gagal mengambil pad konverter untuk input file.\n");
            if (sourcePad != nullptr) {
                gst_object_unref(sourcePad);
            }
            return false;
        }

        const GstPadLinkReturn linkResult = gst_pad_link(sourcePad, streamMuxSinkPad_);
        gst_object_unref(sourcePad);
        if (linkResult != GST_PAD_LINK_OK) {
            g_printerr("Gagal menghubungkan konverter input file ke nvstreammux: %s.\n",
                       gst_pad_link_get_name(linkResult));
            return false;
        }

        g_object_set(G_OBJECT(source), "uri", config_.inputFile.c_str(), nullptr);
        g_signal_connect(source, "pad-added", G_CALLBACK(&DeepStreamApplication::onDecodePadAdded),
                         this);
        return true;
    }

    bool createEncoder(GstElement *&capsFilter, GstElement *&encoder, bool lowLatency) {
        capsFilter = createElement("capsfilter", "encoder-input-caps");
        if (capsFilter == nullptr) {
            return false;
        }

        encoder = gst_element_factory_make("nvv4l2h264enc", "h264-encoder");
        if (encoder != nullptr) {
            if (!addCreatedElement(encoder, "nvv4l2h264enc")) {
                encoder = nullptr;
                return false;
            }
            if (!setCaps(capsFilter,
                         "video/x-raw(memory:NVMM), format=(string)NV12, width=(int)1280, "
                         "height=(int)720")) {
                return false;
            }
            g_object_set(G_OBJECT(encoder), "bitrate", kEncoderBitrateBps, "insert-sps-pps",
                         TRUE, nullptr);
            g_print("Menggunakan hardware encoder Jetson nvv4l2h264enc.\n");
            return true;
        }

        g_printerr("nvv4l2h264enc tidak tersedia; menggunakan fallback x264enc.\n");
        encoder = gst_element_factory_make("x264enc", "h264-encoder");
        if (!addCreatedElement(encoder, "x264enc")) {
            encoder = nullptr;
            return false;
        }
        if (!setCaps(capsFilter,
                     "video/x-raw, format=(string)I420, width=(int)1280, height=(int)720")) {
            return false;
        }

        if (lowLatency) {
            g_object_set(G_OBJECT(encoder), "bitrate", kSoftwareEncoderBitrateKbps,
                         "speed-preset", 1, "tune", 4, nullptr);
        } else {
            g_object_set(G_OBJECT(encoder), "bitrate", kSoftwareEncoderBitrateKbps,
                         "speed-preset", 1, nullptr);
        }
        return true;
    }

    bool buildOutput(GstElement *outputConverter) {
        switch (config_.outputMode) {
            case OutputMode::Rtsp:
                return buildRtspOutput(outputConverter);
            case OutputMode::Monitor:
                return buildMonitorOutput(outputConverter);
            case OutputMode::File:
                return buildFileOutput(outputConverter);
        }
        return false;
    }

    bool buildRtspOutput(GstElement *outputConverter) {
        GstElement *encoderCaps = nullptr;
        GstElement *encoder = nullptr;
        if (!createEncoder(encoderCaps, encoder, true)) {
            return false;
        }

        GstElement *rtpPayloader = createElement("rtph264pay", "rtp-payloader");
        GstElement *udpSink = createElement("udpsink", "udp-sink");
        if (rtpPayloader == nullptr || udpSink == nullptr) {
            return false;
        }

        g_object_set(G_OBJECT(rtpPayloader), "pt", 96, "config-interval", 1, nullptr);
        g_object_set(G_OBJECT(udpSink), "host", "127.0.0.1", "port", kUdpPort, "async",
                     FALSE, "sync", FALSE, nullptr);
        if (!linkElements({outputConverter, encoderCaps, encoder, rtpPayloader, udpSink})) {
            return false;
        }

        return startRtspServer();
    }

    bool startRtspServer() {
        rtspServer_ = gst_rtsp_server_new();
        if (rtspServer_ == nullptr) {
            g_printerr("Gagal membuat RTSP server.\n");
            return false;
        }

        const std::string service = std::to_string(kRtspPort);
        g_object_set(G_OBJECT(rtspServer_), "service", service.c_str(), nullptr);

        GstRTSPMountPoints *mounts = gst_rtsp_server_get_mount_points(rtspServer_);
        GstRTSPMediaFactory *factory = gst_rtsp_media_factory_new();
        if (mounts == nullptr || factory == nullptr) {
            g_printerr("Gagal membuat RTSP mount point atau media factory.\n");
            if (mounts != nullptr) {
                g_object_unref(mounts);
            }
            if (factory != nullptr) {
                g_object_unref(factory);
            }
            return false;
        }

        gst_rtsp_media_factory_set_launch(
            factory,
            "( udpsrc name=pay0 port=5400 buffer-size=524288 "
            "caps=\"application/x-rtp,media=video,clock-rate=90000,"
            "encoding-name=H264,payload=96\" )");
        gst_rtsp_media_factory_set_shared(factory, TRUE);
        gst_rtsp_mount_points_add_factory(mounts, kRtspMountPoint, factory);
        g_object_unref(mounts);

        rtspServerSourceId_ = gst_rtsp_server_attach(rtspServer_, nullptr);
        if (rtspServerSourceId_ == 0) {
            g_printerr("Gagal memasang RTSP server pada GLib main context.\n");
            return false;
        }

        g_print("*** RTSP Stream READY at rtsp://<Jetson-IP>:%u%s ***\n", kRtspPort,
                kRtspMountPoint);
        return true;
    }

    bool buildMonitorOutput(GstElement *outputConverter) {
        GstElement *sink = createElement("nv3dsink", "monitor-sink");
        if (sink == nullptr) {
            return false;
        }

        g_object_set(G_OBJECT(sink), "sync", FALSE, nullptr);
        if (!linkElements({outputConverter, sink})) {
            return false;
        }

        g_print("*** Output akan ditampilkan di Monitor ***\n");
        return true;
    }

    bool buildFileOutput(GstElement *outputConverter) {
        GstElement *encoderCaps = nullptr;
        GstElement *encoder = nullptr;
        if (!createEncoder(encoderCaps, encoder, false)) {
            return false;
        }

        GstElement *parser = createElement("h264parse", "h264-parser");
        GstElement *muxer = createElement("qtmux", "mp4-muxer");
        GstElement *sink = createElement("filesink", "file-sink");
        if (parser == nullptr || muxer == nullptr || sink == nullptr) {
            return false;
        }

        g_object_set(G_OBJECT(sink), "location", config_.outputFile.c_str(), nullptr);
        if (!linkElements({outputConverter, encoderCaps, encoder, parser, muxer, sink})) {
            return false;
        }

        g_print("*** Output akan disimpan ke: %s ***\n", config_.outputFile.c_str());
        return true;
    }

    bool addProbe(GstPad *pad, GstPadProbeCallback callback) {
        if (pad == nullptr) {
            return false;
        }
        gulong probeId = gst_pad_add_probe(pad, GST_PAD_PROBE_TYPE_BUFFER, callback, this, nullptr);
        if (probeId == 0) {
            return false;
        }
        gst_object_ref(pad);
        probes_.push_back({pad, probeId});
        return true;
    }

    bool addProbe(GstElement *element, const char *padName, GstPadProbeCallback callback) {
        GstPad *pad = gst_element_get_static_pad(element, padName);
        if (pad == nullptr) {
            g_printerr("Gagal mengambil pad %s dari elemen %s.\n", padName, GST_ELEMENT_NAME(element));
            return false;
        }
        const bool res = addProbe(pad, callback);
        gst_object_unref(pad);
        return res;
    }

    bool installProbes(GstElement *primaryInference, GstElement *tracker, GstElement *preOsdConverter, GstElement *osd, GstElement *outputConverter) {
        if (!addProbe(streamMuxSinkPad_, &DeepStreamApplication::onMuxSinkProbe)) return false;
        if (!addProbe(primaryInference, "sink", &DeepStreamApplication::onInferSinkProbe)) return false;
        if (!addProbe(tracker, "sink", &DeepStreamApplication::onTrackerSinkProbe)) return false;
        if (!addProbe(preOsdConverter, "sink", &DeepStreamApplication::onPreOsdSinkProbe)) return false;
        if (!addProbe(osd, "sink", &DeepStreamApplication::onOsdBuffer)) return false;
        if (!addProbe(outputConverter, "sink", &DeepStreamApplication::onOutputSinkProbe)) return false;
        if (!addProbe(outputConverter, "src", &DeepStreamApplication::onOutputSrcProbe)) return false;
        return true;
    }

    bool installEventSources() {
        sigintSourceId_ =
            g_unix_signal_add(SIGINT, &DeepStreamApplication::onShutdownSignal, this);
        sigtermSourceId_ =
            g_unix_signal_add(SIGTERM, &DeepStreamApplication::onShutdownSignal, this);
        if (sigintSourceId_ == 0 || sigtermSourceId_ == 0) {
            g_printerr("Gagal memasang signal handler GLib.\n");
            return false;
        }
        return true;
    }

    bool startBenchmarkLogger() {
        if (!config_.benchmarkEnabled) {
            return true;
        }

        loggerQueue_ = g_async_queue_new();
        if (loggerQueue_ == nullptr) {
            g_printerr("Gagal membuat queue benchmark.\n");
            return false;
        }

        try {
            loggerThread_ = std::thread(&DeepStreamApplication::benchmarkLoggerWorker, this);
        } catch (const std::exception &error) {
            g_printerr("Gagal memulai thread benchmark: %s\n", error.what());
            g_async_queue_unref(loggerQueue_);
            loggerQueue_ = nullptr;
            return false;
        }

        g_print("\n=== BENCHMARK AKTIF ===\n");
        g_print("Menyimpan log ke: %s\n\n", config_.benchmarkFile.c_str());
        return true;
    }

    void benchmarkLoggerWorker() {
        std::ifstream existingFile(config_.benchmarkFile, std::ios::binary);
        const bool needsHeader =
            !existingFile.is_open() || existingFile.peek() == std::ifstream::traits_type::eof();
        existingFile.close();

        std::ofstream logFile(config_.benchmarkFile, std::ios::app);
        if (!logFile.is_open()) {
            g_printerr("Gagal membuka file benchmark: %s\n", config_.benchmarkFile.c_str());
        } else {
            if (needsHeader) {
                logFile << "Timestamp,Frame_Number,Media_PTS_ms,Elapsed_ms,FPS,Latency_ms,"
                        << "Lat_PreMux_ms,Lat_Mux_ms,Lat_Infer_ms,Lat_Tracker_ms,Lat_PreOSD_ms,Lat_OSD_ms,Lat_Output_ms\n";
            }
            logFile << std::fixed << std::setprecision(3);
        }

        bool writeErrorReported = false;
        guint recordsSinceFlush = 0;
        while (true) {
            gpointer item = g_async_queue_pop(loggerQueue_);
            if (item == &loggerStopToken_) {
                if (logFile.is_open()) {
                    logFile.flush();
                }
                break;
            }

            std::unique_ptr<BenchmarkData> data(static_cast<BenchmarkData *>(item));
            if (!logFile.is_open()) {
                continue;
            }

            logFile << formatTimestamp(data->wallClockUs) << ',' << data->frameNumber << ','
                    << data->mediaPtsMs << ',' << data->elapsedMs << ',' << data->fps << ','
                    << data->latencyMs << ',' << data->latencyPreMuxMs << ',' << data->latencyMuxMs << ',' 
                    << data->latencyInferMs << ',' << data->latencyTrackerMs << ',' << data->latencyPreOsdMs << ',' 
                    << data->latencyOsdMs << ',' << data->latencyOutputMs << '\n';
            ++recordsSinceFlush;
            if (recordsSinceFlush >= kBenchmarkFlushIntervalRecords) {
                logFile.flush();
                recordsSinceFlush = 0;
            }
            if (!logFile && !writeErrorReported) {
                g_printerr("Gagal menulis data benchmark ke: %s\n",
                           config_.benchmarkFile.c_str());
                writeErrorReported = true;
            }
        }
    }

    void stopBenchmarkLogger() {
        if (loggerQueue_ == nullptr) {
            return;
        }

        if (loggerThread_.joinable()) {
            g_async_queue_push(loggerQueue_, &loggerStopToken_);
            loggerThread_.join();
        }
        g_async_queue_unref(loggerQueue_);
        loggerQueue_ = nullptr;
    }

    void resetMetrics() {
        const auto now = SteadyClock::now();
        metrics_.fpsWindowStart = now;
        metrics_.benchmarkStart = now;
        metrics_.frameCount = 0;
        metrics_.currentFps = 0.0;
    }

    static std::string formatTimestamp(gint64 wallClockUs) {
        const std::time_t seconds =
            static_cast<std::time_t>(wallClockUs / G_USEC_PER_SEC);
        std::tm localTime{};
        if (localtime_r(&seconds, &localTime) == nullptr) {
            return {};
        }

        std::array<char, 32> dateTime{};
        if (std::strftime(dateTime.data(), dateTime.size(), "%Y-%m-%d %H:%M:%S", &localTime) ==
            0) {
            return {};
        }

        const gint64 milliseconds = (wallClockUs % G_USEC_PER_SEC) / 1000;
        std::array<char, 40> timestamp{};
        std::snprintf(timestamp.data(), timestamp.size(), "%s.%03lld", dateTime.data(),
                      static_cast<long long>(milliseconds));
        return timestamp.data();
    }

    void recordTimestamp(GstBuffer *buffer, SteadyClock::time_point ComponentTimestamps::*field) {
        if (buffer == nullptr) return;
        NvDsBatchMeta *batchMeta = gst_buffer_get_nvds_batch_meta(buffer);
        if (batchMeta == nullptr) return;

        auto now = SteadyClock::now();
        std::lock_guard<std::mutex> lock(timestampsMutex_);
        for (NvDsMetaList *l = batchMeta->frame_meta_list; l != nullptr; l = l->next) {
            NvDsFrameMeta *frameMeta = static_cast<NvDsFrameMeta *>(l->data);
            if (frameMeta && GST_CLOCK_TIME_IS_VALID(frameMeta->buf_pts)) {
                (timestampsMap_[frameMeta->buf_pts].*field) = now;
            }
        }
    }

    GstPadProbeReturn processOsdBuffer(GstPadProbeInfo *info) {
        GstBuffer *buffer = GST_PAD_PROBE_INFO_BUFFER(info);
        recordTimestamp(buffer, &ComponentTimestamps::osdIn);
        if (buffer == nullptr) {
            return GST_PAD_PROBE_OK;
        }

        NvDsBatchMeta *batchMeta = gst_buffer_get_nvds_batch_meta(buffer);
        if (batchMeta == nullptr) {
            return GST_PAD_PROBE_OK;
        }

        const auto now = SteadyClock::now();
        metrics_.frameCount += batchMeta->num_frames_in_batch;
        const auto fpsWindowMs =
            std::chrono::duration_cast<std::chrono::milliseconds>(now - metrics_.fpsWindowStart)
                .count();
        if (fpsWindowMs >= 1000) {
            metrics_.currentFps = (metrics_.frameCount * 1000.0) / fpsWindowMs;
            metrics_.frameCount = 0;
            metrics_.fpsWindowStart = now;
        }

        for (NvDsMetaList *frameNode = batchMeta->frame_meta_list; frameNode != nullptr;
             frameNode = frameNode->next) {
            auto *frameMeta = static_cast<NvDsFrameMeta *>(frameNode->data);
            NvDsDisplayMeta *displayMeta = nvds_acquire_display_meta_from_pool(batchMeta);
            if (frameMeta == nullptr || displayMeta == nullptr) {
                continue;
            }

            displayMeta->num_labels = 1;
            NvOSD_TextParams &text = displayMeta->text_params[0];
            std::array<char, 64> fpsText{};
            std::snprintf(fpsText.data(), fpsText.size(), "FPS: %.2f", metrics_.currentFps);
            text.display_text = g_strdup(fpsText.data());
            text.x_offset = 20;
            text.y_offset = 20;
            text.font_params.font_name = const_cast<char *>("Arial");
            text.font_params.font_size = 14;
            text.font_params.font_color = {1.0, 1.0, 1.0, 1.0};
            text.set_bg_clr = 1;
            text.text_bg_clr = {0.0, 0.0, 0.0, 0.5};
            nvds_add_display_meta_to_frame(frameMeta, displayMeta);
        }

        return GST_PAD_PROBE_OK;
    }

    GstPadProbeReturn processOutputSrcBuffer(GstPadProbeInfo *info) {
        auto now = SteadyClock::now();

        if (!config_.benchmarkEnabled || loggerQueue_ == nullptr) {
            return GST_PAD_PROBE_OK;
        }

        GstBuffer *buffer = GST_PAD_PROBE_INFO_BUFFER(info);
        if (buffer == nullptr) return GST_PAD_PROBE_OK;

        NvDsBatchMeta *batchMeta = gst_buffer_get_nvds_batch_meta(buffer);
        if (batchMeta == nullptr) return GST_PAD_PROBE_OK;

        NvDsFrameLatencyInfo latencyInfo[kStreamMuxBatchSize]{};
        const guint measuredFrames = nvds_measure_buffer_latency(buffer, latencyInfo);
        const double elapsedMs = std::chrono::duration<double, std::milli>(now - metrics_.benchmarkStart).count();
        const gint64 wallClockUs = g_get_real_time();

        guint latencyIndex = 0;
        for (NvDsMetaList *frameNode = batchMeta->frame_meta_list; frameNode != nullptr;
             frameNode = frameNode->next) {
            auto *frameMeta = static_cast<NvDsFrameMeta *>(frameNode->data);
            if (frameMeta == nullptr) {
                continue;
            }

            ComponentTimestamps ts;
            if (GST_CLOCK_TIME_IS_VALID(frameMeta->buf_pts)) {
                std::lock_guard<std::mutex> lock(timestampsMutex_);
                auto it = timestampsMap_.find(frameMeta->buf_pts);
                if (it != timestampsMap_.end()) {
                    ts = it->second;
                    ts.outputOut = now;
                    timestampsMap_.erase(it);
                }
                if (timestampsMap_.size() > 1000) {
                    timestampsMap_.clear();
                }
            }

            auto delta = [](SteadyClock::time_point end, SteadyClock::time_point start) {
                if (end.time_since_epoch().count() == 0 || start.time_since_epoch().count() == 0) return 0.0;
                double val = std::chrono::duration<double, std::milli>(end - start).count();
                return val > 0 ? val : 0.0;
            };

            double latMux = delta(ts.inferIn, ts.muxIn);
            double latInfer = delta(ts.trackerIn, ts.inferIn);
            double latTracker = delta(ts.preOsdIn, ts.trackerIn);
            double latPreOsd = delta(ts.osdIn, ts.preOsdIn);
            double latOsd = delta(ts.outputIn, ts.osdIn);
            double latOutput = delta(ts.outputOut, ts.outputIn);

            const guint currentLatencyIndex = latencyIndex++;
            double currentLatency = currentLatencyIndex < measuredFrames ? latencyInfo[currentLatencyIndex].latency : 0.0;
            
            double sumComponents = latMux + latInfer + latTracker + latPreOsd + latOsd + latOutput;
            double latPreMux = currentLatency - sumComponents;
            if (latPreMux < 0) latPreMux = 0;

            auto *data = new (std::nothrow) BenchmarkData;
            if (data == nullptr) {
                continue;
            }

            data->frameNumber = frameMeta->frame_num;
            data->mediaPtsMs = GST_CLOCK_TIME_IS_VALID(frameMeta->buf_pts)
                                   ? static_cast<double>(frameMeta->buf_pts) / GST_MSECOND
                                   : -1.0;
            data->elapsedMs = elapsedMs;
            data->fps = metrics_.currentFps;
            data->latencyMs = currentLatency;
            data->latencyPreMuxMs = latPreMux;
            data->latencyMuxMs = latMux;
            data->latencyInferMs = latInfer;
            data->latencyTrackerMs = latTracker;
            data->latencyPreOsdMs = latPreOsd;
            data->latencyOsdMs = latOsd;
            data->latencyOutputMs = latOutput;
            data->wallClockUs = wallClockUs;
            
            g_async_queue_push(loggerQueue_, data);
        }

        return GST_PAD_PROBE_OK;
    }

    void processDecodePad(GstPad *newPad) {
        if (fileDecodeSinkPad_ == nullptr || gst_pad_is_linked(fileDecodeSinkPad_)) {
            return;
        }

        GstCaps *caps = gst_pad_get_current_caps(newPad);
        if (caps == nullptr) {
            caps = gst_pad_query_caps(newPad, nullptr);
        }
        if (caps == nullptr || gst_caps_is_empty(caps) || gst_caps_is_any(caps)) {
            if (caps != nullptr) {
                gst_caps_unref(caps);
            }
            return;
        }

        const GstStructure *structure = gst_caps_get_structure(caps, 0);
        const gchar *mediaType = structure != nullptr ? gst_structure_get_name(structure) : nullptr;
        const bool isRawVideo = mediaType != nullptr && g_str_has_prefix(mediaType, "video/x-raw");
        gst_caps_unref(caps);
        if (!isRawVideo) {
            return;
        }

        const GstPadLinkReturn linkResult = gst_pad_link(newPad, fileDecodeSinkPad_);
        if (linkResult != GST_PAD_LINK_OK) {
            g_printerr("Gagal menghubungkan decodebin ke konverter NVMM: %s.\n",
                       gst_pad_link_get_name(linkResult));
        }
    }

    gboolean processBusMessage(GstMessage *message) {
        switch (GST_MESSAGE_TYPE(message)) {
            case GST_MESSAGE_EOS:
                removeSource(eosShutdownTimeoutId_);
                g_print("End of stream (EOS) tercapai. Menutup pipeline dengan aman...\n");
                g_main_loop_quit(mainLoop_);
                break;
            case GST_MESSAGE_ERROR: {
                GError *error = nullptr;
                gchar *debug = nullptr;
                gst_message_parse_error(message, &error, &debug);
                g_printerr("Error pada pipeline dari %s: %s\n",
                           GST_OBJECT_NAME(message->src),
                           error != nullptr ? error->message : "unknown error");
                if (debug != nullptr) {
                    g_printerr("Detail GStreamer: %s\n", debug);
                }
                g_clear_error(&error);
                g_free(debug);
                pipelineError_ = true;
                g_main_loop_quit(mainLoop_);
                break;
            }
            default:
                break;
        }
        return G_SOURCE_CONTINUE;
    }

    gboolean processShutdownSignal() {
        if (config_.outputMode == OutputMode::File && !eosRequested_) {
            eosRequested_ = true;
            g_print("\nSinyal shutdown diterima. Mengirim EOS agar MP4 disimpan dengan aman...\n");
            if (gst_element_send_event(pipeline_, gst_event_new_eos())) {
                eosShutdownTimeoutId_ =
                    g_timeout_add_seconds(kEosShutdownTimeoutSeconds,
                                          &DeepStreamApplication::onEosShutdownTimeout, this);
                if (eosShutdownTimeoutId_ != 0) {
                    return G_SOURCE_CONTINUE;
                }
                g_printerr("Gagal memasang timeout EOS; pipeline akan dihentikan langsung.\n");
            } else {
                g_printerr("Gagal mengirim EOS; pipeline akan dihentikan langsung.\n");
            }
        } else {
            g_print("\nSinyal shutdown diterima. Menghentikan pipeline...\n");
        }

        removeSource(eosShutdownTimeoutId_);
        g_main_loop_quit(mainLoop_);
        return G_SOURCE_CONTINUE;
    }

    gboolean processEosShutdownTimeout() {
        eosShutdownTimeoutId_ = 0;
        g_printerr("EOS tidak selesai dalam %u detik; pipeline akan dihentikan langsung.\n",
                   kEosShutdownTimeoutSeconds);
        g_main_loop_quit(mainLoop_);
        return G_SOURCE_REMOVE;
    }

    static gboolean onBusMessage(GstBus *, GstMessage *message, gpointer userData) {
        return static_cast<DeepStreamApplication *>(userData)->processBusMessage(message);
    }

    static gboolean onShutdownSignal(gpointer userData) {
        return static_cast<DeepStreamApplication *>(userData)->processShutdownSignal();
    }

    static gboolean onEosShutdownTimeout(gpointer userData) {
        return static_cast<DeepStreamApplication *>(userData)->processEosShutdownTimeout();
    }

    static void onDecodePadAdded(GstElement *, GstPad *newPad, gpointer userData) {
        static_cast<DeepStreamApplication *>(userData)->processDecodePad(newPad);
    }

    static GstPadProbeReturn onMuxSinkProbe(GstPad *, GstPadProbeInfo *info, gpointer userData) {
        GstBuffer *buffer = GST_PAD_PROBE_INFO_BUFFER(info);
        if (buffer) {
            GstClockTime pts = GST_BUFFER_PTS(buffer);
            if (GST_CLOCK_TIME_IS_VALID(pts)) {
                auto *app = static_cast<DeepStreamApplication *>(userData);
                std::lock_guard<std::mutex> lock(app->timestampsMutex_);
                app->timestampsMap_[pts].muxIn = SteadyClock::now();
            }
        }
        return GST_PAD_PROBE_OK;
    }

    static GstPadProbeReturn onInferSinkProbe(GstPad *, GstPadProbeInfo *info, gpointer userData) {
        static_cast<DeepStreamApplication *>(userData)->recordTimestamp(GST_PAD_PROBE_INFO_BUFFER(info), &ComponentTimestamps::inferIn);
        return GST_PAD_PROBE_OK;
    }

    static GstPadProbeReturn onTrackerSinkProbe(GstPad *, GstPadProbeInfo *info, gpointer userData) {
        static_cast<DeepStreamApplication *>(userData)->recordTimestamp(GST_PAD_PROBE_INFO_BUFFER(info), &ComponentTimestamps::trackerIn);
        return GST_PAD_PROBE_OK;
    }

    static GstPadProbeReturn onPreOsdSinkProbe(GstPad *, GstPadProbeInfo *info, gpointer userData) {
        static_cast<DeepStreamApplication *>(userData)->recordTimestamp(GST_PAD_PROBE_INFO_BUFFER(info), &ComponentTimestamps::preOsdIn);
        return GST_PAD_PROBE_OK;
    }

    static GstPadProbeReturn onOsdBuffer(GstPad *, GstPadProbeInfo *info, gpointer userData) {
        return static_cast<DeepStreamApplication *>(userData)->processOsdBuffer(info);
    }

    static GstPadProbeReturn onOutputSinkProbe(GstPad *, GstPadProbeInfo *info, gpointer userData) {
        static_cast<DeepStreamApplication *>(userData)->recordTimestamp(GST_PAD_PROBE_INFO_BUFFER(info), &ComponentTimestamps::outputIn);
        return GST_PAD_PROBE_OK;
    }

    static GstPadProbeReturn onOutputSrcProbe(GstPad *, GstPadProbeInfo *info, gpointer userData) {
        return static_cast<DeepStreamApplication *>(userData)->processOutputSrcBuffer(info);
    }

    static void removeSource(guint &sourceId) {
        if (sourceId != 0) {
            g_source_remove(sourceId);
            sourceId = 0;
        }
    }

    void cleanup() {
        for (auto &probe : probes_) {
            if (probe.first && probe.second) {
                gst_pad_remove_probe(probe.first, probe.second);
                gst_object_unref(probe.first);
            }
        }
        probes_.clear();

        if (pipeline_ != nullptr) {
            gst_element_set_state(pipeline_, GST_STATE_NULL);
            gst_element_get_state(pipeline_, nullptr, nullptr, 5 * GST_SECOND);
        }

        stopBenchmarkLogger();

        if (streamMux_ != nullptr && streamMuxSinkPad_ != nullptr) {
            gst_element_release_request_pad(streamMux_, streamMuxSinkPad_);
        }
        if (streamMuxSinkPad_ != nullptr) {
            gst_object_unref(streamMuxSinkPad_);
            streamMuxSinkPad_ = nullptr;
        }
        if (fileDecodeSinkPad_ != nullptr) {
            gst_object_unref(fileDecodeSinkPad_);
            fileDecodeSinkPad_ = nullptr;
        }

        removeSource(rtspServerSourceId_);
        removeSource(eosShutdownTimeoutId_);
        removeSource(busWatchId_);
        removeSource(sigintSourceId_);
        removeSource(sigtermSourceId_);

        if (rtspServer_ != nullptr) {
            g_object_unref(rtspServer_);
            rtspServer_ = nullptr;
        }
        if (pipeline_ != nullptr) {
            g_print("Membersihkan resource...\n");
            gst_object_unref(pipeline_);
            pipeline_ = nullptr;
            streamMux_ = nullptr;
        }
        if (mainLoop_ != nullptr) {
            g_main_loop_unref(mainLoop_);
            mainLoop_ = nullptr;
        }
    }

    AppConfig config_;
    GMainLoop *mainLoop_{nullptr};
    GstElement *pipeline_{nullptr};
    GstElement *streamMux_{nullptr};
    GstPad *streamMuxSinkPad_{nullptr};
    GstPad *fileDecodeSinkPad_{nullptr};
    std::vector<std::pair<GstPad*, gulong>> probes_;
    GstRTSPServer *rtspServer_{nullptr};
    guint rtspServerSourceId_{0};
    guint eosShutdownTimeoutId_{0};
    guint busWatchId_{0};
    guint sigintSourceId_{0};
    guint sigtermSourceId_{0};
    GAsyncQueue *loggerQueue_{nullptr};
    std::thread loggerThread_;
    char loggerStopToken_{0};
    std::unordered_map<GstClockTime, ComponentTimestamps> timestampsMap_;
    std::mutex timestampsMutex_;
    MetricsState metrics_;
    bool eosRequested_{false};
    bool pipelineError_{false};
};

}  // namespace

int main(int argc, char *argv[]) {
    AppConfig config;
    const ParseResult parseResult = parseArguments(argc, argv, config);
    if (parseResult == ParseResult::Help) {
        return EXIT_SUCCESS;
    }
    if (parseResult == ParseResult::Error) {
        printUsage(argv[0]);
        return EXIT_FAILURE;
    }

    gst_init(&argc, &argv);
    DeepStreamApplication application(std::move(config));
    return application.run();
}
