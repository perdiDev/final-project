import tensorrt as trt
import json

logger = trt.Logger(trt.Logger.INFO)
trt.init_libnvinfer_plugins(logger, "")
runtime = trt.Runtime(logger)

with open("../../models/yolov8n_kitti_efficientnms.engine", "rb") as f:
    engine = runtime.deserialize_cuda_engine(f.read())

assert engine is not None, "Deserialize gagal."

inspector = engine.create_engine_inspector()
info = json.loads(inspector.get_engine_information(trt.LayerInformationFormat.JSON))
layers = info["Layers"]

for layer in layers:
    if isinstance(layer, str):
        if "nms" in layer.lower() or "plugin" in layer.lower():
            print(f"[nama saja, rebuild dgn DETAILED untuk detail]: {layer}")
    else:
        name = layer.get("Name", "")
        if "nms" in name.lower() or "plugin" in name.lower():
            print(json.dumps(layer, indent=2))
