# Seed3 prototype hardware reference

This reference describes the verified LineRack breadboard prototype and the
planned hand-assembled beta architecture. It is not a production schematic or
frozen commercial bill of materials.

## Prototype modules

| Item | Quantity | Current role | Status |
| --- | ---: | --- | --- |
| Electro-Smith Daisy Seed3 | 1 | STM32 processor, USB-C, TAC5242 stereo codec, QSPI storage | Verified |
| Adafruit 0.91-inch 128x32 I2C OLED, product 4440 | 1 | Preset and EQ display | Verified |
| STEMMA QT/Qwiic cable | 1 | OLED power and I2C connection | Verified |
| Normally-open momentary button | 1 | Display wake, preset selection, display mode | Basic press verified; gesture source awaits flash test |
| Adafruit TRRS jack breakout | 1 output; second required for input tests | Panel audio connection | Output verified |
| Solderless breadboard and jumpers | 1 set | Development interconnect | Verified; not shippable |
| USB-C data cable | 1 | Power, USB Audio, and WebHID | Verified on Mac and iPhone |

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

The TRRS breakout terminals `Sleeve`, `LSw`, and `RSw` remain disconnected. The
preset button uses the Seed3 internal pull-up and has no external resistor.

## Audio and power limits

| Property | Prototype boundary |
| --- | --- |
| Power | USB-C bus power or regulated Seed3 `VIN`; no battery |
| USB playback | UAC1, stereo, 48 kHz, signed 16-bit PCM |
| USB host volume control | UAC1 master volume and mute verified on Mac; iPhone verification pending |
| Codec input | 3.6 V peak-to-peak published maximum, approximately 1 V RMS |
| Analog input front end | Not implemented on the breadboard |
| Headphone output rating | Not established |
| Production USB identity | Not allocated; `CAFE:4C52` is development-only |

Direct codec input requires a carrier front end defining impedance, AC
coupling, RF filtering, input protection, and clipping headroom. Direct headset
output testing does not establish safe output impedance, clean level, DC,
noise, thermal, or short-circuit limits.

## Hand-assembled beta target

The no-custom-PCB beta architecture replaces the breadboard with:

| Item | Target |
| --- | --- |
| Compute/audio module | Permanently soldered Seed3 |
| Interconnect | Adafruit half-size Perma-Proto, product 1609 |
| Display | Panel-mounted product 4440 OLED |
| Control | Panel-mounted normally-open preset button |
| Output | Panel-mounted 3.5 mm stereo jack |
| Enclosure | Printed enclosure with USB strain relief and BOOT/RESET access |

The estimated hardware cost is $51–58 before labor, shipping, payment fees,
compliance, support, and returns. Approximately $75 plus shipping is a
subsidized invite-only beta target; $99–119 is a more credible hand-assembled
cost-recovery range. Neither range is a committed retail price.

The first beta excludes battery power, analog input, a swappable op-amp stage,
and a production headphone amplifier. A custom carrier PCB replaces point
wiring after the beta architecture and I/O requirements are validated.
