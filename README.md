# LVGL Demo on Raspberry Pi Pico with Attached HUB75 RGB LED Matrix

This project demonstrates how to run [LVGL](https://lvgl.io/) on a Raspberry Pi Pico (RP2040 or RP2040-compatible microcontroller) to drive an attached HUB75 RGB LED matrix panel.

## Last Changes

I followed [cmair](https://github.com/cmair)'s suggestion to fetch (download) `LVGL` during the build process. This results in weaker coupling with `LVGL`. It also makes it easy to switch to a different version of `LVGL` by setting `LV_TAG` to the desired version in `CMakeLists.txt`, e.g., `set(LV_TAG v9.4.0)`.

## Demo Effects

This project currently demonstrates three animated demos:

- 🎾 **Bouncing Balls** — includes circular horizontal scrolling text (15 sec)
- 🔥 **Fire Effect** — Animated flame using procedural effect (15 sec)
- 🖼️ **Image Animation** — Rotating static image for 360 degrees, then idle for 15 sec

✨ Transitions (fade or slide) are applied between demos.

💡 At [reddit.com in the raspberrypipico group](https://www.reddit.com/r/raspberrypipico/comments/1kmegkv/lvgl_on_raspberry_pi_pico_driving_hub75_rgb_led/) a video shows the listed demo effects. The video is titled "LVGL on Raspberry Pi Pico driving HUB75 RGB LED Matrix".

---

## Project Goals and Focus

The LED matrix driver used in this project is an evolution of [Pimoroni's HUB75 driver](https://github.com/pimoroni/pimoroni-pico/tree/main/drivers/hub75) which leans on [Raspberry Pi's pico-examples/pio/hub75](https://github.com/raspberrypi/pico-examples). It is an optimised driver which boosts performance through self-paced, interlinked DMA and PIO processes. The LED Matrix driver implementation is described in detail in [Hub75](https://github.com/JuPfu/hub75). In this referenced project the examples utilise [Pimoroni's Pico Graphics library](https://github.com/pimoroni/pimoroni-pico/tree/main/libraries/pico_graphics) to show the capabilities of the LED matrix driver. `Pimoroni's Pico Graphics library` is a tiny graphics library ...
> which supports drawing text, primitive and individual pixels and includes basic types such as Rect and Point brimming with methods to help you develop games and applications.

The goal of this project is to substitute `Pimoroni's Pico Graphics library` with the **[Light and Versatile Graphics Library](https://lvgl.io/)** (LVGL), which claims ...
> to be the most popular free and open-source embedded graphics library to create beautiful UIs for any MCU, MPU and display type.

## Hardware Setup

- **Controller**: Raspberry Pi Pico (RP2040 or RP2040-compatible)
- **Display**: 64×64 HUB75 RGB LED matrix panel  
  > ⚠️ Other panel sizes can be supported with small adjustments
- **Power**: External 5V supply for the LED matrix is required

---

## Core Distribution Diagram

```plaintext
+----------------------+       +----------------------+
|        Core 0        |       |        Core 1        |
|                      |       |                      |
|  - LVGL              |       |  - HUB75 Driver      |
|  - Demo Effects      |       |                      |
+----------------------+       +----------------------+
```

The HUB75 driver runs on **core 1**, utilizing **PIO** and **DMA**, freeing up **core 0** for LVGL rendering and animation logic.

---

## Building the Project

This section walks through everything needed to go from a fresh clone to a flashable `.uf2` file — whether you prefer the command line or VSCode.

### What happens during a build (in plain words)

Building this project involves three things that happen automatically, so you normally don't need to set anything up by hand:

1. **The Raspberry Pi Pico SDK** is located (or downloaded if missing).
2. **LVGL** (the graphics library) is downloaded once and cached in `build/_deps/`.
3. **CMake** generates build files, and **Ninja** compiles everything into `build/hub75_lvgl.uf2`.

Steps 1 and 2 only take noticeable time on the **very first build**. After that, everything is cached and rebuilds are fast.

> 📡 **You need an internet connection for the first build**, since LVGL is fetched from GitHub.

---

### Option A — Build from the Command Line (macOS / Linux)

```bash
git clone https://github.com/JuPfu/hub75_lvgl
cd hub75_lvgl
./build.sh
```

That's it. `build.sh` does the following for you:

```bash
rm -rf build      # start with a clean build directory
mkdir build
cd build

cmake .. -G Ninja -DCMAKE_BUILD_TYPE=Release   # configure the project, fetch LVGL if needed
ninja                                          # compile everything
```

While `cmake` runs for the first time, you'll see output showing LVGL being downloaded — this is normal and may take 1–2 minutes depending on your connection. Subsequent runs skip this step.

When the build finishes successfully, you'll find the firmware here:

```
build/hub75_lvgl.uf2
```

#### Flashing the firmware

Connect your Pico in BOOTSEL mode (hold the BOOTSEL button while plugging it in), then run:

```bash
picotool load -f -x ./build/hub75_lvgl.uf2
```

> 🐧 Not yet tested on Linux, but it should work the same way. If you try it, feedback is welcome!

---

### Option B — Build with VSCode

If you'd rather use a graphical interface, VSCode with the **Raspberry Pi Pico extension** handles most of this for you.

**1. Clone the repository**

- Press `Ctrl+Shift+P` and select `Git: Clone`
- Paste the URL: `https://github.com/JuPfu/hub75_lvgl`

  <img src="assets/VSCode_1.png" width="460" height="116">

- Choose a local directory to clone into

  <img src="assets/VSCode_2.png" width="603" height="400">

**2. Accept the project import prompt**

When VSCode asks *"Do you want to import this project as Raspberry Pi Pico project?"*, click **Yes** (or just wait — it proceeds automatically after a few seconds).

  <img src="assets/VSCode_3.png" width="603" height="400">

**3. Configure Pico SDK settings**

A settings page opens automatically. The default settings work fine for most setups.

  <img src="assets/VSCode_4.png" width="603" height="400">

- Click **Import** to finish setup
- Switch the board type to match your Pico model (e.g. Pico, Pico 2)

  <img src="assets/VSCode_5.png" width="599" height="415">

**4. Wait for setup to complete**

VSCode downloads the Pico SDK, toolchain, and extension dependencies. A status message ("Activating content...") shows this is in progress — this can take a few minutes the first time.

**5. Connect your hardware**

- Wire up the HUB75 LED matrix to your Pico
- Plug the Pico into your computer via USB

**6. Build and upload**

Click the **Run** button in the bottom status bar.

  <img src="assets/VSCode_6.png" width="600" height="416">

> ⏳ **The first build takes longer** — LVGL is downloaded into `build/_deps/` during this step. You'll see progress messages in the **Output** panel (select the *CMake/Build* channel). Every build after that is much faster, since LVGL is already cached locally.

If everything is wired up correctly, your LED matrix should light up with the demo animations! 🎉

---

### Troubleshooting the Build

| Symptom | Likely cause / fix |
|---|---|
| First build hangs at "Activating content" | This is normal — LVGL is being downloaded. Check the **Output → CMake/Build** panel for progress. |
| `fatal error: lvgl/...: No such file or directory` | LVGL include paths changed — make sure source files use `#include "lvgl.h"`. |
| CMake errors about duplicate `lvgl` targets | Run `rm -rf build` and reconfigure — a stale `build/` directory can conflict with a fresh LVGL fetch. For VSCode manualy create a new build directory before compiling again.|
| Build seems stuck with no output | Try running from the command line (`./build.sh`) instead, where progress is more visible. |

---

## Integrating a Different LVGL Version

Currently **LVGL v9.4.0** is integrated into the project. To switch to a different version:

1. **Set the LVGL version** — edit the `LV_TAG` variable near the top of `CMakeLists.txt` (around line 49) to the tag 🏷️ of the version you want (e.g. `v9.5.0`). Valid tags are listed on the [LVGL Releases page](https://github.com/lvgl/lvgl/releases).

2. **Update `lv_conf.h`** for the new version:
   - After a build, copy `lv_conf_template.h` from `build/_deps/lvgl-src/` to the project's top-level directory
   - Rename it to `lv_conf.h`
   - Adjust settings to match your needs (use the existing `lv_conf.h` in this project as a reference)

3. **Rebuild** — run `./build.sh` again. The new LVGL version will be fetched automatically.

Your directory structure should look like this:

```bash
assets
build
examples
src
utils
build.sh
CMakeLists.txt
hub75_lvgl.cpp
lv_conf.h
README.md
```

---

## Connecting LVGL to the HUB75 Driver

The steps below describe how **LVGL** is connected to the HUB75 driver in this project. This can be the basis for your modifications.

### 1. Millisecond Tick Source

```c
uint32_t get_milliseconds_since_boot()
{
    critical_section_enter_blocking(&crit_sec);
    uint32_t ms = to_ms_since_boot(get_absolute_time());
    critical_section_exit(&crit_sec);
    return ms;
}
```

### 2. Display Flush Callback

Connects LVGL's draw buffer to the HUB75 display. The `area` parameter is not used, since LVGL is configured to always pass the complete display buffer (see [Choose LV_DISPLAY_RENDER_MODE_FULL](#3-choose-lv_display_render_mode_full)).

```c
void flush_cb(lv_display_t *display, const lv_area_t *area, uint8_t *px_map)
{
    update(px_map);                  // Transfer buffer to HUB75 driver
    lv_display_flush_ready(display); // Notify LVGL that flush is complete
}
```

> `update()` is provided by the optimised [`hub75`](https://github.com/JuPfu/hub75/blob/main/hub75.cpp) driver.

### 3. Choose LV_DISPLAY_RENDER_MODE_FULL

With `LV_DISPLAY_RENDER_MODE_FULL`, the buffer size must match the size of the display, and LVGL renders directly into the correct location of that buffer. The buffer therefore always contains the complete display image.

```c
    lv_init();
    lv_tick_set_cb(get_milliseconds_since_boot);

    display1 = lv_display_create(MATRIX_PANEL_WIDTH, MATRIX_PANEL_HEIGHT);
    if (display1 == NULL)
    {
        printf("lv_display_create failed\n");
        return -1;
    }

    lv_display_set_buffers_with_stride(display1, buf1, NULL, sizeof(buf1), MATRIX_PANEL_WIDTH * 3, LV_DISPLAY_RENDER_MODE_FULL);
    lv_display_set_flush_cb(display1, flush_cb);
```

### 4. Periodic Timer Handler Call

In your main loop, call `lv_timer_handler()`:

```c
while (true)
{
    if (load_anim)
    {
        load_anim = false;
        setup_demo(frame_index, bouncingBalls, fireEffect, imageAnimation, timer);
    }

    update_demo(frame_index, bouncingBalls, fireEffect, imageAnimation, timer);

    lv_timer_handler();
    sleep_ms(frame_delay_ms);
}
```

## Dependencies

- [LVGL](https://github.com/lvgl/lvgl)
- [hub75_lvgl](https://github.com/JuPfu/hub75_lvgl) (custom optimized driver)
- CMake build system (standard for Pico SDK projects)

---

## Tested Hardware

⚠️ The examples contained in `hub75_lvgl.cpp` have been tested with a Raspberry Pi Pico 2 (RP2350). On a Raspberry Pi Pico (RP2040), you may need to comment out some demo effects due to its more limited memory.

Ask if you need support 🙂

---

## Next Steps

- **Add more graphics examples** to explore the capabilities of LVGL on Pico.

## Support

Any contribution to the project is appreciated!

For any question or problem, feel free to open an issue!

## License

[MIT License](https://github.com/JuPfu/hub75#MIT-1-ov-file)

---

[![License](https://img.shields.io/github/license/JuPfu/hub75_lvgl)](LICENSE)
[![Platform](https://img.shields.io/badge/Platform-Raspberry%20Pi%20Pico-blue)]()
[![LVGL](https://img.shields.io/badge/Graphics-LVGL-orange)](https://lvgl.io/)
