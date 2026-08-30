#include "daisy_seed.h"
#include "dev/oled_ssd130x.h"

using namespace daisy;

namespace
{
constexpr uint8_t kSlotCount = 4;

DaisySeed hardware;
Switch preset_button;
OledDisplay<SSD130xI2c128x32Driver> display;
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

void DrawActiveSlot()
{
    const char label[] = {static_cast<char>('0' + active_slot), '\0'};
    display.Fill(false);
    display.SetCursor(58, 7);
    display.WriteString(label, Font_11x18, true);
    display.Update();
}
} // namespace

int main()
{
    hardware.Init();

    preset_button.Init(seed::D10);

    OledDisplay<SSD130xI2c128x32Driver>::Config display_config;
    auto& i2c = display_config.driver_config.transport_config.i2c_config;
    i2c.periph = I2CHandle::Config::Peripheral::I2C_1;
    i2c.speed = I2CHandle::Config::Speed::I2C_400KHZ;
    i2c.pin_config.scl = seed::D11;
    i2c.pin_config.sda = seed::D12;
    display_config.driver_config.transport_config.i2c_address = 0x3c;
    display.Init(display_config);
    DrawActiveSlot();

    hardware.StartAudio(AudioCallback);

    while(true)
    {
        preset_button.Debounce();
        if(preset_button.RisingEdge())
        {
            active_slot = active_slot % kSlotCount + 1;
            DrawActiveSlot();
        }
        System::Delay(1);
    }
}
