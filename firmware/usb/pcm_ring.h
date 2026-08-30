#pragma once

#include <stdbool.h>
#include <stdint.h>

#define PCM_RING_CAPACITY_FRAMES 4096U

typedef struct
{
    int16_t           samples[PCM_RING_CAPACITY_FRAMES][2];
    volatile uint32_t read_sequence;
    volatile uint32_t write_sequence;
} PcmRingBuffer;

void     PcmRingReset(PcmRingBuffer *ring);
uint32_t PcmRingWriteLe16(PcmRingBuffer *ring,
                          const uint8_t *bytes,
                          uint32_t       byte_count);
bool     PcmRingRead(PcmRingBuffer *ring, int16_t *left, int16_t *right);
uint32_t PcmRingFill(const PcmRingBuffer *ring);
