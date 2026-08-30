#pragma once

#include <algorithm>
#include <cstddef>
#include <cmath>
#include <cstdint>

namespace linerack
{
namespace dsp
{
inline float DbToLinear(float decibels)
{
    return std::pow(10.0f, decibels / 20.0f);
}

class Gain
{
  public:
    void SetGainDb(float decibels) { multiplier_ = DbToLinear(decibels); }

    void Process(float& left, float& right) const
    {
        left *= multiplier_;
        right *= multiplier_;
    }

  private:
    float multiplier_ = 0.0f;
};

class Biquad
{
  public:
    void Configure(float b0, float b1, float b2, float a1, float a2)
    {
        b0_ = b0;
        b1_ = b1;
        b2_ = b2;
        a1_ = a1;
        a2_ = a2;
        Reset();
    }

    void Reset()
    {
        left_ = {};
        right_ = {};
    }

    void Process(float& left, float& right)
    {
        left = ProcessSample(left, left_);
        right = ProcessSample(right, right_);
    }

  private:
    struct State
    {
        float z1 = 0.0f;
        float z2 = 0.0f;
    };

    float ProcessSample(float input, State& state) const
    {
        const float output = b0_ * input + state.z1;
        state.z1 = b1_ * input - a1_ * output + state.z2;
        state.z2 = b2_ * input - a2_ * output;
        return output;
    }

    float b0_ = 0.0f;
    float b1_ = 0.0f;
    float b2_ = 0.0f;
    float a1_ = 0.0f;
    float a2_ = 0.0f;
    State left_;
    State right_;
};

class ParametricEq
{
  public:
    void Configure(float frequency_hz,
                   float gain_db,
                   float q,
                   float sample_rate)
    {
        constexpr float kPi = 3.141592654f;
        const float amplitude = DbToLinear(gain_db * 0.5f);
        const float omega = 2.0f * kPi * frequency_hz / sample_rate;
        const float alpha = std::sin(omega) / (2.0f * q);
        const float cosine = std::cos(omega);
        const float a0 = 1.0f + alpha / amplitude;

        filter_.Configure((1.0f + alpha * amplitude) / a0,
                          (-2.0f * cosine) / a0,
                          (1.0f - alpha * amplitude) / a0,
                          (-2.0f * cosine) / a0,
                          (1.0f - alpha / amplitude) / a0);
    }

    void Reset() { filter_.Reset(); }

    void Process(float& left, float& right) { filter_.Process(left, right); }

  private:
    Biquad filter_;
};

enum class PassFilterMode
{
    HighPass,
    LowPass,
};

class PassFilter
{
  public:
    void Configure(PassFilterMode mode,
                   float          cutoff_hz,
                   float          slope_db_per_octave,
                   float          sample_rate)
    {
        stage_count_ = slope_db_per_octave >= 24.0f ? 2U : 1U;
        if(stage_count_ == 2U)
        {
            ConfigureStage(filters_[0], mode, cutoff_hz, 0.541196f, sample_rate);
            ConfigureStage(filters_[1], mode, cutoff_hz, 1.306563f, sample_rate);
        }
        else
            ConfigureStage(filters_[0], mode, cutoff_hz, 0.70710678f, sample_rate);
    }

    void Process(float& left, float& right)
    {
        for(uint8_t stage = 0U; stage < stage_count_; ++stage)
            filters_[stage].Process(left, right);
    }

  private:
    static void ConfigureStage(Biquad&        filter,
                               PassFilterMode mode,
                               float          cutoff_hz,
                               float          q,
                               float          sample_rate)
    {
        constexpr float kPi = 3.141592654f;
        const float omega = 2.0f * kPi * cutoff_hz / sample_rate;
        const float cosine = std::cos(omega);
        const float alpha = std::sin(omega) / (2.0f * q);
        const float a0 = 1.0f + alpha;
        const bool high_pass = mode == PassFilterMode::HighPass;
        const float b0 = ((high_pass ? 1.0f + cosine : 1.0f - cosine) * 0.5f) / a0;
        const float b1 = (high_pass ? -(1.0f + cosine) : 1.0f - cosine) / a0;
        filter.Configure(b0,
                         b1,
                         b0,
                         (-2.0f * cosine) / a0,
                         (1.0f - alpha) / a0);
    }

    Biquad filters_[2];
    uint8_t stage_count_ = 0U;
};

class NoiseGate
{
  public:
    void Configure(float threshold_db,
                   float attack_ms,
                   float hold_ms,
                   float release_ms,
                   float range_db,
                   float sample_rate)
    {
        threshold_ = DbToLinear(threshold_db);
        floor_ = DbToLinear(range_db);
        attack_coefficient_ = TimeCoefficient(attack_ms, sample_rate);
        release_coefficient_ = TimeCoefficient(release_ms, sample_rate);
        hold_samples_ = static_cast<uint32_t>(hold_ms * 0.001f * sample_rate);
        Reset();
    }

    void Reset()
    {
        gain_ = 1.0f;
        hold_remaining_ = 0U;
    }

    void Process(float& left, float& right)
    {
        const float peak = std::max(std::fabs(left), std::fabs(right));
        float target = floor_;
        if(peak >= threshold_)
        {
            hold_remaining_ = hold_samples_;
            target = 1.0f;
        }
        else if(hold_remaining_ > 0U)
        {
            --hold_remaining_;
            target = 1.0f;
        }
        const float coefficient = target > gain_ ? attack_coefficient_
                                                  : release_coefficient_;
        gain_ = coefficient * gain_ + (1.0f - coefficient) * target;
        left *= gain_;
        right *= gain_;
    }

  private:
    static float TimeCoefficient(float milliseconds, float sample_rate)
    {
        const float samples = milliseconds * 0.001f * sample_rate;
        return samples > 0.0f ? std::exp(-1.0f / samples) : 0.0f;
    }

    float threshold_ = 0.0f;
    float floor_ = 0.0f;
    float attack_coefficient_ = 0.0f;
    float release_coefficient_ = 0.0f;
    float gain_ = 0.0f;
    uint32_t hold_samples_ = 0U;
    uint32_t hold_remaining_ = 0U;
};

class StereoCompressor
{
  public:
    void Configure(float threshold_db,
                   float ratio,
                   float attack_ms,
                   float release_ms,
                   float sample_rate)
    {
        threshold_ = DbToLinear(threshold_db);
        slope_ = 1.0f - 1.0f / ratio;
        attack_coefficient_ = TimeCoefficient(attack_ms, sample_rate);
        release_coefficient_ = TimeCoefficient(release_ms, sample_rate);
        Reset();
    }

    void Reset() { gain_ = 1.0f; }

    void Process(float& left, float& right)
    {
        const float peak = std::max(std::fabs(left), std::fabs(right));
        float target_gain = 1.0f;
        if(peak > threshold_)
            target_gain = std::pow(threshold_ / peak, slope_);

        const float coefficient = target_gain < gain_ ? attack_coefficient_
                                                       : release_coefficient_;
        gain_ = coefficient * gain_ + (1.0f - coefficient) * target_gain;
        left *= gain_;
        right *= gain_;
    }

  private:
    static float TimeCoefficient(float milliseconds, float sample_rate)
    {
        const float samples = milliseconds * 0.001f * sample_rate;
        return samples > 0.0f ? std::exp(-1.0f / samples) : 0.0f;
    }

    float threshold_ = 0.0f;
    float slope_ = 0.0f;
    float attack_coefficient_ = 0.0f;
    float release_coefficient_ = 0.0f;
    float gain_ = 0.0f;
};

class StereoReverb
{
  public:
    static constexpr size_t kMemorySize = 8192U;

    void Configure(float *memory, float size, float damping, float mix)
    {
        static constexpr uint16_t kMinimumLengths[4] = {
            631U,
            727U,
            809U,
            887U,
        };
        static constexpr uint16_t kMaximumLengths[4] = {
            1499U,
            1601U,
            1867U,
            2053U,
        };
        const float size_ratio = std::max(0.0f, std::min(size, 100.0f)) * 0.01f;
        const float damping_ratio =
            std::max(0.0f, std::min(damping, 100.0f)) * 0.01f;
        memory_ = memory;
        for(uint8_t line = 0U; line < 4U; ++line)
            lengths_[line] = static_cast<uint16_t>(
                kMinimumLengths[line]
                + (kMaximumLengths[line] - kMinimumLengths[line])
                      * size_ratio);
        feedback_ = 0.58f + 0.36f * size_ratio;
        damping_coefficient_ = 0.4f - 0.39f * damping_ratio;
        mix_ = std::max(0.0f, std::min(mix, 100.0f)) * 0.01f;
        Reset();
    }

    void Reset()
    {
        if(memory_ != nullptr)
            std::fill(memory_, memory_ + kMemorySize, 0.0f);
        for(uint8_t line = 0U; line < 4U; ++line)
        {
            indices_[line] = 0U;
            filtered_[line] = 0.0f;
        }
    }

    void Process(float& left, float& right)
    {
        if(memory_ == nullptr)
            return;
        static constexpr uint16_t kOffsets[4] = {
            0U,
            1500U,
            3102U,
            4970U,
        };
        float delayed[4];
        for(uint8_t line = 0U; line < 4U; ++line)
        {
            delayed[line] = memory_[kOffsets[line] + indices_[line]];
            filtered_[line] +=
                damping_coefficient_ * (delayed[line] - filtered_[line]);
        }

        const float feedback[4] = {
            0.5f
                * (filtered_[0] + filtered_[1] + filtered_[2]
                   + filtered_[3]),
            0.5f
                * (filtered_[0] - filtered_[1] + filtered_[2]
                   - filtered_[3]),
            0.5f
                * (filtered_[0] + filtered_[1] - filtered_[2]
                   - filtered_[3]),
            0.5f
                * (filtered_[0] - filtered_[1] - filtered_[2]
                   + filtered_[3]),
        };
        const float input[4] = {
            0.35f * (left + right),
            0.35f * (left - right),
            0.35f * (-left + right),
            -0.35f * (left + right),
        };
        for(uint8_t line = 0U; line < 4U; ++line)
        {
            memory_[kOffsets[line] + indices_[line]] =
                input[line] + feedback_ * feedback[line];
            indices_[line] = static_cast<uint16_t>(indices_[line] + 1U);
            if(indices_[line] >= lengths_[line])
                indices_[line] = 0U;
        }

        const float wet_left = 0.5f * (delayed[0] + delayed[2]);
        const float wet_right = 0.5f * (delayed[1] + delayed[3]);
        const float dry = 1.0f - mix_;
        left = dry * left + mix_ * wet_left;
        right = dry * right + mix_ * wet_right;
    }

  private:
    float *memory_ = nullptr;
    uint16_t lengths_[4] = {};
    uint16_t indices_[4] = {};
    float filtered_[4] = {};
    float feedback_ = 0.0f;
    float damping_coefficient_ = 0.0f;
    float mix_ = 0.0f;
};

class PeakLimiter
{
  public:
    void Configure(float ceiling_db, float release_ms, float sample_rate)
    {
        ceiling_ = DbToLinear(ceiling_db);
        const float release_samples = release_ms * 0.001f * sample_rate;
        release_coefficient_ =
            release_samples > 0.0f ? std::exp(-1.0f / release_samples) : 0.0f;
        Reset();
    }

    void Reset() { gain_ = 1.0f; }

    void Process(float& left, float& right)
    {
        const float peak = std::max(std::fabs(left), std::fabs(right));
        const float required_gain = peak > ceiling_ ? ceiling_ / peak : 1.0f;

        if(required_gain < gain_)
            gain_ = required_gain;
        else
            gain_ = release_coefficient_ * gain_
                    + (1.0f - release_coefficient_);

        left *= gain_;
        right *= gain_;
    }

  private:
    float ceiling_ = 0.0f;
    float release_coefficient_ = 0.0f;
    float gain_ = 0.0f;
};

class StereoProcessor
{
  public:
    void Configure(float gain_db,
                   float limiter_ceiling_db,
                   float limiter_release_ms,
                   float sample_rate)
    {
        Configure(gain_db,
                  1000.0f,
                  0.0f,
                  1.0f,
                  limiter_ceiling_db,
                  limiter_release_ms,
                  sample_rate);
    }

    void Configure(float gain_db,
                   float eq_frequency_hz,
                   float eq_gain_db,
                   float eq_q,
                   float limiter_ceiling_db,
                   float limiter_release_ms,
                   float sample_rate)
    {
        gain_.SetGainDb(gain_db);
        equalizer_.Configure(
            eq_frequency_hz, eq_gain_db, eq_q, sample_rate);
        limiter_.Configure(
            limiter_ceiling_db, limiter_release_ms, sample_rate);
    }

    void Process(float& left, float& right)
    {
        gain_.Process(left, right);
        equalizer_.Process(left, right);
        limiter_.Process(left, right);
    }

  private:
    Gain gain_;
    ParametricEq equalizer_;
    PeakLimiter limiter_;
};
} // namespace dsp
} // namespace linerack
