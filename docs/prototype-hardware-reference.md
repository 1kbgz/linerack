# Seed3 prototype hardware reference

Verified modules, connections, and electrical boundaries for the current
LineRack development prototype. This is not a production schematic.

## Modules

| Item | Quantity | Role | Status |
| --- | ---: | --- | --- |
| Electro-Smith Daisy Seed3 | 1 | STM32 processor, USB-C, TAC5242 stereo codec, QSPI storage | Verified |
| Adafruit 0.91-inch 128×32 I2C OLED, product 4440 | 1 | Preset, EQ, visualizer, and volume display | Verified |
| STEMMA QT/Qwiic cable | 1 | OLED power and I2C | Verified |
| Normally-open momentary button | 1 | Display wake, preset selection, display mode | Basic operation verified; full gesture timing needs a repeat test |
| Adafruit TRRS jack breakout | 1 output; second required for input tests | Panel audio connection | Output verified |
| Solderless breadboard and jumpers | 1 set | Development interconnect | Verified; not shippable |
| USB-C data cable | 1 | Power, USB Audio, and WebHID | Verified on Mac and iPhone |

See `hardware/bom/dev.csv` for exact development parts and links.

## Seed3 connections

| Function | Seed3 connection | Peripheral connection |
| --- | --- | --- |
| Preset button | `D10` and `DGND` | One terminal to each |
| OLED power | `3V3 Digital` | Red STEMMA QT wire |
| OLED ground | `DGND` | Black STEMMA QT wire |
| OLED data | `D12 / SDA` | Blue STEMMA QT wire |
| OLED clock | `D11 / SCL` | Yellow STEMMA QT wire |
| Stereo input left | `Audio In 1`, physical pin 16 | Jack `Left` |
| Stereo input right | `Audio In 2`, physical pin 17 | Jack `Right` |
| Stereo input ground | `AGND`, physical pin 20 | Jack `Ring` |
| Stereo output left | `Audio Out 1` | Jack `Left` |
| Stereo output right | `Audio Out 2` | Jack `Right` |
| Stereo output ground | `AGND` | Jack `Ring` |

Leave TRRS terminals `Sleeve`, `LSw`, and `RSw` disconnected. The button uses
Seed3's internal pull-up and needs no external resistor.

## Button gestures

| Gesture | Behavior |
| --- | --- |
| Single press | Wake display and show current state |
| Quick double press | Activate next preset |
| Hold for 5–10 seconds | Cycle persistent display mode |
| Hold for 10 seconds or longer | Reserved |

Preset changes temporarily show the preset view before the configured display
mode returns.

## Audio and power limits

| Property | Prototype boundary |
| --- | --- |
| Power | USB-C bus power or regulated Seed3 `VIN`; no battery |
| USB playback | UAC1, stereo, 48 kHz, signed 16-bit PCM |
| USB host volume | UAC1 master volume and mute verified on Mac; iPhone verification pending |
| Codec input | 3.6 V peak-to-peak published maximum, about 1 V RMS |
| Analog input front end | Not implemented on the prototype |
| Headphone output rating | Not established |
| USB identity | `CAFE:4C52`, development only |

Direct codec input still requires impedance definition, AC coupling, RF
filtering, protection, and clipping headroom. Direct headset output testing does
not establish safe output impedance, clean level, DC, noise, thermal behavior,
or short-circuit limits.

## Recovery controls

Seed3's onboard **RESET** control can restore audio after some host or USB
faults. **BOOT** plus **RESET** enters DFU for firmware recovery. The current
development enclosure exposes both only after opening the case.

Some 128×32 SSD1306 modules can render content about 32 pixels to the right due
to a controller column-offset mismatch. Record the module and behavior before
changing display code; the fault does not affect audio or HID control.
