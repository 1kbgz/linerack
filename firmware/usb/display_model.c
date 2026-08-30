#include "display_model.h"

#include <math.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#define LINERACK_PI 3.14159265358979323846f

static const char font_characters[] = " -.?0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
static const uint8_t font_rows[][7] = {
    {0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 31, 0, 0, 0},
    {0, 0, 0, 0, 0, 6, 6},
    {14, 17, 1, 2, 4, 0, 4},
    {14, 17, 19, 21, 25, 17, 14},
    {4, 12, 4, 4, 4, 4, 14},
    {14, 17, 1, 2, 4, 8, 31},
    {30, 1, 1, 14, 1, 1, 30},
    {2, 6, 10, 18, 31, 2, 2},
    {31, 16, 16, 30, 1, 1, 30},
    {15, 16, 16, 30, 17, 17, 14},
    {31, 1, 2, 4, 8, 8, 8},
    {14, 17, 17, 14, 17, 17, 14},
    {14, 17, 17, 15, 1, 1, 30},
    {14, 17, 17, 31, 17, 17, 17},
    {30, 17, 17, 30, 17, 17, 30},
    {15, 16, 16, 16, 16, 16, 15},
    {30, 17, 17, 17, 17, 17, 30},
    {31, 16, 16, 30, 16, 16, 31},
    {31, 16, 16, 30, 16, 16, 16},
    {15, 16, 16, 23, 17, 17, 15},
    {17, 17, 17, 31, 17, 17, 17},
    {14, 4, 4, 4, 4, 4, 14},
    {1, 1, 1, 1, 17, 17, 14},
    {17, 18, 20, 24, 20, 18, 17},
    {16, 16, 16, 16, 16, 16, 31},
    {17, 27, 21, 21, 17, 17, 17},
    {17, 25, 21, 19, 17, 17, 17},
    {14, 17, 17, 17, 17, 17, 14},
    {30, 17, 17, 30, 16, 16, 16},
    {14, 17, 17, 17, 21, 18, 13},
    {30, 17, 17, 30, 20, 18, 17},
    {15, 16, 16, 14, 1, 1, 30},
    {31, 4, 4, 4, 4, 4, 4},
    {17, 17, 17, 17, 17, 17, 14},
    {17, 17, 17, 17, 17, 10, 4},
    {17, 17, 17, 21, 21, 21, 10},
    {17, 17, 10, 4, 10, 17, 17},
    {17, 17, 10, 4, 4, 4, 4},
    {31, 1, 2, 4, 8, 16, 31},
};

typedef struct
{
    float b0;
    float b1;
    float b2;
    float a1;
    float a2;
} BiquadCoefficients;

static void SetPixel(LineRackDisplayFrame *frame, int x, int y)
{
    if(x < 0 || x >= (int)LINERACK_DISPLAY_WIDTH
       || y < 0 || y >= (int)LINERACK_DISPLAY_HEIGHT)
        return;
    frame->data[(size_t)(y / 8) * LINERACK_DISPLAY_WIDTH + (size_t)x]
        |= (uint8_t)(1U << (uint8_t)(y % 8));
}

static void DrawLine(LineRackDisplayFrame *frame,
                     int start_x,
                     int start_y,
                     int end_x,
                     int end_y)
{
    int x = start_x;
    int y = start_y;
    const int delta_x = abs(end_x - start_x);
    const int delta_y = -abs(end_y - start_y);
    const int step_x = start_x < end_x ? 1 : -1;
    const int step_y = start_y < end_y ? 1 : -1;
    int error = delta_x + delta_y;
    while(true)
    {
        SetPixel(frame, x, y);
        if(x == end_x && y == end_y)
            break;
        const int doubled_error = 2 * error;
        if(doubled_error >= delta_y)
        {
            error += delta_y;
            x += step_x;
        }
        if(doubled_error <= delta_x)
        {
            error += delta_x;
            y += step_y;
        }
    }
}

static const uint8_t *Glyph(char character)
{
    if(character >= 'a' && character <= 'z')
        character = (char)(character - 'a' + 'A');
    const char *position = strchr(font_characters, character);
    return font_rows[position == NULL ? 3U : (size_t)(position - font_characters)];
}

static void DrawText(LineRackDisplayFrame *frame,
                     const char           *text,
                     int                   x,
                     int                   y,
                     int                   scale)
{
    int cursor = x;
    for(const char *character = text; *character != '\0'; ++character)
    {
        const uint8_t *rows = Glyph(*character);
        for(int row = 0; row < 7; ++row)
        {
            for(int column = 0; column < 5; ++column)
            {
                if((rows[row] & (1U << (4 - column))) == 0U)
                    continue;
                for(int scale_y = 0; scale_y < scale; ++scale_y)
                    for(int scale_x = 0; scale_x < scale; ++scale_x)
                        SetPixel(frame,
                                 cursor + column * scale + scale_x,
                                 y + row * scale + scale_y);
            }
        }
        cursor += 6 * scale;
    }
}

static float BiquadMagnitudeDb(float frequency_hz,
                               const BiquadCoefficients *coefficients)
{
    const float radians = 2.0f * LINERACK_PI * frequency_hz / 48000.0f;
    const float cosine_1 = cosf(radians);
    const float sine_1 = sinf(radians);
    const float cosine_2 = cosf(2.0f * radians);
    const float sine_2 = sinf(2.0f * radians);
    const float numerator_real = coefficients->b0
                                 + coefficients->b1 * cosine_1
                                 + coefficients->b2 * cosine_2;
    const float numerator_imaginary = -coefficients->b1 * sine_1
                                      - coefficients->b2 * sine_2;
    const float denominator_real = 1.0f + coefficients->a1 * cosine_1
                                   + coefficients->a2 * cosine_2;
    const float denominator_imaginary = -coefficients->a1 * sine_1
                                        - coefficients->a2 * sine_2;
    const float numerator_power = numerator_real * numerator_real
                                  + numerator_imaginary * numerator_imaginary;
    const float denominator_power = denominator_real * denominator_real
                                    + denominator_imaginary * denominator_imaginary;
    return 10.0f * log10f(numerator_power / denominator_power);
}

static float PeakMagnitudeDb(float frequency_hz,
                             float center_hz,
                             float gain_db,
                             float q)
{
    if(gain_db == 0.0f)
        return 0.0f;
    const float amplitude = powf(10.0f, gain_db / 40.0f);
    const float center_radians = 2.0f * LINERACK_PI * center_hz / 48000.0f;
    const float alpha = sinf(center_radians) / (2.0f * q);
    const float a0 = 1.0f + alpha / amplitude;
    const BiquadCoefficients coefficients = {
        (1.0f + alpha * amplitude) / a0,
        (-2.0f * cosf(center_radians)) / a0,
        (1.0f - alpha * amplitude) / a0,
        (-2.0f * cosf(center_radians)) / a0,
        (1.0f - alpha / amplitude) / a0,
    };
    return BiquadMagnitudeDb(frequency_hz, &coefficients);
}

static float FilterMagnitudeDb(float frequency_hz,
                               float cutoff_hz,
                               float q,
                               bool high_pass)
{
    const float cutoff_radians = 2.0f * LINERACK_PI * cutoff_hz / 48000.0f;
    const float cosine = cosf(cutoff_radians);
    const float alpha = sinf(cutoff_radians) / (2.0f * q);
    const float a0 = 1.0f + alpha;
    const BiquadCoefficients coefficients = {
        ((high_pass ? 1.0f + cosine : 1.0f - cosine) / 2.0f) / a0,
        (high_pass ? -(1.0f + cosine) : 1.0f - cosine) / a0,
        ((high_pass ? 1.0f + cosine : 1.0f - cosine) / 2.0f) / a0,
        (-2.0f * cosine) / a0,
        (1.0f - alpha) / a0,
    };
    return BiquadMagnitudeDb(frequency_hz, &coefficients);
}

static float BlockMagnitudeDb(const LineRackBlock *block, float frequency_hz)
{
    if(block->enabled == 0U)
        return 0.0f;
    if(block->type == LINERACK_BLOCK_PARAMETRIC_EQ)
        return PeakMagnitudeDb(frequency_hz,
                               block->parameters[0],
                               block->parameters[1],
                               block->parameters[2]);
    if(block->type != LINERACK_BLOCK_HIGH_PASS
       && block->type != LINERACK_BLOCK_LOW_PASS)
        return 0.0f;
    const bool high_pass = block->type == LINERACK_BLOCK_HIGH_PASS;
    if(block->parameters[1] == 24.0f)
        return FilterMagnitudeDb(frequency_hz,
                                 block->parameters[0],
                                 0.541196f,
                                 high_pass)
               + FilterMagnitudeDb(frequency_hz,
                                   block->parameters[0],
                                   1.306563f,
                                   high_pass);
    return FilterMagnitudeDb(frequency_hz,
                             block->parameters[0],
                             0.70710678118f,
                             high_pass);
}

static float PresetMagnitudeDb(const LineRackPreset *preset, uint8_t x)
{
    const float position = (float)x / (LINERACK_DISPLAY_WIDTH - 1U);
    const float frequency_hz = 20.0f * powf(1000.0f, position);
    float gain_db = 0.0f;
    for(uint8_t index = 0U; index < preset->block_count; ++index)
        gain_db += BlockMagnitudeDb(&preset->blocks[index], frequency_hz);
    return gain_db;
}

static void RenderPreset(LineRackDisplayFrame *frame,
                         const LineRackPreset *preset)
{
    char number[2] = {(char)('0' + preset->number), '\0'};
    char block_count[10];
    uint8_t enabled_blocks = 0U;
    for(uint8_t index = 0U; index < preset->block_count; ++index)
        if(preset->blocks[index].enabled != 0U)
            ++enabled_blocks;
    size_t block_count_offset = 0U;
    if(enabled_blocks >= 10U)
        block_count[block_count_offset++] = '1';
    block_count[block_count_offset++] = (char)('0' + enabled_blocks % 10U);
    memcpy(&block_count[block_count_offset], " BLOCKS", 8U);
    DrawText(frame, number, 2, 5, 3);
    DrawLine(frame, 22, 2, 22, 29);
    DrawText(frame, preset->name, 28, 3, 1);
    DrawText(frame, block_count, 28, 21, 1);
    DrawText(frame, "LIVE", 100, 21, 1);
}

static void RenderEq(LineRackDisplayFrame *frame,
                     const LineRackPreset *preset)
{
    for(uint16_t x = 0U; x < LINERACK_DISPLAY_WIDTH; x += 2U)
        SetPixel(frame, x, 16);
    int previous_y = 16;
    for(uint16_t x = 0U; x < LINERACK_DISPLAY_WIDTH; ++x)
    {
        float gain_db = PresetMagnitudeDb(preset, (uint8_t)x);
        if(gain_db < -18.0f) gain_db = -18.0f;
        if(gain_db > 18.0f) gain_db = 18.0f;
        gain_db = roundf(gain_db * 4.0f) / 4.0f;
        const int y = (int)lroundf(16.0f - gain_db / 18.0f * 13.0f);
        if(x == 0U)
            SetPixel(frame, x, y);
        else
            DrawLine(frame, x - 1, previous_y, x, y);
        previous_y = y;
    }
    DrawText(frame, "EQ", 2, 2, 1);
    DrawText(frame, "20", 2, 23, 1);
    DrawText(frame, "20K", 108, 23, 1);
}

static void DrawMeter(LineRackDisplayFrame *frame,
                      const char           *label,
                      uint8_t               percent,
                      int                   top)
{
    if(percent > 100U)
        percent = 100U;
    DrawText(frame, label, 2, top + 1, 1);
    DrawLine(frame, 11, top, 126, top);
    DrawLine(frame, 11, top + 9, 126, top + 9);
    DrawLine(frame, 11, top, 11, top + 9);
    DrawLine(frame, 126, top, 126, top + 9);
    const int fill_width = (int)percent * 112 / 100;
    for(int x = 13; x < 13 + fill_width; ++x)
        for(int y = top + 2; y <= top + 7; ++y)
            SetPixel(frame, x, y);
}

static void PercentText(char text[5], uint8_t percent)
{
    if(percent > 100U)
        percent = 100U;
    size_t offset = 0U;
    if(percent == 100U)
        text[offset++] = '1';
    if(percent >= 10U)
        text[offset++] = (char)('0' + percent / 10U % 10U);
    text[offset++] = (char)('0' + percent % 10U);
    text[offset++] = '%';
    text[offset] = '\0';
}

void LineRackDisplayOverrideStart(LineRackDisplayOverride *override,
                                  uint32_t                 now_ms)
{
    if(override == NULL)
        return;
    override->started_at_ms = now_ms;
    override->active = true;
}

LineRackDisplayMode LineRackDisplayResolveMode(LineRackDisplayMode preferred,
                                                LineRackDisplayOverride *override,
                                                uint32_t now_ms)
{
    if(override == NULL || !override->active)
        return preferred;
    if((uint32_t)(now_ms - override->started_at_ms)
       < LINERACK_PRESET_OVERRIDE_MS)
        return LINERACK_DISPLAY_PRESET;
    override->active = false;
    return preferred;
}

void LineRackDisplayPowerInit(LineRackDisplayPower *power, uint32_t now_ms)
{
    if(power == NULL)
        return;
    power->last_activity_ms = now_ms;
    power->awake = true;
}

void LineRackDisplayPowerWake(LineRackDisplayPower *power, uint32_t now_ms)
{
    LineRackDisplayPowerInit(power, now_ms);
}

bool LineRackDisplayPowerResolve(LineRackDisplayPower *power,
                                 bool                  blanking_enabled,
                                 uint32_t              now_ms)
{
    if(power == NULL)
        return false;
    if(!blanking_enabled)
    {
        power->awake = true;
        return true;
    }
    if(power->awake
       && (uint32_t)(now_ms - power->last_activity_ms)
              >= LINERACK_DISPLAY_BLANK_AFTER_MS)
        power->awake = false;
    return power->awake;
}

void LineRackVolumeDisplayStart(LineRackVolumeDisplay *volume,
                                uint8_t                percent,
                                bool                   muted,
                                uint32_t               now_ms)
{
    if(volume == NULL)
        return;
    volume->started_at_ms = now_ms;
    volume->percent = percent > 100U ? 100U : percent;
    volume->muted = muted;
    volume->active = true;
}

bool LineRackVolumeDisplayActive(LineRackVolumeDisplay *volume,
                                 uint32_t               now_ms)
{
    if(volume == NULL || !volume->active)
        return false;
    if((uint32_t)(now_ms - volume->started_at_ms)
       < LINERACK_VOLUME_OVERRIDE_MS)
        return true;
    volume->active = false;
    return false;
}

void LineRackDisplayRender(LineRackDisplayFrame     *frame,
                           const LineRackPresetBank *bank,
                           uint8_t                   active_slot_number,
                           LineRackDisplayMode       mode)
{
    if(frame == NULL)
        return;
    memset(frame, 0, sizeof(*frame));
    if(bank == NULL || active_slot_number < 1U
       || active_slot_number > LINERACK_PRESET_COUNT)
        return;
    const LineRackPreset *preset = &bank->presets[active_slot_number - 1U];
    if(mode == LINERACK_DISPLAY_EQ_RESPONSE)
        RenderEq(frame, preset);
    else if(mode == LINERACK_DISPLAY_VISUALIZER)
        LineRackDisplayRenderVisualizer(frame, 0U, 0U);
    else
        RenderPreset(frame, preset);
}

void LineRackDisplayRenderVisualizer(LineRackDisplayFrame *frame,
                                     uint8_t               left_percent,
                                     uint8_t               right_percent)
{
    if(frame == NULL)
        return;
    memset(frame, 0, sizeof(*frame));
    DrawMeter(frame, "L", left_percent, 3);
    DrawMeter(frame, "R", right_percent, 20);
}

void LineRackDisplayRenderVolume(LineRackDisplayFrame *frame,
                                 uint8_t               percent,
                                 bool                  muted)
{
    if(frame == NULL)
        return;
    memset(frame, 0, sizeof(*frame));
    if(muted)
    {
        DrawText(frame, "MUTE", 40, 8, 2);
        return;
    }
    char text[5];
    PercentText(text, percent);
    DrawText(frame, "VOL", 2, 2, 1);
    DrawText(frame, text, 62, 2, 2);
    DrawLine(frame, 2, 22, 125, 22);
    DrawLine(frame, 2, 30, 125, 30);
    DrawLine(frame, 2, 22, 2, 30);
    DrawLine(frame, 125, 22, 125, 30);
    const int fill_width = (int)(percent > 100U ? 100U : percent) * 120 / 100;
    for(int x = 4; x < 4 + fill_width; ++x)
        for(int y = 24; y <= 28; ++y)
            SetPixel(frame, x, y);
}

bool LineRackDisplayPixel(const LineRackDisplayFrame *frame,
                          uint8_t                     x,
                          uint8_t                     y)
{
    return frame != NULL && x < LINERACK_DISPLAY_WIDTH
           && y < LINERACK_DISPLAY_HEIGHT
           && (frame->data[(size_t)(y / 8U) * LINERACK_DISPLAY_WIDTH + x]
               & (uint8_t)(1U << (y % 8U))) != 0U;
}
