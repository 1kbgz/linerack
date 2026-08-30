#include "adaptive_playback.h"

#define CONTROL_UPDATE_FRAMES 48U
#define RATIO_PER_ERROR_FRAME 0.000001f
#define MIN_INPUT_RATIO 0.999f
#define MAX_INPUT_RATIO 1.001f
#define INT16_TO_FLOAT (1.0f / 32768.0f)

static void UpdateRatio(AdaptivePlayback *playback, const PcmRingBuffer *ring)
{
    const int32_t fill_error =
        (int32_t)PcmRingFill(ring) - (int32_t)ADAPTIVE_PLAYBACK_TARGET_FRAMES;
    float ratio = 1.0f + (float)fill_error * RATIO_PER_ERROR_FRAME;

    if(ratio < MIN_INPUT_RATIO)
        ratio = MIN_INPUT_RATIO;
    else if(ratio > MAX_INPUT_RATIO)
        ratio = MAX_INPUT_RATIO;

    playback->input_frames_per_output_frame = ratio;
    playback->frames_until_control_update = CONTROL_UPDATE_FRAMES;
}

void AdaptivePlaybackReset(AdaptivePlayback *playback)
{
    playback->started = false;
    playback->phase = 0.0f;
    playback->input_frames_per_output_frame = 1.0f;
    playback->frames_until_control_update = 0U;
    playback->current[0] = 0;
    playback->current[1] = 0;
    playback->next[0] = 0;
    playback->next[1] = 0;
}

static bool StartPlayback(AdaptivePlayback *playback, PcmRingBuffer *ring)
{
    if(PcmRingFill(ring) < ADAPTIVE_PLAYBACK_START_FRAMES)
        return false;

    if(!PcmRingRead(ring, &playback->current[0], &playback->current[1])
       || !PcmRingRead(ring, &playback->next[0], &playback->next[1]))
        return false;

    playback->started = true;
    playback->phase = 0.0f;
    UpdateRatio(playback, ring);
    return true;
}

bool AdaptivePlaybackRead(AdaptivePlayback *playback,
                          PcmRingBuffer     *ring,
                          float             *left,
                          float             *right,
                          bool              *starved)
{
    *starved = false;
    if(!playback->started && !StartPlayback(playback, ring))
        return false;

    if(playback->frames_until_control_update == 0U)
        UpdateRatio(playback, ring);
    --playback->frames_until_control_update;

    *left = ((float)playback->current[0]
             + playback->phase
                   * (float)(playback->next[0] - playback->current[0]))
            * INT16_TO_FLOAT;
    *right = ((float)playback->current[1]
              + playback->phase
                    * (float)(playback->next[1] - playback->current[1]))
             * INT16_TO_FLOAT;

    playback->phase += playback->input_frames_per_output_frame;
    while(playback->phase >= 1.0f)
    {
        playback->phase -= 1.0f;
        playback->current[0] = playback->next[0];
        playback->current[1] = playback->next[1];
        if(!PcmRingRead(ring, &playback->next[0], &playback->next[1]))
        {
            playback->started = false;
            playback->phase = 0.0f;
            *starved = true;
            break;
        }
    }

    return true;
}

float AdaptivePlaybackRatio(const AdaptivePlayback *playback)
{
    return playback->input_frames_per_output_frame;
}
