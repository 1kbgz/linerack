#include "audio_router.h"

#include <assert.h>
#include <math.h>

static bool Near(float left, float right)
{
    return fabsf(left - right) < 0.00001f;
}

int main(void)
{
    LineRackPresetBank bank;
    LineRackAudioRouter router;
    float left;
    float right;
    LineRackPresetBankDefaults(&bank);

    LineRackAudioRouterConfigure(&router, &bank);
    LineRackAudioRouterProcess(
        &router, true, 0.25f, -0.5f, 0.75f, 0.5f, &left, &right);
    assert(Near(left, 0.25f));
    assert(Near(right, -0.5f));
    LineRackAudioRouterProcess(
        &router, false, 0.25f, -0.5f, 0.75f, 0.5f, &left, &right);
    assert(Near(left, 0.0f));
    assert(Near(right, 0.0f));

    bank.source_mode = LINERACK_SOURCE_ANALOG;
    bank.analog_trim_half_db = -12;
    LineRackAudioRouterConfigure(&router, &bank);
    LineRackAudioRouterProcess(
        &router, true, 0.25f, -0.5f, 0.75f, 0.5f, &left, &right);
    assert(Near(left, 0.75f * powf(10.0f, -6.0f / 20.0f)));
    assert(Near(right, 0.5f * powf(10.0f, -6.0f / 20.0f)));

    bank.source_mode = LINERACK_SOURCE_MIX;
    bank.usb_trim_half_db = -13;
    bank.analog_trim_half_db = -13;
    LineRackAudioRouterConfigure(&router, &bank);
    LineRackAudioRouterProcess(
        &router, true, 1.0f, -1.0f, 1.0f, -1.0f, &left, &right);
    const float mixed_peak = 2.0f * powf(10.0f, -6.5f / 20.0f);
    assert(mixed_peak < 1.0f);
    assert(Near(left, mixed_peak));
    assert(Near(right, -mixed_peak));
    return 0;
}
