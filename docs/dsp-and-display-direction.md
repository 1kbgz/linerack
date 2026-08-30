# DSP and display direction

This document explains how LineRack can grow from fixed Gain and Limiter
endpoints plus eight effect positions into a useful portable effects platform
without turning every UI variation into a separate firmware algorithm. It also
separates sound processing from display and analysis features so visual modes
cannot accidentally alter a preset.

## Two independent graphs

LineRack has two related but independent paths:

```text
audio -> fixed Gain -> ordered effects -> fixed Limiter -> output
  |
  +---- bounded analysis taps -> display mode
```

The DSP chain owns sound. A device preset stores its ordered blocks and their
parameters. The analysis path observes audio or configured filter coefficients.
A display mode selects how those observations appear. Changing from a preset
screen to a spectrum or loudness screen must not change the DSP chain.

Product language should call these **display modes** or **monitor modes**, not
“line mode.” Line already means line-level analog audio and would make adapter
and routing documentation ambiguous.

This distinction also makes resource accounting clearer. An effect declares
audio callback CPU, state memory, scratch memory, latency, and tail time. An
analysis mode declares capture work, background processing, display bandwidth,
and refresh rate. Firmware can reject combinations that exceed either budget.

## Processing-engine foundations

More algorithms should follow a common versioned block contract rather than add
one-off preset fields. Each block needs:

- a stable algorithm identifier and schema version;
- typed parameters with units, ranges, defaults, and display scale;
- instance-count, state-memory, scratch-memory, CPU, and latency costs;
- bypass behavior and parameter-smoothing rules;
- stereo-link and channel-layout behavior;
- reset, tail, and preset-transition behavior;
- factory recipes represented as ordinary editable parameter values.

The chain validator remains authoritative. It accounts for the complete chain
before activation and preserves Gain-first and Limiter-last positions. Both
endpoints are bypassable, but bypassing Limiter explicitly removes the output
ceiling. Parameter updates need bounded ramps; structural changes need a short crossfade or another
click-safe bank transition. Reverb and delay tails need an explicit policy:
truncate, preserve until silent, or crossfade between old and new processors.

The current ten-block source image occupies 109,824 bytes with link-time
optimization, leaving 17,152 bytes below the enforced 126,976-byte gate. It includes
compressor, reverb, active/staging processor banks, USB host volume control, and
live audio diagnostics. The preceding 108,416-byte image is hardware-verified on
Mac; the 109,592-byte button, Visualizer, and volume-overlay image is flashed and
awaiting user confirmation. The gate reserves at least 4 KiB below the 131,072-byte
linker boundary.

The 2026-08-30 resource profile also shows why flash and effect state need
separate treatment:

| Resource | Current use | Relevant implication |
| --- | ---: | --- |
| Internal-flash image | 109,824 bytes | 17,152 bytes remain below the enforced gate; a growing catalogue still needs bootloader/QSPI execution |
| Internal SRAM and D2 RAM | 76,980 bytes | USB/HID buffers dominate, but capacity remains |
| Active/staging chain state | 4,528 bytes | Four presets remain stored, but only two processor chains are live |
| External SDRAM | 640 KiB of 64 MiB | Twenty per-block reverb workspaces support repeated instances across active/staging chains |
| CBOR application object before LTO | 7,565 bytes of code | Capability metadata has a real cost but is not the first optimization target |

The runtime retains four stored presets but builds only an active and a staging
processor chain. Preset activation configures the staging chain outside the
audio callback and atomically swaps it in. This reduced chain state before
adding large-state effects and gives tail-bearing processors one explicit
transition boundary. It does not solve flash growth; the supported Daisy
bootloader and QSPI/SRAM execution path remains the durable answer to the
STM32H750 internal-flash ceiling.

## Equalization

The first major editor should be a full-spectrum EQ. One scalable biquad-bank
engine can support simple and advanced interfaces without separate 3-, 5-, 7-,
and 9-band algorithms.

### Graphic view

A graphic mode exposes fixed logarithmic centers. Nine bands provide a useful
default set:

```text
63, 125, 250, 500, 1k, 2k, 4k, 8k, 16k Hz
```

Three-, five-, and seven-control layouts can expose subsets or grouped controls
while compiling to the same underlying filters. This gives a simple visual
editor and recognizable recipes such as Flat, Bass Lift, Vocal Presence,
Treble Reduce, and Loudness without multiplying firmware implementations.

### Parametric view

An advanced mode exposes up to a capability-declared number of bands. Each band
has frequency, gain, Q, enabled state, and filter type. Initial types should be
bell, low shelf, and high shelf; high-pass and low-pass can share the response
renderer but remain separate chain blocks when their slope and resonance need
independent control.

The browser draws each block's response on a logarithmic 20 Hz–20 kHz axis as
the primary solid curve. When other enabled EQ/filter blocks exist, a lighter
dashed curve shows net chain response. Numeric controls remain available for
exact editing. Firmware stores coefficients or parameters, not browser pixels.
Recipes copy parameters into the editable band list.

The OLED can show a simplified configured response derived from coefficients.
That graph requires no live FFT and is therefore the cheapest analysis mode.

The browser prototype implements the nine fixed centers, per-block and net
log-frequency response graphs, and Flat, Bass, Clarity, Bright, and Loudness
recipes in simulator capabilities. The connected Seed3 does not advertise the
nine-band block; firmware instead supports repeatable bell EQ instances. The
same response math drives a 128-point monochrome OLED preview, which keeps the
editor and future device rendering aligned.

Repeated parametric instances combine in net response. Firmware and simulator
capabilities include fixed-Butterworth high-pass and low-pass blocks with 12 or
24 dB/octave slopes; both feed the per-block/net graph and monochrome preview.

## Filters and utility processing

Utility blocks solve routing, compatibility, and correction problems before
creative effects are added. The initial set should include:

- input trim and output trim;
- balance, channel swap, left-only, right-only, and mono sum;
- left/right polarity inversion;
- stereo width using mid/side gain;
- DC blocking;
- high-pass and low-pass filters with cutoff, resonance, and 12 or 24 dB/octave
  slope;
- low and high shelves;
- headphone crossfeed;
- per-channel delay for alignment;
- mute and fade behavior used by transitions rather than exposed as arbitrary
  unsafe discontinuities.

Static loudness is a utility/tone block: bass and treble contours plus
compensating gain. Playback-level-aware equal-loudness correction is different.
It needs a trustworthy output-level model and should not be claimed until the
analog output and headphone load are characterized.

## Dynamics and output protection

The current source implements the existing browser contract: stereo-linked
threshold, ratio, attack, and release. It uses a peak detector, hard-knee gain
computer, and shared channel gain. Native tests cover static ratio, stereo
linking, attack/release timing, validation, and repeated instances. Target
tests must still cover bypass, parameter extremes, repeated-instance callback
CPU, and listening behavior before compressor support is considered verified.

Knee, makeup gain, wet/dry mix, sidechain high-pass filtering, auto release,
auto makeup, and a gain-reduction meter are useful follow-ons. Lookahead is
optional because it adds declared latency and memory.

The output stage should evolve separately:

- current stereo-linked sample-peak limiter;
- smoothed attack/release and transition behavior;
- optional soft clipper before the limiter;
- oversampled or true-peak protection only after CPU and latency measurements;
- output peak and limiter gain-reduction telemetry.

Noise gate/expander, de-esser, and multiband compression belong after the basic
compressor and EQ engine are measured. A multiband processor combines filters,
dynamics, crossovers, and latency, so it should not be the first dynamics block.

The stereo-linked firmware and simulator noise gate expose threshold, attack,
hold, release, and attenuation range. It does not contribute to the EQ graph
because its transfer depends on input level rather than frequency alone.

## Reverb

A useful embedded reverb should resemble the control range and recipe-driven
workflow of Apple's matrix reverb without trying to reproduce Apple's private
algorithm. Apple's public parameter set includes dry/wet mix, pre-delay,
small/large size, density, brightness, delay ranges, small/large balance, and
modulation rate/depth. See Apple's
[AUMatrixReverb parameter reference](https://developer.apple.com/documentation/audiotoolbox/1389801-aumatrixreverb-parameters).

The first target implementation should be a compact stereo feedback-delay
network with its delay storage in external SDRAM. It should initially expose
the existing browser parameters—size, damping, and wet/dry mix—and allow one
instance per preset. One active and one staging network make preset changes
atomic without allocating a delay network for every stored preset. Native
impulse-decay and stability tests come before target CPU, SDRAM,
tail-transition, and listening tests.

The comprehensive version can add early reflections and modulation. Its
expanded user-facing parameters should be:

- dry/wet mix;
- pre-delay;
- size;
- decay time;
- damping or brightness;
- diffusion or density;
- early/late reflection mix;
- stereo width;
- modulation rate and depth.

Factory recipes should cover Small Room, Medium Room, Chamber, Plate, Hall,
Large Hall, Cathedral, and Ambient. These mirror familiar room families—Apple
also exposes room, chamber, hall, plate, and cathedral presets—but remain
editable LineRack values. See Apple's
[room-type reference](https://developer.apple.com/documentation/audiotoolbox/aureverbroomtype).

The browser now includes ten starter recipes spanning neutral playback, bass
and treble reduction, low-level listening, movies, dialogue, untreated and
recorded podcast speech, moderate live-room ambience, and a larger hall. A recipe replaces only the
selected slot and remains an unapplied draft until explicitly stored. The same
slot can be exported as a versioned `linerack-preset` JSON file and imported
into any slot, independently of complete four-slot setup files. Exact
SoundSource-derived recipes should be added only after their parameter values
are captured; matching names or subjective descriptions is not sufficient.

Convolution reverb is a later, separate block. It needs impulse transfer,
partitioned convolution, larger memory, file validation, and a latency policy.
Algorithmic reverb gives a broader first product with smaller configurations.

## Time, modulation, and creative effects

After core correction and mastering blocks, the catalogue can expand in groups:

| Family | Candidate blocks |
| --- | --- |
| Time | Stereo delay, ping-pong delay, tape-style delay, early reflections |
| Modulation | Chorus, flanger, phaser, tremolo, auto-pan |
| Pitch/time | Detune, pitch shift, octave; only after latency/CPU proof |
| Saturation | Tape/tube-style saturation, overdrive, soft clip, hard clip, fuzz |
| Lo-fi | Bit crusher, sample-rate reducer, bandwidth/telephone recipes |
| Stereo | Crossfeed, width, Haas delay with mono-compatibility guardrails |
| Mastering | Loudness contour, compressor, maximizer, optional multiband dynamics |

Each group should land only after native response tests, target CPU/memory
measurements, transition tests, and capability reporting exist. “Available in
the browser” must always mean “compiled and budgeted on the connected device.”

## Display and analysis modes

The default screen remains preset number plus short name. Optional modes are:

| Mode | Source | Meaning and cost |
| --- | --- | --- |
| Preset | Device state | Number, short name, bypass/write status |
| EQ response | Filter coefficients | Configured response; low cost, no live analysis |
| Spectrum | Decimated audio FFT | Log-frequency energy; medium background cost |
| Peak/RMS meter | Callback accumulators | Immediate level and headroom; low cost |
| Loudness | K-weighted history | Momentary/short-term LUFS first; gating and integrated history add complexity |
| Gain reduction | Compressor/limiter telemetry | Dynamics activity; low incremental cost |
| Stereo field | L/R statistics | Correlation meter or compact vectorscope |
| Oscilloscope | Decimated sample window | Waveform view; bounded capture buffer |
| Visualizer | Per-callback stereo output peaks | Live L/R amplitude meters first; beat and tempo tracking later |
| Diagnostic | USB/DSP counters | Ring fill, overruns, underruns, CPU, temperature where available |

Analysis in the audio callback should be limited to constant-time accumulators
and bounded decimation into a lock-free buffer. FFT, loudness history, beat
detection, and drawing belong outside the callback. The 128x32 SSD1306 has only
512 display bytes, but a full I2C frame still consumes meaningful bus time;
roughly 10–15 frames per second is enough for these modes.

The browser and composite firmware now render fixture-identical 128x32 one-bit
frames for Preset, configured EQ-response, and stereo-level Visualizer modes.
Preset view includes large slot number, 12-character name area, enabled-block
count, and active/edit state.
The composite sends the packed 512-byte framebuffer to the wired SSD1306 at
address `0x3C` over 400 kHz I2C. Native tests cover initialization, page
addressing, and exact frame bytes. The physical display is correctly aligned
and follows preset changes. Refresh timing and live telemetry remain unmeasured.

The browser treats Preset, EQ Response, or Visualizer as a device-wide preferred
mode. An active-preset change temporarily shows Preset for five seconds, then
returns to that preference. The device Visualizer uses live post-chain stereo
peaks; the browser preview uses simulated levels because HID does not stream
audio telemetry. The preference persists in JSON, CBOR, and QSPI, and the
override runs in composite firmware.

OLED lifetime also favors motion and blanking. The display should dim or blank
after inactivity while processing continues. Current firmware defaults to
turning the panel off after 20 seconds. Physical preset presses, successful
Apply operations, and local web edits wake it and restart that timer; edit wakes
are throttled and do not apply the draft. The web app can disable blanking for
users who accept a prominent burn-in warning. Screen savers can be useful, but
must not hide clipping, persistence failure, or another safety condition.

## Device interaction

The source interaction maps a single tap to display wake, a quick double tap to
the next preset, and a 5–10 second hold to the next device-wide display mode.
A hold of at least 10 seconds is reserved and currently only wakes the display.
Single-tap action waits 350 ms so firmware can distinguish it from a double tap.
The intended momentary button and basic debounce behavior pass; the gesture
mapping still needs a physical flash test. The web app configures the
persisted default mode and previews the five-second preset-change override.
It also persists whether 20-second inactivity blanking is enabled. Configurable
timeout length and additional enabled mode lists remain future work.

Host volume or mute changes temporarily replace any display mode with a large
percentage and bar (or `MUTE`) for two seconds, then return to the prior mode.

Display mode is device-wide in the first design. Per-preset display modes add
configuration and interaction complexity without improving audio. A later
product can revisit that choice if users consistently want a particular meter
attached to a particular preset.
