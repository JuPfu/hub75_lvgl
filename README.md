# LVGL Demo on Raspberry Pi Pico with HUB75 RGB LED Matrix

⚠️ **Work in progress - this is an interim version which will definitely change a lot**

This project demonstrates how to run [LVGL](https://lvgl.io/) on a Raspberry Pi Pico (RP2040 or RP2040-compatible) to drive a HUB75 RGB LED matrix panel using the optimized DMA-based [hub75 driver](https://github.com/JuPfu/hub75).

It showcases three demo effects:
- **Bouncing Balls** – includes circular vertical scrolling text (15 seconds)
- **Fire Effect** – a 15-second animated flame effect
- **Image Animation** – displays a rotating image for two full rotations, followed by a 15-second pause

Transition effects like fade-ins or slide-outs are applied between demos.

---

## Hardware Setup

- **Controller**: Raspberry Pi Pico (RP2040 or RP2040-compatible)
- **Display**: 64×64 HUB75 RGB LED matrix panel  
  > _Note: Other panel sizes can be supported with small adjustments._
- **Power**: External 5V supply for the LED matrix is required

---

## Core Distribution Diagram

```plaintext
+------------------------+
|        Core 0          |
|                        |
|  - LVGL                |
|  - Demo Effects        |
+------------------------+

+------------------------+
|        Core 1          |
|                        |
|  - HUB75 Driver        |
+------------------------+
```

The HUB75 driver runs on **core 1**, utilizing **PIO** and **DMA**, freeing up **core 0** for LVGL rendering and animation logic.

---

## Integrating LVGL into a Pico Project

1. **Download** the latest version of [LVGL](https://github.com/lvgl/lvgl)
2. **Extract** the zip and rename the folder to `lvgl`
3. **Copy** it into your project's top-level directory
4. **Add this CMake snippet** to your `CMakeLists.txt`:

```cmake
# LVGL configuration
message(NOTICE "===>>> LVGL configuration start ===")

set(LVGL_DIR_NAME lvgl)
set(LVGL_DIR ${CMAKE_CURRENT_LIST_DIR})
set(LV_CONF_PATH ${CMAKE_CURRENT_LIST_DIR}/lv_conf.h)
set(LV_CONF_INCLUDE_SIMPLE ${CMAKE_CURRENT_LIST_DIR}/lv_conf.h)

message(NOTICE "LVGL folder name: ${LVGL_DIR_NAME}")
message(NOTICE "Path to LVGL folder: ${LVGL_DIR}")
message(NOTICE "Path to config file: ${LV_CONF_PATH}")
message(NOTICE "Include path: ${LV_CONF_INCLUDE_SIMPLE}")

add_subdirectory(${LVGL_DIR_NAME})

message(NOTICE "=== LVGL configuration end <<<===")
```

5. **Configure LVGL**:
   - Copy `lv_conf_template.h` to your top-level directory
   - Rename it to `lv_conf.h`
   - Modify it to match your needs (use this project as reference)
6. Follow the [LVGL Integration Guide](https://docs.lvgl.io/master/details/integration/index.html)

---

## Connecting LVGL to the HUB75 Driver

To render LVGL output on the HUB75 panel, you need:

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

Connects LVGL's draw buffer to the HUB75 display:

```c
void flush_cb(lv_display_t *display, const lv_area_t *area, uint8_t *px_map)
{
    update_bgr(px_map);              // Transfer buffer to HUB75 driver
    lv_display_flush_ready(display); // Notify LVGL that flush is complete
}
```

> `update_bgr()` is provided by the custom [`hub75`](https://github.com/JuPfu/hub75) driver.

### 3. Periodic Timer Handler Call

In your main loop, call:

```c
while (true)
{
    if (load_anim)
    {
        load_anim = false;
        setup_demo(frame_index, bouncingBalls, fireEffect, imageAnimation, timer);
    }

    update_demo(frame_index, bouncingBalls, fireEffect, imageAnimation, timer);

    lv_timer_handler_run_in_period(frame_delay_ms);
    sleep_ms(frame_delay_ms / 2);
}
```



## Dependencies

* [LVGL](https://github.com/lvgl/lvgl)
* [hub75](https://github.com/JuPfu/hub75) (custom optimized driver)
* CMake build system (standard for Pico SDK projects)

---

## How to Use This Project in VSCode

You can easily use this project with VSCode, especially with the **Raspberry Pi Pico plugin** installed. Follow these steps:

1. **Open VSCode and start a new window**.
2. **Clone the repository**:
   - Press `Ctrl+Shift+P` and select `Git: Clone`.
   - Paste the URL: `https://github.com/JuPfu/hub75`

      <img src="assets/VSCode_1.png" width="460" height="116">

   - Choose a local directory to clone the repository into.

      <img src="assets/VSCode_2.png" width="603" height="400"> 


3. **Project Import Prompt**:
   - Consent to open the project.

      <img src="assets/VSCode_3.png" width="603" height="400"> 

   - When prompted, "Do you want to import this project as Raspberry Pi Pico project?", click **Yes** or wait a few seconds until the dialog prompt disappears by itself.

4. **Configure Pico SDK Settings**:
   - A settings page will open automatically.
   - Use the default settings unless you have a specific setup.

      <img src="assets/VSCode_4.png" width="603" height="400"> 

   - Click **Import** to finalize project setup.
   - Switch the board-type to your Pico model.

      <img src="assets/VSCode_5.png" width="599" height="415"> 

5. **Wait for Setup Completion**:
   - VSCode will download required tools, the Pico SDK, and any plugins.

6. **Connect the Hardware**:
   - Make sure the HUB75 LED matrix is properly connected to the Raspberry Pi Pico.
   - Attach the Rasberry Pi Pico USB cable to your computer

7. **Build and Upload**:
   - Compiling the project can be done without a Pico attached to the computer.

      <img src="assets/VSCode_6.png" width="600" height="416"> 

   - Click the **Run** button in the bottom taskbar.
   - VSCode will compile and upload the firmware to your Pico board.

> 💡 If everything is set up correctly, your matrix should come to life with the updated HUB75 DMA driver.

---

## Next Steps

- **Add some more graphics examples** to explore the capabilities of LVGL on Pico.

For any questions or discussions, feel free to contribute or open an issue!


## License

MIT License (or your preferred license)

---

