#include "daisy_seed.h"

using namespace daisy;

int main()
{
    DaisySeed hardware;
    hardware.Init();

    bool led_on = false;
    while(true)
    {
        hardware.SetLed(led_on);
        led_on = !led_on;
        System::Delay(250);
    }
}
