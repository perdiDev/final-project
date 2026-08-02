#include <algorithm>
#include <cmath>
#include <cstdint>
#include <iostream>
#include <limits>
#include <string>
#include <vector>

#include "nvdsinfer_custom_impl.h"

namespace {

const NvDsInferLayerInfo* findLayer(
    const std::vector<NvDsInferLayerInfo>& layers, const char* name) {
    for (const auto& layer : layers) {
        if (layer.layerName != nullptr && std::string(layer.layerName) == name) {
            return &layer;
        }
    }
    return nullptr;
}

bool isFloatLayer(const NvDsInferLayerInfo& layer) {
    return layer.dataType == FLOAT;
}

bool isIntLayer(const NvDsInferLayerInfo& layer) {
    return layer.dataType == INT32 || layer.dataType == INT64;
}

float readFloat(const NvDsInferLayerInfo& layer, unsigned int index) {
    if (layer.dataType == FLOAT) {
        return static_cast<const float*>(layer.buffer)[index];
    }

    // EfficientNMS_TRT normally emits FLOAT for boxes/scores. Supporting
    // INT32 here makes malformed/mismatched engines fail safely in the caller.
    if (layer.dataType == INT32) {
        return static_cast<float>(static_cast<const int32_t*>(layer.buffer)[index]);
    }
    if (layer.dataType == INT64) {
        return static_cast<float>(static_cast<const int64_t*>(layer.buffer)[index]);
    }
    return std::numeric_limits<float>::quiet_NaN();
}

int64_t readInt(const NvDsInferLayerInfo& layer, unsigned int index) {
    if (layer.dataType == INT32) {
        return static_cast<const int32_t*>(layer.buffer)[index];
    }
    if (layer.dataType == INT64) {
        return static_cast<const int64_t*>(layer.buffer)[index];
    }
    // Some TensorRT/plugin versions expose class IDs as FLOAT.
    if (layer.dataType == FLOAT) {
        return static_cast<int64_t>(std::lrint(
            static_cast<const float*>(layer.buffer)[index]));
    }
    return -1;
}

float classThreshold(
    const NvDsInferParseDetectionParams& params, unsigned int classId) {
    if (classId < params.perClassPreclusterThreshold.size()) {
        return params.perClassPreclusterThreshold[classId];
    }
    return 0.0F;
}

}  // namespace

extern "C" bool NvDsInferParseEfficientNMS(
    std::vector<NvDsInferLayerInfo> const& outputLayersInfo,
    NvDsInferNetworkInfo const& networkInfo,
    NvDsInferParseDetectionParams const& detectionParams,
    std::vector<NvDsInferObjectDetectionInfo>& objectList) {
    const NvDsInferLayerInfo* countLayer =
        findLayer(outputLayersInfo, "num_detections");
    const NvDsInferLayerInfo* boxesLayer =
        findLayer(outputLayersInfo, "detection_boxes");
    const NvDsInferLayerInfo* scoresLayer =
        findLayer(outputLayersInfo, "detection_scores");
    const NvDsInferLayerInfo* classesLayer =
        findLayer(outputLayersInfo, "detection_classes");

    if (countLayer == nullptr || boxesLayer == nullptr ||
        scoresLayer == nullptr || classesLayer == nullptr) {
        std::cerr << "EfficientNMS parser: output layer tidak lengkap. "
                     "Dibutuhkan num_detections, detection_boxes, "
                     "detection_scores, detection_classes."
                  << std::endl;
        return false;
    }
    if (countLayer->buffer == nullptr || boxesLayer->buffer == nullptr ||
        scoresLayer->buffer == nullptr || classesLayer->buffer == nullptr) {
        std::cerr << "EfficientNMS parser: buffer output bernilai null."
                  << std::endl;
        return false;
    }
    if (!isFloatLayer(*boxesLayer) || !isFloatLayer(*scoresLayer) ||
        !isIntLayer(*countLayer)) {
        std::cerr << "EfficientNMS parser: tipe output tidak sesuai. "
                     "Boxes/scores harus FLOAT dan num_detections INT32/INT64."
                  << std::endl;
        return false;
    }

    const unsigned int boxCapacity = boxesLayer->inferDims.numElements / 4U;
    const unsigned int scoreCapacity = scoresLayer->inferDims.numElements;
    const unsigned int classCapacity = classesLayer->inferDims.numElements;
    const unsigned int capacity =
        std::min({boxCapacity, scoreCapacity, classCapacity});
    if (capacity == 0U) {
        return true;
    }

    const int64_t rawCount = readInt(*countLayer, 0U);
    const unsigned int count = static_cast<unsigned int>(std::max<int64_t>(
        0, std::min<int64_t>(rawCount, static_cast<int64_t>(capacity))));

    for (unsigned int index = 0U; index < count; ++index) {
        const float score = readFloat(*scoresLayer, index);
        const int64_t rawClassId = readInt(*classesLayer, index);
        if (!std::isfinite(score) || rawClassId < 0 ||
            rawClassId >= static_cast<int64_t>(detectionParams.numClassesConfigured)) {
            continue;
        }

        const unsigned int classId = static_cast<unsigned int>(rawClassId);
        if (score < classThreshold(detectionParams, classId)) {
            continue;
        }

        // The EfficientNMS builder uses box_coding=0, i.e. absolute xyxy
        // coordinates in the model input coordinate system (normally 640x640).
        const float x1 = readFloat(*boxesLayer, index * 4U + 0U);
        const float y1 = readFloat(*boxesLayer, index * 4U + 1U);
        const float x2 = readFloat(*boxesLayer, index * 4U + 2U);
        const float y2 = readFloat(*boxesLayer, index * 4U + 3U);
        if (!std::isfinite(x1) || !std::isfinite(y1) ||
            !std::isfinite(x2) || !std::isfinite(y2)) {
            continue;
        }

        const float left = std::clamp(x1, 0.0F,
                                      static_cast<float>(networkInfo.width));
        const float top = std::clamp(y1, 0.0F,
                                     static_cast<float>(networkInfo.height));
        const float right = std::clamp(x2, 0.0F,
                                       static_cast<float>(networkInfo.width));
        const float bottom = std::clamp(y2, 0.0F,
                                        static_cast<float>(networkInfo.height));
        if (right <= left || bottom <= top) {
            continue;
        }

        NvDsInferObjectDetectionInfo object{};
        object.classId = classId;
        object.left = left;
        object.top = top;
        object.width = right - left;
        object.height = bottom - top;
        object.detectionConfidence = score;
        objectList.emplace_back(object);
    }

    return true;
}

CHECK_CUSTOM_PARSE_FUNC_PROTOTYPE(NvDsInferParseEfficientNMS);
