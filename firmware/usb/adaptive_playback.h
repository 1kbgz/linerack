#pragma once

#include <stdbool.h>
#include <stdint.h>

#include "pcm_ring.h"

#define ADAPTIVE_PLAYBACK_START_FRAMES 2048U
#define ADAPTIVE_PLAYBACK_TARGET_FRAMES 2046U

typedef struct
{
    bool     started;
    float    phase;
    float    input_frames_per_output_frame;
    uint32_t frames_until_control_update;
    int16_t  current[2];
    int16_t  next[2];
} AdaptivePlayback;

void AdaptivePlaybackReset(AdaptivePlayback *playback);
bool AdaptivePlaybackRead(AdaptivePlayback *playback,
                          PcmRingBuffer     *ring,
                          float             *left,
                          float             *right,
                          bool              *starved);
float AdaptivePlaybackRatio(const AdaptivePlayback *playback);
