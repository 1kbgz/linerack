# Development enclosure reference

The active development enclosure keeps the verified Seed3, OLED, button, and
TRRS breakout layout. Both variants use the same 60 × 31 × 20.7 mm external
envelope and internal component coordinates.

## Active sources

| Source | Closure | Required case hardware |
| --- | --- | --- |
| `enclosure-dev-friction.scad` | Four shallow snap tabs | None |
| `enclosure-dev-screw.scad` | Four corner screws into printed pilot bosses | Four M2×8 button-head machine screws |

The screw bosses occupy only the upper 5.8 mm of the base. This avoids
full-height posts through the Seed3 footprint. Pilot holes are 1.6 mm and lid
clearance holes are 2.4 mm.

Both sources accept these `part` values:

| Value | Output |
| --- | --- |
| `base` | Enclosure base |
| `lid` | Lid |
| `assembly` | Transparent component fit preview |
| `layout` | Base and lid arranged for printing |

The friction source also accepts `snap-fit-test`.

## Dimensions

| Property | Value |
| --- | ---: |
| External length | 60 mm |
| External width | 31 mm |
| Assembled height | 20.7 mm |
| Base height | 18.3 mm |
| Lid thickness | 2.4 mm |
| Wall and floor thickness | 2 mm |
| Lid fit clearance | 0.3 mm |
| Lid skirt depth | 6 mm |

The prior printed enclosure was 23 mm high. The active sources reduce total
height by 10%. This height is not physically verified. The modeled component
envelopes overlap vertically after the reduction because they conservatively
represent each breakout as a full rectangular volume; inspect the first print
for actual board, connector, and wire interference.

## Component geometry

| Module | CAD envelope | Mounting pattern |
| --- | --- | --- |
| Electro-Smith Daisy Seed3 | 51.26 × 18.24 × 10 mm | Floor guides; no fasteners |
| Adafruit 4440 OLED | 33.02 × 21.59 × 6 mm | Four 2.5 mm holes on 27.94 × 16.51 mm centers |
| Adafruit 5764 TRRS breakout | 17.145 × 17.78 × 6.5 mm | Two 2.0 mm holes, 12.7 mm apart |
| Adafruit 1119 button | 12 × 6 × 12 mm holder envelope | Side-mounted press fit |

OLED and TRRS fasteners are hidden inside the lid and use M2×4 button-head
machine screws. The TRRS
board and end-wall opening share the enclosure width centerline. The USB-C and
TRRS vertical centerlines remain fit-test parameters.

The development BOM links one Amazon assortment containing M2×4 and M2×8
stainless button-head socket-cap screws, plus matching nuts and washers. Current
1.6 mm printed pilots rely on the machine screws cutting into PLA. This is
adequate for a fit test but may loosen after repeated assembly; both sizes
remain physically unverified in this enclosure.

## Generated meshes

Generated STLs live under `hardware/mechanical/generated/` and remain ignored.
Current names are:

- `linerack-enclosure-dev-friction-{base,lid,layout}.stl`
- `linerack-enclosure-dev-friction-fit-test.stl`
- `linerack-enclosure-dev-screw-{base,lid,layout}.stl`

Example:

```sh
openscad -o hardware/mechanical/generated/linerack-enclosure-dev-screw-layout.stl \
  -D 'part="layout"' hardware/mechanical/enclosure-dev-screw.scad
```

## Archive

`old/` contains superseded enclosure sources, fit coupons, and generated
meshes. These files are retained only as mechanical history:

- `old/enclosure-v0.scad`: original 105 × 42 mm M2/M4 design;
- `old/compact-23mm/`: prior 60 × 31 × 23 mm friction enclosure;
- `old/v0/`, `old/v0-m4/`, and `old/tests/`: prior generated meshes.

## Fit status

| Property | Status |
| --- | --- |
| 60 × 31 mm footprint | Printed and usable |
| 23 mm enclosure height | Printed and usable |
| 20.7 mm enclosure height | Unprinted |
| OLED opening and mounting | Printed and usable |
| TRRS horizontal alignment | Corrected and physically verified |
| Friction closure | Printed; too loose |
| M2 screw closure | Unprinted |
| USB-C vertical centerline | Working; not measured from datum |
| TRRS vertical centerline | Working; not measured from datum |
| PLA compensation | Printer-specific; not recorded |

The enclosure remains a development fit mule. It does not establish headphone
output safety, strain-relief durability, drop resistance, thermal performance,
or production tolerances.
