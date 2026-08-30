# How to verify USB Audio on Seed3

LineRack's intended audio source is USB Audio Class playback, not an analog
input or a generated test tone. The first target is a class-compliant stereo
playback device that receives PCM from a computer or phone, processes it on
Seed3, and sends it to the onboard codec outputs.

## Initial format

- USB Audio Class 1.0 playback device;
- stereo output from host to LineRack;
- 48 kHz fixed sample rate;
- signed 16-bit interleaved PCM;
- full-speed USB on the Seed3 onboard USB-C connector;
- bus powered;
- one master Feature Unit for host volume and mute control;
- no capture endpoint or sample-rate switching.

This format needs a fixed 192-byte isochronous OUT packet each 1 ms. It is a
bring-up format, not the final product-quality decision. Evaluate 24-bit PCM
only after stable playback and control traffic are proven.

## Playback image

Build the current enumeration-only image with:

```sh
make -C firmware -f Makefile.usb-audio
```

After placing Seed3 in DFU mode, flash it with:

```sh
make -C firmware -f Makefile.usb-audio program-dfu
```

The composite Audio + HID image is built with:

```sh
make -C firmware -f Makefile.usb-composite
```

It remains separate from the audio-only diagnostic target because both
Makefiles share `firmware/build`; clean before switching targets. The composite
image implements LineRack RPC and QSPI preset storage.

The image buffers USB PCM in D2 SRAM before the codec callback consumes it. Its
LED blinks slowly before USB configuration, stays on after configuration, and
blinks rapidly if USB startup fails. macOS lists `LineRack USB Audio Dev` as a
two-channel, 48 kHz output device.

The first physical playback test passed on 2026-08-24: macOS enumerated
`CAFE:4C52`, selected stereo 48 kHz output, and familiar program audio played
through both codec outputs with correct mapping, pitch, and continuity.

The follow-up DSP test also passed on hardware. D10 cycled flat, bass boost,
mid cut, and presence boost presets while real USB program audio continued
cleanly. All four responses were distinct and preset switching behaved as
expected. A longer run eventually triggered the rapid buffer-error LED, proving
that the fixed one-input-frame-per-output-frame consumer does not tolerate the
independent USB and codec clocks. The adaptive linear-resampling consumer
adjusts input consumption by at most 1,000 ppm around the ring's target fill.
It subsequently ran roughly one to two hours on target without the rapid
ring-error blink or a reported playback fault, passing the endurance gate.
The USB descriptor declares the playback endpoint adaptive (`bmAttributes =
0x09`) to match this clock-reconciliation model. iPhone selected the stream but
sent no packets when the endpoint incorrectly declared no synchronization.

## Implementation direction

Current libDaisy does not expose USB Audio through its public USB wrapper. Its
vendored ST USB Device middleware does contain a UAC1 stereo speaker class at
48 kHz/16-bit, plus Custom HID and a composite-device builder. Use that UAC1
class for the first enumeration and playback proof. Keep TinyUSB as the fallback
if ST's audio class or composite builder blocks reliable playback plus HID.

The first data path is:

```text
USB isochronous OUT packets
  -> D2 SRAM stereo PCM ring buffer
  -> sample conversion to float
  -> fixed Gain -> up to 8 effects -> fixed Limiter
  -> Seed3 codec callback
  -> analog stereo output
```

USB and codec audio clocks are independent. A ring buffer alone only delays an
eventual underrun or overrun. Instrument buffer fill, underruns, overruns, and
packet counts from the first streaming build. Stable playback requires either
an asynchronous feedback endpoint or a bounded adaptive-resampling strategy;
occasional unreported sample drops or duplicates are not an acceptable final
solution.

Current firmware-source effects are repeatable parametric EQ, 12/24 dB/octave
high-pass and low-pass, stereo-linked noise gate and compressor, and compact
stereo FDN reverb. Gain and Limiter are fixed at the first and last positions;
both can be bypassed. A UAC1 Feature Unit applies host volume and mute after the
complete preset chain. USB callbacks store integer requests; gain calculation
and application stay in the audio callback. Current source builds to 109,864
bytes under its 126,976-byte build gate and has not yet been flashed. Preceding
hardware builds pass Mac playback, host volume/mute, compressor, reverb, WebHID,
and preset switching together. A preceding image passed iPhone playback;
combined-image iPhone host-volume testing remains.

## Ordered checks

1. Enumerate as one bus-powered UAC1 playback device on macOS. **Passed.**
2. Accept a 48 kHz/16-bit stereo stream and count packets. **Passed.**
3. Start codec output only after the ring buffer reaches its target fill.
   **Passed.**
4. Play known left/right material through the DSP chain. **Passed with four
   switch-selected presets.**
5. Run at least 30 minutes while recording fill range and error counters.
   **Passed for roughly one to two hours without error indication.**
6. Exercise stop, start, host sleep, cable removal, and sample-format rejection.
   **Cable removal/reconnect passed; other cases remain.**
7. Add vendor-defined HID and repeat streaming tests during control traffic.
   **Passed on macOS: Audio, IOHID, Chrome discovery, read/write RPC, activation,
   status events, six repeated writes during playback, and QSPI reload.**
8. Test Windows, Linux, and a representative iPhone USB host setup.
   **Passed on iPhone: bus power, production Audio+HID playback, effects, and
   preset changes work with an adaptive isochronous endpoint. Windows and Linux
   remain untested.**

## Composite control result

The current Custom HID interface uses 64-byte unnumbered reports, OUT endpoint
`0x02`, and IN endpoint `0x81`. An initial `0x82` IN assignment transmitted the
first response report but never completed the multi-report sequence on Seed3.
Endpoint `0x81` uses the dedicated USB-FS endpoint-1 IN interrupt. A native
macOS probe then received all 18 chunks of the 913-byte `Hello` response, and
the browser completed `Hello` plus `ReadPresets` and entered hardware editing
mode.

The endpoint-1 target checks passed on 2026-08-25:

1. Two-channel 48 kHz CoreAudio enumeration after the endpoint change.
2. Familiar stereo playback while reading and writing presets.
3. Browser rename, audible parameter edit, storage, and activation.
4. Physical-switch DSP selection and `StatusChanged` synchronization.
5. QSPI preset reload after USB power cycling.
6. Six consecutive writes without gaps, timeout, error blink, or playback fault.

## How to recover audio after a SoundSource reconnect

Rogue Amoeba SoundSource retained a stale mute route during one USB reconnect,
even though LineRack re-enumerated and WebHID editing worked. macOS output
switching and resetting Seed3 did not restore that route.

If LineRack appears in CoreAudio and Chrome but produces no output after a USB
reconnect:

1. Quit SoundSource completely from its menu-bar command.
2. Test direct playback to `LineRack USB Audio Dev`.
3. Relaunch SoundSource and select LineRack again.

Direct playback and a fresh SoundSource route passed without reflashing or
changing the persisted setup. A subsequent unplug/replug with SoundSource
running restored Chrome hardware editing and audio normally. Diagnose this
host-routing case before changing firmware.

Older builds reused one serial and device release across different USB
descriptors, which could leave macOS with a stale route. Current source derives
the serial from the STM32 unique ID and increments `bcdDevice` when descriptors
change. If a stale route remains, select another output, disconnect LineRack for
ten seconds, reconnect, select LineRack again, and restart playback before
changing firmware.

## Success criteria

- Host selects LineRack as a stereo output without a custom driver.
- Left and right remain correctly mapped.
- Playback has no audible periodic ticks, gaps, speed drift, or pitch drift.
- Ring-buffer fill remains bounded with zero underruns and overruns.
- DSP and HID traffic do not destabilize the audio stream.
- Disconnect and reconnect recover without reflashing or entering DFU.

## USB identity

Local development uses the explicitly approved temporary identity
`CAFE:4C52`. It must not ship. Production hardware requires an owned or
properly allocated VID/PID.
