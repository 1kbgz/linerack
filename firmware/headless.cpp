#include "daisy_seed.h"

using namespace daisy;

namespace
{
constexpr uint8_t kSlotCount = 4;

DaisySeed hardware;
Switch preset_button;
uint8_t active_slot = 1;

void AudioCallback(AudioHandle::InputBuffer input,
                   AudioHandle::OutputBuffer output,
                   size_t                    size)
{
    for(size_t sample = 0; sample < size; ++sample)
    {
        output[0][sample] = input[0][sample];
        output[1][sample] = input[1][sample];
    }
}

void BlinkActiveSlot()
{
    for(uint8_t blink = 0; blink < active_slot; ++blink)
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
    preset_button.Init(seed::D10);

    hardware.StartAudio(AudioCallback);
    BlinkActiveSlot();

    while(true)
    {
        preset_button.Debounce();
        if(preset_button.RisingEdge() || preset_button.FallingEdge())
        {
            active_slot = active_slot % kSlotCount + 1;
            BlinkActiveSlot();
        }
        System::Delay(1);
    }
}
