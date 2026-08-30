import { describe, expect, it } from "vitest";
import { createPresetFromRecipe, presetLibrary } from "./preset-library";
import { createDefaultPresetFile, findCompatibilityIssues, pluginCatalog } from "./presets";

const currentFirmwarePlugins = new Set([
  "gain",
  "parametric-eq",
  "high-pass",
  "low-pass",
  "noise-gate",
  "compressor",
  "reverb",
  "limiter",
]);

describe("preset library", () => {
  it("provides distinct recipes that fit current firmware", () => {
    expect(new Set(presetLibrary.map((recipe) => recipe.id)).size).toBe(presetLibrary.length);
    expect(new Set(presetLibrary.map((recipe) => recipe.name)).size).toBe(presetLibrary.length);

    for (const recipe of presetLibrary) {
      const presetFile = createDefaultPresetFile();
      presetFile.slots[0] = createPresetFromRecipe(recipe, 1);

      expect(new TextEncoder().encode(recipe.name).length).toBeLessThanOrEqual(12);
      expect(
        presetFile.slots[0].plugins.every((plugin) => currentFirmwarePlugins.has(plugin.pluginId)),
      ).toBe(true);
      expect(findCompatibilityIssues(presetFile)).toEqual([]);

      for (const block of recipe.blocks) {
        const definition = pluginCatalog.find((plugin) => plugin.id === block.pluginId);
        expect(definition, block.pluginId).toBeDefined();
        for (const [parameterId, value] of Object.entries(block.parameters ?? {})) {
          const parameter = definition?.parameters.find(
            (candidate) => candidate.id === parameterId,
          );
          expect(parameter, `${block.pluginId}.${parameterId}`).toBeDefined();
          expect(value).toBeGreaterThanOrEqual(parameter?.min ?? Number.POSITIVE_INFINITY);
          expect(value).toBeLessThanOrEqual(parameter?.max ?? Number.NEGATIVE_INFINITY);
        }
      }
    }
  });

  it("creates fresh plugin instance identifiers each time", () => {
    const first = createPresetFromRecipe(presetLibrary[0], 1);
    const second = createPresetFromRecipe(presetLibrary[0], 1);

    expect(first.plugins.map((plugin) => plugin.id)).not.toEqual(
      second.plugins.map((plugin) => plugin.id),
    );
  });
});
