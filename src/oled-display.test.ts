import { describe, expect, it } from "vitest";
import { calculateEqResponse } from "./eq";
import {
  createOledFrame,
  OLED_HEIGHT,
  OLED_WIDTH,
  oledPixel,
  PRESET_CHANGE_DISPLAY_MS,
  resolveOledDisplayMode,
} from "./oled-display";
import type { PresetSlot } from "./presets";

const slot: PresetSlot = {
  number: 2,
  name: "Web Low",
  plugins: [
    { id: "gain", pluginId: "gain", pluginVersion: 1, enabled: true, parameters: {} },
    { id: "limiter", pluginId: "limiter", pluginVersion: 1, enabled: true, parameters: {} },
  ],
};

const frameHash = (pixels: Uint8Array): number => {
  let hash = 2166136261;
  for (const pixel of pixels) {
    hash = Math.imul(hash ^ pixel, 16777619) >>> 0;
  }
  return hash;
};

describe("OLED display model", () => {
  it("renders an exact 128 by 32 monochrome preset frame", () => {
    const frame = createOledFrame("preset", slot, true, []);

    expect(frame.width).toBe(OLED_WIDTH);
    expect(frame.height).toBe(OLED_HEIGHT);
    expect(frame.pixels).toHaveLength(OLED_WIDTH * OLED_HEIGHT);
    expect(frame.pixels.some((pixel) => pixel === 1)).toBe(true);
    expect([...frame.pixels].every((pixel) => pixel === 0 || pixel === 1)).toBe(true);
    expect(frameHash(frame.pixels)).toBe(3820898106);
  });

  it("matches the firmware EQ response fixture", () => {
    const bassSlot: PresetSlot = {
      number: 2,
      name: "Bass",
      plugins: [
        {
          id: "gain",
          pluginId: "gain",
          pluginVersion: 1,
          enabled: true,
          parameters: { gainDb: -3 },
        },
        {
          id: "eq",
          pluginId: "parametric-eq",
          pluginVersion: 1,
          enabled: true,
          parameters: { frequencyHz: 120, gainDb: 9, q: 0.8 },
        },
        {
          id: "limiter",
          pluginId: "limiter",
          pluginVersion: 1,
          enabled: true,
          parameters: { ceilingDb: -1 },
        },
      ],
    };
    const response = calculateEqResponse(bassSlot.plugins, 48000, OLED_WIDTH);

    expect(frameHash(createOledFrame("eq-response", bassSlot, true, response).pixels)).toBe(
      3633784874,
    );
  });

  it("draws positive EQ gain above the center line", () => {
    const frame = createOledFrame("eq-response", slot, true, [
      { frequencyHz: 20, gainDb: 0 },
      { frequencyHz: 1000, gainDb: 12 },
      { frequencyHz: 20000, gainDb: 0 },
    ]);

    expect(oledPixel(frame, 64, 7)).toBe(true);
  });

  it("renders stereo visualizer levels", () => {
    const frame = createOledFrame("visualizer", slot, true, [], { left: 25, right: 75 });

    expect(oledPixel(frame, 13, 5)).toBe(true);
    expect(oledPixel(frame, 50, 5)).toBe(false);
    expect(oledPixel(frame, 90, 22)).toBe(true);
  });

  it("shows preset after a preset change, then restores the preferred mode", () => {
    const changedAt = 1000;
    const displayUntil = changedAt + PRESET_CHANGE_DISPLAY_MS;

    expect(resolveOledDisplayMode("eq-response", displayUntil, changedAt)).toBe("preset");
    expect(resolveOledDisplayMode("eq-response", displayUntil, displayUntil - 1)).toBe("preset");
    expect(resolveOledDisplayMode("eq-response", displayUntil, displayUntil)).toBe("eq-response");
  });
});
