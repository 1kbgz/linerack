# Build and verify the Seed3 prototype

This tutorial builds one working LineRack prototype, from an unwired Seed3 to
USB Audio playback, browser configuration, physical control, and OLED output.

## Before you start

Complete [Set up LineRack development](how-to-set-up-development.md).
Keep the [prototype hardware reference](prototype-hardware-reference.md)
open for the verified parts list, pinout, and electrical limits.

Use powered speakers or an audio-interface line input at minimum volume for
initial output tests. Direct headphone testing does not establish a safe load,
clean output level, or short-circuit limit.

## 1. Verify the unwired Seed3

Leave all peripherals disconnected. Enter DFU by holding **BOOT**, pressing and
releasing **RESET**, then releasing **BOOT**. Flash the board diagnostic:

```sh
dfu-util --list
make -C firmware -f Makefile.diagnostics program-dfu
```

The onboard LED should blink twice per second. Press and release **RESET** once
if it does not start after flashing.

`dfu-util` 0.11 on macOS may report `Error during download get_status` after
`File downloaded successfully`. Treat the transfer as successful only when the
expected firmware behavior follows.

## 2. Wire the prototype

Disconnect USB power. Wire the button, OLED, and stereo output exactly as shown
in the [prototype hardware reference](prototype-hardware-reference.md).
Use Seed3's printed pin labels, not breadboard row numbers.

Inspect solder joints and confirm no power rail is shorted to ground before
reconnecting USB.

## 3. Verify stereo output

Enter DFU and flash the output diagnostic:

```sh
make -C firmware -f Makefile.output-test program-dfu
```

The onboard LED should remain solid. The output should carry a quiet 440 Hz
tone on the left and 660 Hz tone on the right, each near -30 dBFS. Confirm both
channels and their mapping before continuing.

## 4. Flash the integrated firmware

Download the current setup before flashing firmware that changes
`LineRackPresetBank`. QSPI records with a different payload size are rejected
and replaced with defaults.

Enter DFU again, then build and flash the composite image:

```sh
make -C firmware -f Makefile.usb-composite clean
make -C firmware -f Makefile.usb-composite -j4
make -C firmware -f Makefile.usb-composite program-dfu
```

The host should list `LineRack USB Audio Dev` as a two-channel, 48 kHz output.
Select it, start familiar program audio at low volume, and confirm clean left
and right playback.

## 5. Connect the configurator

Run the local site with `pnpm dev`, open its origin at `/configure` in desktop
Chrome, and select the LineRack device when prompted. The configurator should
report hardware editing instead of simulator editing.

Change one audible parameter and apply the setup. Playback should continue and
the effect should change without a USB reconnect.

## 6. Verify physical controls and display

Check each user-facing interaction:

1. Single-press the button. The OLED should wake.
2. Quickly double-press it. The active preset should advance and briefly appear
   on both the OLED and browser preview.
3. Hold it for 5–10 seconds. The persistent display mode should cycle among
   Preset, EQ Response, and Visualizer.
4. Change host volume. The OLED should show the new percentage temporarily.
5. Apply another browser edit. The OLED should wake.

## 7. Verify persistence

Rename a preset, apply the setup, and disconnect USB for five seconds. Reconnect
the device and confirm the name, effect values, display settings, and active
preset reload.

You now have a complete development prototype. Continue with the focused
[USB Audio verification](usb-audio-bring-up.md),
[iPhone test](how-to-test-on-iphone.md), or
[development enclosure guide](how-to-print-prototype-enclosure.md).
