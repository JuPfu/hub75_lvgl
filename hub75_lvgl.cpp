#include "pico/stdio.h"
#include "pico/stdlib.h"
#include "pico/printf.h"
#include "pico/multicore.h"

// Pico W devices use a GPIO on the WIFI chip for the LED,
// so when building for Pico W, CYW43_WL_GPIO_LED_PIN will be defined
#ifdef CYW43_WL_GPIO_LED_PIN
#include "pico/cyw43_arch.h"
#endif

#include "hardware/clocks.h"

#include "hub75.hpp"

#include "lvgl.h"

// Panel/pin/color/rotation configuration - see include/hub75.hpp for field docs.
// Matches a single generic 64x64 panel wired to GPIO 0-13.
constexpr Hub75Config panel_cfg{
    .panel = {
        .matrix_panel_width = 64,
        .matrix_panel_height = 64,
        .chain_rows = 1,
        .chain_cols = 1,
        .chain_mode = Hub75ChainMode::SERPENTINE,
        .panel_kind = RowMapping::Standard,
        .panel_chip = Hub75PanelChip::GENERIC,
        .inverted_stb = false,
        .sm_clockdiv_factor = 1.0f,
        .base_latch_ns = 80,
        .base_addr_ns = 160,
    },
    .screen = {
        .rotation = Hub75Rotation::DEG_0,
    },
    .pins = {
        .data_base_pin = 30,
        .data_n_pins = 6,
        .rowsel_base_pin = 36,
        .rowsel_n_pins = 5,
        .clk_pin = 41,
        .strobe_pin = 42,
        .oen_pin = 43,
    },
    .color = {
        .bitplanes = 10,
        .separate_cie_channels = true,
        .balanced_light_output = true,
        .ccm_rg_shift = 6,
        .ccm_gb_shift = 7,
    },
    .frame_rate_debug = true,
};

using Panel = Hub75Driver<panel_cfg>;

// Large fixed-size buffers - must have static storage duration, not live on the stack.
static Panel driver;

// Example effects
#include "bouncing_balls.hpp"
#include "fire_effect.hpp"
#include "image_animation.hpp"
#include "colour_check.hpp"

//--------------------------------------------------------------------------------
// Constants and Globals
//--------------------------------------------------------------------------------

#define BYTES_PER_PIXEL (LV_COLOR_FORMAT_GET_SIZE(LV_COLOR_FORMAT_RGB888)) ///< RGB888 color depth

/// @brief Enum for selecting animation demos
enum DemoIndex
{
    DEMO_BOUNCE,
    DEMO_FIRE,
    DEMO_IMAGE,
    DEMO_COLOUR,
};

static critical_section_t crit_sec = {0};                                                     ///< Synchronization for safe time reading
static int frame_index = DEMO_BOUNCE;                                                         ///< Current demo index
alignas(4) static uint8_t buf1[Panel::SCREEN_WIDTH * Panel::SCREEN_HEIGHT * BYTES_PER_PIXEL]; ///< Drawing buffer for LVGL

static lv_display_t *display1; ///< LVGL display handle

static bool load_anim = true; ///< Flag to trigger animation setup

using BouncingBalls_T = BouncingBalls<Panel::SCREEN_WIDTH, Panel::SCREEN_HEIGHT>;
using FireEffect_T = FireEffect<Panel::SCREEN_WIDTH, Panel::SCREEN_HEIGHT>;
using ColourCheck_T = ColourCheck<Panel::SCREEN_WIDTH, Panel::SCREEN_HEIGHT>;
using ImageAnimation_T = ImageAnimation<Panel::SCREEN_WIDTH, Panel::SCREEN_HEIGHT>;
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
 * (full frame flush), and the buffer is passed to `update()` which converts
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
    driver.update_bgr(px_map);       ///< Transfer buffer to display driver
    lv_display_flush_ready(display); ///< Notify LVGL that flush is complete
}

// Perform initialisation
int pico_led_init(void)
{
#if defined(PICO_DEFAULT_LED_PIN)
    // A device like Pico that uses a GPIO for the LED will define PICO_DEFAULT_LED_PIN
    // so we can use normal GPIO functionality to turn the led on and off
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
    return PICO_OK;
#elif defined(CYW43_WL_GPIO_LED_PIN)
    // For Pico W devices we need to initialise the driver etc
    return cyw43_arch_init();
#else
    return PICO_OK;
#endif
}

// Turn the led on or off
void pico_set_led(bool led_on)
{
#if defined(PICO_DEFAULT_LED_PIN)
    // Just set the GPIO on or off
    gpio_put(PICO_DEFAULT_LED_PIN, led_on);
#elif defined(CYW43_WL_GPIO_LED_PIN)
    // Ask the wifi "driver" to set the GPIO on or off
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, led_on);
#endif
}

// Pico - please, blink LED when program starts
int led_init(void)
{
    int rc = pico_led_init(); // Initialize the LED
    hard_assert(rc == PICO_OK);

    for (int i = 0; i < 8; i++)
    {
        pico_set_led(true);
        sleep_ms(250); // Wait 250ms
        pico_set_led(false);
        sleep_ms(250); // Wait 250ms
    }
    return PICO_OK;
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
    driver.create();
    driver.start();

    // KEEP CORE 1 ALIVE — without this, Core 1's NVIC is torn down and DMA_IRQ_1 stops firing
    //
    // Add your additional tasks for core1 here
    while (true)
    {
        tight_loop_contents();
    }
}

/**
 * @brief Initializes the Pico system and launches core 1.
 */
void initialize()
{
    // Set system clock to 266MHz - just to show that it is possible to drive the HUB75 panel with a high clock speed
    set_sys_clock_khz(266000, true);

    stdio_init_all(); // Initialize Pico SDK

    critical_section_init(&crit_sec);

    led_init(); // Initialize LED - blinking at program start

    if constexpr (HUB75_MULTICORE)
    {
        // Run hub75 driver on core1
        multicore_reset_core1();             // Reset core 1
        multicore_launch_core1(core1_entry); // Launch core 1 entry function - the Hub75 driver is doing its job there
    }
    else
    {
        // Run hub75 on core0 - the Hub75 driver is doing its job here
        driver.create();
        driver.start();
    }
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
template <uint32_t W, uint32_t H>
void setup_demo(int index, BouncingBalls<W, H> &bouncingBalls, FireEffect<W, H> &fireEffect, ImageAnimation<W, H> &imageAnimation, ColourCheck<W, H> &colourCheck, struct repeating_timer &timer)
{
    switch (index)
    {
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
template <uint32_t W, uint32_t H>
void update_demo(int index, BouncingBalls<W, H> &bouncingBalls, FireEffect<W, H> &fireEffect, ImageAnimation<W, H> &imageAnimation, ColourCheck<W, H> &colourCheck, struct repeating_timer &timer)
{
    switch (index)
    {
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

    sleep_ms(1000); // Allow screen + hardware to stabilize

    lv_init();
    lv_tick_set_cb(get_milliseconds_since_boot);

    display1 = lv_display_create(Panel::SCREEN_WIDTH, Panel::SCREEN_HEIGHT);
    if (display1 == NULL)
    {
        printf("lv_display_create failed\n");
        return -1;
    }

    lv_display_set_buffers_with_stride(display1, buf1, NULL, sizeof(buf1), Panel::SCREEN_WIDTH * BYTES_PER_PIXEL, LV_DISPLAY_RENDER_MODE_FULL);
    lv_display_set_flush_cb(display1, flush_cb);

    // Only now is LVGL fully set up — safe to construct LVGL-backed objects
    static BouncingBalls<Panel::SCREEN_WIDTH, Panel::SCREEN_HEIGHT> bouncingBalls(10);
    static FireEffect<Panel::SCREEN_WIDTH, Panel::SCREEN_HEIGHT> fireEffect;
    static ColourCheck<Panel::SCREEN_WIDTH, Panel::SCREEN_HEIGHT> colourCheck;
    static ImageAnimation<Panel::SCREEN_WIDTH, Panel::SCREEN_HEIGHT> imageAnimation;

    // The Hub75 driver is constantly running on core 1 with a frequency much higher than 200Hz. CPU load on core 1 is low due to DMA and PIO usage.
    // The animated examples are updated at 120 Hz.
    const float fps = 100.0f;
    const float frame_delay_ms = 1000.0f / fps;

    struct repeating_timer timer;
    add_repeating_timer_ms(15000, skip_to_next_demo, NULL, &timer);

    // set basis brightness of matrix panel
    driver.setBasisBrightness(8);

    // set full brightness of panel
    float intensity = 1.0f;
    driver.setIntensity(intensity);

    absolute_time_t next_frame = make_timeout_time_ms((uint32_t)frame_delay_ms);

    while (true)
    {
        if (load_anim)
        {
            load_anim = false;
            setup_demo(frame_index, bouncingBalls, fireEffect, imageAnimation, colourCheck, timer);
        }

        update_demo(frame_index, bouncingBalls, fireEffect, imageAnimation, colourCheck, timer);

        lv_timer_handler();
        sleep_until(next_frame);
        next_frame = delayed_by_ms(next_frame, (uint32_t)frame_delay_ms);
    }
}
