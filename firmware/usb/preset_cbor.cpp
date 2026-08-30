#include "preset_cbor.h"

#include <cmath>
#include <cstring>

namespace
{
class Writer
{
  public:
    Writer(uint8_t *output, size_t capacity)
    : output_(output), capacity_(capacity)
    {
    }

    void Map(size_t count) { Head(5U, count); }
    void Array(size_t count) { Head(4U, count); }
    void Uint(uint64_t value) { Head(0U, value); }
    void Bool(bool value) { Byte(value ? 0xf5U : 0xf4U); }
    void Text(const char *value)
    {
        const size_t length = std::strlen(value);
        Head(3U, length);
        Bytes(reinterpret_cast<const uint8_t *>(value), length);
    }
    void Float(float value)
    {
        uint32_t bits;
        std::memcpy(&bits, &value, sizeof(bits));
        Byte(0xfaU);
        Byte(static_cast<uint8_t>(bits >> 24U));
        Byte(static_cast<uint8_t>(bits >> 16U));
        Byte(static_cast<uint8_t>(bits >> 8U));
        Byte(static_cast<uint8_t>(bits));
    }
    size_t Size() const { return valid_ ? size_ : 0U; }

  private:
    void Head(uint8_t major, uint64_t value)
    {
        const uint8_t prefix = static_cast<uint8_t>(major << 5U);
        if(value < 24U)
            Byte(static_cast<uint8_t>(prefix | value));
        else if(value <= 0xffU)
        {
            Byte(static_cast<uint8_t>(prefix | 24U));
            Byte(static_cast<uint8_t>(value));
        }
        else if(value <= 0xffffU)
        {
            Byte(static_cast<uint8_t>(prefix | 25U));
            Byte(static_cast<uint8_t>(value >> 8U));
            Byte(static_cast<uint8_t>(value));
        }
        else
        {
            Byte(static_cast<uint8_t>(prefix | 26U));
            Byte(static_cast<uint8_t>(value >> 24U));
            Byte(static_cast<uint8_t>(value >> 16U));
            Byte(static_cast<uint8_t>(value >> 8U));
            Byte(static_cast<uint8_t>(value));
        }
    }
    void Byte(uint8_t value)
    {
        if(size_ < capacity_)
            output_[size_] = value;
        else
            valid_ = false;
        ++size_;
    }
    void Bytes(const uint8_t *value, size_t length)
    {
        for(size_t index = 0U; index < length; ++index)
            Byte(value[index]);
    }

    uint8_t *output_;
    size_t   capacity_;
    size_t   size_ = 0U;
    bool     valid_ = true;
};

class Reader
{
  public:
    Reader(const uint8_t *input, size_t size) : input_(input), size_(size) {}

    bool Map(size_t &count) { return Container(5U, count); }
    bool Array(size_t &count) { return Container(4U, count); }
    bool Uint(uint64_t &value)
    {
        uint8_t major;
        return Head(major, value) && major == 0U;
    }
    bool Bool(bool &value)
    {
        uint8_t byte;
        if(!Byte(byte) || (byte != 0xf4U && byte != 0xf5U))
            return false;
        value = byte == 0xf5U;
        return true;
    }
    bool Text(const char *&value, size_t &length)
    {
        uint8_t  major;
        uint64_t encoded_length;
        if(!Head(major, encoded_length) || major != 3U
           || encoded_length > size_ - offset_)
            return false;
        value = reinterpret_cast<const char *>(input_ + offset_);
        length = static_cast<size_t>(encoded_length);
        offset_ += length;
        return true;
    }
    bool Number(float &value)
    {
        if(offset_ >= size_)
            return false;
        if(input_[offset_] == 0xfaU)
        {
            ++offset_;
            uint32_t bits;
            if(!BigEndian32(bits))
                return false;
            std::memcpy(&value, &bits, sizeof(value));
            return std::isfinite(value);
        }
        if(input_[offset_] == 0xfbU)
        {
            ++offset_;
            uint64_t bits = 0U;
            for(uint8_t index = 0U; index < 8U; ++index)
            {
                uint8_t byte;
                if(!Byte(byte))
                    return false;
                bits = (bits << 8U) | byte;
            }
            double decoded;
            std::memcpy(&decoded, &bits, sizeof(decoded));
            value = static_cast<float>(decoded);
            return std::isfinite(value);
        }

        uint8_t  major;
        uint64_t integer;
        if(!Head(major, integer) || (major != 0U && major != 1U))
            return false;
        value = major == 0U ? static_cast<float>(integer)
                            : static_cast<float>(-1.0 - static_cast<double>(integer));
        return true;
    }
    bool Skip()
    {
        return Skip(0U);
    }
    bool Finished() const { return offset_ == size_; }

  private:
    static constexpr uint8_t kMaximumSkipDepth = 8U;

    bool Skip(uint8_t depth)
    {
        if(depth > kMaximumSkipDepth || offset_ >= size_)
            return false;
        const uint8_t initial = input_[offset_];
        if(initial == 0xf4U || initial == 0xf5U || initial == 0xf6U)
        {
            ++offset_;
            return true;
        }
        if(initial == 0xfaU)
            return Advance(5U);
        if(initial == 0xfbU)
            return Advance(9U);

        uint8_t  major;
        uint64_t value;
        if(!Head(major, value))
            return false;
        if(major == 0U || major == 1U)
            return true;
        if(major == 2U || major == 3U)
            return Advance(static_cast<size_t>(value));
        if(major == 4U)
        {
            for(uint64_t index = 0U; index < value; ++index)
                if(!Skip(static_cast<uint8_t>(depth + 1U)))
                    return false;
            return true;
        }
        if(major == 5U)
        {
            for(uint64_t index = 0U; index < value; ++index)
                if(!Skip(static_cast<uint8_t>(depth + 1U))
                   || !Skip(static_cast<uint8_t>(depth + 1U)))
                    return false;
            return true;
        }
        return false;
    }
    bool Container(uint8_t expected_major, size_t &count)
    {
        uint8_t  major;
        uint64_t encoded_count;
        if(!Head(major, encoded_count) || major != expected_major
           || encoded_count > 255U)
            return false;
        count = static_cast<size_t>(encoded_count);
        return true;
    }
    bool Head(uint8_t &major, uint64_t &value)
    {
        uint8_t initial;
        if(!Byte(initial))
            return false;
        major = initial >> 5U;
        const uint8_t additional = initial & 0x1fU;
        if(additional < 24U)
        {
            value = additional;
            return true;
        }
        if(additional == 24U)
        {
            uint8_t byte;
            if(!Byte(byte))
                return false;
            value = byte;
            return true;
        }
        if(additional == 25U)
        {
            uint8_t high;
            uint8_t low;
            if(!Byte(high) || !Byte(low))
                return false;
            value = static_cast<uint16_t>((high << 8U) | low);
            return true;
        }
        if(additional == 26U)
        {
            uint32_t decoded;
            if(!BigEndian32(decoded))
                return false;
            value = decoded;
            return true;
        }
        return false;
    }
    bool BigEndian32(uint32_t &value)
    {
        uint8_t bytes[4];
        for(uint8_t &byte : bytes)
            if(!Byte(byte))
                return false;
        value = (static_cast<uint32_t>(bytes[0]) << 24U)
                | (static_cast<uint32_t>(bytes[1]) << 16U)
                | (static_cast<uint32_t>(bytes[2]) << 8U) | bytes[3];
        return true;
    }
    bool Byte(uint8_t &value)
    {
        if(offset_ >= size_)
            return false;
        value = input_[offset_++];
        return true;
    }
    bool Advance(size_t count)
    {
        if(count > size_ - offset_)
            return false;
        offset_ += count;
        return true;
    }

    const uint8_t *input_;
    size_t         size_;
    size_t         offset_ = 0U;
};

bool KeyEquals(const char *key, size_t length, const char *expected)
{
    return std::strlen(expected) == length
           && std::memcmp(key, expected, length) == 0;
}

bool HalfDb(float value, int8_t &encoded)
{
    const float scaled = value * 2.0f;
    const float rounded = std::round(scaled);
    if(value < -24.0f || value > 0.0f
       || std::fabs(scaled - rounded) > 0.001f)
        return false;
    encoded = static_cast<int8_t>(rounded);
    return true;
}

void Status(Writer &writer,
            uint8_t active_slot,
            const LineRackDeviceDiagnostics *diagnostics)
{
    writer.Map(diagnostics == nullptr ? 2U : 3U);
    writer.Text("connected");
    writer.Bool(true);
    writer.Text("activeSlot");
    writer.Uint(active_slot);
    if(diagnostics != nullptr)
    {
        writer.Text("diagnostics");
        writer.Map(4U);
        writer.Text("usbPackets"); writer.Uint(diagnostics->usb_packets);
        writer.Text("underruns"); writer.Uint(diagnostics->underruns);
        writer.Text("overruns"); writer.Uint(diagnostics->overruns);
        writer.Text("bufferFillFrames"); writer.Uint(diagnostics->buffer_fill_frames);
    }
}

void NumberParameter(Writer     &writer,
                     const char *id,
                     const char *label,
                     float       default_value,
                     float       minimum,
                     float       maximum,
                     float       step,
                     const char *unit)
{
    writer.Map(unit == nullptr ? 7U : 8U);
    writer.Text("id"); writer.Text(id);
    writer.Text("label"); writer.Text(label);
    writer.Text("kind"); writer.Text("number");
    writer.Text("default"); writer.Float(default_value);
    writer.Text("min"); writer.Float(minimum);
    writer.Text("max"); writer.Float(maximum);
    writer.Text("step"); writer.Float(step);
    if(unit != nullptr) { writer.Text("unit"); writer.Text(unit); }
}

void PluginDefinition(Writer     &writer,
                      const char *id,
                      const char *name,
                      const char *description,
                      uint8_t     parameter_count)
{
    writer.Map(5U);
    writer.Text("id"); writer.Text(id);
    writer.Text("name"); writer.Text(name);
    writer.Text("version"); writer.Uint(1U);
    writer.Text("description"); writer.Text(description);
    writer.Text("parameters"); writer.Array(parameter_count);
}

void Capabilities(Writer &writer)
{
    writer.Map(12U);
    writer.Text("product"); writer.Text("LineRack Seed3");
    writer.Text("firmwareVersion"); writer.Text("dev");
    writer.Text("engineVersion"); writer.Text("1");
    writer.Text("slotCount"); writer.Uint(LINERACK_PRESET_COUNT);
    writer.Text("sampleRate"); writer.Uint(48000U);
    writer.Text("channels"); writer.Uint(2U);
    writer.Text("maxPluginsPerSlot"); writer.Uint(LINERACK_MAX_BLOCKS);
    writer.Text("displayBlanking"); writer.Bool(true);
    writer.Text("displayWake"); writer.Bool(true);
    writer.Text("sourceModes"); writer.Array(3U);
    writer.Text("usb"); writer.Text("analog"); writer.Text("mix");
    writer.Text("plugins"); writer.Array(8U);
    PluginDefinition(writer, "gain", "Gain", "Input headroom before the rest of the chain.", 1U);
    NumberParameter(writer, "gainDb", "Gain", 0.0f, -24.0f, 12.0f, 0.5f, "dB");
    PluginDefinition(writer, "parametric-eq", "Parametric EQ", "One fully parametric bell filter. Add more instances for more bands.", 3U);
    NumberParameter(writer, "frequencyHz", "Frequency", 1000.0f, 20.0f, 20000.0f, 1.0f, "Hz");
    NumberParameter(writer, "gainDb", "Gain", 0.0f, -18.0f, 18.0f, 0.5f, "dB");
    NumberParameter(writer, "q", "Q", 1.0f, 0.1f, 18.0f, 0.01f, nullptr);
    PluginDefinition(writer, "high-pass", "High-pass Filter", "Remove low frequencies below the selected cutoff.", 2U);
    NumberParameter(writer, "cutoffHz", "Cutoff", 80.0f, 20.0f, 20000.0f, 1.0f, "Hz");
    NumberParameter(writer, "slopeDbPerOct", "Slope", 12.0f, 12.0f, 24.0f, 12.0f, "dB/oct");
    PluginDefinition(writer, "low-pass", "Low-pass Filter", "Remove high frequencies above the selected cutoff.", 2U);
    NumberParameter(writer, "cutoffHz", "Cutoff", 18000.0f, 20.0f, 20000.0f, 1.0f, "Hz");
    NumberParameter(writer, "slopeDbPerOct", "Slope", 12.0f, 12.0f, 24.0f, 12.0f, "dB/oct");
    PluginDefinition(writer, "noise-gate", "Noise Gate", "Attenuate low-level noise when the signal falls below threshold.", 5U);
    NumberParameter(writer, "thresholdDb", "Threshold", -50.0f, -80.0f, 0.0f, 1.0f, "dB");
    NumberParameter(writer, "attackMs", "Attack", 2.0f, 0.1f, 100.0f, 0.1f, "ms");
    NumberParameter(writer, "holdMs", "Hold", 50.0f, 0.0f, 500.0f, 1.0f, "ms");
    NumberParameter(writer, "releaseMs", "Release", 150.0f, 5.0f, 2000.0f, 1.0f, "ms");
    NumberParameter(writer, "rangeDb", "Range", -60.0f, -80.0f, 0.0f, 1.0f, "dB");
    PluginDefinition(writer, "compressor", "Compressor", "Reduce dynamic range above a threshold.", 4U);
    NumberParameter(writer, "thresholdDb", "Threshold", -18.0f, -60.0f, 0.0f, 1.0f, "dB");
    NumberParameter(writer, "ratio", "Ratio", 3.0f, 1.0f, 20.0f, 0.5f, ":1");
    NumberParameter(writer, "attackMs", "Attack", 10.0f, 0.1f, 200.0f, 0.1f, "ms");
    NumberParameter(writer, "releaseMs", "Release", 100.0f, 10.0f, 2000.0f, 1.0f, "ms");
    PluginDefinition(writer, "reverb", "Reverb", "Compact stereo feedback-delay ambience.", 3U);
    NumberParameter(writer, "size", "Size", 35.0f, 0.0f, 100.0f, 1.0f, "%");
    NumberParameter(writer, "damping", "Damping", 50.0f, 0.0f, 100.0f, 1.0f, "%");
    NumberParameter(writer, "mix", "Mix", 10.0f, 0.0f, 100.0f, 1.0f, "%");
    PluginDefinition(writer, "limiter", "Limiter", "Final safety ceiling for the headphone output.", 1U);
    NumberParameter(writer, "ceilingDb", "Ceiling", -1.0f, -12.0f, 0.0f, 0.5f, "dB");
    writer.Text("presetNameMaxLength"); writer.Uint(LINERACK_PRESET_NAME_BYTES - 1U);
}

void Parameters(Writer &writer, const LineRackBlock &block)
{
    switch(block.type)
    {
        case LINERACK_BLOCK_GAIN:
            writer.Map(1U);
            writer.Text("gainDb"); writer.Float(block.parameters[0]);
            break;
        case LINERACK_BLOCK_PARAMETRIC_EQ:
            writer.Map(3U);
            writer.Text("frequencyHz"); writer.Float(block.parameters[0]);
            writer.Text("gainDb"); writer.Float(block.parameters[1]);
            writer.Text("q"); writer.Float(block.parameters[2]);
            break;
        case LINERACK_BLOCK_HIGH_PASS:
        case LINERACK_BLOCK_LOW_PASS:
            writer.Map(2U);
            writer.Text("cutoffHz"); writer.Float(block.parameters[0]);
            writer.Text("slopeDbPerOct"); writer.Float(block.parameters[1]);
            break;
        case LINERACK_BLOCK_NOISE_GATE:
            writer.Map(5U);
            writer.Text("thresholdDb"); writer.Float(block.parameters[0]);
            writer.Text("attackMs"); writer.Float(block.parameters[1]);
            writer.Text("holdMs"); writer.Float(block.parameters[2]);
            writer.Text("releaseMs"); writer.Float(block.parameters[3]);
            writer.Text("rangeDb"); writer.Float(block.parameters[4]);
            break;
        case LINERACK_BLOCK_COMPRESSOR:
            writer.Map(4U);
            writer.Text("thresholdDb"); writer.Float(block.parameters[0]);
            writer.Text("ratio"); writer.Float(block.parameters[1]);
            writer.Text("attackMs"); writer.Float(block.parameters[2]);
            writer.Text("releaseMs"); writer.Float(block.parameters[3]);
            break;
        case LINERACK_BLOCK_REVERB:
            writer.Map(3U);
            writer.Text("size"); writer.Float(block.parameters[0]);
            writer.Text("damping"); writer.Float(block.parameters[1]);
            writer.Text("mix"); writer.Float(block.parameters[2]);
            break;
        case LINERACK_BLOCK_LIMITER:
            writer.Map(1U);
            writer.Text("ceilingDb"); writer.Float(block.parameters[0]);
            break;
        default: writer.Map(0U); break;
    }
}

const char *PluginId(uint8_t type)
{
    switch(type)
    {
        case LINERACK_BLOCK_GAIN: return "gain";
        case LINERACK_BLOCK_PARAMETRIC_EQ: return "parametric-eq";
        case LINERACK_BLOCK_LIMITER: return "limiter";
        case LINERACK_BLOCK_HIGH_PASS: return "high-pass";
        case LINERACK_BLOCK_LOW_PASS: return "low-pass";
        case LINERACK_BLOCK_NOISE_GATE: return "noise-gate";
        case LINERACK_BLOCK_COMPRESSOR: return "compressor";
        case LINERACK_BLOCK_REVERB: return "reverb";
        default: return "";
    }
}

enum ParameterSlot
{
    kGainDb,
    kFrequencyHz,
    kQ,
    kCeilingDb,
    kCutoffHz,
    kSlopeDbPerOct,
    kThresholdDb,
    kAttackMs,
    kHoldMs,
    kReleaseMs,
    kRangeDb,
    kRatio,
    kSize,
    kDamping,
    kMix,
    kParameterCount,
};

constexpr uint16_t ParameterBit(ParameterSlot slot)
{
    return static_cast<uint16_t>(1U << static_cast<uint8_t>(slot));
}

struct ParsedPlugin
{
    char  plugin_id[20] = {};
    bool  enabled = false;
    bool  enabled_seen = false;
    float parameter_values[kParameterCount] = {};
    uint16_t parameter_mask = 0U;
};

bool CopyText(Reader &reader, char *destination, size_t capacity)
{
    const char *value;
    size_t      length;
    if(!reader.Text(value, length) || length == 0U || length >= capacity)
        return false;
    std::memcpy(destination, value, length);
    destination[length] = '\0';
    return true;
}

int FindParameter(const char *key, size_t length)
{
    if(KeyEquals(key, length, "gainDb")) return kGainDb;
    if(KeyEquals(key, length, "frequencyHz")) return kFrequencyHz;
    if(KeyEquals(key, length, "q")) return kQ;
    if(KeyEquals(key, length, "ceilingDb")) return kCeilingDb;
    if(KeyEquals(key, length, "cutoffHz")) return kCutoffHz;
    if(KeyEquals(key, length, "slopeDbPerOct")) return kSlopeDbPerOct;
    if(KeyEquals(key, length, "thresholdDb")) return kThresholdDb;
    if(KeyEquals(key, length, "attackMs")) return kAttackMs;
    if(KeyEquals(key, length, "holdMs")) return kHoldMs;
    if(KeyEquals(key, length, "releaseMs")) return kReleaseMs;
    if(KeyEquals(key, length, "rangeDb")) return kRangeDb;
    if(KeyEquals(key, length, "ratio")) return kRatio;
    if(KeyEquals(key, length, "size")) return kSize;
    if(KeyEquals(key, length, "damping")) return kDamping;
    if(KeyEquals(key, length, "mix")) return kMix;
    return -1;
}

bool DecodeParameters(Reader &reader, ParsedPlugin &plugin)
{
    size_t count;
    if(!reader.Map(count)) return false;
    for(size_t index = 0U; index < count; ++index)
    {
        const char *key;
        size_t      length;
        if(!reader.Text(key, length)) return false;
        const int parameter = FindParameter(key, length);
        const uint16_t bit = parameter < 0
                                 ? 0U
                                 : ParameterBit(static_cast<ParameterSlot>(parameter));
        if(parameter < 0 || (plugin.parameter_mask & bit) != 0U
           || !reader.Number(plugin.parameter_values[parameter]))
            return false;
        plugin.parameter_mask |= bit;
    }
    return true;
}

bool DecodePlugin(Reader &reader, LineRackBlock &block)
{
    size_t       count;
    ParsedPlugin plugin;
    bool         id_seen = false;
    bool         version_seen = false;
    bool         parameters_seen = false;
    if(!reader.Map(count)) return false;
    for(size_t index = 0U; index < count; ++index)
    {
        const char *key;
        size_t      length;
        if(!reader.Text(key, length)) return false;
        if(KeyEquals(key, length, "pluginId"))
        {
            if(id_seen || !CopyText(reader, plugin.plugin_id, sizeof(plugin.plugin_id))) return false;
            id_seen = true;
        }
        else if(KeyEquals(key, length, "pluginVersion"))
        {
            uint64_t version;
            if(version_seen || !reader.Uint(version) || version != 1U) return false;
            version_seen = true;
        }
        else if(KeyEquals(key, length, "enabled"))
        {
            if(plugin.enabled_seen || !reader.Bool(plugin.enabled)) return false;
            plugin.enabled_seen = true;
        }
        else if(KeyEquals(key, length, "parameters"))
        {
            if(parameters_seen || !DecodeParameters(reader, plugin)) return false;
            parameters_seen = true;
        }
        else if(!reader.Skip()) return false;
    }
    if(!id_seen || !version_seen || !plugin.enabled_seen || !parameters_seen) return false;
    std::memset(&block, 0, sizeof(block));
    if(std::strcmp(plugin.plugin_id, "gain") == 0)
    {
        if(plugin.parameter_mask != ParameterBit(kGainDb)) return false;
        block.type = LINERACK_BLOCK_GAIN;
        block.parameters[0] = plugin.parameter_values[kGainDb];
    }
    else if(std::strcmp(plugin.plugin_id, "parametric-eq") == 0)
    {
        if(plugin.parameter_mask != (ParameterBit(kGainDb)
                                     | ParameterBit(kFrequencyHz)
                                     | ParameterBit(kQ))) return false;
        block.type = LINERACK_BLOCK_PARAMETRIC_EQ;
        block.parameters[0] = plugin.parameter_values[kFrequencyHz];
        block.parameters[1] = plugin.parameter_values[kGainDb];
        block.parameters[2] = plugin.parameter_values[kQ];
    }
    else if(std::strcmp(plugin.plugin_id, "limiter") == 0)
    {
        if(plugin.parameter_mask != ParameterBit(kCeilingDb)) return false;
        block.type = LINERACK_BLOCK_LIMITER;
        block.parameters[0] = plugin.parameter_values[kCeilingDb];
    }
    else if(std::strcmp(plugin.plugin_id, "high-pass") == 0
            || std::strcmp(plugin.plugin_id, "low-pass") == 0)
    {
        if(plugin.parameter_mask != (ParameterBit(kCutoffHz)
                                     | ParameterBit(kSlopeDbPerOct))) return false;
        block.type = std::strcmp(plugin.plugin_id, "high-pass") == 0
                         ? LINERACK_BLOCK_HIGH_PASS
                         : LINERACK_BLOCK_LOW_PASS;
        block.parameters[0] = plugin.parameter_values[kCutoffHz];
        block.parameters[1] = plugin.parameter_values[kSlopeDbPerOct];
    }
    else if(std::strcmp(plugin.plugin_id, "noise-gate") == 0)
    {
        if(plugin.parameter_mask != (ParameterBit(kThresholdDb)
                                     | ParameterBit(kAttackMs)
                                     | ParameterBit(kHoldMs)
                                     | ParameterBit(kReleaseMs)
                                     | ParameterBit(kRangeDb))) return false;
        block.type = LINERACK_BLOCK_NOISE_GATE;
        block.parameters[0] = plugin.parameter_values[kThresholdDb];
        block.parameters[1] = plugin.parameter_values[kAttackMs];
        block.parameters[2] = plugin.parameter_values[kHoldMs];
        block.parameters[3] = plugin.parameter_values[kReleaseMs];
        block.parameters[4] = plugin.parameter_values[kRangeDb];
    }
    else if(std::strcmp(plugin.plugin_id, "compressor") == 0)
    {
        if(plugin.parameter_mask != (ParameterBit(kThresholdDb)
                                     | ParameterBit(kRatio)
                                     | ParameterBit(kAttackMs)
                                     | ParameterBit(kReleaseMs))) return false;
        block.type = LINERACK_BLOCK_COMPRESSOR;
        block.parameters[0] = plugin.parameter_values[kThresholdDb];
        block.parameters[1] = plugin.parameter_values[kRatio];
        block.parameters[2] = plugin.parameter_values[kAttackMs];
        block.parameters[3] = plugin.parameter_values[kReleaseMs];
    }
    else if(std::strcmp(plugin.plugin_id, "reverb") == 0)
    {
        if(plugin.parameter_mask != (ParameterBit(kSize)
                                     | ParameterBit(kDamping)
                                     | ParameterBit(kMix))) return false;
        block.type = LINERACK_BLOCK_REVERB;
        block.parameters[0] = plugin.parameter_values[kSize];
        block.parameters[1] = plugin.parameter_values[kDamping];
        block.parameters[2] = plugin.parameter_values[kMix];
    }
    else return false;
    block.enabled = plugin.enabled ? 1U : 0U;
    return true;
}

bool DecodeSlot(Reader &reader, LineRackPreset &preset)
{
    size_t count;
    bool   number_seen = false;
    bool   name_seen = false;
    bool   plugins_seen = false;
    if(!reader.Map(count)) return false;
    for(size_t index = 0U; index < count; ++index)
    {
        const char *key;
        size_t      length;
        if(!reader.Text(key, length)) return false;
        if(KeyEquals(key, length, "number"))
        {
            uint64_t number;
            if(number_seen || !reader.Uint(number) || number > 255U) return false;
            preset.number = static_cast<uint8_t>(number);
            number_seen = true;
        }
        else if(KeyEquals(key, length, "name"))
        {
            if(name_seen || !CopyText(reader, preset.name, sizeof(preset.name))) return false;
            name_seen = true;
        }
        else if(KeyEquals(key, length, "plugins"))
        {
            size_t plugin_count;
            if(plugins_seen || !reader.Array(plugin_count) || plugin_count > LINERACK_MAX_BLOCKS) return false;
            preset.block_count = static_cast<uint8_t>(plugin_count);
            for(size_t plugin = 0U; plugin < plugin_count; ++plugin)
                if(!DecodePlugin(reader, preset.blocks[plugin])) return false;
            plugins_seen = true;
        }
        else if(!reader.Skip()) return false;
    }
    return number_seen && name_seen && plugins_seen;
}
} // namespace

size_t LineRackCborEncodeHello(uint8_t *output,
                               size_t capacity,
                               uint8_t active_slot,
                               const LineRackDeviceDiagnostics *diagnostics)
{
    Writer writer(output, capacity);
    writer.Map(2U);
    writer.Text("capabilities"); Capabilities(writer);
    writer.Text("status"); Status(writer, active_slot, diagnostics);
    return writer.Size();
}

size_t LineRackCborEncodePresets(uint8_t *output, size_t capacity, const LineRackPresetBank *bank)
{
    if(!LineRackPresetBankValid(bank)) return 0U;
    Writer writer(output, capacity);
    writer.Map(6U);
    writer.Text("format"); writer.Text("linerack-presets");
    writer.Text("schemaVersion"); writer.Uint(1U);
    writer.Text("engine"); writer.Map(2U);
    writer.Text("sampleRate"); writer.Uint(48000U);
    writer.Text("channels"); writer.Uint(2U);
    writer.Text("display"); writer.Map(2U);
    writer.Text("defaultMode");
    const uint8_t display_mode =
        bank->display_mode & LINERACK_DISPLAY_MODE_MASK;
    writer.Text(display_mode == LINERACK_DISPLAY_EQ_RESPONSE
                    ? "eq-response"
                    : display_mode == LINERACK_DISPLAY_VISUALIZER
                          ? "visualizer"
                          : "preset");
    writer.Text("blankingEnabled");
    writer.Bool((bank->display_mode & LINERACK_DISPLAY_BLANKING_DISABLED) == 0U);
    writer.Text("routing"); writer.Map(3U);
    writer.Text("sourceMode");
    writer.Text(bank->source_mode == LINERACK_SOURCE_ANALOG
                    ? "analog"
                    : bank->source_mode == LINERACK_SOURCE_MIX ? "mix" : "usb");
    writer.Text("usbTrimDb"); writer.Float(bank->usb_trim_half_db * 0.5f);
    writer.Text("analogTrimDb"); writer.Float(bank->analog_trim_half_db * 0.5f);
    writer.Text("slots"); writer.Array(LINERACK_PRESET_COUNT);
    for(uint8_t slot_index = 0U; slot_index < LINERACK_PRESET_COUNT; ++slot_index)
    {
        const LineRackPreset &preset = bank->presets[slot_index];
        writer.Map(3U);
        writer.Text("number"); writer.Uint(preset.number);
        writer.Text("name"); writer.Text(preset.name);
        writer.Text("plugins"); writer.Array(preset.block_count);
        for(uint8_t block_index = 0U; block_index < preset.block_count; ++block_index)
        {
            const LineRackBlock &block = preset.blocks[block_index];
            char instance_id[16];
            instance_id[0] = 's'; instance_id[1] = static_cast<char>('0' + preset.number);
            instance_id[2] = '-';
            if(block_index == 9U)
            {
                instance_id[3] = '1'; instance_id[4] = '0'; instance_id[5] = '\0';
            }
            else
            {
                instance_id[3] = static_cast<char>('0' + block_index + 1U);
                instance_id[4] = '\0';
            }
            writer.Map(5U);
            writer.Text("id"); writer.Text(instance_id);
            writer.Text("pluginId"); writer.Text(PluginId(block.type));
            writer.Text("pluginVersion"); writer.Uint(1U);
            writer.Text("enabled"); writer.Bool(block.enabled != 0U);
            writer.Text("parameters"); Parameters(writer, block);
        }
    }
    return writer.Size();
}

size_t LineRackCborEncodeStatus(uint8_t *output,
                                size_t capacity,
                                uint8_t active_slot,
                                const LineRackDeviceDiagnostics *diagnostics)
{
    Writer writer(output, capacity);
    Status(writer, active_slot, diagnostics);
    return writer.Size();
}

size_t LineRackCborEncodeError(uint8_t *output, size_t capacity, const char *message)
{
    Writer writer(output, capacity);
    writer.Map(1U);
    writer.Text("message"); writer.Text(message);
    return writer.Size();
}

bool LineRackCborDecodePresets(const uint8_t *input, size_t size, LineRackPresetBank *bank)
{
    if(input == nullptr || bank == nullptr) return false;
    Reader reader(input, size);
    LineRackPresetBank decoded = {};
    decoded.schema_version = 1U;
    size_t count;
    bool format_seen = false;
    bool schema_seen = false;
    bool engine_seen = false;
    bool display_seen = false;
    bool routing_seen = false;
    bool slots_seen = false;
    if(!reader.Map(count)) return false;
    for(size_t index = 0U; index < count; ++index)
    {
        const char *key;
        size_t      length;
        if(!reader.Text(key, length)) return false;
        if(KeyEquals(key, length, "format"))
        {
            const char *format;
            size_t format_length;
            if(format_seen || !reader.Text(format, format_length)
               || !KeyEquals(format, format_length, "linerack-presets")) return false;
            format_seen = true;
        }
        else if(KeyEquals(key, length, "schemaVersion"))
        {
            uint64_t schema;
            if(schema_seen || !reader.Uint(schema) || schema != 1U) return false;
            schema_seen = true;
        }
        else if(KeyEquals(key, length, "engine"))
        {
            size_t engine_count;
            bool sample_rate_seen = false;
            bool channels_seen = false;
            if(engine_seen || !reader.Map(engine_count)) return false;
            for(size_t engine_index = 0U; engine_index < engine_count; ++engine_index)
            {
                const char *engine_key;
                size_t engine_key_length;
                uint64_t value;
                if(!reader.Text(engine_key, engine_key_length) || !reader.Uint(value)) return false;
                if(KeyEquals(engine_key, engine_key_length, "sampleRate")) sample_rate_seen = value == 48000U;
                else if(KeyEquals(engine_key, engine_key_length, "channels")) channels_seen = value == 2U;
                else return false;
            }
            if(!sample_rate_seen || !channels_seen) return false;
            engine_seen = true;
        }
        else if(KeyEquals(key, length, "slots"))
        {
            size_t slot_count;
            if(slots_seen || !reader.Array(slot_count) || slot_count != LINERACK_PRESET_COUNT) return false;
            for(size_t slot = 0U; slot < slot_count; ++slot)
                if(!DecodeSlot(reader, decoded.presets[slot])) return false;
            slots_seen = true;
        }
        else if(KeyEquals(key, length, "display"))
        {
            size_t display_count;
            bool default_mode_seen = false;
            bool blanking_seen = false;
            if(display_seen || !reader.Map(display_count)) return false;
            for(size_t display_index = 0U;
                display_index < display_count;
                ++display_index)
            {
                const char *display_key;
                size_t display_key_length;
                if(!reader.Text(display_key, display_key_length)) return false;
                if(KeyEquals(display_key, display_key_length, "defaultMode"))
                {
                    const char *mode;
                    size_t mode_length;
                    if(default_mode_seen || !reader.Text(mode, mode_length)) return false;
                    if(KeyEquals(mode, mode_length, "preset"))
                        decoded.display_mode =
                            (decoded.display_mode
                             & LINERACK_DISPLAY_BLANKING_DISABLED)
                            | LINERACK_DISPLAY_PRESET;
                    else if(KeyEquals(mode, mode_length, "eq-response"))
                        decoded.display_mode =
                            (decoded.display_mode
                             & LINERACK_DISPLAY_BLANKING_DISABLED)
                            | LINERACK_DISPLAY_EQ_RESPONSE;
                    else if(KeyEquals(mode, mode_length, "visualizer"))
                        decoded.display_mode =
                            (decoded.display_mode
                             & LINERACK_DISPLAY_BLANKING_DISABLED)
                            | LINERACK_DISPLAY_VISUALIZER;
                    else return false;
                    default_mode_seen = true;
                }
                else if(KeyEquals(display_key,
                                  display_key_length,
                                  "blankingEnabled"))
                {
                    bool enabled;
                    if(blanking_seen || !reader.Bool(enabled)) return false;
                    if(enabled)
                        decoded.display_mode &=
                            (uint8_t)~LINERACK_DISPLAY_BLANKING_DISABLED;
                    else
                        decoded.display_mode |=
                            LINERACK_DISPLAY_BLANKING_DISABLED;
                    blanking_seen = true;
                }
                else if(!reader.Skip()) return false;
            }
            if(!default_mode_seen) return false;
            display_seen = true;
        }
        else if(KeyEquals(key, length, "routing"))
        {
            size_t routing_count;
            bool source_mode_seen = false;
            bool usb_trim_seen = false;
            bool analog_trim_seen = false;
            if(routing_seen || !reader.Map(routing_count)) return false;
            for(size_t routing_index = 0U;
                routing_index < routing_count;
                ++routing_index)
            {
                const char *routing_key;
                size_t routing_key_length;
                if(!reader.Text(routing_key, routing_key_length)) return false;
                if(KeyEquals(routing_key, routing_key_length, "sourceMode"))
                {
                    const char *mode;
                    size_t mode_length;
                    if(source_mode_seen || !reader.Text(mode, mode_length)) return false;
                    if(KeyEquals(mode, mode_length, "usb"))
                        decoded.source_mode = LINERACK_SOURCE_USB;
                    else if(KeyEquals(mode, mode_length, "analog"))
                        decoded.source_mode = LINERACK_SOURCE_ANALOG;
                    else if(KeyEquals(mode, mode_length, "mix"))
                        decoded.source_mode = LINERACK_SOURCE_MIX;
                    else return false;
                    source_mode_seen = true;
                }
                else if(KeyEquals(routing_key, routing_key_length, "usbTrimDb"))
                {
                    float value;
                    if(usb_trim_seen || !reader.Number(value)
                       || !HalfDb(value, decoded.usb_trim_half_db)) return false;
                    usb_trim_seen = true;
                }
                else if(KeyEquals(routing_key, routing_key_length, "analogTrimDb"))
                {
                    float value;
                    if(analog_trim_seen || !reader.Number(value)
                       || !HalfDb(value, decoded.analog_trim_half_db)) return false;
                    analog_trim_seen = true;
                }
                else if(!reader.Skip()) return false;
            }
            if(!source_mode_seen || !usb_trim_seen || !analog_trim_seen) return false;
            routing_seen = true;
        }
        else if(!reader.Skip()) return false;
    }
    if(!format_seen || !schema_seen || !engine_seen || !slots_seen
       || !reader.Finished() || !LineRackPresetBankValid(&decoded)) return false;
    *bank = decoded;
    return true;
}

bool LineRackCborDecodeSlotNumber(const uint8_t *input, size_t size, uint8_t *slot_number)
{
    if(input == nullptr || slot_number == nullptr) return false;
    Reader reader(input, size);
    size_t count;
    bool seen = false;
    if(!reader.Map(count)) return false;
    for(size_t index = 0U; index < count; ++index)
    {
        const char *key;
        size_t length;
        if(!reader.Text(key, length)) return false;
        if(KeyEquals(key, length, "slotNumber"))
        {
            uint64_t number;
            if(seen || !reader.Uint(number) || number < 1U || number > LINERACK_PRESET_COUNT) return false;
            *slot_number = static_cast<uint8_t>(number);
            seen = true;
        }
        else if(!reader.Skip()) return false;
    }
    return seen && reader.Finished();
}
