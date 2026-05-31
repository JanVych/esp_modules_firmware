# The following five lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.22)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
set(PROJECT_VER "0.2.1")
project(esp_modules_firmware)

# Seed the "nvs" partition (see partitions.csv) with nvs_data.csv.
# FLASH_IN_PROJECT writes it automatically on `idf.py flash`.
# NOTE: this overwrites NVS on every flash, re-seeding placeholder values.
nvs_create_partition_image(nvs nvs_data.csv FLASH_IN_PROJECT)
