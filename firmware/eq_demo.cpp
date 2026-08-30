#include <atomic>
#include <cmath>

#include "daisy_seed.h"
#include "dsp/processor.h"

using namespace daisy;

namespace
{
constexpr uint8_t kSlotCount = 4;
constexpr float kAmplitude = 0.007943f;
constexpr float kLeftFrequency = 440.0f;
constexpr float kRightFrequency = 660.0f;
constexpr float kLimiterCeilingDb = -6.0f;
constexpr float kLimiterReleaseMs = 100.0f;
constexpr float kTau = 6.283185307f;

struct EqPreset
{
    float frequency_hz;
    float gain_db;
    float q;
};

constexpr EqPreset kPresets[kSlotCount] = {
    {1000.0f, 0.0f, 1.0f},
    {440.0f, 12.0f, 8.0f},
    {440.0f, -12.0f, 8.0f},
    {660.0f, 12.0f, 8.0f},
};

DaisySeed hardware;
Switch preset_switch;
linerack::dsp::StereoProcessor processors[kSlotCount];
std::atomic<uint8_t> active_slot_index{0};
float left_phase = 0.0f;
float right_phase = 0.0f;
float left_increment = 0.0f;
float right_increment = 0.0f;

void AudioCallback(AudioHandle::InputBuffer,
                   AudioHandle::OutputBuffer output,
                   size_t                    size)
{
    const uint8_t slot = active_slot_index.load(std::memory_order_relaxed);

    for(size_t sample = 0; sample < size; ++sample)
    {
        float left = kAmplitude * std::sin(left_phase);
        float right = kAmplitude * std::sin(right_phase);
        processors[slot].Process(left, right);
        output[0][sample] = left;
        output[1][sample] = right;

        left_phase += left_increment;
        right_phase += right_increment;
        if(left_phase >= kTau)
            left_phase -= kTau;
        if(right_phase >= kTau)
            right_phase -= kTau;
    }
}

void BlinkSlot(uint8_t slot_index)
{
    for(uint8_t blink = 0; blink <= slot_index; ++blink)
    {
        hardware.SetLed(true);
        System::Delay(100);
        hardware.SetLed(false);
        System::Delay(150);
    }
}
} // namespace

int main()
{
    hardware.Init();
    preset_switch.Init(seed::D10);

    const float sample_rate = hardware.AudioSampleRate();
    left_increment = kTau * kLeftFrequency / sample_rate;
    right_increment = kTau * kRightFrequency / sample_rate;
    for(uint8_t slot = 0; slot < kSlotCount; ++slot)
    {
        processors[slot].Configure(0.0f,
                                   kPresets[slot].frequency_hz,
                                   kPresets[slot].gain_db,
                                   kPresets[slot].q,
                                   kLimiterCeilingDb,
                                   kLimiterReleaseMs,
                                   sample_rate);
    }

    hardware.StartAudio(AudioCallback);
    BlinkSlot(0);

    while(true)
    {
        preset_switch.Debounce();
        if(preset_switch.RisingEdge() || preset_switch.FallingEdge())
        {
            const uint8_t next_slot =
                (active_slot_index.load(std::memory_order_relaxed) + 1)
                % kSlotCount;
            active_slot_index.store(next_slot, std::memory_order_relaxed);
            BlinkSlot(next_slot);
        }
        System::Delay(1);
    }
}
