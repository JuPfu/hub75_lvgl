// Example derived from https://github.com/pimoroni/pimoroni-pico/blob/main/examples/interstate75/interstate75_fire_effect.cpp
#include <cstdio>

#include "pico/stdlib.h"

#include "lvgl/src/misc/lv_types.h"
#include "lvgl/src/misc/lv_color.h"
#include "lvgl/src/widgets/canvas/lv_canvas.h"

#define BYTES_PER_PIXEL (LV_COLOR_FORMAT_GET_SIZE(LV_COLOR_FORMAT_RGB888))

class FireEffect
{
private:
    float *heat; // Pointer to dynamically allocated memory

    uint width, height;
    bool landscape = true;
    lv_obj_t *screen;
    lv_obj_t *canvas;
    lv_draw_buf_t *draw_buf;
    lv_layer_t layer;
    uint8_t *data_buf;

public:
    explicit FireEffect(uint width = 64, uint height = 64) : width(width), height(height)
    {
        heat = new float[width * height](); // Allocate memory and zero-initialize

        data_buf = new uint8_t[width * height * BYTES_PER_PIXEL]();
        if (data_buf == nullptr)
        {
            printf("Failed to allocate data_buf\n");
            return;
        }

        /*Create a buffer for the canvas*/
        draw_buf = lv_draw_buf_create(width, height, LV_COLOR_FORMAT_RGB888, LV_STRIDE_AUTO);
        lv_result_t res = lv_draw_buf_init(draw_buf, width, height, LV_COLOR_FORMAT_RGB888, LV_STRIDE_AUTO, data_buf, width * height * BYTES_PER_PIXEL);
        if (res != LV_RESULT_OK)
        {
            printf("lv_draw_buf_init failed %d\n", res);
        }

        screen = lv_obj_create(NULL);
        canvas = lv_canvas_create(screen);
        lv_canvas_set_buffer(canvas, data_buf, width, height, LV_COLOR_FORMAT_RGB888);
        lv_canvas_set_draw_buf(canvas, draw_buf);
        lv_obj_center(canvas);
        lv_canvas_fill_bg(canvas, lv_color_make(200, 120, 70), LV_OPA_COVER);
    }

    ~FireEffect()
    {
        lv_draw_buf_destroy(draw_buf); // Free the draw buffer

        delete[] heat; // Properly deallocate memory
    }

    void set(int x, int y, float v)
    {
        if (x >= 0 && x < width && y >= 0 && y < height)
        {
            heat[x + y * width] = v;
        }
    }

    float get(int x, int y)
    {
        if (y >= height)
            y = height - 1;
        else if (y < 0)
            y = 0;
        if (x >= width)
            x = width - 1;
        else if (x < 0)
            x = 0;

        return heat[x + y * width];
    }

    void burn();

    void show()
    {
        lv_screen_load_anim(screen, LV_SCR_LOAD_ANIM_MOVE_TOP, 1000, 0, false);
    }

    inline lv_color_t heat_to_color(float value);
};
