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
# As far as I know the Raspberry Pi Pico plugin for VSCode does not (yet) support bare RP2350 microcontrollers without a board.
# To adapt the C-SDK for a standard Pico board configure this CMakeLists.txt file via the Raspberry Pi Pico plugin's "Switch Board" option.
set(PICO_BOARD pico2_w CACHE STRING "Board type")

# Uncomment the following two lines to compile for bare RP2350 without a board
# set(PICO_PLATFORM rp2350)
# set(PICO_BOARD none CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(demo C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name
add_executable(${PROJECT_NAME}
    src/hub75_lvgl.cpp
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
    # Nothing here yet. See further below for more library settings.
)

set(HUB75_SUPPORT true) # Enable the HUB75 library
set(LVGL_SUPPORT true)  # Enable LVGL support

add_subdirectory(libraries)
# add_subdirectory(utils)

pico_set_program_name(${PROJECT_NAME} "${PROJECT_NAME}")
pico_set_program_version(${PROJECT_NAME} "0.1")
pico_set_float_implementation(${PROJECT_NAME} pico)

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(${PROJECT_NAME} 0)
pico_enable_stdio_usb(${PROJECT_NAME} 1)


if(HUB75_SUPPORT)
    # No need to modify preprocessor defines in hub75.cpp - instead set their values here.
    #
    # Example:
    # Settings for a RP2350B microcontroller with GPIO pins spanning from 30 to 43.
    # Beware to set `PICO_PLATFORM rp2350` and `PICO_BOARD none` prior to `include(pico_sdk_import.cmake)`
    target_compile_definitions(hub75 PUBLIC
        HUB75_SUPPORT=true
        # PICO_RP2350A=0              # PICO_RP2350A=0` means not a RP2350A but a RP2350B microcontroller - uncomment for RP235xB microcontroller only!
        USE_PICO_GRAPHICS=false      # set to false if you use hub75 as a library - any reference to pico_graphics is removed
        MATRIX_PANEL_WIDTH=64       # your matrix panel width
        MATRIX_PANEL_HEIGHT=32      # your matrix panel height
        CHAIN_MODE=CHAIN_MODE_SERPENTINE # set chain-mode - default is serpentine (U-Turn with compensation for 180° rotation)
        CHAIN_COLS=1                # number of panels chained left-to-right in a single chain row (columns)
        CHAIN_ROWS=3                # number of chain rows stacked vertically (rows)
        DATA_BASE_PIN=0             # base GPIO pin (aka start index) of R0, G0, B0, R1, G1, B1 GPIO pins - use 30 for PICO_RP2350B
        DATA_N_PINS=6               # number (count) of colour pins (usually 6)
        ROWSEL_BASE_PIN=6           # base GPIO address pin (aka start index) of A, B (, C, D. E) GPIO pins - use 36 for PICO_RP2350B
        ROWSEL_N_PINS=3             # number (count) of address pins available on your matrix panel board (look at your panels connector)
        CLK_PIN=11                  # GPIO pin for CLK - use pin 41 for PICO_RP2350B
        STROBE_PIN=12               # GPIO pin for STROBE (LATCH) - use pin 42 for PICO_RP2350B
        OEN_PIN=13                  # GPIO pin for OE pin - use pin 43 for PICO_RP2350B
        HUB75_P3_1415_16S_64X64_S31                 #
        PANEL_TYPE=PANEL_GENERIC    # select PANEL_TYPE
        INVERTED_STB=false          # inverted pin signal for OE (untested)
        SM_CLOCKDIV_FACTOR=1.0f     # to prevent flicker or ghosting it might be worth a try to reduce state machine speed
        BITPLANES=10                # number (count) of bit-planes used for BCM (Binary Code Modulation) - valid values for BIT_DEPTH are 8 or 10
        BALANCED_LIGHT_OUTPUT=true  # allthough it uses some more memory it improves effective refresh rate and really cuts down flicker
        SEPARATE_CIE_CHANNELS=true  # use separate CIE channels for improved colour representation - needs more memory
        CCM_RG_SHIFT=6              # CCM Cross-channel mixing - mix ~1.6% green into the red channel
        CCM_GB_SHIFT=7              # CCM Cross-channel mixing - mix ~0.8% blue into the green channel
        BASE_LATCH_NS=80            # wait time in nano-seconds to stabilise latch
        BASE_ADDR_NS=120            # wait time in nano-seconds to stabilise row addressing
        BASE_OE_NS=40               # wait time in nano-seconds to stabilise row addressing
        HUB75_MULTICORE=true        # use core1 for the hub75 driver
        FRAME_RATE=true            # for testing and debugging purpose only: output frame rate information (printf) in monitor - set to `false` for production
    )

    target_link_libraries(${PROJECT_NAME}
        hardware_pio
        hardware_dma
        hub75

    )
endif()


# Link wifi support if available and set wifi credentials from environment variables
if(PICO_CYW43_SUPPORTED)

    target_compile_definitions(${PROJECT_NAME} PUBLIC
        CYW43_PIO_CLOCK_DIV_DYNAMIC=true
    )

    target_sources(${PROJECT_NAME} PRIVATE
        src/fota.cpp
        src/network.cpp
    )

    target_link_libraries(${PROJECT_NAME}
        pico_cyw43_arch_lwip_threadsafe_background
    )

#    pico_use_wifi_firmware_partition(${PROJECT_NAME})

    if(DEFINED ENV{WIFI_SSID})
        target_compile_definitions(${PROJECT_NAME} PRIVATE WIFI_SSID="$ENV{WIFI_SSID}")
    else()
        message(FATAL_ERROR "WIFI_SSID environment variable is not set!")
    endif()
    if(DEFINED ENV{WIFI_PASSWORD})
        target_compile_definitions(${PROJECT_NAME} PRIVATE WIFI_PASSWORD="$ENV{WIFI_PASSWORD}")
    else()
        message(FATAL_ERROR "WIFI_PASSWORD environment variable is not set!")
    endif()
endif()


if(LVGL_SUPPORT)
    target_compile_definitions(${PROJECT_NAME} PUBLIC
        LVGL_SUPPORT=true
        LV_CONF_PATH="${CMAKE_SOURCE_DIR}/lv_conf.h"
    )

    target_sources(${PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/src/hub75_lvgl.cpp
        ${CMAKE_CURRENT_LIST_DIR}/examples/bouncing_balls.cpp
        ${CMAKE_CURRENT_LIST_DIR}/examples/colour_check.cpp
        ${CMAKE_CURRENT_LIST_DIR}/examples/fire_effect.cpp
    )

    target_link_libraries(${PROJECT_NAME}
        lvgl
    )
endif()


# Add the standard library to the build
target_link_libraries(${PROJECT_NAME}
        pico_stdlib
        pico_multicore
        pico_aon_timer
        pico_sha256
        hardware_vreg
)

# Add the standard include files to the build
target_include_directories(${PROJECT_NAME} PRIVATE
        ${CMAKE_CURRENT_LIST_DIR}/src
        ${CMAKE_CURRENT_LIST_DIR}/examples
        ${CMAKE_SOURCE_DIR}
)

pico_hash_binary(${PROJECT_NAME})
if(EXISTS ${PRIVATE_SIGNING_KEY})
    message(STATUS "Using key file ${PRIVATE_SIGNING_KEY} to sign the firmware image.")
    pico_sign_binary(${PROJECT_NAME} ${PRIVATE_SIGNING_KEY})
else()
    message(STATUS "Missing private key file ${PRIVATE_SIGNING_KEY}. The firmware will NOT be signed.")
endif()

pico_add_extra_outputs(${PROJECT_NAME})