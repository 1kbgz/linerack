#pragma once

#include <stdbool.h>

#include "preset_model.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct
{
    uint8_t source_mode;
    float   usb_gain;
    float   analog_gain;
} LineRackAudioRouter;

void LineRackAudioRouterConfigure(LineRackAudioRouter       *router,
                                  const LineRackPresetBank  *bank);
void LineRackAudioRouterProcess(const LineRackAudioRouter *router,
                                bool                       usb_available,
                                float                      usb_left,
                                float                      usb_right,
                                float                      analog_left,
                                float                      analog_right,
                                float                     *output_left,
                                float                     *output_right);

#ifdef __cplusplus
}
#endif
