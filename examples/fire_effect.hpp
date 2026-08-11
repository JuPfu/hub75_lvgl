// Example derived from https://github.com/pimoroni/pimoroni-pico/blob/main/examples/interstate75/interstate75_fire_effect.cpp
#include <cstdio>
#include <cstdlib>

#include "pico/stdlib.h"

#include "lvgl.h"

#define BYTES_PER_PIXEL (LV_COLOR_FORMAT_GET_SIZE(LV_COLOR_FORMAT_RGB888))

template <uint32_t W, uint32_t H>
class FireEffect
{
private:
    alignas(4) uint8_t data_buf[W * H * BYTES_PER_PIXEL];

    alignas(4) float heat[W * H];

    bool landscape = true;

    lv_obj_t *screen;
    lv_obj_t *canvas;
    lv_draw_buf_t *draw_buf;
    lv_layer_t layer;

public:
    explicit FireEffect()
    {
        /*Create a buffer for the canvas*/
        draw_buf = lv_draw_buf_create(W, H, LV_COLOR_FORMAT_RGB888, LV_STRIDE_AUTO);
        lv_result_t res = lv_draw_buf_init(draw_buf, W, H, LV_COLOR_FORMAT_RGB888, LV_STRIDE_AUTO, data_buf, W * H * BYTES_PER_PIXEL);
        if (res != LV_RESULT_OK)
        {
            printf("lv_draw_buf_init failed %d\n", res);
        }

        screen = lv_obj_create(NULL);
        canvas = lv_canvas_create(screen);
        lv_canvas_set_buffer(canvas, data_buf, W, H, LV_COLOR_FORMAT_RGB888);
        lv_canvas_set_draw_buf(canvas, draw_buf);
        lv_obj_center(canvas);
        lv_canvas_fill_bg(canvas, lv_color_make(200, 120, 70), LV_OPA_COVER);
    }

    ~FireEffect()
    {
        lv_draw_buf_destroy(draw_buf); // Free the draw buffer
    }

    void set(int x, int y, float v)
    {
        if (x >= 0 && x < W && y >= 0 && y < H)
        {
            heat[x + y * W] = v;
        }
    }

    float get(int x, int y)
    {
        if (y >= H)
            y = H - 1;
        else if (y < 0)
            y = 0;
        if (x >= W)
            x = W - 1;
        else if (x < 0)
            x = 0;

        return heat[x + y * W];
    }

    void burn();

    void show()
    {
        lv_screen_load_anim(screen, LV_SCREEN_LOAD_ANIM_MOVE_TOP, 1000, 0, false);
    }

    inline lv_color_t heat_to_color(float value);
};

template <uint32_t W, uint32_t H>
void FireEffect<W, H>::burn()
{
    lv_canvas_init_layer(canvas, &layer);

    for (int y = 0; y < H; y++)
    {
        for (int x = 0; x < W; x++)
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
    for (int x = 0; x < W; x++)
    {
        set(x, H - 1, (rand() % 200) / 1000.0f);
    }

    // add a new random heat source
    int source_count = landscape ? 7 : 3;
    for (int c = 0; c < source_count; c++)
    {
        int px = (rand() % (W - 4)) + 2;
        set(px, H - 2, 1.0f);
        set(px + 1, H - 2, 1.0f);
        set(px - 1, H - 2, 1.0f);
        set(px, H - 1, 1.0f);
        set(px + 1, H - 1, 1.0f);
        set(px - 1, H - 1, 1.0f);
    }
}

template <uint32_t W, uint32_t H>
lv_color_t FireEffect<W, H>::heat_to_color(float value)
{
    uint8_t r, g, b;

    if (value > 0.5f)
    {
        uint8_t c = 25 - static_cast<int>((255 * value) * 0.1f);
        r = 255 - c;
        g = r;
        b = static_cast<uint8_t>(150 * value) + 105;
    }
    else if (value > 0.4f)
    {
        b = static_cast<uint8_t>(350 * value) - 140;
        r = 220 + (b >> 1);
        g = 160;
    }
    else if (value > 0.3f)
    {
        b = static_cast<uint8_t>(500 * value) - 150;
        r = 180 + (b >> 1);
        g = b;
    }
    else
    {
        r = static_cast<uint8_t>(150 * value);
        g = r;
        b = r;
    }

    return lv_color_make(r, g, b);
}
