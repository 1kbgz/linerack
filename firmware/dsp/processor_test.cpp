#include <cassert>
#include <cmath>
#include <iostream>

#include "processor.h"

namespace
{
bool Near(float actual, float expected, float tolerance = 0.0001f)
{
    return std::fabs(actual - expected) <= tolerance;
}
} // namespace

int main()
{
    using linerack::dsp::DbToLinear;
    using linerack::dsp::Gain;
    using linerack::dsp::ParametricEq;
    using linerack::dsp::NoiseGate;
    using linerack::dsp::PassFilter;
    using linerack::dsp::PassFilterMode;
    using linerack::dsp::PeakLimiter;
    using linerack::dsp::StereoCompressor;
    using linerack::dsp::StereoReverb;

    assert(Near(DbToLinear(0.0f), 1.0f));
    assert(Near(DbToLinear(-6.0f), 0.501187f));

    Gain gain;
    gain.SetGainDb(6.0f);
    float left = 0.25f;
    float right = -0.125f;
    gain.Process(left, right);
    assert(Near(left, 0.498816f));
    assert(Near(right, -0.249408f));

    PeakLimiter limiter;
    limiter.Configure(-6.0f, 100.0f, 48000.0f);
    left = 1.0f;
    right = -0.5f;
    limiter.Process(left, right);
    assert(Near(left, DbToLinear(-6.0f)));
    assert(Near(right, -0.5f * DbToLinear(-6.0f)));

    left = 0.25f;
    right = -0.25f;
    limiter.Reset();
    limiter.Process(left, right);
    assert(Near(left, 0.25f));
    assert(Near(right, -0.25f));

    ParametricEq equalizer;
    equalizer.Configure(1000.0f, 0.0f, 1.0f, 48000.0f);
    left = 0.25f;
    right = -0.125f;
    equalizer.Process(left, right);
    assert(Near(left, 0.25f));
    assert(Near(right, -0.125f));

    equalizer.Configure(1000.0f, 6.0f, 1.0f, 48000.0f);
    constexpr int kSampleCount = 48000;
    constexpr int kSettlingSamples = 12000;
    constexpr float kTau = 6.283185307f;
    const float phase_increment = kTau * 1000.0f / 48000.0f;
    float phase = 0.0f;
    float input_energy = 0.0f;
    float output_energy = 0.0f;
    for(int sample = 0; sample < kSampleCount; ++sample)
    {
        const float input = 0.1f * std::sin(phase);
        left = input;
        right = 0.0f;
        equalizer.Process(left, right);
        assert(Near(right, 0.0f));
        if(sample >= kSettlingSamples)
        {
            input_energy += input * input;
            output_energy += left * left;
        }
        phase += phase_increment;
        if(phase >= kTau)
            phase -= kTau;
    }
    const float measured_gain_db =
        10.0f * std::log10(output_energy / input_energy);
    assert(Near(measured_gain_db, 6.0f, 0.05f));

    PassFilter high_pass;
    high_pass.Configure(PassFilterMode::HighPass, 1000.0f, 12.0f, 48000.0f);
    phase = 0.0f;
    input_energy = 0.0f;
    output_energy = 0.0f;
    for(int sample = 0; sample < kSampleCount; ++sample)
    {
        const float input = 0.1f * std::sin(phase);
        left = input;
        right = 0.0f;
        high_pass.Process(left, right);
        if(sample >= kSettlingSamples)
        {
            input_energy += input * input;
            output_energy += left * left;
        }
        phase += kTau * 100.0f / 48000.0f;
        if(phase >= kTau) phase -= kTau;
    }
    assert(10.0f * std::log10(output_energy / input_energy) < -35.0f);

    PassFilter low_pass;
    low_pass.Configure(PassFilterMode::LowPass, 1000.0f, 24.0f, 48000.0f);
    phase = 0.0f;
    input_energy = 0.0f;
    output_energy = 0.0f;
    for(int sample = 0; sample < kSampleCount; ++sample)
    {
        const float input = 0.1f * std::sin(phase);
        left = input;
        right = 0.0f;
        low_pass.Process(left, right);
        if(sample >= kSettlingSamples)
        {
            input_energy += input * input;
            output_energy += left * left;
        }
        phase += kTau * 10000.0f / 48000.0f;
        if(phase >= kTau) phase -= kTau;
    }
    assert(10.0f * std::log10(output_energy / input_energy) < -70.0f);

    NoiseGate gate;
    gate.Configure(-20.0f, 1.0f, 0.0f, 1.0f, -60.0f, 1000.0f);
    for(int sample = 0; sample < 20; ++sample)
    {
        left = 0.001f;
        right = -0.001f;
        gate.Process(left, right);
    }
    assert(std::fabs(left) < 0.00001f);
    left = 1.0f;
    right = -0.5f;
    gate.Process(left, right);
    assert(left > 0.6f);
    assert(Near(right, -0.5f * left, 0.001f));

    StereoCompressor compressor;
    compressor.Configure(-12.0f, 4.0f, 0.0f, 0.0f, 48000.0f);
    left = 1.0f;
    right = -0.25f;
    compressor.Process(left, right);
    const float compressed_gain = DbToLinear(-9.0f);
    assert(Near(left, compressed_gain));
    assert(Near(right, -0.25f * compressed_gain));

    compressor.Configure(-12.0f, 4.0f, 1000.0f, 0.0f, 1000.0f);
    left = 1.0f;
    right = 0.0f;
    compressor.Process(left, right);
    const float one_sample_coefficient = std::exp(-0.001f);
    assert(Near(left,
                one_sample_coefficient
                        + (1.0f - one_sample_coefficient) * compressed_gain));

    compressor.Configure(-12.0f, 4.0f, 0.0f, 1000.0f, 1000.0f);
    left = 1.0f;
    right = 0.0f;
    compressor.Process(left, right);
    left = 0.1f;
    compressor.Process(left, right);
    assert(Near(left,
                0.1f
                    * (one_sample_coefficient * compressed_gain
                       + (1.0f - one_sample_coefficient))));

    float reverb_memory[StereoReverb::kMemorySize] = {};
    StereoReverb reverb;
    reverb.Configure(reverb_memory, 50.0f, 50.0f, 0.0f);
    left = 0.25f;
    right = -0.5f;
    reverb.Process(left, right);
    assert(Near(left, 0.25f));
    assert(Near(right, -0.5f));

    reverb.Configure(reverb_memory, 60.0f, 35.0f, 100.0f);
    float early_energy = 0.0f;
    float late_energy = 0.0f;
    float right_energy = 0.0f;
    for(int sample = 0; sample < 120000; ++sample)
    {
        left = sample == 0 ? 1.0f : 0.0f;
        right = 0.0f;
        reverb.Process(left, right);
        assert(std::isfinite(left));
        assert(std::isfinite(right));
        assert(std::fabs(left) < 2.0f);
        assert(std::fabs(right) < 2.0f);
        if(sample >= 1000 && sample < 30000)
            early_energy += left * left + right * right;
        if(sample >= 90000)
            late_energy += left * left + right * right;
        right_energy += right * right;
    }
    assert(early_energy > 0.001f);
    assert(late_energy < early_energy * 0.01f);
    assert(right_energy > 0.001f);

    std::cout << "DSP tests passed\n";
}
