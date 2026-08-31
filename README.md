# LineRack

LineRack is a USB-powered audio processor with browser-configurable effect
chains. The repository contains the configurator, Seed3 firmware, hardware
designs, and developer documentation for the working prototype.

LineRack is under active development. The current hardware is suitable for
development and listening tests, not production or safety-critical use.

## Repository map

| Path | Contents |
| --- | --- |
| `src/` | React configurator, simulator, and documentation site |
| `firmware/` | Seed3 diagnostics and composite USB Audio/WebHID firmware |
| `firmware/dsp/` | Host-tested signal-processing implementation |
| `firmware/usb/` | USB transport, preset storage, display, and protocol logic |
| `hardware/` | Development BOMs, enclosure sources, and carrier-PCB work |
| `docs/` | Tutorials, how-to guides, references, and architecture explanations |

## Start developing

Install Node.js 22 or newer, pnpm 11.9, GNU Make, Git, and the Arm GNU
toolchain. Then prepare the checkout:

```sh
make develop
```

Run the configurator locally:

```sh
pnpm dev
```

Open the printed origin at `/configure`.

Run the complete verification lifecycle:

```sh
make lint
make check
make test
make build
```

See [How to set up LineRack development](docs/how-to-set-up-development.md) for
prerequisites, expected outputs, and firmware flashing.

## Developer documentation

### Tutorial

- [Build and verify the Seed3 prototype](docs/prototype-bring-up.md) follows one
  complete path from an unwired board to integrated USB Audio, WebHID, DSP,
  button, display, and persisted presets.

### How-to guides

- [Set up LineRack development](docs/how-to-set-up-development.md)
- [Verify USB Audio on Seed3](docs/usb-audio-bring-up.md)
- [Test LineRack on iPhone](docs/how-to-test-on-iphone.md)
- [Print the development enclosure](docs/how-to-print-prototype-enclosure.md)
- [Update from the repository template](docs/how-to-update-template.md)
- [Develop a new DSP plugin](docs/how-to-develop-dsp-plugin.md)
- [Deploy the website](docs/how-to-deploy-website.md)

### Reference

- [HID control protocol](docs/hid-protocol.md)
- [Preset file formats](docs/preset-file-format.md)
- [Prototype hardware](docs/prototype-hardware-reference.md)
- [Firmware build targets](docs/firmware-build-targets.md)
- [Web deployment](docs/deployment-reference.md)

### Explanation

- [Why Seed3 can handle analog input](docs/analog-input-direction.md)
- [Why DSP and display processing stay separate](docs/dsp-and-display-direction.md)

The same pages are published at [linerack.dev/docs](https://linerack.dev/docs).

## Implementation boundary

Current composite firmware provides 48 kHz stereo USB Audio playback, UAC1 host
volume and mute, WebHID configuration, four persisted presets, one-button
control, and a 128×32 OLED. Supported firmware blocks are Gain, repeatable
parametric EQ, high-pass, low-pass, noise gate, compressor, compact stereo
reverb, and Limiter. Gain and Limiter are fixed chain endpoints with eight
ordered effect positions between them.

Development USB identity `CAFE:4C52` is temporary and must not ship. Firmware
builds enforce a 126,976-byte internal-flash gate. Production USB identity,
headphone-output safety, analog-input protection, compliance, and firmware
update design remain open engineering work.

## License

Except where a file says otherwise, LineRack software, firmware, documentation,
and hardware design files are available under the
[PolyForm Noncommercial License 1.0.0](LICENSE). Personal and other
noncommercial use, modification, building, and sharing are permitted.
Commercial use or resale requires a separate written license from 1kbgz.
Third-party components remain under their own licenses.

This summary is informational. The terms in [LICENSE](LICENSE) control.
