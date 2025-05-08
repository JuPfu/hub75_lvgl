# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.1.1)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.1.1)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico2_w CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(hub75_lvgl_2 C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1

add_executable(hub75_lvgl_2 hub75_lvgl_2.cpp)

pico_set_program_name(hub75_lvgl_2 "hub75_lvgl_2")
pico_set_program_version(hub75_lvgl_2 "0.9")

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(hub75_lvgl_2 0)
pico_enable_stdio_usb(hub75_lvgl_2 1)

# Generate PIO header
pico_generate_pio_header(hub75_lvgl_2 ${CMAKE_CURRENT_LIST_DIR}/hub75.pio)

# Add the lvgl library
message(NOTICE "===>>> LVGL configuration start ===") 

set(LVGL_DIR_NAME lvgl) #The name of the lvgl folder (change this if you have renamed it)
set(LVGL_DIR  ${CMAKE_CURRENT_LIST_DIR}) #The path where the lvgl folder is
set(LV_CONF_PATH  ${CMAKE_CURRENT_LIST_DIR}/lv_conf.h) #The path to the lv_conf.h file
set(LV_CONF_INCLUDE_SIMPLE  ${CMAKE_CURRENT_LIST_DIR}/lv_conf.h) #The path to the lv_conf.h file

message(NOTICE "LVGL folder name: ${LVGL_DIR_NAME}") 
message(NOTICE "Path to LVGL folder: ${LVGL_DIR}")
message(NOTICE "Path to config file: ${LV_CONF_PATH}")
message(NOTICE "Include path set for `LV_CONF_INCLUDE_SIMPLE` ${LV_CONF_INCLUDE_SIMPLE}")

add_subdirectory(${LVGL_DIR_NAME})

message(NOTICE "=== LVGL configuraton end <<<===") 

target_sources(hub75_lvgl_2 PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/hub75.cpp
        ${CMAKE_CURRENT_LIST_DIR}/hub75_lvgl_2.cpp
        ${CMAKE_CURRENT_LIST_DIR}/bouncing_balls.cpp
        ${CMAKE_CURRENT_LIST_DIR}/fire_effect.cpp
        ${CMAKE_CURRENT_LIST_DIR}/image_animation.hpp
        )

# Add the standard include files to the build
target_include_directories(hub75_lvgl_2 PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}
)

# Add any user requested libraries
target_link_libraries(hub75_lvgl_2 
        pico_stdlib
        pico_multicore
        pico_cyw43_arch_none
        hardware_pio
        hardware_dma
        hardware_timer
        lvgl
        )

pico_add_extra_outputs(hub75_lvgl_2)

