#include "host_volume.h"

#include <assert.h>
#include <math.h>

static bool Near(float left, float right)
{
    return fabsf(left - right) < 0.00001f;
}

int main(void)
{
    LineRackHostVolume volume;
    float left = 0.5f;
    float right = -0.25f;

    LineRackHostVolumeInit(&volume);
    assert(LineRackHostVolumePercent(&volume) == 100U);
    assert(!LineRackHostVolumeMuted(&volume));
    LineRackHostVolumeProcess(&volume, &left, &right);
    assert(Near(left, 0.5f));
    assert(Near(right, -0.25f));

    LineRackHostVolumeSetPercent(&volume, 50U);
    assert(LineRackHostVolumePercent(&volume) == 50U);
    assert(volume.gain == 1.0f);
    left = 1.0f;
    right = -0.5f;
    LineRackHostVolumeProcess(&volume, &left, &right);
    assert(Near(left, powf(10.0f, -30.0f / 20.0f)));
    assert(Near(right, -0.5f * left));

    LineRackHostVolumeSetPercent(&volume, 255U);
    left = 0.25f;
    right = -0.25f;
    LineRackHostVolumeProcess(&volume, &left, &right);
    assert(left == 0.25f);
    assert(right == -0.25f);

    LineRackHostVolumeSetMute(&volume, true);
    assert(LineRackHostVolumeMuted(&volume));
    LineRackHostVolumeProcess(&volume, &left, &right);
    assert(left == 0.0f);
    assert(right == 0.0f);
    return 0;
}
