# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.3.0)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.3.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================

# set(PICO_BOARD pico2 CACHE STRING "Board type")

# Uncomment the following two lines to compile for bare RP2350 without a board
set(PICO_PLATFORM rp2350)
set(PICO_BOARD none CACHE STRING "Board type")

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
message(STATUS "Step 1/4 — Initialising Pico SDK...")
# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()
message(STATUS "Step 1/4 — Pico SDK ✓  (${PICO_SDK_VERSION_STRING})")
message(STATUS "")

add_executable(${PROJECT_NAME} hub75_lvgl.cpp)

pico_set_program_name(${PROJECT_NAME} "${PROJECT_NAME}")
pico_set_program_version(${PROJECT_NAME} "3.4.0")

# ── Step 2 — Fetch LVGL ───────────────────────────────────────────────────────

# Set LV_TAG to the LVGL version you want to use
# Do not forget to configure lv_conf.h!!! See README.md for details. 
set(LV_TAG v9.5.0)

message(STATUS "Step 2/4 — Fetching LVGL ${LV_TAG} from GitHub...")
message(STATUS "           First clone may take 1-2 minutes.")
message(STATUS "           Subsequent builds use the local cache instantly.")
message(STATUS "")

include(FetchContent)

# Allow FetchContent_Populate() without deprecation warning
cmake_policy(SET CMP0169 OLD)

# Show git clone progress in the Output panel
set(FETCHCONTENT_QUIET OFF CACHE BOOL "" FORCE)

# Clone LVGL/hub75 into a persistent ".deps" folder OUTSIDE the build tree.
# This means deleting build/ for a clean build no longer forces a full
# re-clone of both dependencies from GitHub.
set(FETCHCONTENT_BASE_DIR ${CMAKE_SOURCE_DIR}/.deps CACHE PATH
    "Location for FetchContent sources (kept outside build/ on purpose)" FORCE)

# Pass -DHUB75_LVGL_FRESH_DEPS=ON to wipe the dependency cache and re-fetch
# LVGL/hub75 from scratch (e.g. after pinning a new HUB75_TAG/LV_TAG).
option(HUB75_LVGL_FRESH_DEPS "Wipe cached FetchContent sources in .deps and re-fetch from scratch" OFF)
if(HUB75_LVGL_FRESH_DEPS)
    message(STATUS "HUB75_LVGL_FRESH_DEPS=ON — removing ${FETCHCONTENT_BASE_DIR}")
    file(REMOVE_RECURSE ${FETCHCONTENT_BASE_DIR})
endif()

# Tell LVGL where lv_conf.h lives — must be set before FetchContent_MakeAvailable
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
message(STATUS "Step 2/4 — LVGL ✓  (${_fetch_elapsed}s)")
message(STATUS "           Source : ${lvgl_SOURCE_DIR}")
message(STATUS "           Config : ${LV_CONF_PATH}")
message(STATUS "")

# Specify a tag from the hub75 project to fall back to a specific version
# set(HUB75_TAG v4.0.0)
# Set HUB75_TAG to main to compile with the latest version
# NOTE: tracking "main" means this build can silently break the moment a breaking PR merges upstream (e.g. an API-changing PR). 
# Prefer pinning to a tag or specific commit hash once such changes land, then update deliberately.
set(HUB75_TAG dual_instance_driver_0.9)

FetchContent_Declare(
    hub75
    GIT_REPOSITORY https://github.com/JuPfu/hub75.git
    GIT_TAG        ${HUB75_TAG}
    GIT_SHALLOW    TRUE
    GIT_PROGRESS   TRUE
)

string(TIMESTAMP _fetch_start "%s")
FetchContent_Populate(hub75)
string(TIMESTAMP _fetch_end "%s")
math(EXPR _fetch_elapsed "${_fetch_end} - ${_fetch_start}")

message(STATUS "")
message(STATUS "Step 3/4 — Hub75 ✓  (${_fetch_elapsed}s)")
message(STATUS "           Source : ${hub75_SOURCE_DIR}")
message(STATUS "           Include : ${hub75_SOURCE_DIR}/include")
message(STATUS "")

# Use local cie.py to generate cie.hpp
find_package(Python3 REQUIRED COMPONENTS Interpreter)

set(CIE_HPP ${hub75_SOURCE_DIR}/include/cie.hpp)

add_custom_command(
    OUTPUT  ${CIE_HPP}
    COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_LIST_DIR}/utils/cie.py > ${CIE_HPP}
    DEPENDS ${CMAKE_CURRENT_LIST_DIR}/utils/cie.py
    COMMENT "Generating cie.hpp from cie.py"
    VERBATIM
)

add_custom_target(generate_cie_hpp DEPENDS ${CIE_HPP})
add_dependencies(${PROJECT_NAME} generate_cie_hpp)

# ── Step 3 — Configure targets ────────────────────────────────────────────────
message(STATUS "Step 4/4 — Configuring targets...")



# No need to modify preprocessor defines in hub75.cpp - instead set their values here.
#
# Example:
# Settings for a RP2350B microcontroller with GPIO pins spanning from 30 to 43.
# Beware to set `PICO_PLATFORM rp2350` and `PICO_BOARD none` prior to `include(pico_sdk_import.cmake)`
target_compile_definitions(hub75_lvgl PRIVATE
    PICO_RP2350A=0              # PICO_RP2350A=0` means not a RP2350A but a RP2350B microcontroller - uncomment for RP235xB microcontroller only!
    HUB75_MULTICORE=true        # use core1 for the hub75 driver
)

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(${PROJECT_NAME} 0)
pico_enable_stdio_usb(${PROJECT_NAME}  1)

# Generate PIO header
pico_generate_pio_header(${PROJECT_NAME} ${hub75_SOURCE_DIR}/src/hub75.pio)

target_sources(${PROJECT_NAME} PRIVATE
    ${hub75_SOURCE_DIR}/src/hub75.cpp
    ${hub75_SOURCE_DIR}/src/fm6126a.cpp
    ${hub75_SOURCE_DIR}/src/rul6024.cpp
    ${CMAKE_CURRENT_LIST_DIR}/hub75_lvgl.cpp
    ${CMAKE_CURRENT_LIST_DIR}/examples/bouncing_balls.hpp
    ${CMAKE_CURRENT_LIST_DIR}/examples/fire_effect.hpp
    ${CMAKE_CURRENT_LIST_DIR}/examples/colour_check.hpp
)

target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}           # lv_conf.h
    ${hub75_SOURCE_DIR}/src             # hub75.pio.h (generated), hub75.cpp internals
    ${hub75_SOURCE_DIR}/include         # hub75.hpp  ← add this
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

message(STATUS "Step 4/4 — Targets configured ✓")
message(STATUS "")
message(STATUS "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━")
message(STATUS "  Configuration complete.")
message(STATUS "  Press Build (Ctrl+Shift+B) or click the build button in the status bar.")
message(STATUS "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━")
message(STATUS "")