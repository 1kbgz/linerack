#include "preset_cbor.h"

#include <algorithm>
#include <cassert>
#include <cstring>

int main()
{
    LineRackPresetBank original;
    LineRackPresetBank decoded;
    uint8_t            payload[8192];

    LineRackPresetBankDefaults(&original);
    original.display_mode = LINERACK_DISPLAY_VISUALIZER
                            | LINERACK_DISPLAY_BLANKING_DISABLED;
    original.source_mode = LINERACK_SOURCE_MIX;
    original.usb_trim_half_db = -13;
    original.analog_trim_half_db = -16;
    const size_t preset_size =
        LineRackCborEncodePresets(payload, sizeof(payload), &original);
    assert(preset_size > 0U);
    assert(LineRackCborDecodePresets(payload, preset_size, &decoded));
    assert(LineRackPresetBankEqual(&original, &decoded));
    assert(decoded.display_mode
           == (LINERACK_DISPLAY_VISUALIZER
               | LINERACK_DISPLAY_BLANKING_DISABLED));
    assert(decoded.source_mode == LINERACK_SOURCE_MIX);
    assert(decoded.usb_trim_half_db == -13);
    assert(decoded.analog_trim_half_db == -16);

    LineRackPreset &expanded = original.presets[0];
    expanded.block_count = 10U;
    std::memset(&expanded.blocks[1], 0, sizeof(LineRackBlock) * 9U);
    expanded.blocks[1].type = LINERACK_BLOCK_HIGH_PASS;
    expanded.blocks[1].enabled = 1U;
    expanded.blocks[1].parameters[0] = 80.0f;
    expanded.blocks[1].parameters[1] = 24.0f;
    expanded.blocks[2].type = LINERACK_BLOCK_PARAMETRIC_EQ;
    expanded.blocks[2].enabled = 1U;
    expanded.blocks[2].parameters[0] = 200.0f;
    expanded.blocks[2].parameters[1] = 3.0f;
    expanded.blocks[2].parameters[2] = 0.8f;
    expanded.blocks[3] = expanded.blocks[2];
    expanded.blocks[3].parameters[0] = 4000.0f;
    expanded.blocks[3].parameters[1] = -2.0f;
    expanded.blocks[4].type = LINERACK_BLOCK_NOISE_GATE;
    expanded.blocks[4].enabled = 1U;
    expanded.blocks[4].parameters[0] = -50.0f;
    expanded.blocks[4].parameters[1] = 2.0f;
    expanded.blocks[4].parameters[2] = 50.0f;
    expanded.blocks[4].parameters[3] = 150.0f;
    expanded.blocks[4].parameters[4] = -60.0f;
    expanded.blocks[5].type = LINERACK_BLOCK_LOW_PASS;
    expanded.blocks[5].enabled = 1U;
    expanded.blocks[5].parameters[0] = 18000.0f;
    expanded.blocks[5].parameters[1] = 12.0f;
    expanded.blocks[6].type = LINERACK_BLOCK_COMPRESSOR;
    expanded.blocks[6].enabled = 1U;
    expanded.blocks[6].parameters[0] = -18.0f;
    expanded.blocks[6].parameters[1] = 3.0f;
    expanded.blocks[6].parameters[2] = 10.0f;
    expanded.blocks[6].parameters[3] = 100.0f;
    expanded.blocks[7] = expanded.blocks[6];
    expanded.blocks[7].parameters[0] = -12.0f;
    expanded.blocks[8].type = LINERACK_BLOCK_REVERB;
    expanded.blocks[8].enabled = 1U;
    expanded.blocks[8].parameters[0] = 60.0f;
    expanded.blocks[8].parameters[1] = 40.0f;
    expanded.blocks[8].parameters[2] = 20.0f;
    expanded.blocks[9].type = LINERACK_BLOCK_LIMITER;
    expanded.blocks[9].enabled = 0U;
    expanded.blocks[9].parameters[0] = -1.0f;

    for(uint8_t slot = 1U; slot < LINERACK_PRESET_COUNT; ++slot)
    {
        original.presets[slot].block_count = expanded.block_count;
        std::memcpy(original.presets[slot].blocks,
                    expanded.blocks,
                    sizeof(expanded.blocks));
    }

    const size_t expanded_size =
        LineRackCborEncodePresets(payload, sizeof(payload), &original);
    assert(expanded_size > 0U);
    assert(LineRackCborDecodePresets(payload, expanded_size, &decoded));
    assert(LineRackPresetBankEqual(&original, &decoded));

    const LineRackDeviceDiagnostics diagnostics = {1234U, 2U, 3U, 4U};
    const size_t hello_size =
        LineRackCborEncodeHello(payload, sizeof(payload), 1U, &diagnostics);
    assert(hello_size > 0U);
    const char compressor_id[] = "compressor";
    assert(std::search(payload,
                       payload + hello_size,
                       compressor_id,
                       compressor_id + sizeof(compressor_id) - 1U)
           != payload + hello_size);
    const char reverb_id[] = "reverb";
    assert(std::search(payload,
                       payload + hello_size,
                       reverb_id,
                       reverb_id + sizeof(reverb_id) - 1U)
           != payload + hello_size);
    const size_t status_size = LineRackCborEncodeStatus(
        payload, sizeof(payload), 2U, &diagnostics);
    assert(status_size > 0U);
    const char diagnostics_key[] = "diagnostics";
    assert(std::search(payload,
                       payload + status_size,
                       diagnostics_key,
                       diagnostics_key + sizeof(diagnostics_key) - 1U)
           != payload + status_size);
    assert(LineRackCborEncodeError(payload, sizeof(payload), "bad request") > 0U);
    assert(LineRackCborEncodeHello(payload, 8U, 1U, &diagnostics) == 0U);

    const uint8_t slot_request[] = {0xa1U, 0x6aU, 's', 'l', 'o', 't', 'N',
                                    'u',   'm',   'b', 'e', 'r', 0x03U};
    uint8_t slot = 0U;
    assert(LineRackCborDecodeSlotNumber(
        slot_request, sizeof(slot_request), &slot));
    assert(slot == 3U);

    payload[expanded_size - 1U] ^= 0xffU;
    assert(!LineRackCborDecodePresets(payload, expanded_size, &decoded));
    return 0;
}
