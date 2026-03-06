#!/bin/bash

PROJECT_DIR="/home/mola/Desktop/Auto_Watering_System/Auto_Irrigation_System_v2/"

cd "$PROJECT_DIR" || { echo "Directory not found!"; exit 1; }

# Activate PlatformIO official virtual environment
source ~/.platformio/venv/bin/activate || { echo "PlatformIO venv not found!"; exit 1; }

# Compile
echo "Compiling the project..."
pio run || { echo "Compilation failed!"; exit 1; }

# Upload
echo "Uploading the firmware..."
pio run --target upload || { echo "Upload failed!"; exit 1; }

echo "Done!"