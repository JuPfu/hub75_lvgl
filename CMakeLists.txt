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
set(sdkVersion 2.2.0)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.2.0-a4)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico2_w CACHE STRING "Board type")

# Uncomment the following two lines to compile for bare RP2350 without a board
# set(PICO_PLATFORM rp2350)
# set(PICO_BOARD pico2 CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(hub75_lvgl C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1
add_executable(hub75_lvgl hub75_lvgl.cpp)

pico_set_program_name(hub75_lvgl "hub75_lvgl")
pico_set_program_version(hub75_lvgl "2.0")

set(PANEL_GENERIC 0)
set(PANEL_FM6126A 1)
set(PANEL_RUL6024 2)

# No need to modify preprocessor defines in hub75.cpp - instead set their values here.
#
# Example:
# Settings for a RP2350B microcontroller with GPIO pins spanning from 30 to 43.
# Beware to set `PICO_PLATFORM rp2350` and `PICO_BOARD none` prior to `include(pico_sdk_import.cmake)`
target_compile_definitions(hub75_lvgl PRIVATE
    # PICO_RP2350A=0              # `PICO_RP2350A=0` means not a RP2350A but a RP2350B microcontroller - uncomment for RP235xB microcontroller only!
    # USE_PICO_GRAPHICS=true      # set to false if you use hub75 as a library - any reference to pico_graphics is removed
    # MATRIX_PANEL_WIDTH=64       # your matrix panel width
    # MATRIX_PANEL_HEIGHT=64      # your matrix panel height
    # DATA_BASE_PIN=30            # base GPIO pin (aka start index) of R0, G0, B0, R1, G1, B1 GPIO pins
    # DATA_N_PINS=6               # number (count) of colour pins (usually 6)
    # ROWSEL_BASE_PIN=36          # base GPIO address pin (aka start index) of A, B (, C, D. E) GPIO pins
    # ROWSEL_N_PINS=5             # number (count) of address pins available on your matrix panel board (look at your panels connector)
    # CLK_PIN=41                  # GPIO pin for CLK
    # STROBE_PIN=42               # GPIO pin for STROBE (LATCH)
    # OEN_PIN=43                  # GPIO for OE pin
    # PANEL_TYPE=PANEL_GENERIC    # select PANEL_TYPE
    # INVERTED_STB=false          # inverted pin signal for OE (untested)
    # TEMPORAL_DITHERING=false    # experimental - switch on temporal dithering to improve colour depth (currently 2 additional bits)
    # SM_CLOCKDIV_FACTOR=1.0f     # to prevent flicker or ghosting it might be worth a try to reduce state machine speed
)

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(hub75_lvgl 0)
pico_enable_stdio_usb(hub75_lvgl 1)

# Generate PIO header
pico_generate_pio_header(hub75_lvgl ${CMAKE_CURRENT_LIST_DIR}/src/hub75.pio)

# Add the lvgl library
message(NOTICE "===>>> LVGL configuration start ===") 

set(LVGL_DIR_NAME lvgl) # The name of the lvgl folder (change this if you have renamed it)
set(LVGL_DIR  ${CMAKE_CURRENT_LIST_DIR}) # The path where the lvgl folder is
set(LV_CONF_PATH  ${CMAKE_CURRENT_LIST_DIR}/lv_conf.h) # The path to the lv_conf.h file
set(LV_CONF_INCLUDE_SIMPLE  ${CMAKE_CURRENT_LIST_DIR}/lv_conf.h) # The path to the lv_conf.h file

message(NOTICE "LVGL folder name: ${LVGL_DIR_NAME}") 
message(NOTICE "Path to LVGL folder: ${LVGL_DIR}")
message(NOTICE "Path to config file: ${LV_CONF_PATH}")
message(NOTICE "Include path set for `LV_CONF_INCLUDE_SIMPLE` ${LV_CONF_INCLUDE_SIMPLE}")

add_subdirectory(${LVGL_DIR_NAME})

message(NOTICE "=== LVGL configuraton end <<<===") 

target_sources(hub75_lvgl PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/src/hub75.cpp
        ${CMAKE_CURRENT_LIST_DIR}/hub75_lvgl.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/fm6126a.cpp
        ${CMAKE_CURRENT_LIST_DIR}/src/rul6024.cpp
        ${CMAKE_CURRENT_LIST_DIR}/examples/bouncing_balls.cpp
        ${CMAKE_CURRENT_LIST_DIR}/examples/fire_effect.cpp
        ${CMAKE_CURRENT_LIST_DIR}/examples/image_animation.hpp
        ${CMAKE_CURRENT_LIST_DIR}/examples/colour_check.cpp
        )

# Add the standard include files to the build
target_include_directories(hub75_lvgl PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/src
        ${CMAKE_CURRENT_LIST_DIR}/examples
        ${CMAKE_CURRENT_LIST_DIR}
)

# Add any user requested libraries
target_link_libraries(hub75_lvgl 
        pico_stdlib
        pico_multicore
        hardware_pio
        hardware_dma
        hardware_timer
        lvgl
        )

if(PICO_CYW43_SUPPORTED)
    target_link_libraries(hub75_lvgl
                          pico_cyw43_arch_none)
endif()

pico_add_extra_outputs(hub75_lvgl)

