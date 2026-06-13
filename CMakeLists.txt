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

set(PICO_BOARD pico2 CACHE STRING "Board type")

# Uncomment the following two lines to compile for bare RP2350 without a board
# set(PICO_PLATFORM rp2350)
# set(PICO_BOARD none CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(hub75_lvgl C CXX ASM)

# ── Banner ────────────────────────────────────────────────────────────────────
message(STATUS "")
message(STATUS "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━")
message(STATUS "  hub75_lvgl — CMake configuration")
message(STATUS "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━")
message(STATUS "")

# ── Step 1 — Pico SDK ─────────────────────────────────────────────────────────
message(STATUS "Step 1/3 — Initialising Pico SDK...")
# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()
message(STATUS "Step 1/3 — Pico SDK ✓  (${PICO_SDK_VERSION_STRING})")
message(STATUS "")

# ── Step 2 — Fetch LVGL ───────────────────────────────────────────────────────
set(LV_TAG v9.4.0)

message(STATUS "Step 2/3 — Fetching LVGL ${LV_TAG} from GitHub...")
message(STATUS "           First clone may take 1-2 minutes.")
message(STATUS "           Subsequent builds use the local cache instantly.")
message(STATUS "")

include(FetchContent)

# Show git clone progress in the Output panel
set(FETCHCONTENT_QUIET OFF CACHE BOOL "" FORCE)

# Tell LVGL where lv_conf.h lives — must be set BEFORE FetchContent_MakeAvailable
set(LV_CONF_PATH ${CMAKE_CURRENT_LIST_DIR}/lv_conf.h CACHE STRING "" FORCE)

# Disable demos and examples — not needed on embedded target, saves flash/RAM
set(LV_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(LV_BUILD_DEMOS    OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    lvgl
    GIT_REPOSITORY https://github.com/lvgl/lvgl.git
    GIT_TAG        ${LV_TAG}
    GIT_SHALLOW    TRUE
    GIT_PROGRESS   TRUE
)

string(TIMESTAMP _fetch_start "%s")
FetchContent_MakeAvailable(lvgl)
string(TIMESTAMP _fetch_end "%s")
math(EXPR _fetch_elapsed "${_fetch_end} - ${_fetch_start}")

message(STATUS "")
message(STATUS "Step 2/3 — LVGL ✓  (${_fetch_elapsed}s)")
message(STATUS "           Source : ${lvgl_SOURCE_DIR}")
message(STATUS "           Config : ${LV_CONF_PATH}")
message(STATUS "")

# ── Step 3 — Configure targets ────────────────────────────────────────────────
message(STATUS "Step 3/3 — Configuring targets...")

set(PANEL_GENERIC 0)
set(PANEL_FM6126A 1)
set(PANEL_RUL6024 2)

set(CHAIN_MODE_SERPENTINE 0)
set(CHAIN_MODE_RASTER 1)

add_executable(${PROJECT_NAME} hub75_lvgl.cpp)

pico_set_program_name(${PROJECT_NAME} "${PROJECT_NAME}")
pico_set_program_version(${PROJECT_NAME} "3.3")

# No need to modify preprocessor defines in hub75.cpp - instead set their values here.
#
# Example:
# Settings for a RP2350B microcontroller with GPIO pins spanning from 30 to 43.
# Beware to set `PICO_PLATFORM rp2350` and `PICO_BOARD none` prior to `include(pico_sdk_import.cmake)`
target_compile_definitions(hub75_lvgl PRIVATE
    # PICO_RP2350A=0              # PICO_RP2350A=0` means not a RP2350A but a RP2350B microcontroller - uncomment for RP235xB microcontroller only!
    USE_PICO_GRAPHICS=true      # set to false if you use hub75 as a library - any reference to pico_graphics is removed
    MATRIX_PANEL_WIDTH=64       # your matrix panel width
    MATRIX_PANEL_HEIGHT=64      # your matrix panel height
    CHAIN_MODE=CHAIN_MODE_SERPENTINE # set chain-mode - default is serpentine (U-Turn with compensation for 180° rotation)
    CHAIN_COLS=2                # number of panels chained left-to-right in a single chain row (columns)
    CHAIN_ROWS=1                # number of chain rows stacked vertically (rows)
    DATA_BASE_PIN=0             # base GPIO pin (aka start index) of R0, G0, B0, R1, G1, B1 GPIO pins - use 30 for PICO_RP2350B
    DATA_N_PINS=6               # number (count) of colour pins (usually 6)
    ROWSEL_BASE_PIN=6           # base GPIO address pin (aka start index) of A, B (, C, D. E) GPIO pins - use 36 for PICO_RP2350B
    ROWSEL_N_PINS=5             # number (count) of address pins available on your matrix panel board (look at your panels connector)
    CLK_PIN=11                  # GPIO pin for CLK - use pin 41 for PICO_RP2350B
    STROBE_PIN=12               # GPIO pin for STROBE (LATCH) - use pin 42 for PICO_RP2350B
    OEN_PIN=13                  # GPIO for OE pin - use pin 43 for PICO_RP2350B
    HUB75=1                     # 
    PANEL_TYPE=PANEL_RUL6024    # select PANEL_TYPE
    INVERTED_STB=false          # inverted pin signal for OE (untested)
    SM_CLOCKDIV_FACTOR=1.0f     # to prevent flicker or ghosting it might be worth a try to reduce state machine speed
    BITPLANES=8                 # number (count) of bit-planes used for BCM (Binary Code Modulation) - valid values for BIT_DEPTH are 8 or 10
    BALANCED_LIGHT_OUTPUT=false # allthough it uses some more memory it improves effective refresh rate and really cuts down flicker
    SEPARATE_CIE_CHANNELS=false # use separate CIE channels for improved colour representation - needs more memory
    CCM_RG_SHIFT=6              # CCM Cross-channel mixing - mix ~1.6% green into the red channel
    CCM_GB_SHIFT=7              # CCM Cross-channel mixing - mix ~0.8% blue into the green channel
    BASE_LATCH_NS=80            # wait time in nano-seconds to stabilise latch
    BASE_ADDR_NS=120            # wait time in nano-seconds to stabilise row addressing
    BASE_OE_NS=40               # pre-Oe guard wait time in nano-seconds (prevents ghost flashes)
    HUB75_MULTICORE=true        # use core1 for the hub75 driver
    FRAME_RATE=false            # for testing and debugging purpose only: output frame rate information (printf) in monitor - set to `false` for production
)

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(${PROJECT_NAME} 0)
pico_enable_stdio_usb(${PROJECT_NAME}  1)

# Generate PIO header
pico_generate_pio_header(${PROJECT_NAME} ${CMAKE_CURRENT_LIST_DIR}/src/hub75.pio)

target_sources(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/src/hub75.cpp
    ${CMAKE_CURRENT_LIST_DIR}/hub75_lvgl.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/fm6126a.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/rul6024.cpp
    ${CMAKE_CURRENT_LIST_DIR}/examples/bouncing_balls.cpp
    ${CMAKE_CURRENT_LIST_DIR}/examples/image_animation.hpp
    ${CMAKE_CURRENT_LIST_DIR}/examples/fire_effect.cpp
    ${CMAKE_CURRENT_LIST_DIR}/examples/colour_check.cpp
)

# Add path to include files to the build
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}           # lv_conf.h
    ${CMAKE_CURRENT_LIST_DIR}/src
    ${CMAKE_CURRENT_LIST_DIR}/examples
    ${lvgl_SOURCE_DIR}                  # resolves "lvgl/src/..." style includes
)

# Add any user requested libraries
target_link_libraries(${PROJECT_NAME}
    pico_stdlib
    pico_multicore
    hardware_pio
    hardware_dma
    hardware_timer
    lvgl
)

if(PICO_CYW43_SUPPORTED)
    target_link_libraries(${PROJECT_NAME} pico_cyw43_arch_none)
endif()

pico_add_extra_outputs(${PROJECT_NAME})

message(STATUS "Step 3/3 — Targets configured ✓")
message(STATUS "")
message(STATUS "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━")
message(STATUS "  Configuration complete.")
message(STATUS "  Press Build (Ctrl+Shift+B) or click the")
message(STATUS "  build button in the status bar.")
message(STATUS "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━")
message(STATUS "")