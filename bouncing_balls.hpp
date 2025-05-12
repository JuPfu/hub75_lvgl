// Example derived from https://github.com/pimoroni/pimoroni-pico/blob/main/examples/interstate75/interstate75_balls_demo.cpp

#include <cstdio>
#include <vector>

#include "pico/stdlib.h"

#include "lvgl/src/misc/lv_types.h"
#include "lvgl/src/misc/lv_color.h"
#include "lvgl/src/widgets/canvas/lv_canvas.h"
#include "lvgl/src/draw/lv_draw_rect.h"
#include "lvgl/src/widgets/label/lv_label.h"

#define BYTES_PER_PIXEL (LV_COLOR_FORMAT_GET_SIZE(LV_COLOR_FORMAT_RGB888))

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

    uint width, height;

    std::vector<mPoint> mShapes;

    void mCreateShapes(int quantityOfBalls);

    lv_obj_t *canvas;
    lv_draw_buf_t *draw_buf;
    lv_layer_t layer;
    uint8_t *data_buf;
    lv_obj_t *screen;
    lv_draw_rect_dsc_t circle_dsc;
    lv_style_t label_style;
    lv_style_t style_shadow;
    lv_style_t scrolling_label_style;

public:
    explicit BouncingBalls(uint quantityOfBalls = 10, uint width = 64, uint height = 64) : width(width), height(height)
    {
        mShapes.reserve(quantityOfBalls);

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

        mCreateShapes(quantityOfBalls);

        lv_style_init(&label_style);
        lv_style_set_text_color(&label_style, lv_color_make(250, 250, 250));
        lv_obj_t *label1 = lv_label_create(screen);
        lv_label_set_text(label1, "Hello\nworld\xEF\x80\x8C");
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
    }

    void bounce();

    void show()
    {
        lv_screen_load_anim(screen, LV_SCR_LOAD_ANIM_FADE_IN, 1000, 0, false);
    }
};