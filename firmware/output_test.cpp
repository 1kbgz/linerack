#include <cmath>

#include "daisy_seed.h"

using namespace daisy;

namespace
{
constexpr float kAmplitude = 0.0316f;
constexpr float kLeftFrequency = 440.0f;
constexpr float kRightFrequency = 660.0f;
constexpr float kTau = 6.283185307f;

DaisySeed hardware;
float left_phase = 0.0f;
float right_phase = 0.0f;
float left_increment = 0.0f;
float right_increment = 0.0f;

void AudioCallback(AudioHandle::InputBuffer,
                   AudioHandle::OutputBuffer output,
                   size_t                    size)
{
    for(size_t sample = 0; sample < size; ++sample)
    {
        output[0][sample] = kAmplitude * std::sin(left_phase);
        output[1][sample] = kAmplitude * std::sin(right_phase);

        left_phase += left_increment;
        right_phase += right_increment;
        if(left_phase >= kTau)
            left_phase -= kTau;
        if(right_phase >= kTau)
            right_phase -= kTau;
    }
}
} // namespace

int main()
{
    hardware.Init();

    const float sample_rate = hardware.AudioSampleRate();
    left_increment = kTau * kLeftFrequency / sample_rate;
    right_increment = kTau * kRightFrequency / sample_rate;

    hardware.SetLed(true);
    hardware.StartAudio(AudioCallback);

    while(true)
        System::Delay(1000);
}
