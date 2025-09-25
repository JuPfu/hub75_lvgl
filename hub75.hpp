#include "pico.h"

#define BIT_DEPTH 10 ///< Number of bit planes

enum PanelType
{
    PANEL_GENERIC = 0,
    PANEL_FM6126A
};

void setBasisBrightness(uint8_t factor);
void setIntensity(float intensity);

void create_hub75_driver(uint w, uint h, PanelType panel_type, bool stb_inverted);
void start_hub75_driver();
void update_bgr(const uint8_t *src);
void update(const uint8_t *src);
void update_area_bgr(const uint8_t *src, const int32_t x1, const int32_t y1, const int32_t x2, const int32_t y2);