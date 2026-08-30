# Seed3 carrier PCB reference

This KiCad project fixes the first carrier's mechanical envelope and connector
roles. It is intentionally not a schematic, routed board, or fabrication
release.

## Mechanical contract

| Property | Current value |
| --- | --- |
| Board outline | 35 × 82 mm |
| Board thickness | 1.6 mm placeholder |
| Mounting holes | Four 2.7 mm non-plated holes on a 28 × 75 mm pattern |
| Seed3 mounting | Two 20-pin through-hole rows, 15.24 mm apart, 2.54 mm pitch |
| Seed3 orientation | USB-C faces the short edge nearest H1/H2 |
| Peripheral area | Approximately 27 mm beyond Seed3 pin span |

The Seed3 pad and fabrication geometry is transcribed from Electro-Smith's
official MIT-licensed DaisyKiCad `DAISY_SEED` footprint. Before routing or
ordering, compare the installed Seed3 revision with the current official
KiCad and STEP assets at <https://docs.daisy.audio/hardware/Seed3?embed=1>.

## Connector contract

The connector footprints reserve placement only. The future schematic must
implement these pin assignments before copper is added.

| Connector | Pin | Signal | Seed3 physical pin |
| --- | ---: | --- | ---: |
| J1 OLED | 1 | `3V3_D` | 38 |
| J1 OLED | 2 | `DGND` | 40 |
| J1 OLED | 3 | `SDA / D12` | 13 |
| J1 OLED | 4 | `SCL / D11` | 12 |
| J2 Button | 1 | `D10` | 11 |
| J2 Button | 2 | `DGND` | 40 |
| J3 Audio out | 1 | `OUT_L` | 18 |
| J3 Audio out | 2 | `OUT_R` | 19 |
| J3 Audio out | 3 | `AGND` | 20 |
| J4 Audio in, reserved | 1 | `IN_L` | 16 |
| J4 Audio in, reserved | 2 | `IN_R` | 17 |
| J4 Audio in, reserved | 3 | `AGND` | 20 |

J4 reserves mechanical space only. Its production circuit still needs input
impedance, AC coupling, filtering, ESD/overvoltage protection, and measured
headroom. See [`docs/analog-input-direction.md`](../../../docs/analog-input-direction.md).

## Before schematic capture

- Confirm socket versus permanently soldered Seed3 assembly and total height.
- Select locking connectors for OLED, button, and panel audio wiring.
- Decide whether the audio jack belongs on the carrier or enclosure panel.
- Measure USB-C, BOOT, and RESET locations against the case datum.
- Add accessible power, ground, I2C, audio, and debug test points.
- Define analog/digital ground connection from the Seed3 datasheet.
- Select ESD protection and output/load protection from measured requirements.

Do not generate Gerbers or order this board. The project contains no schematic,
net assignments, copper routing, ground planes, design rules, or manufacturing
outputs.

After placement is electrically complete, export the board assembly with
`kicad-cli pcb export step` and use that STEP file as the enclosure datum. Do
not maintain an independent second copy of the production PCB dimensions in
OpenSCAD.

## License

These PCB design files are available under the repository's
[PolyForm Noncommercial License 1.0.0](../../../LICENSE).

Required Notice: Copyright 2025–2026 1kbgz.
