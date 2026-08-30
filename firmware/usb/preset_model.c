#include "preset_model.h"

#include <math.h>
#include <string.h>

static void SetBlock(LineRackBlock *block,
                     uint8_t        type,
                     float          first,
                     float          second,
                     float          third)
{
    memset(block, 0, sizeof(*block));
    block->type = type;
    block->enabled = 1U;
    block->parameters[0] = first;
    block->parameters[1] = second;
    block->parameters[2] = third;
}

void LineRackPresetBankDefaults(LineRackPresetBank *bank)
{
    static const char *names[LINERACK_PRESET_COUNT] = {
        "Clean",
        "Punch",
        "Small Room",
        "Wide Hall",
    };
    static const float gain_db[LINERACK_PRESET_COUNT] = {0.0f, -3.0f, -3.0f, -4.0f};

    memset(bank, 0, sizeof(*bank));
    bank->schema_version = 1U;
    bank->display_mode = LINERACK_DISPLAY_PRESET;
    bank->source_mode = LINERACK_SOURCE_USB;
    for(uint32_t index = 0U; index < LINERACK_PRESET_COUNT; ++index)
    {
        LineRackPreset *preset = &bank->presets[index];
        preset->number = (uint8_t)(index + 1U);
        preset->block_count = 3U;
        strncpy(preset->name, names[index], LINERACK_PRESET_NAME_BYTES - 1U);
        SetBlock(&preset->blocks[0], LINERACK_BLOCK_GAIN, gain_db[index], 0.0f, 0.0f);
        SetBlock(&preset->blocks[1], LINERACK_BLOCK_PARAMETRIC_EQ, 1000.0f, 0.0f, 1.0f);
        SetBlock(&preset->blocks[2], LINERACK_BLOCK_LIMITER, -1.0f, 0.0f, 0.0f);
    }

    LineRackPreset *punch = &bank->presets[1];
    punch->block_count = 4U;
    SetBlock(&punch->blocks[1], LINERACK_BLOCK_PARAMETRIC_EQ, 120.0f, 3.0f, 0.8f);
    SetBlock(&punch->blocks[2], LINERACK_BLOCK_COMPRESSOR, -18.0f, 3.0f, 10.0f);
    punch->blocks[2].parameters[3] = 120.0f;
    SetBlock(&punch->blocks[3], LINERACK_BLOCK_LIMITER, -1.0f, 0.0f, 0.0f);

    LineRackPreset *room = &bank->presets[2];
    room->block_count = 4U;
    SetBlock(&room->blocks[1], LINERACK_BLOCK_COMPRESSOR, -16.0f, 2.0f, 15.0f);
    room->blocks[1].parameters[3] = 180.0f;
    SetBlock(&room->blocks[2], LINERACK_BLOCK_REVERB, 30.0f, 55.0f, 15.0f);
    SetBlock(&room->blocks[3], LINERACK_BLOCK_LIMITER, -1.0f, 0.0f, 0.0f);

    LineRackPreset *hall = &bank->presets[3];
    hall->block_count = 4U;
    SetBlock(&hall->blocks[1], LINERACK_BLOCK_COMPRESSOR, -20.0f, 2.5f, 25.0f);
    hall->blocks[1].parameters[3] = 250.0f;
    SetBlock(&hall->blocks[2], LINERACK_BLOCK_REVERB, 80.0f, 40.0f, 28.0f);
    SetBlock(&hall->blocks[3], LINERACK_BLOCK_LIMITER, -1.0f, 0.0f, 0.0f);
}

static bool InRange(float value, float minimum, float maximum)
{
    return isfinite(value) && value >= minimum && value <= maximum;
}

static bool PresetNameCharacterValid(char character)
{
    return (character >= 'A' && character <= 'Z')
           || (character >= 'a' && character <= 'z')
           || (character >= '0' && character <= '9')
           || character == ' ' || character == '.' || character == '?'
           || character == '-';
}

static bool PresetValid(const LineRackPreset *preset, uint8_t expected_number)
{
    if(preset->number != expected_number || preset->block_count == 0U
       || preset->block_count > LINERACK_MAX_BLOCKS
       || preset->name[0] == '\0'
       || memchr(preset->name, '\0', LINERACK_PRESET_NAME_BYTES) == NULL)
        return false;
    for(size_t index = 0U; preset->name[index] != '\0'; ++index)
        if(!PresetNameCharacterValid(preset->name[index]))
            return false;

    bool gain_seen = false;
    bool limiter_seen = false;
    for(uint8_t index = 0U; index < preset->block_count; ++index)
    {
        const LineRackBlock *block = &preset->blocks[index];
        switch(block->type)
        {
            case LINERACK_BLOCK_GAIN:
                if(gain_seen || index != 0U
                   || !InRange(block->parameters[0], -24.0f, 12.0f))
                    return false;
                gain_seen = true;
                break;
            case LINERACK_BLOCK_PARAMETRIC_EQ:
                if(!InRange(block->parameters[0], 20.0f, 20000.0f)
                   || !InRange(block->parameters[1], -18.0f, 18.0f)
                   || !InRange(block->parameters[2], 0.1f, 18.0f))
                    return false;
                break;
            case LINERACK_BLOCK_HIGH_PASS:
            case LINERACK_BLOCK_LOW_PASS:
                if(!InRange(block->parameters[0], 20.0f, 20000.0f)
                   || (block->parameters[1] != 12.0f
                       && block->parameters[1] != 24.0f))
                    return false;
                break;
            case LINERACK_BLOCK_NOISE_GATE:
                if(!InRange(block->parameters[0], -80.0f, 0.0f)
                   || !InRange(block->parameters[1], 0.1f, 100.0f)
                   || !InRange(block->parameters[2], 0.0f, 500.0f)
                   || !InRange(block->parameters[3], 5.0f, 2000.0f)
                   || !InRange(block->parameters[4], -80.0f, 0.0f))
                    return false;
                break;
            case LINERACK_BLOCK_COMPRESSOR:
                if(!InRange(block->parameters[0], -60.0f, 0.0f)
                   || !InRange(block->parameters[1], 1.0f, 20.0f)
                   || !InRange(block->parameters[2], 0.1f, 200.0f)
                   || !InRange(block->parameters[3], 10.0f, 2000.0f))
                    return false;
                break;
            case LINERACK_BLOCK_REVERB:
                if(!InRange(block->parameters[0], 0.0f, 100.0f)
                   || !InRange(block->parameters[1], 0.0f, 100.0f)
                   || !InRange(block->parameters[2], 0.0f, 100.0f))
                    return false;
                break;
            case LINERACK_BLOCK_LIMITER:
                if(limiter_seen || index != preset->block_count - 1U
                   || !InRange(block->parameters[0], -12.0f, 0.0f))
                    return false;
                limiter_seen = true;
                break;
            default: return false;
        }
    }
    return gain_seen && limiter_seen;
}

bool LineRackPresetBankValid(const LineRackPresetBank *bank)
{
    if(bank == NULL || bank->schema_version != 1U
       || (bank->display_mode
           & ~(LINERACK_DISPLAY_MODE_MASK
               | LINERACK_DISPLAY_BLANKING_DISABLED)) != 0U
       || (bank->display_mode & LINERACK_DISPLAY_MODE_MASK)
              > LINERACK_DISPLAY_VISUALIZER
       || bank->source_mode > LINERACK_SOURCE_MIX
       || bank->usb_trim_half_db < -48 || bank->usb_trim_half_db > 0
       || bank->analog_trim_half_db < -48 || bank->analog_trim_half_db > 0
       || (bank->source_mode == LINERACK_SOURCE_MIX
           && (bank->usb_trim_half_db > -13
               || bank->analog_trim_half_db > -13)))
        return false;
    for(uint8_t index = 0U; index < LINERACK_PRESET_COUNT; ++index)
    {
        if(!PresetValid(&bank->presets[index], (uint8_t)(index + 1U)))
            return false;
    }
    return true;
}

bool LineRackPresetBankEqual(const LineRackPresetBank *left,
                             const LineRackPresetBank *right)
{
    return left != NULL && right != NULL
           && memcmp(left, right, sizeof(*left)) == 0;
}
