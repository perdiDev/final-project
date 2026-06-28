# ────────────────────────────────────────────────────────────────────────
# Makefile  –  builds libnvds_custom_nms.so
# Target   : Jetson Orin Nano  (sm_87, JetPack 6.x, CUDA 12.6, DS 7.1)
#
# Usage:
#   make            – build the shared library
#   make clean      – remove build artefacts
#   make DS_VERSION=7.0  – override DeepStream version
# ────────────────────────────────────────────────────────────────────────

DS_VERSION  ?= 7.1
CUDA_VER    ?= 12.6

DS_ROOT     := /opt/nvidia/deepstream/deepstream-$(DS_VERSION)
CUDA_ROOT   := /usr/local/cuda-$(CUDA_VER)

NVCC        := $(CUDA_ROOT)/bin/nvcc
CXX         := g++

# Jetson Orin Nano  – Ampere, compute capability 8.7
CUDA_ARCH   := -gencode arch=compute_87,code=sm_87

# ── GStreamer flags (split into two variables) ───────────────────────
# pkg-config --cflags emits -pthread which g++ accepts but nvcc rejects.
# We extract only the -I paths for NVCC, pass the full set to g++ only.
GST_CFLAGS  := $(shell pkg-config --cflags gstreamer-1.0)
GST_LIBS    := $(shell pkg-config --libs   gstreamer-1.0)
GST_INC     := $(filter -I%,$(GST_CFLAGS))

# ── Include paths ────────────────────────────────────────────────────
# INC_COMMON: pure -I flags, safe for both g++ and nvcc
INC_COMMON := \
    -I$(DS_ROOT)/sources/includes \
    -I$(CUDA_ROOT)/include \
    -Isrc \
    $(GST_INC)

# ── Link libraries ───────────────────────────────────────────────────
LIBS := \
    -L$(CUDA_ROOT)/lib64 -lcudart \
    -L$(DS_ROOT)/lib -lnvdsgst_meta -lnvds_meta \
    $(GST_LIBS) \
    -lglib-2.0

# ── Compiler flags ───────────────────────────────────────────────────
# g++ gets the full GST_CFLAGS (including -pthread etc.)
CXXFLAGS  := -std=c++17 -O2 -fPIC -Wall -Wextra $(INC_COMMON) $(GST_CFLAGS)

# nvcc receives only clean -I flags; -pthread goes inside --compiler-options
NVCCFLAGS := -O2 $(CUDA_ARCH) \
             --compiler-options '-fPIC,-Wall,-pthread' \
             $(INC_COMMON)

# ── Output ───────────────────────────────────────────────────────────
TARGET    := lib/libnvds_custom_nms.so
BUILD_DIR := build

OBJS := \
    $(BUILD_DIR)/cuda_nms.o \
    $(BUILD_DIR)/nvds_postprocess_nms.o

# ── Rules ────────────────────────────────────────────────────────────
.PHONY: all clean dirs

all: dirs $(TARGET)

dirs:
	@mkdir -p $(BUILD_DIR) lib

$(TARGET): $(OBJS)
	$(CXX) -shared -o $@ $^ $(LIBS)
	@echo ""
	@echo "Built: $@"

# Dependencies list only the source file – headers are found via -Isrc.
# Listing header paths as prerequisites causes "no rule to make target"
# when Make cannot locate them relative to the working directory.
$(BUILD_DIR)/cuda_nms.o: src/cuda_nms.cu
	$(NVCC) $(NVCCFLAGS) -c -o $@ $<

$(BUILD_DIR)/nvds_postprocess_nms.o: src/nvds_postprocess_nms.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -rf $(BUILD_DIR) $(TARGET)
	@echo "Cleaned"
