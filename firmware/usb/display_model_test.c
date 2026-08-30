#include "display_model.h"

#include <assert.h>
#include <stdint.h>
#include <string.h>

static uint32_t FrameHash(const LineRackDisplayFrame *frame)
{
    uint32_t hash = 2166136261U;
    for(uint16_t y = 0U; y < LINERACK_DISPLAY_HEIGHT; ++y)
    {
        for(uint16_t x = 0U; x < LINERACK_DISPLAY_WIDTH; ++x)
        {
            hash ^= LineRackDisplayPixel(frame, (uint8_t)x, (uint8_t)y) ? 1U : 0U;
            hash *= 16777619U;
        }
    }
    return hash;
}

int main(void)
{
    LineRackPresetBank bank;
    LineRackDisplayFrame frame;
    LineRackPresetBankDefaults(&bank);

    LineRackPreset *preset = &bank.presets[1];
    strcpy(preset->name, "Web Low");
    preset->block_count = 2U;
    preset->blocks[1] = preset->blocks[2];
    LineRackDisplayRender(&frame, &bank, 2U, LINERACK_DISPLAY_PRESET);
    assert(FrameHash(&frame) == 3820898106U);

    LineRackPresetBankDefaults(&bank);
    LineRackDisplayRender(&frame, &bank, 2U, LINERACK_DISPLAY_EQ_RESPONSE);
    assert(FrameHash(&frame) == 2867898635U);

    LineRackDisplayRenderVisualizer(&frame, 25U, 75U);
    assert(LineRackDisplayPixel(&frame, 13U, 5U));
    assert(!LineRackDisplayPixel(&frame, 50U, 5U));
    assert(LineRackDisplayPixel(&frame, 90U, 22U));

    LineRackDisplayRenderVolume(&frame, 50U, false);
    assert(LineRackDisplayPixel(&frame, 60U, 25U));
    assert(!LineRackDisplayPixel(&frame, 70U, 25U));
    LineRackDisplayRenderVolume(&frame, 50U, true);
    assert(LineRackDisplayPixel(&frame, 40U, 8U));

    LineRackVolumeDisplay volume = {0};
    LineRackVolumeDisplayStart(&volume, 125U, false, 1000U);
    assert(volume.percent == 100U);
    assert(LineRackVolumeDisplayActive(&volume, 2999U));
    assert(!LineRackVolumeDisplayActive(&volume, 3000U));

    LineRackDisplayOverride override = {0U, false};
    LineRackDisplayOverrideStart(&override, 1000U);
    assert(LineRackDisplayResolveMode(LINERACK_DISPLAY_EQ_RESPONSE,
                                      &override,
                                      5999U)
           == LINERACK_DISPLAY_PRESET);
    assert(LineRackDisplayResolveMode(LINERACK_DISPLAY_EQ_RESPONSE,
                                      &override,
                                      6000U)
           == LINERACK_DISPLAY_EQ_RESPONSE);
    assert(!override.active);

    LineRackDisplayOverrideStart(&override, UINT32_MAX - 1000U);
    assert(LineRackDisplayResolveMode(LINERACK_DISPLAY_EQ_RESPONSE,
                                      &override,
                                      3998U)
           == LINERACK_DISPLAY_PRESET);
    assert(LineRackDisplayResolveMode(LINERACK_DISPLAY_EQ_RESPONSE,
                                      &override,
                                      3999U)
           == LINERACK_DISPLAY_EQ_RESPONSE);

    LineRackDisplayOverrideStart(&override, 1000U);
    assert(LineRackDisplayResolveMode(LINERACK_DISPLAY_EQ_RESPONSE,
                                      &override,
                                      0x80001000U)
           == LINERACK_DISPLAY_EQ_RESPONSE);
    assert(!override.active);

    LineRackDisplayPower power;
    LineRackDisplayPowerInit(&power, 1000U);
    assert(LineRackDisplayPowerResolve(&power, true, 20999U));
    assert(!LineRackDisplayPowerResolve(&power, true, 21000U));
    LineRackDisplayPowerWake(&power, 22000U);
    assert(LineRackDisplayPowerResolve(&power, true, 22000U));
    assert(LineRackDisplayPowerResolve(&power, false, 50000U));

    LineRackDisplayPowerInit(&power, UINT32_MAX - 1000U);
    assert(LineRackDisplayPowerResolve(&power, true, 18998U));
    assert(!LineRackDisplayPowerResolve(&power, true, 18999U));
    return 0;
}
