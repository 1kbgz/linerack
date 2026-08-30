#include "pcm_ring.h"

static uint32_t LoadAcquire(const volatile uint32_t *value)
{
    return __atomic_load_n(value, __ATOMIC_ACQUIRE);
}

static uint32_t LoadRelaxed(const volatile uint32_t *value)
{
    return __atomic_load_n(value, __ATOMIC_RELAXED);
}

static void StoreRelease(volatile uint32_t *destination, uint32_t value)
{
    __atomic_store_n(destination, value, __ATOMIC_RELEASE);
}

void PcmRingReset(PcmRingBuffer *ring)
{
    ring->read_sequence = 0U;
    ring->write_sequence = 0U;
}

uint32_t PcmRingWriteLe16(PcmRingBuffer *ring,
                          const uint8_t *bytes,
                          uint32_t       byte_count)
{
    if((byte_count % 4U) != 0U)
        return 0U;

    const uint32_t frame_count = byte_count / 4U;
    const uint32_t write = LoadRelaxed(&ring->write_sequence);
    const uint32_t read = LoadAcquire(&ring->read_sequence);
    const uint32_t fill = write - read;

    if(frame_count > PCM_RING_CAPACITY_FRAMES - fill)
        return 0U;

    for(uint32_t frame = 0U; frame < frame_count; ++frame)
    {
        const uint32_t source = frame * 4U;
        const uint32_t destination =
            (write + frame) & (PCM_RING_CAPACITY_FRAMES - 1U);
        ring->samples[destination][0] =
            (int16_t)((uint16_t)bytes[source]
                      | ((uint16_t)bytes[source + 1U] << 8U));
        ring->samples[destination][1] =
            (int16_t)((uint16_t)bytes[source + 2U]
                      | ((uint16_t)bytes[source + 3U] << 8U));
    }

    StoreRelease(&ring->write_sequence, write + frame_count);
    return frame_count;
}

bool PcmRingRead(PcmRingBuffer *ring, int16_t *left, int16_t *right)
{
    const uint32_t read = LoadRelaxed(&ring->read_sequence);
    const uint32_t write = LoadAcquire(&ring->write_sequence);

    if(read == write)
        return false;

    const uint32_t source = read & (PCM_RING_CAPACITY_FRAMES - 1U);
    *left = ring->samples[source][0];
    *right = ring->samples[source][1];
    StoreRelease(&ring->read_sequence, read + 1U);
    return true;
}

uint32_t PcmRingFill(const PcmRingBuffer *ring)
{
    const uint32_t write = LoadAcquire(&ring->write_sequence);
    const uint32_t read = LoadAcquire(&ring->read_sequence);
    return write - read;
}
