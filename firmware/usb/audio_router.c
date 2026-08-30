#include "audio_router.h"

#include <math.h>
#include <stddef.h>

static float GainFromHalfDb(int8_t half_db)
{
    return powf(10.0f, half_db / 40.0f);
}

void LineRackAudioRouterConfigure(LineRackAudioRouter      *router,
                                  const LineRackPresetBank *bank)
{
    if(router == NULL || bank == NULL)
        return;
    router->source_mode = bank->source_mode;
    router->usb_gain = GainFromHalfDb(bank->usb_trim_half_db);
    router->analog_gain = GainFromHalfDb(bank->analog_trim_half_db);
}

void LineRackAudioRouterProcess(const LineRackAudioRouter *router,
                                bool                       usb_available,
                                float                      usb_left,
                                float                      usb_right,
                                float                      analog_left,
                                float                      analog_right,
                                float                     *output_left,
                                float                     *output_right)
{
    if(router == NULL || output_left == NULL || output_right == NULL)
        return;
    const float routed_usb_left = usb_available ? usb_left * router->usb_gain : 0.0f;
    const float routed_usb_right = usb_available ? usb_right * router->usb_gain : 0.0f;
    if(router->source_mode == LINERACK_SOURCE_ANALOG)
    {
        *output_left = analog_left * router->analog_gain;
        *output_right = analog_right * router->analog_gain;
    }
    else if(router->source_mode == LINERACK_SOURCE_MIX)
    {
        *output_left = routed_usb_left + analog_left * router->analog_gain;
        *output_right = routed_usb_right + analog_right * router->analog_gain;
    }
    else
    {
        *output_left = routed_usb_left;
        *output_right = routed_usb_right;
    }
}
