import { describe, expect, it } from "vitest";
import { effectPresets, matchingEffectPreset } from "./effect-presets";
import { pluginCatalog } from "./presets";

describe("effect presets", () => {
  it("provides three to ten valid presets for every processor", () => {
    for (const plugin of pluginCatalog) {
      const presets = effectPresets[plugin.id];
      expect(presets, plugin.id).toBeDefined();
      expect(presets.length, plugin.id).toBeGreaterThanOrEqual(3);
      expect(presets.length, plugin.id).toBeLessThanOrEqual(10);
      expect(new Set(presets.map((preset) => preset.id)).size, plugin.id).toBe(presets.length);
      expect(new Set(presets.map((preset) => preset.name)).size, plugin.id).toBe(presets.length);

      for (const preset of presets) {
        expect(Object.keys(preset.parameters).sort(), preset.name).toEqual(
          plugin.parameters.map((parameter) => parameter.id).sort(),
        );
        for (const parameter of plugin.parameters) {
          const value = Number(preset.parameters[parameter.id]);
          expect(value, `${plugin.id}.${preset.id}.${parameter.id}`).toBeGreaterThanOrEqual(
            parameter.min,
          );
          expect(value, `${plugin.id}.${preset.id}.${parameter.id}`).toBeLessThanOrEqual(
            parameter.max,
          );
          const steps = (value - parameter.min) / parameter.step;
          expect(
            Math.abs(steps - Math.round(steps)),
            `${plugin.id}.${preset.id}.${parameter.id} must align to its step`,
          ).toBeLessThan(0.0001);
        }
      }
    }
  });

  it("recognizes exact parameter matches and leaves edits custom", () => {
    expect(matchingEffectPreset("gain", { gainDb: -3 })).toBe("headroom");
    expect(matchingEffectPreset("gain", { gainDb: -3.00000001 })).toBe("headroom");
    expect(matchingEffectPreset("gain", { gainDb: -2.5 })).toBe("");
  });
});
