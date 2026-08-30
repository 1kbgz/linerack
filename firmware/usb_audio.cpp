#include <atomic>
#include <cmath>
#include <cstring>

#include "daisy_seed.h"
#include "dsp/processor.h"

extern "C"
{
#include "audio_device.h"
#include "usb/audio_router.h"
#include "usb/button_gesture.h"
#include "usb/display_model.h"
#include "usb/oled_ssd1306.h"
#ifdef USE_USBD_COMPOSITE
#include "usb/hid_rpc.h"
#endif
#include "usb/preset_model.h"
#include "usb/preset_storage.h"
}

using namespace daisy;

namespace
{
constexpr float kLimiterReleaseMs = 100.0f;
constexpr uint32_t kPresetSectorA = 0x7fe000U;
constexpr uint32_t kPresetSectorB = 0x7ff000U;
constexpr uint16_t kOledAddress = 0x3cU;

class ChainProcessor
{
  public:
    void Configure(
        const LineRackPreset &preset,
        float                 sample_rate,
        float reverb_memory[LINERACK_MAX_BLOCKS]
                           [linerack::dsp::StereoReverb::kMemorySize])
    {
        block_count_ = preset.block_count;
        for(uint8_t index = 0U; index < block_count_; ++index)
        {
            const LineRackBlock &block = preset.blocks[index];
            types_[index] = block.type;
            enabled_[index] = block.enabled != 0U;
            if(block.type == LINERACK_BLOCK_GAIN)
                gains_[index].SetGainDb(block.parameters[0]);
            else if(block.type == LINERACK_BLOCK_PARAMETRIC_EQ)
                equalizers_[index].Configure(block.parameters[0],
                                             block.parameters[1],
                                             block.parameters[2],
                                             sample_rate);
            else if(block.type == LINERACK_BLOCK_HIGH_PASS)
                filters_[index].Configure(
                    linerack::dsp::PassFilterMode::HighPass,
                    block.parameters[0],
                    block.parameters[1],
                    sample_rate);
            else if(block.type == LINERACK_BLOCK_LOW_PASS)
                filters_[index].Configure(
                    linerack::dsp::PassFilterMode::LowPass,
                    block.parameters[0],
                    block.parameters[1],
                    sample_rate);
            else if(block.type == LINERACK_BLOCK_NOISE_GATE)
                gates_[index].Configure(block.parameters[0],
                                        block.parameters[1],
                                        block.parameters[2],
                                        block.parameters[3],
                                        block.parameters[4],
                                        sample_rate);
            else if(block.type == LINERACK_BLOCK_COMPRESSOR)
                compressors_[index].Configure(block.parameters[0],
                                               block.parameters[1],
                                               block.parameters[2],
                                               block.parameters[3],
                                               sample_rate);
            else if(block.type == LINERACK_BLOCK_REVERB)
                reverbs_[index].Configure(reverb_memory[index],
                                          block.parameters[0],
                                          block.parameters[1],
                                          block.parameters[2]);
            else if(block.type == LINERACK_BLOCK_LIMITER)
                limiters_[index].Configure(
                    block.parameters[0], kLimiterReleaseMs, sample_rate);
        }
    }

    void Process(float &left, float &right)
    {
        for(uint8_t index = 0U; index < block_count_; ++index)
        {
            if(!enabled_[index]) continue;
            if(types_[index] == LINERACK_BLOCK_GAIN)
                gains_[index].Process(left, right);
            else if(types_[index] == LINERACK_BLOCK_PARAMETRIC_EQ)
                equalizers_[index].Process(left, right);
            else if(types_[index] == LINERACK_BLOCK_HIGH_PASS
                    || types_[index] == LINERACK_BLOCK_LOW_PASS)
                filters_[index].Process(left, right);
            else if(types_[index] == LINERACK_BLOCK_NOISE_GATE)
                gates_[index].Process(left, right);
            else if(types_[index] == LINERACK_BLOCK_COMPRESSOR)
                compressors_[index].Process(left, right);
            else if(types_[index] == LINERACK_BLOCK_REVERB)
                reverbs_[index].Process(left, right);
            else if(types_[index] == LINERACK_BLOCK_LIMITER)
                limiters_[index].Process(left, right);
        }
    }

  private:
    uint8_t block_count_ = 0U;
    uint8_t types_[LINERACK_MAX_BLOCKS] = {};
    bool enabled_[LINERACK_MAX_BLOCKS] = {};
    linerack::dsp::Gain gains_[LINERACK_MAX_BLOCKS];
    linerack::dsp::ParametricEq equalizers_[LINERACK_MAX_BLOCKS];
    linerack::dsp::PassFilter filters_[LINERACK_MAX_BLOCKS];
    linerack::dsp::NoiseGate gates_[LINERACK_MAX_BLOCKS];
    linerack::dsp::StereoCompressor compressors_[LINERACK_MAX_BLOCKS];
    linerack::dsp::StereoReverb reverbs_[LINERACK_MAX_BLOCKS];
    linerack::dsp::PeakLimiter limiters_[LINERACK_MAX_BLOCKS];
};

DaisySeed hardware;
Switch preset_switch;
LineRackPresetBank preset_bank;
ChainProcessor processors[2];
DSY_SDRAM_BSS float
    reverb_memory[2][LINERACK_MAX_BLOCKS]
                 [linerack::dsp::StereoReverb::kMemorySize];
LineRackAudioRouter audio_routers[2];
std::atomic<uint8_t> active_processor_bank{0};
std::atomic<uint8_t> active_slot_index{0};
LineRackPresetStorage preset_storage;
LineRackDisplayFrame display_frame;
LineRackDisplayMode rendered_display_mode = LINERACK_DISPLAY_PRESET;
LineRackDisplayOverride preset_display_override = {};
LineRackDisplayPower display_power = {};
LineRackVolumeDisplay volume_display = {};
LineRackButtonGesture button_gesture = {};
bool display_dirty = true;
bool rendered_volume_display = false;
uint32_t last_visualizer_refresh_ms = 0U;
std::atomic<uint16_t> visualizer_left_peak{0U};
std::atomic<uint16_t> visualizer_right_peak{0U};
I2CHandle display_i2c;
bool display_ready = false;

bool OledWrite(void *context, const uint8_t *data, size_t size)
{
    I2CHandle *i2c = static_cast<I2CHandle *>(context);
    return size <= UINT16_MAX
           && i2c->TransmitBlocking(kOledAddress,
                                    const_cast<uint8_t *>(data),
                                    static_cast<uint16_t>(size),
                                    10U) == I2CHandle::Result::OK;
}

const LineRackOledTransport oled_transport = {&display_i2c, OledWrite};

bool DisplayBlankingEnabled()
{
    return (preset_bank.display_mode
            & LINERACK_DISPLAY_BLANKING_DISABLED) == 0U;
}

LineRackDisplayMode PreferredDisplayMode()
{
    return static_cast<LineRackDisplayMode>(
        preset_bank.display_mode & LINERACK_DISPLAY_MODE_MASK);
}

bool InitializeDisplay()
{
    I2CHandle::Config config;
    config.periph = I2CHandle::Config::Peripheral::I2C_1;
    config.speed = I2CHandle::Config::Speed::I2C_400KHZ;
    config.mode = I2CHandle::Config::Mode::I2C_MASTER;
    config.pin_config.scl = seed::D11;
    config.pin_config.sda = seed::D12;
    return display_i2c.Init(config) == I2CHandle::Result::OK
           && LineRackOledInit(&oled_transport);
}

void WakeDisplay()
{
    const bool was_awake = display_power.awake;
    LineRackDisplayPowerWake(&display_power, System::GetNow());
    if(display_ready && !was_awake
       && !LineRackOledSetEnabled(&oled_transport, true))
        display_ready = false;
    display_dirty = true;
}

void ShowPresetTemporarily()
{
    LineRackDisplayOverrideStart(&preset_display_override, System::GetNow());
    display_dirty = true;
}

void RefreshDisplay()
{
    const uint32_t now_ms = System::GetNow();
    rendered_display_mode = LineRackDisplayResolveMode(
        PreferredDisplayMode(),
        &preset_display_override,
        now_ms);
    rendered_volume_display =
        LineRackVolumeDisplayActive(&volume_display, now_ms);
    if(rendered_volume_display)
        LineRackDisplayRenderVolume(&display_frame,
                                    volume_display.percent,
                                    volume_display.muted);
    else if(rendered_display_mode == LINERACK_DISPLAY_VISUALIZER)
        LineRackDisplayRenderVisualizer(
            &display_frame,
            static_cast<uint8_t>(
                visualizer_left_peak.load(std::memory_order_relaxed) / 10U),
            static_cast<uint8_t>(
                visualizer_right_peak.load(std::memory_order_relaxed) / 10U));
    else
        LineRackDisplayRender(
            &display_frame,
            &preset_bank,
            static_cast<uint8_t>(
                active_slot_index.load(std::memory_order_relaxed) + 1U),
            rendered_display_mode);
    if(display_ready && display_power.awake
       && !LineRackOledWriteFrame(&oled_transport, &display_frame))
        display_ready = false;
    display_dirty = false;
}

void ConfigureProcessorBank(uint8_t                   bank_index,
                            const LineRackPresetBank &bank,
                            uint8_t                   slot_index)
{
    LineRackAudioRouterConfigure(&audio_routers[bank_index], &bank);
    processors[bank_index].Configure(
        bank.presets[slot_index],
        hardware.AudioSampleRate(),
        reverb_memory[bank_index]);
}

bool FlashRead(void *context, uint32_t address, uint8_t *data, size_t size)
{
    QSPIHandle *qspi = static_cast<QSPIHandle *>(context);
    std::memcpy(data, qspi->GetData(address), size);
    return true;
}

bool FlashErase(void *context, uint32_t address)
{
    QSPIHandle *qspi = static_cast<QSPIHandle *>(context);
    return qspi->EraseSector(address) == QSPIHandle::OK;
}

bool FlashWrite(void *context,
                uint32_t address,
                const uint8_t *data,
                size_t size)
{
    QSPIHandle *qspi = static_cast<QSPIHandle *>(context);
    return qspi->Write(address,
                       static_cast<uint32_t>(size),
                       const_cast<uint8_t *>(data)) == QSPIHandle::OK;
}

uint8_t ActiveSlot(void *)
{
    return static_cast<uint8_t>(
        active_slot_index.load(std::memory_order_relaxed) + 1U);
}

bool ActivateSlot(void *, uint8_t slot_number)
{
    if(slot_number < 1U || slot_number > LINERACK_PRESET_COUNT)
        return false;
    const uint8_t slot_index = static_cast<uint8_t>(slot_number - 1U);
    const uint8_t next_processor_bank = static_cast<uint8_t>(
        active_processor_bank.load(std::memory_order_relaxed) ^ 1U);
    ConfigureProcessorBank(next_processor_bank, preset_bank, slot_index);
    active_slot_index.store(slot_index, std::memory_order_relaxed);
    active_processor_bank.store(next_processor_bank, std::memory_order_release);
    ShowPresetTemporarily();
    WakeDisplay();
    return true;
}

bool CyclePreset(void *)
{
    const uint8_t next_slot = static_cast<uint8_t>(
        (active_slot_index.load(std::memory_order_relaxed) + 1U)
        % LINERACK_PRESET_COUNT);
    return ActivateSlot(nullptr, static_cast<uint8_t>(next_slot + 1U));
}

void CycleDisplayMode()
{
    const uint8_t previous = preset_bank.display_mode;
    const uint8_t current = previous & LINERACK_DISPLAY_MODE_MASK;
    const uint8_t next = static_cast<uint8_t>(
        (current + 1U) % (LINERACK_DISPLAY_VISUALIZER + 1U));
    preset_bank.display_mode =
        (previous & LINERACK_DISPLAY_BLANKING_DISABLED) | next;
    if(!LineRackPresetStorageSave(&preset_storage, &preset_bank))
        preset_bank.display_mode = previous;
    preset_display_override.active = false;
    WakeDisplay();
}

void WakeDisplayFromHid(void *)
{
    WakeDisplay();
}

void ReadPresets(void *, LineRackPresetBank *bank)
{
    *bank = preset_bank;
}

bool WritePresets(void *, const LineRackPresetBank *bank)
{
    if(!LineRackPresetStorageSave(&preset_storage, bank))
        return false;
    const uint8_t next_processor_bank = static_cast<uint8_t>(
        active_processor_bank.load(std::memory_order_relaxed) ^ 1U);
    ConfigureProcessorBank(
        next_processor_bank,
        *bank,
        active_slot_index.load(std::memory_order_relaxed));
    preset_bank = *bank;
    WakeDisplay();
    active_processor_bank.store(
        next_processor_bank, std::memory_order_release);
    display_dirty = true;
    return true;
}

void UsbPlaybackCallback(AudioHandle::InputBuffer input,
                         AudioHandle::OutputBuffer output,
                         size_t                    size)
{
    float left_peak = 0.0f;
    float right_peak = 0.0f;
    for(size_t sample = 0; sample < size; ++sample)
    {
        float usb_left = 0.0f;
        float usb_right = 0.0f;
        float left_output;
        float right_output;
        const bool usb_available =
            LineRackUsbAudioReadFrame(&usb_left, &usb_right);
        const uint8_t processor_bank =
            active_processor_bank.load(std::memory_order_acquire);
        LineRackAudioRouterProcess(&audio_routers[processor_bank],
                                   usb_available,
                                   usb_left,
                                   usb_right,
                                   input[0][sample],
                                   input[1][sample],
                                   &left_output,
                                   &right_output);
        processors[processor_bank].Process(left_output, right_output);
        LineRackUsbAudioApplyHostVolume(&left_output, &right_output);
        left_peak = fmaxf(left_peak, fabsf(left_output));
        right_peak = fmaxf(right_peak, fabsf(right_output));
        output[0][sample] = left_output;
        output[1][sample] = right_output;
    }
    visualizer_left_peak.store(
        static_cast<uint16_t>(sqrtf(fminf(left_peak, 1.0f)) * 1000.0f),
        std::memory_order_relaxed);
    visualizer_right_peak.store(
        static_cast<uint16_t>(sqrtf(fminf(right_peak, 1.0f)) * 1000.0f),
        std::memory_order_relaxed);
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
    preset_storage = {
        &hardware.qspi,
        FlashRead,
        FlashErase,
        FlashWrite,
        {kPresetSectorA, kPresetSectorB},
    };
    if(!LineRackPresetStorageLoad(&preset_storage, &preset_bank))
        LineRackPresetBankDefaults(&preset_bank);
    ConfigureProcessorBank(0U, preset_bank, 0U);
    LineRackDisplayPowerInit(&display_power, System::GetNow());
    LineRackButtonGestureInit(&button_gesture);
    display_ready = InitializeDisplay();
    RefreshDisplay();

#ifdef USE_USBD_COMPOSITE
    const LineRackHidRpcCallbacks hid_callbacks = {
        ActiveSlot,
        ActivateSlot,
        CyclePreset,
        ReadPresets,
        WritePresets,
        WakeDisplayFromHid,
        nullptr,
    };
    LineRackHidRpcInit(&hid_callbacks);
#endif

    const bool usb_started = LineRackUsbAudioStart() == 0;
    uint8_t observed_host_volume = LineRackUsbAudioHostVolumePercent();
    bool observed_host_mute = LineRackUsbAudioHostMuted();
    hardware.StartAudio(UsbPlaybackCallback);
    BlinkSlot(0);

    while(true)
    {
        preset_switch.Debounce();
        const LineRackButtonAction button_action =
            LineRackButtonGestureUpdate(&button_gesture,
                                        preset_switch.Pressed(),
                                        System::GetNow());
        if(button_action == LINERACK_BUTTON_WAKE_DISPLAY)
        {
            WakeDisplay();
        }
        else if(button_action == LINERACK_BUTTON_CYCLE_PRESET)
        {
            CyclePreset(nullptr);
            RefreshDisplay();
            const uint8_t next_slot =
                active_slot_index.load(std::memory_order_relaxed);
#ifdef USE_USBD_COMPOSITE
            LineRackHidRpcNotifyStatusChanged();
#endif
            BlinkSlot(next_slot);
        }
        else if(button_action == LINERACK_BUTTON_CYCLE_DISPLAY)
        {
            CycleDisplayMode();
            RefreshDisplay();
        }
        else if(button_action == LINERACK_BUTTON_RESERVED)
        {
            WakeDisplay();
        }

        const uint8_t host_volume = LineRackUsbAudioHostVolumePercent();
        const bool host_mute = LineRackUsbAudioHostMuted();
        if(host_volume != observed_host_volume
           || host_mute != observed_host_mute)
        {
            observed_host_volume = host_volume;
            observed_host_mute = host_mute;
            LineRackVolumeDisplayStart(
                &volume_display, host_volume, host_mute, System::GetNow());
            WakeDisplay();
        }

        if(!usb_started || LineRackUsbAudioUnderrunCount() > 0U
           || LineRackUsbAudioOverrunCount() > 0U)
        {
            hardware.SetLed((System::GetNow() / 100U) % 2U);
        }
        else if(LineRackUsbAudioConfigured())
        {
            hardware.SetLed(true);
        }
        else
        {
            hardware.SetLed((System::GetNow() / 500U) % 2U);
        }

        const LineRackDisplayMode current_display_mode =
            LineRackDisplayResolveMode(
                PreferredDisplayMode(),
                &preset_display_override,
                System::GetNow());
        const bool volume_display_active =
            LineRackVolumeDisplayActive(&volume_display, System::GetNow());
        const bool visualizer_refresh_due =
            current_display_mode == LINERACK_DISPLAY_VISUALIZER
            && !volume_display_active
            && display_power.awake
            && System::GetNow() - last_visualizer_refresh_ms >= 100U;
        if(display_dirty || current_display_mode != rendered_display_mode
           || volume_display_active != rendered_volume_display
           || visualizer_refresh_due)
        {
            RefreshDisplay();
            if(current_display_mode == LINERACK_DISPLAY_VISUALIZER)
                last_visualizer_refresh_ms = System::GetNow();
        }

        const bool was_awake = display_power.awake;
        const bool is_awake = LineRackDisplayPowerResolve(
            &display_power, DisplayBlankingEnabled(), System::GetNow());
        if(display_ready && was_awake != is_awake)
        {
            if(!LineRackOledSetEnabled(&oled_transport, is_awake))
                display_ready = false;
            else if(is_awake)
                RefreshDisplay();
        }

#ifdef USE_USBD_COMPOSITE
        LineRackHidRpcPoll();
#endif
        System::Delay(1);
    }
}
