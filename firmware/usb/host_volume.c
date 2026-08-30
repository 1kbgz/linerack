#include "host_volume.h"

#include <math.h>
#include <stddef.h>

void LineRackHostVolumeInit(LineRackHostVolume *volume)
{
    if(volume == NULL)
        return;
    volume->percent = 100U;
    volume->applied_percent = 100U;
    volume->gain = 1.0f;
    volume->muted = 0U;
}

void LineRackHostVolumeSetPercent(LineRackHostVolume *volume, uint8_t percent)
{
    if(volume == NULL)
        return;
    if(percent > 100U)
        percent = 100U;
    volume->percent = percent;
}

void LineRackHostVolumeSetMute(LineRackHostVolume *volume, bool muted)
{
    if(volume != NULL)
        volume->muted = muted ? 1U : 0U;
}

uint8_t LineRackHostVolumePercent(const LineRackHostVolume *volume)
{
    return volume == NULL ? 100U : volume->percent;
}

bool LineRackHostVolumeMuted(const LineRackHostVolume *volume)
{
    return volume != NULL && volume->muted != 0U;
}

void LineRackHostVolumeProcess(LineRackHostVolume *volume,
                               float              *left,
                               float              *right)
{
    if(volume == NULL || left == NULL || right == NULL)
        return;
    const uint8_t percent = volume->percent;
    if(percent != volume->applied_percent)
    {
        const float decibels = -60.0f + 0.6f * percent;
        volume->gain = percent == 100U ? 1.0f : powf(10.0f, decibels / 20.0f);
        volume->applied_percent = percent;
    }
    const float gain = volume->muted != 0U ? 0.0f : volume->gain;
    *left *= gain;
    *right *= gain;
}
