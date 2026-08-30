# Why Seed3 does not need an external audio ADC

Seed3's existing codec can digitize LineRack's analog stereo source. An extra
ADC or codec breakout would duplicate that function without solving the analog
front-end work a product still needs.

## The codec boundary

Seed3 uses TI's TAC5242, which contains two ADC and two DAC channels. Seed3
exposes the codec inputs as `AUDIO IN L` and `AUDIO IN R` on physical pins 16
and 17, with analog ground on pin 20. Their published maximum is 3.6 V
peak-to-peak, about 1 V RMS. They are audio inputs, not STM32 control ADC pins.

Firmware can select USB, analog, or a trimmed mix before routing stereo samples
through the active effect chain and codec outputs. Native tests cover this path;
physical analog-input verification still requires a second input jack.

## Why a carrier front end is still required

An onboard ADC does not make a panel jack production-ready. The carrier must
define:

- Input impedance and connector mapping
- AC coupling and DC bias behavior
- RF filtering and transient protection
- Clipping headroom
- Line-level or instrument-level input behavior

Daisy's line-input reference uses about 20 kΩ input impedance plus op-amp,
filtering, and coupling stages. Its instrument reference uses 1 MΩ. A stereo
line adapter and mono instrument adapter may share the DSP interface, but they
should not share an unspecified analog circuit.

## Source mixing and headroom

USB-only and analog-only modes select one stereo source. Mix mode sums both
sources after independent trims from -24 to 0 dB in 0.5 dB steps. Firmware
requires both mix trims at or below -6.5 dB so correlated full-scale sources do
not clip before DSP.

Analog-only describes the signal source, not the power source. Seed3 still
needs USB-C power or regulated `VIN`; a phone charger or ordinary USB power bank
is enough for a battery-free prototype.

## Tradeoff

Reusing TAC5242 keeps the prototype small and avoids another digital audio
interface. It also makes the custom carrier responsible for input safety and
signal conditioning. That trade is appropriate for the current line-level
prototype, but it does not settle a future instrument-input design.

## Sources

- [Daisy Seed3 documentation](https://docs.daisy.audio/hardware/Seed3)
- [Daisy Seed3 datasheet](https://daisy.nyc3.cdn.digitaloceanspaces.com/products/seed3/Daisy_Seed3_datasheet.pdf)
- [TI TAC5242 product page](https://www.ti.com/product/TAC5242)
