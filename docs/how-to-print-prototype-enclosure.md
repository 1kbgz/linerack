# How to print the development enclosure

Print and fit-check the current Seed3 development enclosure.

## Prerequisites

- OpenSCAD
- A PLA-capable printer
- Four M2×8 case screws
- Six M2×4 breakout-board screws

## Render the enclosure

Use `hardware/mechanical/enclosure-dev-screw.scad` for the primary enclosure.
Set `part = "assembly"`, render it, and inspect Seed3, OLED, TRRS, button, and
wire clearance. The 20.7 mm height has not been physically verified.

Set `part = "layout"` to export one STL, or export `base` and `lid` separately.

Use `hardware/mechanical/enclosure-dev-friction.scad` only to evaluate the
retained friction closure. Print its `snap-fit-test` part before full parts.

## Print and assemble

1. Place the base floor and lid exterior face on the build plate.
2. Print without supports using 0.2 mm layers, three perimeters, and PLA.
3. Confirm M2 screws enter the 1.6 mm pilots without splitting bosses. Stop if
   insertion whitens PLA or requires excessive torque.
4. Mount the OLED and TRRS boards with M2×4 screws.
5. Insulate Seed3 and solder joints, restrain wiring, and keep BOOT and RESET
   reachable by opening the lid.

## Verify the fit

Connect USB and audio while watching the mounted boards. Connectors must align
with their openings without moving a board or loading point-to-point wiring.

See `hardware/mechanical/README.md` for dimensions, output filenames, fit
status, and archived designs. This enclosure does not validate headphone-output
safety, production assembly, or environmental compliance.
