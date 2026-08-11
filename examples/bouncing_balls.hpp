// Example derived from https://github.com/pimoroni/pimoroni-pico/blob/main/examples/interstate75/interstate75_balls_demo.cpp

#include <cstdio>
#include <vector>

#include "pico/stdlib.h"

#include "lvgl.h"

#include <random>

#define BYTES_PER_PIXEL (LV_COLOR_FORMAT_GET_SIZE(LV_COLOR_FORMAT_RGB888))

template <uint32_t W, uint32_t H>
class BouncingBalls
{
private:
    struct mPoint
    {
        float x;
        float y;
        float r;
        float dx;
        float dy;
        lv_color_t pen;
    };

    alignas(4) uint8_t data_buf[W * H * BYTES_PER_PIXEL];

    uint quantityOfBalls;

    std::vector<mPoint> mShapes;

    void mCreateShapes(int quantityOfBalls);

    lv_obj_t *canvas;
    lv_draw_buf_t *draw_buf;
    lv_layer_t layer;
    lv_obj_t *screen;
    lv_draw_rect_dsc_t circle_dsc;
    lv_style_t label_style;
    lv_style_t style_shadow;
    lv_style_t scrolling_label_style;

public:
    explicit BouncingBalls(uint quantityOfBalls = 10) : quantityOfBalls(quantityOfBalls)
    {
        if (H <= 32)
        {
            quantityOfBalls = std::min((uint)3, quantityOfBalls);
        }

        printf("Constructor BouncingBalls vor mShapes.reserve\n");
        mShapes.reserve(quantityOfBalls);

        /*Create a buffer for the canvas*/
        printf("Constructor BouncingBalls vor lv_draw_buf_create.reserve\n");
        draw_buf = lv_draw_buf_create(W, H, LV_COLOR_FORMAT_RGB888, LV_STRIDE_AUTO);
        printf("Constructor BouncingBalls vor lv_draw_buf_init %lu\n", sizeof(data_buf));
        lv_result_t res = lv_draw_buf_init(draw_buf, W, H, LV_COLOR_FORMAT_RGB888, LV_STRIDE_AUTO, data_buf, W * H * BYTES_PER_PIXEL);
        if (res != LV_RESULT_OK)
        {
            printf("lv_draw_buf_init failed %d\n", res);
        }

        printf("Constructor BouncingBalls vor lv_obj_create\n");
        screen = lv_obj_create(NULL);

        canvas = lv_canvas_create(screen);
        printf("Constructor BouncingBalls nach lv_canvas_create\n");
        lv_canvas_set_buffer(canvas, data_buf, W, H, LV_COLOR_FORMAT_RGB888);
        printf("Constructor BouncingBalls vor lv_canvas_set_draw_buf\n");
        lv_canvas_set_draw_buf(canvas, draw_buf);
        printf("Constructor BouncingBalls vor lv_obj_center\n");
        lv_obj_center(canvas);
        printf("Constructor BouncingBalls vor lv_canvas_fill_bg\n");
        lv_canvas_fill_bg(canvas, lv_color_make(200, 120, 70), LV_OPA_COVER);

        printf("Constructor BouncingBalls vor mCreateShapes\n");
        mCreateShapes(quantityOfBalls);

        printf("Constructor BouncingBalls vor lv_style_init");
        lv_style_init(&label_style);
        lv_style_set_text_color(&label_style, lv_color_make(250, 250, 250));
        lv_obj_t *label1 = lv_label_create(screen);
        if (W <= 32)
        {
            lv_label_set_text(label1, "Hello");
        }
        else
        {
            lv_label_set_text(label1, "Hello\nworld\xEF\x80\x8C");
        }
        lv_obj_align(label1, LV_ALIGN_TOP_MID, 0, 0);
        lv_obj_add_style(label1, &label_style, LV_STATE_DEFAULT);

        /*Create a style for the shadow*/
        lv_style_init(&style_shadow);
        lv_style_set_text_opa(&style_shadow, LV_OPA_30);
        lv_style_set_text_color(&style_shadow, lv_color_black());

        /*Create a label for the shadow first (it's in the background)*/
        lv_obj_t *shadow_label = lv_label_create(screen);
        lv_obj_add_style(shadow_label, &style_shadow, 0);

        lv_label_set_text(shadow_label, lv_label_get_text(label1));

        /*Shift the second label down and to the right by 2 pixel*/
        lv_obj_align_to(shadow_label, label1, LV_ALIGN_TOP_LEFT, 1, 1);

        lv_style_init(&scrolling_label_style);
        lv_style_set_text_color(&scrolling_label_style, lv_color_make(200, 100, 120));
        lv_obj_t *label2 = lv_label_create(screen);
        lv_label_set_long_mode(label2, LV_LABEL_LONG_MODE_SCROLL_CIRCULAR); /*Circular scroll*/
        lv_obj_set_width(label2, 64);

        lv_label_set_text(label2, "This is a circulating scrolling text. ");
        lv_obj_align(label2, LV_ALIGN_CENTER, 0, 20);
        lv_obj_add_style(label2, &scrolling_label_style, LV_STATE_DEFAULT);
        printf("Constructor BouncingBalls ENDE\n");
    }

    void bounce();

    void show()
    {
        printf("Bouncing balls show\n");
        lv_screen_load_anim(screen, LV_SCREEN_LOAD_ANIM_FADE_IN, 2000, 0, false);
    }
};


// Example derived from https://github.com/pimoroni/pimoroni-pico/blob/main/examples/interstate75/interstate75_balls_demo.cpp
template <uint32_t W, uint32_t H>
void BouncingBalls<W, H>::bounce()
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
        else if (shape.x + shape.r >= W)
        {
            shape.dx = -shape.dx;
            shape.x = W - shape.r;
        }

        if (shape.y - shape.r < 0)
        {
            shape.dy = -shape.dy;
            shape.y = shape.r;
        }
        else if (shape.y + shape.r >= H)
        {
            shape.dy = -shape.dy;
            shape.y = H - shape.r;
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

template <uint32_t W, uint32_t H>
void BouncingBalls<W, H>::mCreateShapes(int quantityOfBalls)
{
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<int> rand_x(0, W - 1);
    static std::uniform_int_distribution<int> rand_y(0, H - 1);
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
