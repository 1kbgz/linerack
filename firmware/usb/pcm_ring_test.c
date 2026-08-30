#include <assert.h>
#include <limits.h>
#include <stdio.h>

#include "pcm_ring.h"

static PcmRingBuffer ring;
static uint8_t full_ring[PCM_RING_CAPACITY_FRAMES * 4U];

int main(void)
{
    int16_t left;
    int16_t right;
    const uint8_t stereo_frames[] = {
        0x34, 0x12, 0x00, 0x80,
        0xFF, 0x7F, 0xFE, 0xFF,
    };

    PcmRingReset(&ring);
    assert(PcmRingFill(&ring) == 0U);
    assert(!PcmRingRead(&ring, &left, &right));
    assert(PcmRingWriteLe16(&ring, stereo_frames, sizeof(stereo_frames)) == 2U);
    assert(PcmRingFill(&ring) == 2U);

    assert(PcmRingRead(&ring, &left, &right));
    assert(left == 0x1234);
    assert(right == INT16_MIN);
    assert(PcmRingRead(&ring, &left, &right));
    assert(left == INT16_MAX);
    assert(right == -2);
    assert(!PcmRingRead(&ring, &left, &right));

    assert(PcmRingWriteLe16(&ring, stereo_frames, 3U) == 0U);

    PcmRingReset(&ring);
    assert(PcmRingWriteLe16(&ring, full_ring, sizeof(full_ring))
           == PCM_RING_CAPACITY_FRAMES);
    assert(PcmRingWriteLe16(&ring, stereo_frames, sizeof(stereo_frames)) == 0U);
    assert(PcmRingRead(&ring, &left, &right));
    assert(PcmRingRead(&ring, &left, &right));
    assert(PcmRingWriteLe16(&ring, stereo_frames, sizeof(stereo_frames)) == 2U);
    assert(PcmRingFill(&ring) == PCM_RING_CAPACITY_FRAMES);

    for(uint32_t frame = 0U; frame < PCM_RING_CAPACITY_FRAMES - 2U; ++frame)
        assert(PcmRingRead(&ring, &left, &right));
    assert(PcmRingRead(&ring, &left, &right));
    assert(left == 0x1234);
    assert(right == INT16_MIN);
    assert(PcmRingRead(&ring, &left, &right));
    assert(left == INT16_MAX);
    assert(right == -2);
    assert(!PcmRingRead(&ring, &left, &right));

    printf("PCM ring tests passed\n");
    return 0;
}
