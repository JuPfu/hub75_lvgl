// Example derived from https://github.com/pimoroni/pimoroni-pico/blob/main/examples/interstate75/interstate75_fire_effect.cpp
#include "fire_effect.hpp"

#include <cstdlib>

// Display size in pixels
// Should be either 64x64 or 32x32 but perhaps 64x32 an other sizes will work.
// Note: this example uses only 5 address lines so it's limited to 32*2 pixels.

void FireEffect::burn()
{
    lv_canvas_init_layer(canvas, &layer);

    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            lv_color_t color = heat_to_color(get(x, y));

            if (landscape)
            {
                lv_canvas_set_px(canvas, x, y, color, LV_OPA_70);
            }
            else
            {
                lv_canvas_set_px(canvas, y, x, color, LV_OPA_50);
            }

            // update this pixel by averaging the below pixels
            float average = (get(x, y) + get(x, y + 2) + get(x, y + 1) + get(x - 1, y + 1) + get(x + 1, y + 1)) / 5.0f;

            // damping factor to ensure flame tapers out towards the top of the displays
            average *= landscape ? 0.985f : 0.99f;

            // update the heat map with our newly averaged value
            set(x, y, average);
        }
    }

    lv_canvas_finish_layer(canvas, &layer);
    
    // clear the bottom row and then add a new fire seed to it
    for (int x = 0; x < width; x++)
    {
        set(x, height - 1, (rand() % 200) / 1000.0f);
    }

    // add a new random heat source
    int source_count = landscape ? 7 : 1;
    for (int c = 0; c < source_count; c++)
    {
        int px = (rand() % (width - 4)) + 2;
        set(px, height - 2, 1.0f);
        set(px + 1, height - 2, 1.0f);
        set(px - 1, height - 2, 1.0f);
        set(px, height - 1, 1.0f);
        set(px + 1, height - 1, 1.0f);
        set(px - 1, height - 1, 1.0f);
    }
}

inline lv_color_t FireEffect::heat_to_color(float value) {
    uint8_t r, g, b;

    if (value > 0.5f) {
        uint8_t c = 25 - static_cast<int>((255 * value) * 0.1f);
        r = 255 - c;
        g = r;
        b = static_cast<uint8_t>(150 * value) + 105;
    }
    else if (value > 0.4f) {
        b = static_cast<uint8_t>(350 * value) - 140;
        r = 220 + (b >> 1);
        g = 160;
    }
    else if (value > 0.3f) {
        b = static_cast<uint8_t>(500 * value) - 150;
        r = 180 + (b >> 1);
        g = b;
    }
    else {
        r = static_cast<uint8_t>(200 * value);
        g = r;
        b = r;
    }

    return lv_color_make(r, g, b);
}
