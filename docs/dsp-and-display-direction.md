# How DSP and display state fit together

LineRack has two related but independent graphs: an audio graph that must meet
real-time deadlines, and a display graph that observes state without blocking
audio. Keeping that boundary explicit lets the browser, firmware, and future
hardware evolve without turning UI work into audio risk.

## Two data paths

```text
USB or analog audio
  -> source routing and trim
  -> fixed Gain
  -> up to eight ordered effects
  -> fixed Limiter
  -> host volume and mute
  -> codec output

audio meters + active preset + host volume + display preference
  -> bounded snapshots
  -> display renderer
  -> 128×32 OLED
```

The audio callback performs no HID parsing, flash writes, I2C transfers, or
dynamic allocation. Control code validates a complete setup and prepares a
staging processor bank. Firmware swaps that bank at a safe boundary instead of
mutating active processors during playback.

## A versioned effect contract

Each plugin instance stores an ID, plugin version, bypass state, and named
parameters. The device advertises its supported plugins and limits through the
HID capability response. The browser uses that response as authoritative.

This contract separates three concerns:

- Firmware implements deterministic, bounded processors.
- The configurator edits chains and validates device compatibility.
- Preset recipes provide useful parameter values without becoming new DSP
  types.

A listening label such as “movie” or “live room” is therefore a recipe built
from ordinary filters, dynamics, and reverb. Firmware does not need a special
movie-mode processor.

Current Seed3 firmware advertises Gain, Parametric EQ, High-pass, Low-pass,
Noise Gate, Compressor, Reverb, and Limiter. Gain and Limiter are fixed at the
chain endpoints and may be bypassed. The eight middle positions may contain
repeated or reordered supported effects.

## Resource model

The main constraints differ by resource:

| Resource | Constraint | Design response |
| --- | --- | --- |
| Internal flash | Composite image must remain within 126,976 bytes | Keep the build gate; move to the supported bootloader/SRAM path before exceeding it |
| Real-time CPU | Every stereo frame has a fixed deadline | Use bounded algorithms and measure complete worst-case chains |
| Processor state | Active and staging chains coexist during updates | Allocate fixed banks once; swap prepared state |
| Delay memory | Reverb needs much more history than filters | Keep delay lines in external SDRAM with fixed per-block workspaces |
| USB clock drift | Host and codec clocks are independent | Bound ring fill with adaptive resampling |

This model favors algorithmic reverb over convolution for the current device.
Convolution would add impulse transfer, storage, partitioned FFT work, and a
larger compatibility surface. It can remain a separate future processor rather
than complicating the compact reverb.

## Filter and dynamics behavior

Parametric EQ and pass filters use direct-form biquads with coefficients
prepared outside the audio callback. Repeated Parametric EQ instances provide
multiple bands without a separate fixed-band firmware type. High-pass and
low-pass filters support 12 or 24 dB/octave slopes by changing cascade depth.

Gate and compressor detectors are stereo linked so one channel cannot open or
compress independently and pull the image sideways. Limiter remains the final
safety ceiling. Bypass must preserve a stable chain and avoid discontinuities
when practical.

## Analysis and rendering

Visualization is lower priority than uninterrupted audio. Audio code publishes
small, bounded snapshots such as peak level, spectrum bins, active preset, and
host volume. Display code consumes the latest complete snapshot and may skip a
frame when busy.

The 128×32 OLED supports three persistent modes:

- **Preset**: slot number and short name
- **EQ Response**: current chain response
- **Visualizer**: live left and right level meters

Temporary events override the persistent mode. A preset change shows Preset;
a host volume change shows its percentage. After the timeout, the configured
mode returns. Display blanking is device-wide, defaults to 20 seconds, and
wakes on button input, host volume, browser edits, or an applied setup.

OLED I2C work occurs outside the audio callback. Display failure must not stop
audio or control traffic.

## Why configuration stays in the browser

The hosted configurator owns interaction-heavy work: arranging processors,
drawing response previews, managing recipes, and importing or exporting setup
files. Firmware owns validation, persistence, processing, and simple device UI.
This keeps the embedded surface small while allowing the interface to improve
without reflashing every unit.

The boundary is versioned rather than implicit. A setup is accepted only when
its engine format, chain shape, plugin versions, and parameter values match the
connected device's advertised capabilities.

## Related documentation

- [HID control protocol](hid-protocol.md)
- [Preset file format](preset-file-format.md)
- [Analog input architecture](analog-input-direction.md)
- [Verify USB Audio on Seed3](usb-audio-bring-up.md)
