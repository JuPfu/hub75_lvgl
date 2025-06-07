// #include "pico/stdio.h"
#include "pico/stdlib.h"
#include "pico/printf.h"

#include "lvgl.h"

#include "lvgl/src/misc/lv_types.h"
#include "lvgl/src/misc/lv_anim.h"
#include "lvgl/src/widgets/image/lv_image.h"

#include "colour_squares.h"

#define BYTES_PER_PIXEL (LV_COLOR_FORMAT_GET_SIZE(LV_COLOR_FORMAT_RGB888))
class ColourCheck
{
private:
    lv_obj_t *screen = nullptr;
    lv_obj_t *colour_squares = nullptr;
    lv_image_header_t header;
    lv_image_dsc_t img_desc;

    uint width, height;
    bool done = false;

public:
    explicit ColourCheck(uint width = 64, uint height = 64) : width(width), height(height)
    {
        screen = lv_obj_create(NULL);

        colour_squares = lv_image_create(screen);

        header.magic = LV_IMAGE_HEADER_MAGIC;
        header.w = 256;
        header.h = 180;
        header.cf = LV_COLOR_FORMAT_RGB888;
        header.stride = 256 * BYTES_PER_PIXEL;
        header.flags = 0x0;
        header.reserved_2 = 0;

        img_desc.header = header;
        img_desc.data_size = sizeof(colour_squares_map);
        img_desc.data = colour_squares_map;

        lv_image_set_src(colour_squares, &img_desc);
        lv_image_set_scale_x(colour_squares, 64);
        lv_image_set_scale_y(colour_squares, 92);
        lv_image_set_antialias(colour_squares, true);
        lv_image_set_pivot(colour_squares, 256 / 2, 180 / 2);
        lv_obj_align(colour_squares, LV_ALIGN_CENTER, 0, 0);
    }

    void colour_test();

    void show()
    {
        if (screen)
        {
            lv_screen_load_anim(screen, LV_SCR_LOAD_ANIM_FADE_IN, 2000, 0, false);
        }
    }

    ~ColourCheck()
    {
        if (screen)
            lv_obj_clean(screen);

        if (colour_squares)
        {
            lv_obj_delete_async(colour_squares);
            colour_squares = nullptr;
        }
    }
};
