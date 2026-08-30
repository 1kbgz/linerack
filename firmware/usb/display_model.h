#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "preset_model.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define LINERACK_DISPLAY_WIDTH 128U
#define LINERACK_DISPLAY_HEIGHT 32U
#define LINERACK_DISPLAY_BYTES 512U
#define LINERACK_PRESET_OVERRIDE_MS 5000U
#define LINERACK_VOLUME_OVERRIDE_MS 2000U
#define LINERACK_DISPLAY_BLANK_AFTER_MS 20000U

typedef struct
{
    uint8_t data[LINERACK_DISPLAY_BYTES];
} LineRackDisplayFrame;

typedef struct
{
    uint32_t started_at_ms;
    bool     active;
} LineRackDisplayOverride;

typedef struct
{
    uint32_t last_activity_ms;
    bool     awake;
} LineRackDisplayPower;

typedef struct
{
    uint32_t started_at_ms;
    uint8_t  percent;
    bool     muted;
    bool     active;
} LineRackVolumeDisplay;

void LineRackDisplayOverrideStart(LineRackDisplayOverride *override,
                                  uint32_t                 now_ms);
LineRackDisplayMode LineRackDisplayResolveMode(LineRackDisplayMode preferred,
                                                LineRackDisplayOverride *override,
                                                uint32_t now_ms);
void LineRackDisplayPowerInit(LineRackDisplayPower *power, uint32_t now_ms);
void LineRackDisplayPowerWake(LineRackDisplayPower *power, uint32_t now_ms);
bool LineRackDisplayPowerResolve(LineRackDisplayPower *power,
                                 bool                  blanking_enabled,
                                 uint32_t              now_ms);
void LineRackVolumeDisplayStart(LineRackVolumeDisplay *volume,
                                uint8_t                percent,
                                bool                   muted,
                                uint32_t               now_ms);
bool LineRackVolumeDisplayActive(LineRackVolumeDisplay *volume,
                                 uint32_t               now_ms);
void LineRackDisplayRender(LineRackDisplayFrame      *frame,
                           const LineRackPresetBank  *bank,
                           uint8_t                    active_slot_number,
                           LineRackDisplayMode        mode);
void LineRackDisplayRenderVisualizer(LineRackDisplayFrame *frame,
                                     uint8_t               left_percent,
                                     uint8_t               right_percent);
void LineRackDisplayRenderVolume(LineRackDisplayFrame *frame,
                                 uint8_t               percent,
                                 bool                  muted);
bool LineRackDisplayPixel(const LineRackDisplayFrame *frame,
                          uint8_t                     x,
                          uint8_t                     y);

#ifdef __cplusplus
}
#endif
