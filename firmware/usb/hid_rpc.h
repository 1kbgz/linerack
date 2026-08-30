#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "preset_model.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct
{
    uint8_t (*active_slot)(void *context);
    bool (*activate_slot)(void *context, uint8_t slot_number);
    bool (*cycle_preset)(void *context);
    void (*read_presets)(void *context, LineRackPresetBank *bank);
    bool (*write_presets)(void *context, const LineRackPresetBank *bank);
    void (*wake_display)(void *context);
    void *context;
} LineRackHidRpcCallbacks;

void LineRackHidRpcInit(const LineRackHidRpcCallbacks *callbacks);
void LineRackHidRpcAcceptReport(const uint8_t report[64]);
void LineRackHidRpcPoll(void);
void LineRackHidRpcNotifyStatusChanged(void);

#ifdef __cplusplus
}
#endif
