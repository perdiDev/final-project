# ────────────────────────────────────────────────────────────────────────
# Makefile  –  builds libnvds_custom_nms.so
# Target   : Jetson Orin Nano  (sm_87, JetPack 6.x, CUDA 12.x, DS 7.1)
#
# Usage:
#   make            – build the shared library
#   make clean      – remove build artefacts
#   make DS_VERSION=7.0  – override DeepStream version
# ────────────────────────────────────────────────────────────────────────

DS_VERSION  ?= 7.1
CUDA_VER    ?= 12

DS_ROOT     := /opt/nvidia/deepstream/deepstream-$(DS_VERSION)
CUDA_ROOT   := /usr/local/cuda-$(CUDA_VER)

NVCC        := $(CUDA_ROOT)/bin/nvcc
CXX         := g++

# Jetson Orin Nano  – Ampere, compute capability 8.7
CUDA_ARCH   := -gencode arch=compute_87,code=sm_87

# ── Include paths ────────────────────────────────────────────────────
INC := \
    -I$(DS_ROOT)/sources/includes \
    -I$(CUDA_ROOT)/include \
    -Isrc \
    $(shell pkg-config --cflags gstreamer-1.0)

# ── Link libraries ───────────────────────────────────────────────────
LIBS := \
    -L$(CUDA_ROOT)/lib64 -lcudart \
    -L$(DS_ROOT)/lib -lnvdsgst_meta -lnvds_meta \
    $(shell pkg-config --libs gstreamer-1.0) \
    -lglib-2.0

# ── Compiler flags ───────────────────────────────────────────────────
CXXFLAGS  := -std=c++17 -O2 -fPIC -Wall -Wextra $(INC)
NVCCFLAGS := -O2 $(CUDA_ARCH) --compiler-options '-fPIC,-Wall' $(INC)

# ── Output ───────────────────────────────────────────────────────────
TARGET    := ../lib/libnvds_custom_nms.so
BUILD_DIR := build

OBJS := \
    $(BUILD_DIR)/cuda_nms.o \
    $(BUILD_DIR)/nvds_postprocess_nms.o

# ── Rules ────────────────────────────────────────────────────────────
.PHONY: all clean dirs

all: dirs $(TARGET)

dirs:
	@mkdir -p $(BUILD_DIR) ../lib

$(TARGET): $(OBJS)
	$(CXX) -shared -o $@ $^ $(LIBS)
	@echo ""
	@echo "✓  Built: $@"

$(BUILD_DIR)/cuda_nms.o: src/cuda_nms.cu src/cuda_nms.h
	$(NVCC) $(NVCCFLAGS) -c -o $@ $<

$(BUILD_DIR)/nvds_postprocess_nms.o: src/nvds_postprocess_nms.cpp \
                                      src/nvds_postprocess_nms.h \
                                      src/cuda_nms.h
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -rf $(BUILD_DIR) $(TARGET)
	@echo "✓  Cleaned"
