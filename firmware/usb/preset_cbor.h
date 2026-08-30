#pragma once

#include <stddef.h>
#include <stdint.h>

#include "preset_model.h"

typedef struct
{
    uint32_t usb_packets;
    uint32_t underruns;
    uint32_t overruns;
    uint32_t buffer_fill_frames;
} LineRackDeviceDiagnostics;

size_t LineRackCborEncodeHello(uint8_t *output,
                               size_t   capacity,
                               uint8_t  active_slot,
                               const LineRackDeviceDiagnostics *diagnostics);
size_t LineRackCborEncodePresets(uint8_t                  *output,
                                 size_t                    capacity,
                                 const LineRackPresetBank *bank);
size_t LineRackCborEncodeStatus(uint8_t *output,
                                size_t   capacity,
                                uint8_t  active_slot,
                                const LineRackDeviceDiagnostics *diagnostics);
size_t LineRackCborEncodeError(uint8_t    *output,
                               size_t      capacity,
                               const char *message);
bool LineRackCborDecodePresets(const uint8_t      *input,
                               size_t              size,
                               LineRackPresetBank *bank);
bool LineRackCborDecodeSlotNumber(const uint8_t *input,
                                  size_t         size,
                                  uint8_t       *slot_number);
