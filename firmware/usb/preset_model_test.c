#include "preset_model.h"

#include <assert.h>
#include <string.h>

int main(void)
{
    LineRackPresetBank defaults;
    LineRackPresetBankDefaults(&defaults);
    assert(LineRackPresetBankValid(&defaults));
    assert(defaults.display_mode == LINERACK_DISPLAY_PRESET);
    assert(defaults.source_mode == LINERACK_SOURCE_USB);
    assert(defaults.usb_trim_half_db == 0);
    assert(defaults.analog_trim_half_db == 0);
    assert(defaults.presets[1].blocks[1].parameters[0] == 120.0f);

    LineRackPresetBank copy = defaults;
    assert(LineRackPresetBankEqual(&defaults, &copy));

    copy.presets[0].blocks[2].enabled = 0U;
    assert(LineRackPresetBankValid(&copy));

    copy.presets[0].blocks[0].enabled = 0U;
    assert(LineRackPresetBankValid(&copy));

    copy = defaults;
    copy.display_mode = LINERACK_DISPLAY_VISUALIZER;
    assert(LineRackPresetBankValid(&copy));

    copy.display_mode = 3U;
    assert(!LineRackPresetBankValid(&copy));

    copy = defaults;
    copy.display_mode |= LINERACK_DISPLAY_BLANKING_DISABLED;
    assert(LineRackPresetBankValid(&copy));

    copy = defaults;
    copy.source_mode = LINERACK_SOURCE_MIX;
    assert(!LineRackPresetBankValid(&copy));
    copy.usb_trim_half_db = -13;
    copy.analog_trim_half_db = -13;
    assert(LineRackPresetBankValid(&copy));

    copy = defaults;
    strcpy(copy.presets[0].name, "Caf\xC3\xA9");
    assert(!LineRackPresetBankValid(&copy));

    copy = defaults;
    copy.presets[0].blocks[1].parameters[0] = 24000.0f;
    assert(!LineRackPresetBankValid(&copy));

    copy = defaults;
    copy.presets[0].blocks[1].type = LINERACK_BLOCK_GAIN;
    assert(!LineRackPresetBankValid(&copy));

    copy = defaults;
    copy.presets[0].blocks[0].type = LINERACK_BLOCK_PARAMETRIC_EQ;
    assert(!LineRackPresetBankValid(&copy));

    copy = defaults;
    copy.presets[0].block_count = 7U;
    copy.presets[0].blocks[1].type = LINERACK_BLOCK_HIGH_PASS;
    copy.presets[0].blocks[1].parameters[0] = 80.0f;
    copy.presets[0].blocks[1].parameters[1] = 24.0f;
    copy.presets[0].blocks[2].type = LINERACK_BLOCK_PARAMETRIC_EQ;
    copy.presets[0].blocks[2].parameters[0] = 1000.0f;
    copy.presets[0].blocks[2].parameters[1] = 3.0f;
    copy.presets[0].blocks[2].parameters[2] = 0.8f;
    copy.presets[0].blocks[3] = copy.presets[0].blocks[2];
    copy.presets[0].blocks[3].parameters[0] = 4000.0f;
    copy.presets[0].blocks[4].type = LINERACK_BLOCK_NOISE_GATE;
    copy.presets[0].blocks[4].enabled = 1U;
    copy.presets[0].blocks[4].parameters[0] = -45.0f;
    copy.presets[0].blocks[4].parameters[1] = 5.0f;
    copy.presets[0].blocks[4].parameters[2] = 20.0f;
    copy.presets[0].blocks[4].parameters[3] = 100.0f;
    copy.presets[0].blocks[4].parameters[4] = -60.0f;
    copy.presets[0].blocks[5].type = LINERACK_BLOCK_LOW_PASS;
    copy.presets[0].blocks[5].enabled = 1U;
    copy.presets[0].blocks[5].parameters[0] = 16000.0f;
    copy.presets[0].blocks[5].parameters[1] = 12.0f;
    copy.presets[0].blocks[6].type = LINERACK_BLOCK_LIMITER;
    copy.presets[0].blocks[6].enabled = 1U;
    copy.presets[0].blocks[6].parameters[0] = -1.0f;
    assert(LineRackPresetBankValid(&copy));

    copy.presets[0].blocks[1].parameters[1] = 18.0f;
    assert(!LineRackPresetBankValid(&copy));

    copy.presets[0].blocks[1].parameters[1] = 24.0f;
    copy.presets[0].blocks[4].parameters[3] = 3000.0f;
    assert(!LineRackPresetBankValid(&copy));

    copy = defaults;
    copy.presets[0].block_count = 4U;
    memset(&copy.presets[0].blocks[1], 0, sizeof(LineRackBlock) * 3U);
    copy.presets[0].blocks[1].type = LINERACK_BLOCK_COMPRESSOR;
    copy.presets[0].blocks[1].enabled = 1U;
    copy.presets[0].blocks[1].parameters[0] = -18.0f;
    copy.presets[0].blocks[1].parameters[1] = 3.0f;
    copy.presets[0].blocks[1].parameters[2] = 10.0f;
    copy.presets[0].blocks[1].parameters[3] = 100.0f;
    copy.presets[0].blocks[2] = copy.presets[0].blocks[1];
    copy.presets[0].blocks[2].parameters[0] = -12.0f;
    copy.presets[0].blocks[3] = defaults.presets[0].blocks[2];
    assert(LineRackPresetBankValid(&copy));

    copy.presets[0].blocks[2].parameters[1] = 0.5f;
    assert(!LineRackPresetBankValid(&copy));

    copy = defaults;
    copy.presets[0].blocks[1].type = LINERACK_BLOCK_REVERB;
    copy.presets[0].blocks[1].parameters[0] = 60.0f;
    copy.presets[0].blocks[1].parameters[1] = 40.0f;
    copy.presets[0].blocks[1].parameters[2] = 20.0f;
    assert(LineRackPresetBankValid(&copy));
    copy.presets[0].blocks[1].parameters[2] = 101.0f;
    assert(!LineRackPresetBankValid(&copy));

    copy = defaults;
    const LineRackBlock eq = copy.presets[0].blocks[1];
    const LineRackBlock limiter = copy.presets[0].blocks[2];
    copy.presets[0].block_count = LINERACK_MAX_BLOCKS;
    for(uint8_t index = 1U; index < LINERACK_MAX_BLOCKS - 1U; ++index)
        copy.presets[0].blocks[index] = eq;
    copy.presets[0].blocks[LINERACK_MAX_BLOCKS - 1U] = limiter;
    assert(LineRackPresetBankValid(&copy));

    copy.presets[0].block_count = LINERACK_MAX_BLOCKS + 1U;
    assert(!LineRackPresetBankValid(&copy));

    copy = defaults;
    memset(copy.presets[2].name, 'x', sizeof(copy.presets[2].name));
    assert(!LineRackPresetBankValid(&copy));
    return 0;
}
