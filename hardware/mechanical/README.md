# Development enclosure reference

The active development enclosure mounts a headerless Seed3, OLED, two TRRS
breakouts, and low-profile button beneath one top shell. Components occupy two
rows without overlapping vertically. A flat bottom plate closes with four M2
screws.

## Active source

`enclosure-dev-screw.scad` accepts these `part` values:

| Value | Output |
| --- | --- |
| `base` | Flat screwed bottom plate |
| `lid` | Top shell with component mounts and connector openings |
| `assembly` | Closed component fit preview |
| `layout` | Base and lid arranged for printing |

## Dimensions

| Property | Value |
| --- | ---: |
| External length | 80 mm |
| External width | 50 mm |
| External body height | 12 mm |
| Top shell height | 10.4 mm |
| Bottom plate thickness | 1.6 mm |
| Wall and roof thickness | 1.6 mm |
| Component standoff | 0.8 mm |
| Clearance below 6.5 mm components | 1.5 mm |

External body height excludes protruding screw heads. The 80 × 50 mm footprint
provides clearance for four case bosses while keeping both audio jacks on the
end opposite USB-C.

## Component geometry

| Module | CAD envelope | Mounting |
| --- | --- | --- |
| Electro-Smith Daisy Seed3 | 51.26 × 18.24 × 6.5 mm | Headerless; roof guides and compressible bottom pad |
| Adafruit 4440 OLED | 33.02 × 21.59 × 6 mm | Four 2.5 mm holes on 27.94 × 16.51 mm centers |
| Two Adafruit 5764 TRRS breakouts | 17.145 × 17.78 × 6.5 mm each | Two 2.0 mm holes, 12.7 mm apart |
| Adafruit 367 button | 6 × 6 × 6 mm | Three-sided internal holder with open wiring side |

The OLED envelope retains room for its STEMMA QT connector. Removing that
connector reduces local height but does not change case height, which is set by
the Seed3 and TRRS breakouts.

OLED and TRRS screws enter printed 1.6 mm pilot holes from inside the enclosure.
Use an M2 washer with each M2×4 component screw so the screw cannot mark the
1.6 mm top surface. Four M2×8 screws pass through the bottom plate into printed
corner bosses. Printed pilots are suitable for fit testing, not repeated
production assembly.

## Generated meshes

Generated STLs live under `hardware/mechanical/generated/` and remain ignored:

- `linerack-enclosure-dev-screw-base.stl`
- `linerack-enclosure-dev-screw-lid.stl`
- `linerack-enclosure-dev-screw-layout.stl`
- `linerack-enclosure-dev-screw-assembly.stl`

Example:

```sh
openscad -o hardware/mechanical/generated/linerack-enclosure-dev-screw-layout.stl \
  -D 'part="layout"' hardware/mechanical/enclosure-dev-screw.scad
```

Print the lid with its exterior top surface on the build plate. Print the flat
base in its exported orientation. Supports should not be required.

## Archive

`old/` contains superseded mechanical sources and meshes:

- `old/compact-20.7mm/`: prior 60 × 31 × 20.7 mm friction and screw sources;
- `old/compact-23mm/`: prior 60 × 31 × 23 mm friction enclosure meshes;
- `old/enclosure-v0.scad`: original 105 × 42 mm M2/M4 source;
- `old/v0/`, `old/v0-m4/`, and `old/tests/`: original meshes and fit coupons.

## Fit status

| Property | Status |
| --- | --- |
| 80 × 50 × 12 mm enclosure | Exported; unprinted |
| Headerless Seed3 envelope | Based on physical measurement; unprinted |
| OLED opening and mounting pattern | Reused from verified enclosure |
| Dual TRRS mounting and openings | Second position unverified |
| Adafruit 367 holder | Based on physical measurement; unprinted |
| M2 screw closure | Unprinted |
| USB-C and TRRS vertical alignment | Recalculated for top-mounted components; unprinted |

This remains a development fit mule. It does not establish headphone-output
safety, strain-relief durability, drop resistance, thermal performance, or
production tolerances.

## License

These mechanical sources are available under the repository's
[PolyForm Noncommercial License 1.0.0](../../LICENSE).

Required Notice: Copyright 2025–2026 1kbgz.
