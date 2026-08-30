#include <assert.h>
#include <math.h>
#include <stdio.h>

#include "adaptive_playback.h"

static PcmRingBuffer ring;
static AdaptivePlayback playback;
static uint8_t pcm[(ADAPTIVE_PLAYBACK_START_FRAMES + 100U) * 4U];

static void SetFrame(uint32_t frame, int16_t left, int16_t right)
{
    const uint32_t offset = frame * 4U;
    pcm[offset] = (uint8_t)left;
    pcm[offset + 1U] = (uint8_t)((uint16_t)left >> 8U);
    pcm[offset + 2U] = (uint8_t)right;
    pcm[offset + 3U] = (uint8_t)((uint16_t)right >> 8U);
}

int main(void)
{
    float left;
    float right;
    bool starved;

    SetFrame(0U, 0, INT16_MIN);
    SetFrame(1U, INT16_MAX, 0);

    PcmRingReset(&ring);
    AdaptivePlaybackReset(&playback);
    assert(PcmRingWriteLe16(
               &ring, pcm, ADAPTIVE_PLAYBACK_START_FRAMES * 4U)
           == ADAPTIVE_PLAYBACK_START_FRAMES);
    assert(AdaptivePlaybackRead(
        &playback, &ring, &left, &right, &starved));
    assert(left == 0.0f);
    assert(right == -1.0f);
    assert(!starved);
    assert(fabsf(AdaptivePlaybackRatio(&playback) - 1.0f) < 0.000001f);

    assert(AdaptivePlaybackRead(
        &playback, &ring, &left, &right, &starved));
    assert(fabsf(left - 32767.0f / 32768.0f) < 0.000001f);
    assert(right == 0.0f);

    for(uint32_t frame = 0U; frame < 48U; ++frame)
        assert(AdaptivePlaybackRead(
            &playback, &ring, &left, &right, &starved));
    assert(AdaptivePlaybackRatio(&playback) < 1.0f);

    PcmRingReset(&ring);
    AdaptivePlaybackReset(&playback);
    assert(PcmRingWriteLe16(&ring, pcm, sizeof(pcm))
           == ADAPTIVE_PLAYBACK_START_FRAMES + 100U);
    assert(AdaptivePlaybackRead(
        &playback, &ring, &left, &right, &starved));
    assert(AdaptivePlaybackRatio(&playback) > 1.0f);

    printf("Adaptive playback tests passed\n");
    return 0;
}
