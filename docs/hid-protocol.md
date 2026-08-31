# LineRack HID control protocol

## Scope

This protocol transfers configuration and status data between a LineRack device
and a host configurator. USB Audio Class carries audio separately. Protocol
version 1 uses CBOR messages split across fixed-size USB HID reports.
Objects are encoded as standard CBOR maps; implementation-specific record or
string-packing extensions are not permitted.

## HID interface

| Property | Value |
| --- | --- |
| Usage page | `0xFF00` (vendor-defined) |
| Usage | `0x01` |
| Report ID | None (`0` in WebHID calls and events) |
| Input report | 64 octets |
| Output report | 64 octets |
| Interrupt IN endpoint | `0x81` on current Seed3 composite firmware |
| Interrupt OUT endpoint | `0x02` on current Seed3 composite firmware |

The report descriptor deliberately omits a report ID. Full-speed USB interrupt
endpoints allow at most 64 octets per packet; a numbered 64-octet report would
require an invalid 65-octet packet once its report ID is included.

The USB vendor ID and product ID are not assigned by this protocol. Host
software must supply an explicit device filter containing the IDs selected for
the development board or production hardware.

The endpoint addresses are implementation details rather than framing fields.
Seed3 requires HID IN on endpoint 1 for reliable completion dispatch; endpoint
`0x82` delivered only the first report of a multi-report response during target
testing.

## Report frame

All multibyte integers use little-endian byte order.

| Offset | Size | Field | Value |
| ---: | ---: | --- | --- |
| 0 | 2 | Magic | ASCII `LR` (`0x4C 0x52`) |
| 2 | 1 | Protocol version | `1` |
| 3 | 1 | Message kind | See message kinds |
| 4 | 1 | Command | See commands |
| 5 | 1 | Reserved | `0` when sent; ignored when received |
| 6 | 2 | Request ID | Host-assigned correlation ID |
| 8 | 2 | Chunk index | Zero-based index |
| 10 | 1 | Chunk count | Total reports in logical message |
| 11 | 1 | Payload length | CBOR bytes present in this report, `0`–`52` |
| 12 | 52 | Payload area | CBOR fragment followed by zero padding |

A logical message contains between 1 and 255 reports. Its maximum encoded CBOR
payload is 13,260 octets. All reports in one message have identical message
kind, command, request ID, and chunk count. The receiver assembles fragments by
chunk index and decodes CBOR only after every fragment arrives.

Current Seed3 firmware accepts request payloads up to the framing maximum and
allocates 8,192 octets for an encoded response. A response that exceeds that
buffer is replaced by an error response.

## Message kinds

| Value | Name | Direction | Meaning |
| ---: | --- | --- | --- |
| 1 | Request | Host to device | Command invocation |
| 2 | Response | Device to host | Successful result |
| 3 | Event | Device to host | Unsolicited state change |
| 4 | Error | Device to host | Rejected or failed request |

A response or error repeats its request's command and request ID. Request ID
`0` is reserved for events. Host requests use IDs `1` through `65535` and must
not reuse an ID while its request is outstanding. The browser client serializes
commands because current firmware processes one request transaction at a time.
Firmware abandons an incomplete request after approximately three seconds of
inactivity so a lost HID fragment cannot wedge later commands.

An error payload is a CBOR map with a required text `message` field:

```text
{ "message": "Preset is incompatible" }
```

## Commands

Payload type names refer to the TypeScript definitions in `src/device.ts` and
`src/presets.ts`. Empty requests encode an empty CBOR map.

| Value | Command | Request payload | Response payload |
| ---: | --- | --- | --- |
| 1 | `Hello` | `{ client: string, protocolVersion: number }` | `{ capabilities: DeviceCapabilities, status: DeviceStatus }` |
| 2 | `ReadPresets` | `{}` | `PresetFile` |
| 3 | `WritePresets` | `PresetFile` | Stored `PresetFile` |
| 4 | `ActivateSlot` | `{ slotNumber: number }` | `DeviceStatus` |
| 5 | `GetStatus` | `{}` | `DeviceStatus` |
| 6 | `CyclePreset` | `{}` | `DeviceStatus` |
| 7 | `StatusChanged` | Not a request | `DeviceStatus` event |
| 8 | `WakeDisplay` | `{}` | `DeviceStatus` |

`DeviceStatus` contains `connected`, `activeSlot`, and an optional live
`diagnostics` map with `usbPackets`, `bufferFillFrames`, `underruns`, and
`overruns`. Diagnostic counters reset on device restart. Clients should tolerate
firmware that omits this additive field.

`WritePresets` is atomic at the logical-message level. Firmware validates the
complete document before replacing stored presets. It returns the stored
document on success or an error without changing the prior document.

`CyclePreset` follows the same slot order as a physical-button double tap. The
device emits `StatusChanged` whenever the active slot changes, including changes
caused by the physical button. Command responses still contain the resulting
status so they do not depend on event delivery.

`WakeDisplay` restarts the OLED inactivity timer without changing or persisting
the setup. Firmware advertises support as `DeviceCapabilities.displayWake`.
The browser coalesces rapid edits before sending it so slider drags cannot flood
the serialized HID request queue.

## Device data

`DeviceStatus` has this shape:

```text
{
  "connected": true,
  "activeSlot": 1
}
```

`DeviceCapabilities` declares product and firmware identity plus preset limits
and available plugin definitions. `PresetFile` carries engine metadata, a
device-wide display preference, and the four named slots with their ordered
plugin instances:

```text
{
  "format": "linerack-presets",
  "schemaVersion": 1,
  "engine": { "sampleRate": 48000, "channels": 2 },
  "display": { "defaultMode": "preset", "blankingEnabled": true },
  "routing": {
    "sourceMode": "usb",
    "usbTrimDb": 0,
    "analogTrimDb": 0
  },
  "slots": [ ... ]
}
```

`display.defaultMode` is `preset`, `eq-response`, or `visualizer`.
`display.blankingEnabled` defaults to `true`; current firmware turns the OLED
off after 20 seconds and wakes it on a physical button press or successful
`WritePresets`. When `displayWake` is supported, browser edits also wake it
without applying the draft. The browser treats a missing `display` map or blanking field
from preceding protocol-v1 firmware as Preset mode with blanking enabled, so it
can connect to and upgrade that hardware. Firmware advertises disable support as
`DeviceCapabilities.displayBlanking` and must reject other documents
incompatible with its declared capabilities.

`routing.sourceMode` is `usb`, `analog`, or `mix`. Trims range from -24 to
0 dB in 0.5 dB steps. Mixed mode requires both trims at or below -6.5 dB.
Firmware advertises supported values in `DeviceCapabilities.sourceModes`.
The browser treats a missing `routing` map from older protocol-v1 firmware as
USB-only with 0 dB trims.

Current firmware-source limits are:

| Property | Value |
| --- | ---: |
| Preset slots | 4 |
| Maximum name storage | 12 supported ASCII characters plus terminator |
| Blocks per preset | 10 total |
| Audio format | 48 kHz, two channels |

Each preset contains exactly one Gain at index 0 and exactly one Limiter at the
final index. Up to eight middle blocks may be reordered or repeated. Gain and
Limiter may be disabled but cannot be removed or moved. The device validates
all four presets before storing or activating a replacement bank.

### Plugin contract limits

| Property | Current limit |
| --- | ---: |
| Decoded plugin ID | 19 CBOR text bytes |
| Plugin version | `1` |
| Parameters stored per block | 5 |
| Decoder parameter-ID mask | 16 bits; 15 IDs assigned |
| Plugin instances per preset | 10, including Gain and Limiter |

The parameter-ID mask is shared across the firmware catalogue, not reset per
plugin. Unknown IDs, versions, and parameter names are rejected.

Advertised firmware plugins are:

| Plugin ID | Parameters |
| --- | --- |
| `gain` | `gainDb` (-24 to +12 dB) |
| `parametric-eq` | `frequencyHz` (20–20,000 Hz), `gainDb` (-18 to +18 dB), `q` (0.1–18) |
| `high-pass` | `cutoffHz` (20–20,000 Hz), `slopeDbPerOct` (12 or 24) |
| `low-pass` | `cutoffHz` (20–20,000 Hz), `slopeDbPerOct` (12 or 24) |
| `noise-gate` | `thresholdDb` (-80–0 dB), `attackMs` (0.1–100), `holdMs` (0–500), `releaseMs` (5–2,000), `rangeDb` (-80–0 dB) |
| `compressor` | `thresholdDb` (-60–0 dB), `ratio` (1–20), `attackMs` (0.1–200), `releaseMs` (10–2,000) |
| `reverb` | `size` (0–100%), `damping` (0–100%), `mix` (0–100%) |
| `limiter` | `ceilingDb` (-12–0 dB) |

All current plugin definitions use version `1`. Unknown plugin IDs, versions,
parameters, duplicate endpoint plugins, misplaced endpoints, out-of-range
values, and chains longer than ten blocks are rejected.
