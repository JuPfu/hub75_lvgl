#include "pico.h"

enum PanelType
{
    PANEL_GENERIC = 0,
    PANEL_FM6126A,
};

void create_hub75_driver(uint w, uint h, PanelType panel_type, bool stb_inverted);
void start_hub75_driver();
void update_bgr(uint8_t *src);
void update(uint8_t *src);