# How to bring up the Seed3 prototype

This guide brings a Seed3 from board diagnostics to the current integrated USB
Audio, DSP, WebHID, QSPI-persistence, preset-button, and SSD1306 OLED image. The
composite firmware sends its exact 512-byte display model to the panel over I2C.

## Wiring

Orient the Seed3 using its printed pin labels rather than relying on breadboard
row numbers.

| Function | Seed3 connection | Peripheral connection |
| --- | --- | --- |
| Preset button | `D10` and `DGND` | One terminal to each |
| OLED power | `3V3 Digital` | Red STEMMA QT wire |
| OLED ground | `DGND` | Black STEMMA QT wire |
| OLED data | `D12 / SDA` | Blue STEMMA QT wire |
| OLED clock | `D11 / SCL` | Yellow STEMMA QT wire |
| Stereo input left | `Audio In 1` | Jack `Left` |
| Stereo input right | `Audio In 2` | Jack `Right` |
| Stereo input ground | `AGND` | Jack `Ring` |
| Stereo output left | `Audio Out 1` | Jack `Left` |
| Stereo output right | `Audio Out 2` | Jack `Right` |
| Stereo output ground | `AGND` | Jack `Ring` |

Leave `Sleeve`, `LSw`, and `RSw` disconnected on each Adafruit TRRS breakout.
The button uses the Seed's internal pull-up, so it needs no external resistor.

## Toolchain

Firmware currently uses libDaisy without DaisySP. On macOS, install the ARM GCC
toolchain and DFU utility:

```sh
brew install --cask gcc-arm-embedded
brew install dfu-util
```

Keep third-party source outside version control:

```sh
git clone --depth 1 --recursive https://github.com/electro-smith/libDaisy.git .deps/libDaisy
make -C .deps/libDaisy -j4
make -C firmware -f Makefile.diagnostics
make -C firmware -f Makefile.headless
make -C firmware
```

The verified libDaisy revision is
`cc146d5065dd8286078a662e2830bf820c37a612`. Check out that revision before
building if current libDaisy changes break compatibility.

## Board-only check

Before connecting the breadboard or peripherals, flash the diagnostic image.
It only initializes Seed3 and toggles its onboard LED every 250 milliseconds.

Place Seed3 in DFU mode by holding `BOOT`, pressing and releasing `RESET`, then
releasing `BOOT`. Confirm that it is visible before writing:

```sh
dfu-util --list
make -C firmware -f Makefile.diagnostics program-dfu
```

The onboard LED should blink twice per second. Press and release `RESET` once
if it does not begin automatically after flashing.

With `dfu-util` 0.11 on macOS, the leave request may end with
`Error during download get_status` after reporting `File downloaded
successfully`. The first Seed3 did this, then left DFU and ran the diagnostic
normally. Treat it as a successful flash only when the download-complete
message appears and the expected firmware behavior follows.

## Output-only check

When an input jack is unavailable, the output diagnostic verifies the codec and
stereo output wiring without an audio source:

```sh
make -C firmware -f Makefile.output-test
make -C firmware -f Makefile.output-test program-dfu
```

It holds the onboard LED on and emits a quiet 440 Hz sine on `Audio Out 1` and
660 Hz sine on `Audio Out 2`, each at approximately -30 dBFS. Start with powered
speakers or an audio-interface line input at minimum volume. Do not use passive
headphones for this test.

On 2026-08-24, both Seed3 outputs produced audible test tones through a CTIA
headset wired as tip = `Audio Out 1`, first ring = `Audio Out 2`, second ring =
`AGND`, and sleeve/microphone = disconnected. This establishes basic output
function only; it does not establish a supported headphone impedance, maximum
clean level, noise performance, or thermal margin.

## DSP gain demo

The first portable DSP slice is a fixed gain followed by a stereo-linked sample
peak limiter. Run its native tests and build the Seed3 demo with:

```sh
make -C firmware/dsp test
make -C firmware -f Makefile.dsp-demo
```

The demo uses the same 440 Hz left and 660 Hz right tones as the output check.
The maintained test switch advances through gain values of -18, -12, -6, and
0 dB; the LED blinks the active slot number. A -6 dBFS limiter follows the gain.
Slot 4 has the same output level as the output-only diagnostic, while the other
slots are quieter.

The parametric EQ demo builds with:

```sh
make -C firmware -f Makefile.eq-demo
make -C firmware -f Makefile.eq-demo program-dfu
```

Its four slots are flat, +12 dB at 440 Hz/Q8, -12 dB at 440 Hz/Q8, and +12 dB
at 660 Hz/Q8. With the generated stereo tones, these emphasize the left tone,
de-emphasize the left tone, and emphasize the right tone respectively. The
native DSP test measures the configured center-frequency response before this
audible check. The demo source is -42 dBFS, so a +12 dB center-frequency boost
does not exceed the -30 dBFS output-only test level.

After diagnostics succeed, connect the output and D10 preset button, return to
DFU mode, and build the integrated LineRack image:

```sh
make -C firmware -f Makefile.usb-composite clean
make -C firmware -f Makefile.usb-composite -j4
make -C firmware -f Makefile.usb-composite program-dfu
```

Download the current setup from the browser before flashing a firmware build
that changes `LineRackPresetBank`: QSPI records with a different payload size
are rejected and defaults are loaded. The ten-block image uses this safe reset.

If USB Audio and WebHID are not required and the OLED is unavailable, use the
legacy headless diagnostic instead:

```sh
make -C firmware -f Makefile.headless program-dfu
```

It provides analog stereo pass-through and `D10` preset button behavior but does
not implement the current USB Audio/WebHID chain. On startup and after each
preset change, the onboard LED blinks the active slot number. The temporary
headless build advances on either switch transition, so a maintained SPDT or
DPDT switch can stand in for the intended momentary button.

## First-power sequence

1. Inspect solder joints and check that no power rail is shorted to ground.
2. Connect and flash Seed3 without the audio output attached.
3. Confirm macOS lists `LineRack USB Audio Dev` at 48 kHz with two output
   channels.
4. Connect the output to powered speakers or an audio interface at low volume.
5. Select LineRack as host output and confirm clean left/right program audio.
6. Open the configurator in Chrome, authorize LineRack, and confirm hardware
   editing rather than simulator editing.
7. Quickly double tap the D10 button and confirm active slots advance `2`, `3`,
   `4`, `1` in the browser. Confirm a single tap wakes the OLED and a 5–10
   second hold cycles Preset, EQ Response, and Visualizer modes.
8. Write a changed preset, power-cycle USB, reconnect, and confirm it reloads.

If the OLED content appears shifted approximately 32 pixels to the right, stop
there and record a photo. Some 128x32 SSD1306 modules expose a known libDaisy
column-offset incompatibility; fix it only after confirming the behavior on the
attached display.

## Success criteria

- The board-only diagnostic flashes and the onboard LED blinks twice per
  second.
- Seed3 remains connected and stable over USB power.
- CoreAudio and WebHID enumerate together.
- Browser and physical-button active slots stay synchronized.
- Left and right USB playback pass without swapping, clipping, or obvious noise.
- Presets survive USB power cycling.
- Disconnecting or reconnecting an audio cable does not reset the board.
