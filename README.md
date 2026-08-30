# LineRack

USB-powered portable audio DSP with browser-configurable effect chains.

See [DSP and display direction](docs/dsp-and-display-direction.md) for the
planned EQ, filters, dynamics, reverb, utility processing, meters, spectrum,
loudness, and visualizer architecture.
See [analog input direction](docs/analog-input-direction.md) for the confirmed
Seed3 codec-input path and remaining carrier-front-end work.
See [development enclosure sources](hardware/mechanical) and the
[Seed3 carrier skeleton](hardware/pcb/seed3-carrier) for current mechanical
work.

This repository contains a static browser configurator, simulated device, and
Seed3 USB Audio + WebHID firmware for configurable four-slot hardware.

## Development

Requires Node.js 22 or newer and pnpm 11.9. The complete setup command also
fetches the verified libDaisy revision and builds it with the installed ARM GCC
toolchain.

```sh
make develop
pnpm dev
```

Open the URL printed by Vite in Chrome. The configurator attempts a LineRack
WebHID handshake and edits real hardware when it succeeds; simulator fallback
remains available. It can edit chains, apply drafts, activate slots, and export
or import a complete setup as JSON. Individual presets can also be imported,
downloaded, and loaded into the selected slot from a built-in recipe library;
these operations change only the browser draft until Apply to device is used.

Beta configuration targets desktop Chromium with WebHID. A VIA-like hosted
configurator and landing/product page are planned for Cloudflare; mobile
configuration is deliberately deferred. Playback does not require the browser
after presets are stored on the device.

```sh
make lint
make check
make test
make build
```

## Seed3 firmware

Current firmware source provides 48 kHz stereo USB playback through configurable
Gain, repeatable bell EQ, high-pass, low-pass, noise-gate, compressor, and
compact stereo reverb, and Limiter blocks; USB host volume and mute control;
device-wide USB, analog, and mixed-source routing; four-slot gesture control;
WebHID RPC; and CRC-checked A/B QSPI preset storage.
Each chain has fixed Gain and Limiter endpoints with eight ordered effect slots
between them. See the
[prototype bring-up guide](docs/prototype-bring-up.md) for wiring, toolchain,
build, flash, and first-power instructions.

## Current boundary

- Four preset slots
- Gain, repeatable parametric EQ, nine-band graphic EQ, high-pass, low-pass,
  crossfeed, compressor, noise gate, delay, reverb, and limiter browser
  definitions; current firmware source advertises Gain, repeatable bell EQ,
  both pass filters, noise gate, compressor, reverb, and Limiter
- Per-block EQ/filter response with a secondary dashed net-chain curve, five
  graphic-EQ recipes, and exact 128x32 preset/EQ/level display preview
- Device-wide persisted display preference with a five-second Preset override
  after an active-preset change; the first Visualizer is a live stereo level meter
- Device-wide USB, analog, and mixed-source selection with independent source
  trims; mixed mode caps each source at -6.5 dB before the DSP chain
- Versioned complete-setup and single-preset JSON import/export, plus ten
  editable factory recipes for everyday, movie, spoken-word, and spatial use
- Separate Global and Presets workspaces with collapsible panels, device-wide
  Apply access, a prominent starter setup, and three to ten parameter recipes
  for every processor
- Local-file audio audition through the host-selected LineRack output; files
  remain in the browser and are not uploaded
- Simulated and WebHID device capabilities, storage, activation, and preset cycling
- Fixed Gain-first and Limiter-last endpoints, each independently bypassable,
  plus eight drag/drop effect positions
- Versioned CBOR-over-HID control protocol and browser device adapter

See the [HID protocol reference](docs/hid-protocol.md) for the firmware-facing
wire format. `CAFE:4C52` is temporary development identity and must not ship.
The development firmware derives its USB serial from the STM32 unique ID. See
[How to test LineRack on iPhone](docs/how-to-test-on-iphone.md) for the verified
phone playback procedure and deferred mobile-configuration boundary.
See [How to set up LineRack development](docs/how-to-set-up-development.md) for
the contributor toolchain, verification, and firmware build sequence.

Hardware `Hello`, preset reads/writes, activation/events, audio under repeated
control writes, QSPI reload after power cycling, and the six-algorithm firmware
image pass on macOS. The current 109,592-byte LTO production build is flashed on
the test Seed3. It adds stereo-linked compressor, compact FDN reverb,
active/staging processor banks, and UAC1 host volume control under the 124 KiB
flash gate. Mac playback, host volume/mute, all four listening presets, WebHID,
persistence, OLED, intended button, and reconnect checks pass together. The
preceding image passed iPhone playback and bus power; combined-image iPhone
volume testing remains. Physical volume buttons on LineRack are not planned.
Current source adds live USB packet, buffer-fill, underrun, and overrun
diagnostics to the HID status response and configurator; its 109,824-byte target
build and native tests pass, but those counters still require a physical flash test.
Invalid-write target testing, host
sleep/wake, Windows/Linux coverage, custom-domain deployment, routed PCB
sources, additional firmware effects, arbitrary executable plug-ins, and
impulse-response transfer remain incomplete.

## License

Except where a file says otherwise, LineRack software, firmware, documentation,
and hardware design files are available under the
[PolyForm Noncommercial License 1.0.0](LICENSE). You may use, modify, build, and
share LineRack for personal and other noncommercial purposes. Selling LineRack
devices or using this work commercially requires a separate written license
from 1kbgz. Third-party components remain under their own licenses.

This summary is informational. The terms in [LICENSE](LICENSE) control.
