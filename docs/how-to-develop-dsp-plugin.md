# How to develop a new DSP plugin

Add a compiled DSP plugin to Seed3 firmware, its HID capability contract, and
the browser configurator. LineRack does not dynamically load executable plugin
code; every device plugin is built into firmware.

## Prerequisites

- A checkout that passes [development setup](how-to-set-up-development.md)
- A stable plugin ID and numeric parameter contract
- A native test signal and measurable expected result
- Seed3 hardware for final playback and resource checks

Read the [DSP and display architecture](dsp-and-display-direction.md) before
adding an algorithm with large state, delay memory, or variable execution time.

## 1. Define the contract

Choose the public contract before implementing the processor:

- Use a stable, kebab-case plugin ID no longer than 19 ASCII bytes.
- Start a new plugin at version `1`. Current firmware rejects other versions.
- Define each parameter's ID, default, minimum, maximum, step, and optional unit.
- Keep the plugin at five parameters or fewer unless you first change
  `LINERACK_MAX_PARAMETERS` and the persisted block layout.
- Append new persisted enum values. Never renumber an existing
  `LineRackBlockType` value.

The CBOR decoder's parameter mask has 16 bits, with 15 parameter IDs currently
assigned. If the plugin needs more than one new parameter ID, expand
`ParameterSlot`, its mask type, and associated tests first. Reuse an existing
parameter ID only when its meaning and unit are identical. The
[HID control protocol](hid-protocol.md) is the authoritative limit reference.

## 2. Implement and test the processor

Add the processor to `firmware/dsp/processor.h`. Follow the existing processor
shape:

- `Configure(...)` calculates coefficients and resets state outside the audio
  callback.
- `Process(float& left, float& right)` updates stereo samples in place.
- State uses fixed storage; processing performs no allocation, I/O, logging, or
  unbounded work.
- Stereo-linked dynamics derive control gain from both channels.

Put large delay lines in external SDRAM rather than inside the processor object.
Both active and staging chains exist during a setup update, so budget state for
two complete worst-case chains.

Add focused cases to `firmware/dsp/processor_test.cpp`. Test bypass-equivalent
settings, expected gain or frequency response, stereo behavior, parameter
extremes, finite output, and decay or stability where applicable. Run:

```sh
make -C firmware/dsp test
```

## 3. Add the persisted block type

Update the firmware model:

1. Append the block type in `firmware/usb/preset_model.h`.
2. Validate every parameter range in `LineRackPresetBankValid()` in
   `firmware/usb/preset_model.c`.
3. Add a valid instance and out-of-range cases to
   `firmware/usb/preset_model_test.c`.
4. Change `LineRackPresetBankDefaults()` only if the plugin belongs in a default
   device preset.

If the change alters `LineRackBlock` or `LineRackPresetBank` size, treat it as a
persistence migration. Existing QSPI records with a different payload size are
rejected, so preserve or explicitly migrate stored setups before shipping the
change.

## 4. Wire the real-time chain

Extend `ChainProcessor` in `firmware/usb_audio.cpp`:

1. Add fixed processor storage for every possible block position.
2. Configure the matching processor in `ChainProcessor::Configure()`.
3. Process it in chain order in `ChainProcessor::Process()`.
4. Add external workspaces beside `reverb_memory` when the processor needs
   large state.

The enabled flag is handled by the chain. Do not add plugin-specific bypass
state unless the algorithm needs a click-safe transition.

If the plugin changes frequency response, extend `BlockMagnitudeDb()` in
`firmware/usb/display_model.c` and its cases in
`firmware/usb/display_model_test.c`. Otherwise the device's EQ Response mode
will omit the plugin.

## 5. Extend HID capabilities and preset CBOR

Update `firmware/usb/preset_cbor.cpp` in each of these places:

- Increase the `plugins` array count in `Capabilities()`.
- Emit one `PluginDefinition()` and its `NumberParameter()` entries.
- Encode parameters in `Parameters()`.
- Map the block type in `PluginId()`.
- Add new names to `ParameterSlot` and `FindParameter()`.
- Decode the exact parameter mask and block fields in `DecodePlugin()`.

Extend `firmware/usb/preset_cbor_test.cpp` with capability discovery and preset
round-trip coverage. Keep missing, duplicate, unknown, and out-of-range values
rejected. Run:

```sh
make -C firmware/usb test
```

## 6. Add browser support

Add the same contract to `pluginCatalog` in `src/presets.ts`. This catalogue
drives the simulator; attached hardware remains authoritative through its HID
capability response. The generic editor renders numeric parameters without a
plugin-specific component.

Add three to ten useful parameter presets in `src/effect-presets.ts`. The
existing effect-preset test enforces their count, uniqueness, parameter set,
range, and step alignment.

If the plugin changes frequency response, add its response calculation in
`src/eq.ts` and include its ID in the response sets in `src/eq-editor.tsx` and
`src/App.tsx`. Add a custom editor only when generic numeric sliders cannot
represent the contract.

Update affected tests in `src/presets.test.ts`, `src/effect-presets.test.ts`,
and `src/eq.test.ts`. Add listening recipes in `src/preset-library.ts` only when
the new plugin improves a complete preset.

## 7. Verify the complete implementation

Run all repository gates:

```sh
make lint
make check
make test
make build
```

Confirm the composite image remains within the 126,976-byte flash gate. Do not
raise the gate to make the plugin fit.

Flash Seed3 and verify the plugin with real program audio:

1. Confirm `Hello` advertises the plugin and exact parameter contract.
2. Add, configure, reorder, bypass, remove, apply, and reload the plugin.
3. Test multiple instances and a worst-case ten-block chain.
4. Apply parameter and structural changes during playback.
5. Confirm USB packet count advances and underrun/overrun counters remain zero.
6. For a frequency-shaping plugin, confirm OLED EQ Response includes it.
7. Power-cycle and confirm the setup reloads.

Update the plugin table in [HID control protocol](hid-protocol.md) after the
device and browser checks pass.

## Related documentation

- [DSP and display architecture](dsp-and-display-direction.md)
- [HID control protocol](hid-protocol.md)
- [Preset file format](preset-file-format.md)
- [Firmware build targets](firmware-build-targets.md)
- [Verify USB Audio on Seed3](usb-audio-bring-up.md)
