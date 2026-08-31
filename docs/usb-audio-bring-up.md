# How to verify USB Audio on Seed3

Verify playback, clock recovery, DSP, HID traffic, host volume, and reconnect
behavior on the integrated Seed3 firmware.

## Prerequisites

- A prototype that completed the [Seed3 tutorial](prototype-bring-up.md)
- A desktop host and known stereo test material
- Current firmware dependencies from `make develop-firmware`
- Powered speakers or an audio-interface line input at low volume

## Build and flash

Enter DFU and flash the composite Audio + HID image:

```sh
make -C firmware -f Makefile.usb-composite clean
make -C firmware -f Makefile.usb-composite -j4
make -C firmware -f Makefile.usb-composite program-dfu
```

The expected playback format is USB Audio Class 1, stereo, signed 16-bit PCM at
a fixed 48 kHz sample rate. The device is bus powered and exposes master volume
and mute controls. It has no capture endpoint or sample-rate switching.

## Verify enumeration and playback

Select `LineRack USB Audio Dev` as the host output. On macOS, inspect the device
with:

```sh
system_profiler SPAudioDataType
```

Play familiar stereo material and verify:

1. Left and right channels are correctly mapped.
2. Playback has no ticks, gaps, speed drift, or pitch drift.
3. Host volume and mute change the analog output.
4. The OLED briefly displays host volume as a percentage.
5. Distinct presets remain audible while switching slots.

## Verify stream diagnostics

Connect through desktop Chrome and expand **Global**, then **Diagnostics**.
During playback:

- `usbPackets` should increase continuously.
- `bufferFillFrames` should remain bounded near its operating target.
- `underruns` and `overruns` should remain zero.

Run playback for at least 30 minutes. The firmware reconciles independent USB
and codec clocks with bounded adaptive resampling; a ring buffer alone cannot
prevent eventual underflow or overflow.

## Exercise DSP and HID together

Keep audio playing while you:

1. Read the stored setup.
2. Apply at least six parameter changes.
3. Activate each preset from the browser.
4. Change presets with the physical button.
5. Exercise gain, filters, compressor, reverb, and limiter.

Audio should continue without a disconnect, buffer error, or stale browser
state. Power-cycle USB and confirm the applied setup reloads.

## Exercise host state changes

Verify stop and start, cable removal, reconnect, and host sleep and wake. After
each transition, confirm that USB Audio and WebHID return without reflashing or
entering DFU.

Repeat the playback test on Windows and Linux when those hosts are available.
Use the separate [iPhone test](how-to-test-on-iphone.md) for mobile playback.

## Troubleshoot zero output

If the device enumerates but produces no sound:

1. Confirm the global source mode is **USB** and USB trim is `0 dB`.
2. Confirm the host output route, volume, and mute state.
3. Check Diagnostics. A stationary `usbPackets` counter means the host is not
   sending audio; increasing packets with zero output points later in the path.
4. Quit audio-routing software such as SoundSource and test direct playback.
5. Select another host output, disconnect LineRack for ten seconds, reconnect,
   and select it again.
6. Press Seed3 **RESET** and repeat direct playback.
7. Reflash only after the same failure occurs without third-party routing
   software.

Older builds reused one serial and device release across descriptor changes,
which could leave macOS with a stale route. Current firmware derives the serial
from the STM32 unique ID and changes `bcdDevice` when descriptors change.

## Success criteria

- Host selects LineRack without a custom driver.
- Both channels remain correctly mapped.
- Ring-buffer fill stays bounded with zero underruns and overruns.
- DSP changes and HID traffic do not interrupt playback.
- Host volume, mute, disconnect, and reconnect work without reflashing.

Development firmware uses the temporary USB identity `CAFE:4C52`. Production
hardware requires an owned or allocated VID/PID.
