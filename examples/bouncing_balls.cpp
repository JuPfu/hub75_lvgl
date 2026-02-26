// Example derived from https://github.com/pimoroni/pimoroni-pico/blob/main/examples/interstate75/interstate75_balls_demo.cpp

#include "bouncing_balls.hpp"
#include <random>

#include "lvgl.h"

#include "lvgl/src/font/lv_font.h"

#include "lvgl/src/widgets/label/lv_label.h"

void BouncingBalls::bounce()
{
    lv_canvas_fill_bg(canvas, lv_color_make(100, 80, 170), LV_OPA_COVER);
    lv_canvas_init_layer(canvas, &layer);
    lv_draw_rect_dsc_init(&circle_dsc);

    for (auto &shape : mShapes)
    {
        shape.x += shape.dx;
        shape.y += shape.dy;

        if (shape.x - shape.r < 0)
        {
            shape.dx = -shape.dx;
            shape.x = shape.r;
        }
        else if (shape.x + shape.r >= width)
        {
            shape.dx = -shape.dx;
            shape.x = width - shape.r;
        }

        if (shape.y - shape.r < 0)
        {
            shape.dy = -shape.dy;
            shape.y = shape.r;
        }
        else if (shape.y + shape.r >= height)
        {
            shape.dy = -shape.dy;
            shape.y = height - shape.r;
        }

        circle_dsc.bg_color = shape.pen;
        circle_dsc.bg_opa = LV_OPA_70;
        circle_dsc.radius = shape.r;

        lv_area_t coords = {
            static_cast<lv_coord_t>(shape.x - shape.r),
            static_cast<lv_coord_t>(shape.y - shape.r),
            static_cast<lv_coord_t>(shape.x + shape.r),
            static_cast<lv_coord_t>(shape.y + shape.r)};

        lv_draw_rect(&layer, &circle_dsc, &coords);
    }

    lv_canvas_finish_layer(canvas, &layer);
}

void BouncingBalls::mCreateShapes(int quantityOfBalls)
{
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<int> rand_x(0, width - 1);
    static std::uniform_int_distribution<int> rand_y(0, height - 1);
    static std::uniform_int_distribution<int> rand_r(2, 6);
    static std::uniform_real_distribution<float> rand_speed(-2.0f, 2.0f);
    static std::uniform_int_distribution<uint8_t> rand_color(0, 255);

    for (uint8_t i = 0; i < quantityOfBalls; i++)
    {
        mShapes.emplace_back(mPoint{
            static_cast<float>(rand_x(gen)),
            static_cast<float>(rand_y(gen)),
            static_cast<float>(rand_r(gen)),
            rand_speed(gen),
            rand_speed(gen),
            lv_color_make(rand_color(gen), rand_color(gen), rand_color(gen))});
    }
}
