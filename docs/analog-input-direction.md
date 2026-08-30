# Why Seed3 does not need an external audio ADC

LineRack can prototype its analog chaining path with the Seed3's existing audio
codec. An additional ADC or codec breakout is not required.

Seed3 uses TI's TAC5242, which contains two ADC channels and two DAC channels.
The Seed3 exposes those codec channels directly as `AUDIO IN L` and
`AUDIO IN R` on physical pins 16 and 17; analog ground is pin 20. Its published
input range is 3.6 V peak-to-peak, approximately 1 V RMS. These are dedicated
audio inputs, not the STM32's general-purpose control ADC pins.

Current firmware receives both codec input channels and can select USB, analog,
or a mix before processing the resulting stereo samples through the same active
preset and codec outputs. This route is covered by native tests but has not yet
been verified through a physical Seed3 analog input.

## What the carrier still needs

The onboard ADC eliminates a digital converter board, but it does not eliminate
analog front-end work. A panel jack should not be treated as a production-ready
direct connection to pins 16 and 17. The carrier must define input impedance,
AC coupling, RF filtering, protection, connector channel mapping, and clipping
headroom. Daisy's published line-input reference uses a 20 kΩ typical input
impedance and includes an op-amp, filtering, and coupling around each audio
input. Instrument input requires a different high-impedance front end; Daisy's
reference uses 1 MΩ.

This distinction matters for the modular adapter idea. A 3.5 mm stereo
line-input adapter and a 1/4-inch instrument adapter may share the same digital
DSP boundary, but they should not silently share the same analog circuit.

## Power and source modes

Analog-only means the audio source is analog, not that the unit is unpowered.
Seed3 still needs power from its onboard USB-C port or `VIN`. The first
prototype can therefore use a computer, phone charger, or ordinary USB power
bank while pins 16 and 17 carry audio. No battery-management design is needed.

USB-only and analog-only modes each select one stereo source before the chain.
USB-plus-analog mode sums independently trimmed sources. Each trim ranges from
-24 to 0 dB in 0.5 dB steps, and mixed mode restricts both to -6.5 dB or lower
so two correlated full-scale signals remain below full scale before DSP. These
rules are implemented but still require target headroom and clipping tests.

## Sources

- [Daisy Seed3 documentation](https://docs.daisy.audio/hardware/Seed3) identifies
  the TAC5242, USB-C power, and pin compatibility.
- [Daisy Seed3 datasheet](https://daisy.nyc3.cdn.digitaloceanspaces.com/products/seed3/Daisy_Seed3_datasheet.pdf)
  assigns physical pins 16 and 17 to left/right audio input, specifies their
  electrical range, and provides line- and instrument-level reference circuits.
- [TI TAC5242 product page](https://www.ti.com/product/TAC5242) specifies two ADC
  channels, two DAC channels, line/microphone input support, and simultaneous
  stereo conversion.
