# How to print the development enclosure

This guide produces the current Seed3 development enclosure.

1. Choose `hardware/mechanical/enclosure-dev-screw.scad` for the primary
   enclosure. It requires four M2×8 case screws and six M2×4 breakout-board
   screws.
2. Use `hardware/mechanical/enclosure-dev-friction.scad` only to evaluate the
   retained friction closure. Print `part = "snap-fit-test"` before full parts.
3. Set `part = "assembly"` and render. Check Seed3, OLED, TRRS, button, and wire
   clearance. The 20.7 mm height has not been physically verified.
4. Set `part = "layout"` and export one STL, or export `base` and `lid`
   separately.
5. Print without supports, with base floor and lid exterior face on the build
   plate. Start with 0.2 mm layers, three perimeters, and PLA.
6. For screw closure, confirm M2 screws enter 1.6 mm pilots without splitting
   bosses. Stop if insertion whitens PLA or requires excessive torque.
7. Mount OLED and TRRS boards with M2×4 screws. Insulate Seed3 and solder joints,
   restrain wiring, and leave BOOT and RESET reachable by opening lid.
8. Connect USB and audio while watching mounted boards. Connectors must not move
   or transfer load to point wiring.

Refer to `hardware/mechanical/README.md` for dimensions, output names, fit
status, and archived designs.

Do not use this enclosure as headphone-output safety, production assembly, or
environmental/compliance validation.
