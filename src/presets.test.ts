import { describe, expect, it } from "vitest";
import {
  createSharedPresetFile,
  createDefaultPresetFile,
  createPluginInstance,
  defaultPresetLimits,
  findCompatibilityIssues,
  findPluginDefinition,
  normalizePresetFile,
  parseSharedPresetFile,
  parsePresetFile,
  validatePresetFile,
} from "./presets";

describe("preset files", () => {
  it("creates four valid slots", () => {
    const presetFile = createDefaultPresetFile();

    expect(presetFile.slots).toHaveLength(4);
    expect(presetFile.display).toEqual({ defaultMode: "preset", blankingEnabled: true });
    expect(presetFile.routing).toEqual({ sourceMode: "usb", usbTrimDb: 0, analogTrimDb: 0 });
    for (const slot of presetFile.slots) {
      expect(slot.plugins[0].pluginId).toBe("gain");
      expect(slot.plugins.at(-1)?.pluginId).toBe("limiter");
    }
    expect(validatePresetFile(presetFile)).toEqual([]);
  });

  it("provides four distinct listening recipes", () => {
    const presetFile = createDefaultPresetFile();

    expect(presetFile.slots.map((slot) => slot.name)).toEqual([
      "Clean",
      "Punch",
      "Small Room",
      "Wide Hall",
    ]);
    expect(presetFile.slots.map((slot) => slot.plugins.map((plugin) => plugin.pluginId))).toEqual([
      ["gain", "parametric-eq", "limiter"],
      ["gain", "parametric-eq", "compressor", "limiter"],
      ["gain", "compressor", "reverb", "limiter"],
      ["gain", "compressor", "reverb", "limiter"],
    ]);
  });

  it("round-trips through portable JSON", () => {
    const presetFile = createDefaultPresetFile();

    expect(parsePresetFile(JSON.stringify(presetFile))).toEqual(presetFile);
  });

  it("round-trips one shareable preset without its device slot", () => {
    const presetFile = createDefaultPresetFile();
    const shared = createSharedPresetFile(presetFile.slots[1], presetFile.engine);

    expect(parseSharedPresetFile(JSON.stringify(shared))).toEqual(shared);
    expect(shared.preset).not.toHaveProperty("number");
  });

  it("rejects complete setup files when importing one preset", () => {
    expect(() => parseSharedPresetFile(JSON.stringify(createDefaultPresetFile()))).toThrow(
      "Unsupported preset format",
    );
  });

  it("loads legacy files without display settings as Preset mode", () => {
    const presetFile = createDefaultPresetFile();
    const legacy = { ...presetFile, display: undefined, routing: undefined };

    expect(parsePresetFile(JSON.stringify(legacy)).display.defaultMode).toBe("preset");
    expect(normalizePresetFile(legacy).display.defaultMode).toBe("preset");
    expect(normalizePresetFile(legacy).display.blankingEnabled).toBe(true);
    expect(normalizePresetFile(legacy).routing).toEqual({
      sourceMode: "usb",
      usbTrimDb: 0,
      analogTrimDb: 0,
    });
  });

  it("rejects unsupported display modes", () => {
    const presetFile = createDefaultPresetFile() as unknown as Record<string, unknown>;
    presetFile.display = { defaultMode: "scope" };

    expect(validatePresetFile(presetFile)).toContain(
      "Display default mode must be preset, eq-response, or visualizer",
    );
  });

  it("rejects malformed documents", () => {
    expect(() => parsePresetFile('{"format":"linerack-presets"}')).toThrow(
      "Unsupported schema version",
    );
  });

  it("reports plugins that the current device does not support", () => {
    const presetFile = createDefaultPresetFile();
    presetFile.slots[0].plugins.splice(1, 0, {
      id: "future",
      pluginId: "future-plugin",
      pluginVersion: 1,
      enabled: true,
      parameters: {},
    });

    expect(findCompatibilityIssues(presetFile)).toEqual([
      "Slot 1: unsupported plugin future-plugin",
    ]);
  });

  it("defines the filter and noise-gate parameter sets", () => {
    expect(findPluginDefinition("high-pass")?.parameters.map((parameter) => parameter.id)).toEqual([
      "cutoffHz",
      "slopeDbPerOct",
    ]);
    expect(findPluginDefinition("low-pass")?.parameters.map((parameter) => parameter.id)).toEqual([
      "cutoffHz",
      "slopeDbPerOct",
    ]);
    expect(findPluginDefinition("noise-gate")?.parameters.map((parameter) => parameter.id)).toEqual(
      ["thresholdDb", "attackMs", "holdMs", "releaseMs", "rangeDb"],
    );
  });

  it("accepts multiple parametric EQ instances in simulator-compatible files", () => {
    const presetFile = createDefaultPresetFile();
    const definition = findPluginDefinition("parametric-eq")!;
    presetFile.slots[0].plugins.splice(
      -1,
      0,
      createPluginInstance(definition),
      createPluginInstance(definition),
    );

    expect(findCompatibilityIssues(presetFile)).toEqual([]);
  });

  it("requires fixed gain and limiter endpoints but permits bypassing either", () => {
    const presetFile = createDefaultPresetFile();
    presetFile.slots[0].plugins[0].enabled = false;
    presetFile.slots[0].plugins.at(-1)!.enabled = false;
    expect(findCompatibilityIssues(presetFile)).toEqual([]);

    const [gain] = presetFile.slots[1].plugins.splice(0, 1);
    presetFile.slots[1].plugins.splice(1, 0, gain);
    expect(findCompatibilityIssues(presetFile)).toContain("Slot 2: input gain must be first");
  });

  it("accepts eight middle effects and rejects a ninth", () => {
    const presetFile = createDefaultPresetFile();
    const definition = findPluginDefinition("parametric-eq")!;
    presetFile.slots[0].plugins.splice(1, presetFile.slots[0].plugins.length - 2);
    presetFile.slots[0].plugins.splice(
      1,
      0,
      ...Array.from({ length: 8 }, () => createPluginInstance(definition)),
    );
    expect(findCompatibilityIssues(presetFile)).toEqual([]);

    presetFile.slots[0].plugins.splice(-1, 0, createPluginInstance(definition));
    expect(findCompatibilityIssues(presetFile)).toContain("Slot 1: maximum is 10 plugin instances");
  });

  it("requires device slot numbers rather than any unique positive numbers", () => {
    const presetFile = createDefaultPresetFile();
    presetFile.slots.forEach((slot) => {
      slot.number += 4;
    });

    expect(findCompatibilityIssues(presetFile)).toContain(
      "Device requires preset slot numbers 1 through 4",
    );
  });

  it("measures device preset names in UTF-8 bytes", () => {
    const presetFile = createDefaultPresetFile();
    presetFile.slots[0].name = "🎧🎧🎧🎧";

    expect(findCompatibilityIssues(presetFile)).toContain(
      "Slot 1: name must be at most 12 UTF-8 bytes",
    );
  });

  it("restricts device preset names to the shared display charset", () => {
    const presetFile = createDefaultPresetFile();
    presetFile.slots[0].name = "Café";

    expect(findCompatibilityIssues(presetFile)).toContain(
      "Slot 1: name may contain only letters, numbers, spaces, period, question mark, and hyphen",
    );
  });

  it("requires safe source trims before enabling mixed routing", () => {
    const presetFile = createDefaultPresetFile();
    presetFile.routing.sourceMode = "mix";
    expect(findCompatibilityIssues(presetFile)).toContain(
      "Mixed routing requires USB and analog trims at or below -6.5 dB",
    );

    presetFile.routing.usbTrimDb = -6.5;
    presetFile.routing.analogTrimDb = -6.5;
    expect(findCompatibilityIssues(presetFile)).toEqual([]);
  });

  it("validates source trim range and resolution", () => {
    const presetFile = createDefaultPresetFile();
    presetFile.routing.analogTrimDb = -6.25;

    expect(validatePresetFile(presetFile)).toContain(
      "analogTrimDb must be between -24 and 0 dB in 0.5 dB steps",
    );
  });

  it("rejects disabling blanking on devices without that capability", () => {
    const presetFile = createDefaultPresetFile();
    presetFile.display.blankingEnabled = false;

    expect(
      findCompatibilityIssues(presetFile, {
        ...defaultPresetLimits,
        displayBlanking: false,
      }),
    ).toContain("Device does not support disabling display blanking");
  });
});
