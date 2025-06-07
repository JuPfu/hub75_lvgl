#include "pico/stdio.h"
#include "pico/stdlib.h"
#include "pico/printf.h"
#include "pico/multicore.h"
#include "hardware/clocks.h"

#include "hub75.hpp"
#include "lvgl/src/lv_init.h"
#include "lvgl/src/core/lv_refr.h"
#include "lvgl/src/display/lv_display.h"
#include "lvgl/src/tick/lv_tick.h"

#include "bouncing_balls.hpp"
#include "fire_effect.hpp"
#include "image_animation.hpp"
#include "colour_check.hpp"

//--------------------------------------------------------------------------------
// Constants and Globals
//--------------------------------------------------------------------------------

#define RGB_MATRIX_WIDTH 64                               ///< Display width in pixels
#define RGB_MATRIX_HEIGHT 64                              ///< Display height in pixels
#define OFFSET RGB_MATRIX_WIDTH *(RGB_MATRIX_HEIGHT >> 1) ///< Mid-point index for symmetrical buffers

#define BYTES_PER_PIXEL (LV_COLOR_FORMAT_GET_SIZE(LV_COLOR_FORMAT_RGB888)) ///< RGB888 color depth

/// @brief Enum for selecting animation demos
enum DemoIndex
{
    // DEMO_CLOCK,
    DEMO_BOUNCE,
    DEMO_FIRE,
    DEMO_IMAGE,
    DEMO_COLOUR,
    DEMO_COUNT
};

static critical_section_t crit_sec = {0};                                   ///< Synchronization for safe time reading
static int frame_index = DEMO_BOUNCE;                                       ///< Current demo index
static uint8_t buf1[RGB_MATRIX_WIDTH * RGB_MATRIX_WIDTH * BYTES_PER_PIXEL]; ///< Drawing buffer for LVGL

static lv_display_t *display1; ///< LVGL display handle

static bool load_anim = true; ///< Flag to trigger animation setup

//--------------------------------------------------------------------------------
// Utility Functions
//--------------------------------------------------------------------------------

/**
 * @brief Retrieve the number of milliseconds elapsed since system boot.
 *
 * This function returns a 32-bit unsigned integer representing the number of
 * milliseconds since the system was powered on or reset. It is safe to call
 * from within an LVGL tick callback and is designed to provide consistent time
 * values even when used in concurrent or interrupt-driven environments.
 *
 * The access to `get_absolute_time()` is wrapped in a critical section to
 * ensure atomicity and consistency on multicore or preemptive systems like the
 * RP2040. This prevents potential race conditions if `get_absolute_time()` is
 * not atomic.
 *
 * @return The time since boot in milliseconds.
 */
uint32_t get_milliseconds_since_boot()
{
    critical_section_enter_blocking(&crit_sec);
    uint32_t ms = to_ms_since_boot(get_absolute_time());
    critical_section_exit(&crit_sec);
    return ms;
}

/**
 * @brief Display flush callback for LVGL to update the Hub75 framebuffer.
 *
 * This function is called by LVGL when a part of the screen (or the entire screen)
 * needs to be flushed to the physical display. The pixel data is provided in
 * a linear buffer `px_map` which contains color data (e.g., in RGB888 format,
 * depending on LVGL configuration).
 *
 * For the Hub75 driver, we assume that the entire screen is updated each time
 * (full frame flush), and the buffer is passed to `update_bgr()` which converts
 * and writes it to the physical framebuffer or triggers a transfer.
 *
 * After the pixel data is processed, `lv_display_flush_ready()` must be called
 * to inform LVGL that the flush is complete, allowing it to reuse or update the
 * drawing buffer.
 *
 * @param display The LVGL display object.
 * @param area Area being updated (not used here).
 * @param px_map Pointer to pixel buffer.
 */
void flush_cb(lv_display_t *display, const lv_area_t *area, uint8_t *px_map)
{
    update_bgr(px_map);              ///< Transfer buffer to display driver
    lv_display_flush_ready(display); ///< Notify LVGL that flush is complete
}

/**
 * @brief Timer callback to cycle to the next demo.
 *
 * Called every 15 seconds to switch to the next animation mode.
 *
 * @param t Unused timer pointer.
 * @return true (always continue the timer).
 */
bool skip_to_next_demo(__unused struct repeating_timer *t)
{
    printf("skip_to_next_demo %d\n", frame_index);
    if (frame_index++ >= DEMO_COLOUR)
        frame_index = DEMO_BOUNCE;
    load_anim = true;
    return true;
}

/**
 * @brief Secondary core entry point.
 *
 * Initializes and starts the HUB75 driver on core 1.
 */
void core1_entry()
{
    create_hub75_driver(RGB_MATRIX_WIDTH, RGB_MATRIX_HEIGHT);
    start_hub75_driver();
}

/**
 * @brief Initializes the Pico system and launches core 1.
 */
void initialize()
{
    // Set system clock to 250MHz - just to show that it is possible to drive the HUB75 panel with a high clock speed
    set_sys_clock_khz(250000, true);
    stdio_init_all();
    critical_section_init(&crit_sec);

    // led_init(); // Initialize LED - blinking at program start

    multicore_reset_core1(); // Reset core 1

    multicore_launch_core1(core1_entry); // Launch core 1 entry function - the Hub75 driver is doing its job there
}

/**
 * @brief Sets up the selected animation.
 *
 * This function initializes the animation scene based on the current demo index.
 *
 * @param index Current demo index.
 * @param bouncingBalls Bouncing ball animation instance.
 * @param fireEffect Fire effect instance.
 * @param imageAnimation Image animation instance.
 * @param colorCheck display colour squares
 * @param timer Reference to the demo-switching timer.
 */
void setup_demo(int index, BouncingBalls &bouncingBalls, FireEffect &fireEffect, ImageAnimation &imageAnimation, ColourCheck &colourCheck, struct repeating_timer &timer)
{
    switch (index)
    {
    // case DEMO_CLOCK:
    //     printf("setup_demo DEMO_CLOCK\n");
    //     flipClock.show();
    //     break;
    case DEMO_BOUNCE:
        bouncingBalls.show();
        break;
    case DEMO_FIRE:
        fireEffect.show();
        break;
    case DEMO_IMAGE:
        cancel_repeating_timer(&timer); // prevent premature transition
        imageAnimation.show();
        imageAnimation.start();
        break;
    case DEMO_COLOUR:
        colourCheck.show();
        break;
    }
}

/**
 * @brief Updates the current animation each frame.
 *
 * Handles per-frame logic such as animation updates and polling for completion.
 *
 * @param index Current demo index.
 * @param bouncingBalls Bouncing ball animation instance.
 * @param fireEffect Fire effect instance.
 * @param imageAnimation Image animation instance.
 * @param colorCheck display colour squares
 * @param timer Reference to the demo-switching timer.
 */
void update_demo(int index, BouncingBalls &bouncingBalls, FireEffect &fireEffect, ImageAnimation &imageAnimation, ColourCheck &colourCheck, struct repeating_timer &timer)
{
    switch (index)
    {
    // case DEMO_CLOCK:
    //     // printf("update_demo DEMO_CLOCK\n");
    //     flipClock.tick();
    //     break;
    case DEMO_BOUNCE:
        bouncingBalls.bounce();
        break;
    case DEMO_FIRE:
        fireEffect.burn();
        break;
    case DEMO_IMAGE:
        if (imageAnimation.animation_done())
        {
            imageAnimation.animation_init();
            add_repeating_timer_ms(15000, skip_to_next_demo, NULL, &timer);
        }
        break;
    case DEMO_COLOUR:
        colourCheck.colour_test();
        break;
    }
}

//--------------------------------------------------------------------------------
// Main Entry Point
//--------------------------------------------------------------------------------

/**
 * @brief Main function.
 *
 * Initializes system, configures LVGL, registers the display driver, and enters
 * the main animation loop.
 *
 * @return int Return code.
 */
int main()
{
    initialize();

    sleep_ms(10000); // Allow screen + hardware to stabilize

    lv_init();
    lv_tick_set_cb(get_milliseconds_since_boot);

    display1 = lv_display_create(RGB_MATRIX_WIDTH, RGB_MATRIX_HEIGHT);
    if (display1 == NULL)
    {
        printf("lv_display_create failed\n");
        return -1;
    }

    lv_display_set_buffers_with_stride(display1, buf1, NULL, sizeof(buf1), RGB_MATRIX_WIDTH * 3, LV_DISPLAY_RENDER_MODE_FULL);
    lv_display_set_flush_cb(display1, flush_cb);

    // The Hub75 driver is constantly running on core 1 with a frequency much higher than 200Hz. CPU load on core 1 is low due to DMA and PIO usage.
    // The animated examples are updated at 60Hz.
    const float fps = 120.0f;
    const float frame_delay_ms = 1000.0f / fps;

    BouncingBalls bouncingBalls(15, RGB_MATRIX_WIDTH, RGB_MATRIX_HEIGHT);
    FireEffect fireEffect(RGB_MATRIX_WIDTH, RGB_MATRIX_HEIGHT);
    ImageAnimation imageAnimation(RGB_MATRIX_WIDTH, RGB_MATRIX_HEIGHT);
    ColourCheck colourCheck(RGB_MATRIX_WIDTH, RGB_MATRIX_HEIGHT);

    struct repeating_timer timer;
    add_repeating_timer_ms(15000, skip_to_next_demo, NULL, &timer);

    while (true)
    {
        if (load_anim)
        {
            load_anim = false;
            setup_demo(frame_index, bouncingBalls, fireEffect, imageAnimation, colourCheck, timer);
        }

        update_demo(frame_index, bouncingBalls, fireEffect, imageAnimation, colourCheck, timer);

        lv_timer_handler();
        sleep_ms(frame_delay_ms);
    }
}
