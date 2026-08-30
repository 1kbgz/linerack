import { describe, expect, it } from "vitest";
import {
  calculateEqResponse,
  frequencyAtPosition,
  GRAPHIC_EQ_BANDS,
  GRAPHIC_EQ_RECIPES,
} from "./eq";
import type { PluginInstance } from "./presets";

const plugin = (pluginId: string, parameters: Record<string, number>): PluginInstance => ({
  id: "test",
  pluginId,
  pluginVersion: 1,
  enabled: true,
  parameters,
});

describe("EQ response", () => {
  it("uses a logarithmic frequency axis", () => {
    expect(frequencyAtPosition(0)).toBe(20);
    expect(frequencyAtPosition(0.5)).toBeCloseTo(Math.sqrt(20 * 20000));
    expect(frequencyAtPosition(1)).toBeCloseTo(20000);
  });

  it("models a parametric band's center gain", () => {
    const response = calculateEqResponse(
      [plugin("parametric-eq", { frequencyHz: 1000, gainDb: 9, q: 1 })],
      48000,
      1001,
    );
    const center = response.reduce((nearest, point) =>
      Math.abs(point.frequencyHz - 1000) < Math.abs(nearest.frequencyHz - 1000) ? point : nearest,
    );

    expect(center.gainDb).toBeCloseTo(9, 1);
  });

  it("combines multiple parametric EQ instances", () => {
    const response = calculateEqResponse(
      [
        plugin("parametric-eq", { frequencyHz: 1000, gainDb: 6, q: 1 }),
        plugin("parametric-eq", { frequencyHz: 1000, gainDb: 3, q: 1 }),
      ],
      48000,
      1001,
    );
    const center = response.reduce((nearest, point) =>
      Math.abs(point.frequencyHz - 1000) < Math.abs(nearest.frequencyHz - 1000) ? point : nearest,
    );

    expect(center.gainDb).toBeCloseTo(9, 1);
  });

  it("models high-pass and low-pass rolloff", () => {
    const highPass = calculateEqResponse(
      [plugin("high-pass", { cutoffHz: 1000, slopeDbPerOct: 12 })],
      48000,
      1001,
    );
    const lowPass = calculateEqResponse(
      [plugin("low-pass", { cutoffHz: 1000, slopeDbPerOct: 12 })],
      48000,
      1001,
    );

    expect(highPass[0].gainDb).toBeLessThan(-40);
    expect(highPass.at(-1)!.gainDb).toBeGreaterThan(-1);
    expect(lowPass[0].gainDb).toBeGreaterThan(-1);
    expect(lowPass.at(-1)!.gainDb).toBeLessThan(-40);
  });

  it("makes a 24 dB/octave filter steeper than a 12 dB/octave filter", () => {
    const twelve = calculateEqResponse(
      [plugin("high-pass", { cutoffHz: 1000, slopeDbPerOct: 12 })],
      48000,
      1001,
    );
    const twentyFour = calculateEqResponse(
      [plugin("high-pass", { cutoffHz: 1000, slopeDbPerOct: 24 })],
      48000,
      1001,
    );

    expect(twentyFour[0].gainDb).toBeLessThan(twelve[0].gainDb - 20);
  });

  it("keeps a flat graphic EQ at zero decibels", () => {
    const response = calculateEqResponse([plugin("graphic-eq", {})], 48000, 64);

    expect(response.every((point) => point.gainDb === 0)).toBe(true);
  });

  it("defines complete, in-range graphic EQ recipes", () => {
    for (const recipe of GRAPHIC_EQ_RECIPES) {
      expect(Object.keys(recipe.parameters)).toHaveLength(GRAPHIC_EQ_BANDS.length);
      expect(Object.values(recipe.parameters).every((gain) => gain >= -12 && gain <= 12)).toBe(
        true,
      );
    }
  });
});
