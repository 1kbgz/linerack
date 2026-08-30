#pragma once

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define LINERACK_PRESET_COUNT 4U
#define LINERACK_PRESET_NAME_BYTES 13U
#define LINERACK_MAX_BLOCKS 10U
#define LINERACK_MAX_PARAMETERS 5U
#define LINERACK_DISPLAY_MODE_MASK 0x03U
#define LINERACK_DISPLAY_BLANKING_DISABLED 0x80U

typedef enum
{
    LINERACK_BLOCK_GAIN = 1,
    LINERACK_BLOCK_PARAMETRIC_EQ = 2,
    LINERACK_BLOCK_LIMITER = 3,
    LINERACK_BLOCK_HIGH_PASS = 4,
    LINERACK_BLOCK_LOW_PASS = 5,
    LINERACK_BLOCK_NOISE_GATE = 6,
    LINERACK_BLOCK_COMPRESSOR = 7,
    LINERACK_BLOCK_REVERB = 8,
} LineRackBlockType;

typedef enum
{
    LINERACK_DISPLAY_PRESET = 0,
    LINERACK_DISPLAY_EQ_RESPONSE = 1,
    LINERACK_DISPLAY_VISUALIZER = 2,
} LineRackDisplayMode;

typedef enum
{
    LINERACK_SOURCE_USB = 0,
    LINERACK_SOURCE_ANALOG = 1,
    LINERACK_SOURCE_MIX = 2,
} LineRackSourceMode;

typedef struct
{
    uint8_t type;
    uint8_t enabled;
    uint8_t reserved[2];
    float   parameters[LINERACK_MAX_PARAMETERS];
} LineRackBlock;

typedef struct
{
    uint8_t       number;
    uint8_t       block_count;
    uint8_t       reserved[2];
    char          name[LINERACK_PRESET_NAME_BYTES];
    uint8_t       name_padding[3];
    LineRackBlock blocks[LINERACK_MAX_BLOCKS];
} LineRackPreset;

typedef struct
{
    uint32_t       schema_version;
    uint8_t        display_mode;
    uint8_t        source_mode;
    int8_t         usb_trim_half_db;
    int8_t         analog_trim_half_db;
    LineRackPreset presets[LINERACK_PRESET_COUNT];
} LineRackPresetBank;

void LineRackPresetBankDefaults(LineRackPresetBank *bank);
bool LineRackPresetBankValid(const LineRackPresetBank *bank);
bool LineRackPresetBankEqual(const LineRackPresetBank *left,
                             const LineRackPresetBank *right);

#ifdef __cplusplus
}
#endif
