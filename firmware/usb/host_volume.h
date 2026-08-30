#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef struct
{
    volatile uint8_t percent;
    volatile uint8_t muted;
    uint8_t          applied_percent;
    float            gain;
} LineRackHostVolume;

void LineRackHostVolumeInit(LineRackHostVolume *volume);
void LineRackHostVolumeSetPercent(LineRackHostVolume *volume, uint8_t percent);
void LineRackHostVolumeSetMute(LineRackHostVolume *volume, bool muted);
uint8_t LineRackHostVolumePercent(const LineRackHostVolume *volume);
bool LineRackHostVolumeMuted(const LineRackHostVolume *volume);
void LineRackHostVolumeProcess(LineRackHostVolume *volume,
                               float              *left,
                               float              *right);
