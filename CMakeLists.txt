cmake_minimum_required(VERSION 3.0.0)

project(pami_robot_esp32)

# Options ESP32
set(IDF_TARGET "esp32")
set(ESP_PLATFORM ON)

# ROS 2 micro-ROS
include($ENV{FREERTOS_PATH}/tools/cmake/freertos.cmake)

# Ajouter les dépendances ROS 2
find_package(rcl REQUIRED)
find_package(rosidl_default_generators REQUIRED)

# Source files
file(GLOB_RECURSE SOURCES
    "src/*.cpp"
    "src/*.c"
)

# Créer l'exécutable
idf_component_register(
    SRCS ${SOURCES}
    INCLUDE_DIRS "include"
    REQUIRES driver esp_timer
)

# Lier les libs ROS 2
target_link_libraries(${COMPONENT_LIB}
    rcl
    rclc
    rclc_parameter
)

# Options compilation
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -Wall -Wextra")
