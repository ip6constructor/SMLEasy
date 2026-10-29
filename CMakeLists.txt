cmake_minimum_required(VERSION 3.16)
# ESP-IDF 6.1 is the only supported SDK (the ESP-IDF 4.4.8/PlatformIO release path was retired
# 2026-10-29); every target, including C3, builds src/app_s3's component list (acme_client etc.).
set(EXTRA_COMPONENT_DIRS "${CMAKE_CURRENT_LIST_DIR}/src/app_s3")
file(READ "${CMAKE_CURRENT_LIST_DIR}/build_version.txt" PROJECT_VER)
string(STRIP "${PROJECT_VER}" PROJECT_VER)
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(smartmeter)
