# Firmware build targets

Seed3 firmware targets share the `firmware/build` directory. Run `clean` with
the target you are leaving before switching Makefiles:

```sh
make -C firmware -f Makefile.output-test clean
make -C firmware -f Makefile.usb-composite
```

Build a target with `make -C firmware -f <makefile>`. Add `program-dfu` to flash
an attached Seed3 already in DFU mode.

| Makefile | Image | Purpose |
| --- | --- | --- |
| `Makefile` | `linerack` | Legacy analog pass-through with button-selected OLED slot number |
| `Makefile.diagnostics` | `diagnostics` | Board-only LED diagnostic; no peripheral wiring required |
| `Makefile.output-test` | `output_test` | 440 Hz left and 660 Hz right codec-output tones |
| `Makefile.dsp-demo` | `dsp_demo` | Generated stereo tones through four Gain and Limiter settings |
| `Makefile.eq-demo` | `eq_demo` | Generated stereo tones through four Parametric EQ settings |
| `Makefile.headless` | `headless` | Analog stereo pass-through and LED slot feedback without OLED or USB control |
| `Makefile.usb-audio` | `usb_audio` | USB Audio playback without the WebHID composite interface |
| `Makefile.usb-composite` | `usb_composite` | Current integrated USB Audio, DSP, WebHID, storage, button, and OLED firmware |

`Makefile.usb-composite` is the release-development target and enforces the
126,976-byte internal-flash limit. Other images are diagnostics; they do not
define current device behavior or preset compatibility.

See [Build and verify the Seed3 prototype](prototype-bring-up.md)
for the ordered board, output, and integrated-image workflow.
