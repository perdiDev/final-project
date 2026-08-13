#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <vector>

#include "nvdsinfer_custom_impl.h"

namespace {

const NvDsInferLayerInfo* findLayer(
    const std::vector<NvDsInferLayerInfo>& layers, const char* name) {
    for (const auto& layer : layers) {
        if (layer.layerName != nullptr && std::strcmp(layer.layerName, name) == 0) {
            return &layer;
        }
    }
    return nullptr;
}

bool isIntLayer(const NvDsInferLayerInfo& layer) {
    return layer.dataType == INT32 || layer.dataType == INT64;
}

bool isClassLayer(const NvDsInferLayerInfo& layer) {
    return isIntLayer(layer) || layer.dataType == FLOAT;
}

int64_t readDetectionCount(const NvDsInferLayerInfo& layer) {
    return layer.dataType == INT32
               ? static_cast<const int32_t*>(layer.buffer)[0]
               : static_cast<const int64_t*>(layer.buffer)[0];
}

int64_t readClassId(const NvDsInferLayerInfo& layer, unsigned int index) {
    if (layer.dataType == INT32) {
        return static_cast<const int32_t*>(layer.buffer)[index];
    }
    if (layer.dataType == INT64) {
        return static_cast<const int64_t*>(layer.buffer)[index];
    }
    return static_cast<int64_t>(
        std::lrint(static_cast<const float*>(layer.buffer)[index]));
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
    if (boxesLayer->dataType != FLOAT || scoresLayer->dataType != FLOAT ||
        !isIntLayer(*countLayer) || !isClassLayer(*classesLayer)) {
        std::cerr << "EfficientNMS parser: tipe output tidak sesuai. "
                     "Boxes/scores harus FLOAT, count harus INT32/INT64, dan "
                     "classes harus FLOAT/INT32/INT64."
                  << std::endl;
        return false;
    }
    if (countLayer->inferDims.numElements == 0U) {
        std::cerr << "EfficientNMS parser: num_detections kosong." << std::endl;
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

    const int64_t rawCount = readDetectionCount(*countLayer);
    const unsigned int count = static_cast<unsigned int>(std::max<int64_t>(
        0, std::min<int64_t>(rawCount, static_cast<int64_t>(capacity))));
    const auto* boxes = static_cast<const float*>(boxesLayer->buffer);
    const auto* scores = static_cast<const float*>(scoresLayer->buffer);
    objectList.reserve(objectList.size() + count);

    for (unsigned int index = 0U; index < count; ++index) {
        const float score = scores[index];
        const int64_t rawClassId = readClassId(*classesLayer, index);
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
        const unsigned int boxOffset = index * 4U;
        const float x1 = boxes[boxOffset + 0U];
        const float y1 = boxes[boxOffset + 1U];
        const float x2 = boxes[boxOffset + 2U];
        const float y2 = boxes[boxOffset + 3U];
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
