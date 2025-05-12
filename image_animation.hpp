#include "pico/stdio.h"
#include "pico/stdlib.h"
#include "pico/printf.h"

#include "lvgl/src/misc/lv_types.h"
#include "lvgl/src/misc/lv_anim.h"
#include "lvgl/src/widgets/image/lv_image.h"

#include "vanessa_mai_64x64.h"

#define BYTES_PER_PIXEL (LV_COLOR_FORMAT_GET_SIZE(LV_COLOR_FORMAT_RGB888))
class ImageAnimation
{
private:
    static void image_animation_cb(void *var, int32_t v)
    {
        lv_image_set_rotation((lv_obj_t *)var, v);
    }

    static void image_animation_started_cb(lv_anim_t *a)
    {
        // No-op for now
    }

    static void image_animation_completed_cb(lv_anim_t *a)
    {
        ImageAnimation *self = static_cast<ImageAnimation *>(lv_anim_get_user_data(a));
        self->animation_done = true;
    }

    lv_obj_t *screen = nullptr;
    lv_obj_t *vanessa = nullptr;
    lv_image_dsc_t img_desc;

    lv_anim_t a;

    uint width, height;
    bool animation_done = false;

public:
    explicit ImageAnimation(uint width = 64, uint height = 64) : width(width), height(height)
    {
        screen = lv_obj_create(NULL);

        vanessa = lv_image_create(screen);

        img_desc.data_size = sizeof(vanessa_mai_64x64);
        img_desc.data = vanessa_mai_64x64;
        img_desc.header.w = width;
        img_desc.header.h = height;
        img_desc.header.cf = LV_COLOR_FORMAT_RGB888;
        img_desc.header.stride = width * BYTES_PER_PIXEL;

        lv_image_set_src(vanessa, &img_desc);
        lv_image_set_antialias(vanessa, true);
        lv_image_set_pivot(vanessa, width / 2, height / 2);
        lv_obj_align(vanessa, LV_ALIGN_CENTER, 0, 0);

        lv_anim_init(&a);
        lv_anim_set_var(&a, vanessa);
        lv_anim_set_user_data(&a, this); // Pass self for callback context
        lv_anim_set_values(&a, 0, 3600);

        uint32_t change_per_sec = 100;
        uint32_t duration_in_ms = lv_anim_speed_to_time(change_per_sec, 0, 3600);
        lv_anim_set_duration(&a, duration_in_ms);

        lv_anim_set_repeat_count(&a, 1);
        lv_anim_set_exec_cb(&a, static_cast<lv_anim_exec_xcb_t>(image_animation_cb));
        lv_anim_set_completed_cb(&a, image_animation_completed_cb);
        lv_anim_set_start_cb(&a, image_animation_started_cb);
    }

    void start()
    {
        animation_done = false; // Reset done state
        if (vanessa)
        {
            lv_anim_start(&a);
        }
    }

    void show()
    {
        if (screen)
        {
            lv_screen_load_anim(screen, LV_SCR_LOAD_ANIM_OUT_TOP, 1000, 0, false);
        }
    }

    bool done() const
    {
        return animation_done;
    }

    ~ImageAnimation()
    {
        if (screen)
            lv_obj_clean(screen);

        if (vanessa)
        {
            lv_obj_delete_async(vanessa);
            vanessa = nullptr;
        }
    }
};
