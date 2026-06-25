#!/bin/bash

# Define colors for clean terminal output
GREEN='\030[0;32m'
YELLOW='\030[1;33m'
RED='\030[0;31m'
NC='\030[0m' # No Color

echo -e "${YELLOW}=== Starting Jetson Orin Nano System Cleanup ===${NC}"

# 1. Kill any hung DeepStream/GStreamer processes
echo -e "\n${YELLOW}[1/4] Killing zombie DeepStream/GStreamer processes...${NC}"
sudo killall -9 deepstream-app gst-launch-1.0 v4l2loopback 2>/dev/null
if [ $? -eq 0 ]; then
    echo -e "${GREEN}✔ Active processes terminated successfully.${NC}"
else
    echo -e "${GREEN}✔ No hung DeepStream processes found.${NC}"
fi

# 2. Flush and Clear RAM Caches
echo -e "\n${YELLOW}[2/4] Clearing RAM pagecache, dentries, and inodes...${NC}"
# Sync changes to disk first so we don't lose unwritten data
sudo sync
echo 3 | sudo tee /proc/sys/vm/drop_caches > /dev/null
echo -e "${GREEN}✔ RAM Cache cleared.${NC}"

# 3. Reset UVC Video Driver (ZED Camera)
echo -e "\n${YELLOW}[3/4] Resetting UVC Video Driver...${NC}"
sudo modprobe -r uvcvideo 2>/dev/null
if [ $? -eq 0 ]; then
    sudo modprobe uvcvideo
    echo -e "${GREEN}✔ UVC Driver successfully reloaded.${NC}"
else
    echo -e "${RED}✘ Failed to detach uvcvideo driver. Camera might be physically disconnected or locked hard.${NC}"
fi

# 4. Clear GStreamer Registry Cache
echo -e "\n${YELLOW}[4/4] Clearing GStreamer plugin registry cache...${NC}"
rm -rf ~/.cache/gstreamer-1.0/
echo -e "${GREEN}✔ GStreamer cache registry removed.${NC}"

echo -e "\n${GREEN}=== Cleanup Complete! System is ready for DeepStream. ===${NC}\n"
