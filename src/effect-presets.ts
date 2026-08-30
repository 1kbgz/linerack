import { GRAPHIC_EQ_RECIPES } from "./eq";
import type { ParameterValue } from "./presets";

export interface EffectPreset {
  id: string;
  name: string;
  parameters: Record<string, ParameterValue>;
}

export const effectPresets: Record<string, EffectPreset[]> = {
  gain: [
    { id: "unity", name: "Unity", parameters: { gainDb: 0 } },
    { id: "headroom", name: "Headroom", parameters: { gainDb: -3 } },
    { id: "extra-headroom", name: "Extra headroom", parameters: { gainDb: -6 } },
  ],
  "parametric-eq": [
    {
      id: "flat",
      name: "Flat",
      parameters: { frequencyHz: 1000, gainDb: 0, q: 1 },
    },
    {
      id: "bass-lift",
      name: "Bass lift",
      parameters: { frequencyHz: 100, gainDb: 3, q: 0.7 },
    },
    {
      id: "bass-cut",
      name: "Bass cut",
      parameters: { frequencyHz: 120, gainDb: -4, q: 0.7 },
    },
    {
      id: "vocal-presence",
      name: "Vocal presence",
      parameters: { frequencyHz: 3000, gainDb: 2.5, q: 0.9 },
    },
    {
      id: "de-ess",
      name: "De-ess",
      parameters: { frequencyHz: 7000, gainDb: -3, q: 2.5 },
    },
  ],
  "graphic-eq": GRAPHIC_EQ_RECIPES.map((recipe) => ({
    id: recipe.name.toLowerCase(),
    name: recipe.name,
    parameters: recipe.parameters,
  })),
  "high-pass": [
    { id: "subsonic", name: "Subsonic", parameters: { cutoffHz: 25, slopeDbPerOct: 12 } },
    {
      id: "music-rumble",
      name: "Music rumble",
      parameters: { cutoffHz: 40, slopeDbPerOct: 12 },
    },
    { id: "dialogue", name: "Dialogue", parameters: { cutoffHz: 75, slopeDbPerOct: 12 } },
    {
      id: "strong-cleanup",
      name: "Strong cleanup",
      parameters: { cutoffHz: 100, slopeDbPerOct: 24 },
    },
  ],
  "low-pass": [
    {
      id: "ultrasonic",
      name: "Ultrasonic cleanup",
      parameters: { cutoffHz: 18000, slopeDbPerOct: 12 },
    },
    { id: "smooth", name: "Smooth", parameters: { cutoffHz: 14000, slopeDbPerOct: 12 } },
    { id: "lo-fi", name: "Lo-fi", parameters: { cutoffHz: 8000, slopeDbPerOct: 24 } },
    { id: "dark", name: "Dark", parameters: { cutoffHz: 5000, slopeDbPerOct: 24 } },
  ],
  crossfeed: [
    { id: "off", name: "Off", parameters: { amount: 0 } },
    { id: "subtle", name: "Subtle", parameters: { amount: 12 } },
    { id: "natural", name: "Natural", parameters: { amount: 25 } },
    { id: "strong", name: "Strong", parameters: { amount: 40 } },
  ],
  compressor: [
    {
      id: "light-glue",
      name: "Light glue",
      parameters: { thresholdDb: -14, ratio: 1.5, attackMs: 25, releaseMs: 250 },
    },
    {
      id: "listening",
      name: "Listening",
      parameters: { thresholdDb: -20, ratio: 2, attackMs: 15, releaseMs: 180 },
    },
    {
      id: "voice",
      name: "Voice",
      parameters: { thresholdDb: -22, ratio: 3, attackMs: 8, releaseMs: 140 },
    },
    {
      id: "punch",
      name: "Punch",
      parameters: { thresholdDb: -16, ratio: 4, attackMs: 20, releaseMs: 100 },
    },
    {
      id: "late-night",
      name: "Late night",
      parameters: { thresholdDb: -28, ratio: 4, attackMs: 10, releaseMs: 250 },
    },
  ],
  "noise-gate": [
    {
      id: "gentle",
      name: "Gentle",
      parameters: { thresholdDb: -60, attackMs: 2, holdMs: 100, releaseMs: 250, rangeDb: -20 },
    },
    {
      id: "room-noise",
      name: "Room noise",
      parameters: { thresholdDb: -50, attackMs: 2, holdMs: 80, releaseMs: 180, rangeDb: -40 },
    },
    {
      id: "speech",
      name: "Speech",
      parameters: { thresholdDb: -45, attackMs: 1, holdMs: 100, releaseMs: 160, rangeDb: -50 },
    },
    {
      id: "aggressive",
      name: "Aggressive",
      parameters: { thresholdDb: -35, attackMs: 1, holdMs: 50, releaseMs: 100, rangeDb: -70 },
    },
  ],
  delay: [
    { id: "slap", name: "Slap", parameters: { timeMs: 90, feedback: 12, mix: 10 } },
    { id: "short", name: "Short echo", parameters: { timeMs: 180, feedback: 20, mix: 15 } },
    { id: "echo", name: "Echo", parameters: { timeMs: 350, feedback: 32, mix: 20 } },
    { id: "long", name: "Long echo", parameters: { timeMs: 550, feedback: 42, mix: 24 } },
    { id: "ambient", name: "Ambient", parameters: { timeMs: 750, feedback: 55, mix: 30 } },
  ],
  reverb: [
    { id: "small-room", name: "Small room", parameters: { size: 25, damping: 65, mix: 8 } },
    { id: "live-room", name: "Live room", parameters: { size: 45, damping: 55, mix: 13 } },
    { id: "chamber", name: "Chamber", parameters: { size: 55, damping: 50, mix: 18 } },
    { id: "hall", name: "Hall", parameters: { size: 75, damping: 42, mix: 24 } },
    { id: "cathedral", name: "Cathedral", parameters: { size: 95, damping: 35, mix: 32 } },
  ],
  limiter: [
    { id: "transparent", name: "Transparent", parameters: { ceilingDb: -1 } },
    { id: "conservative", name: "Conservative", parameters: { ceilingDb: -2 } },
    { id: "headroom", name: "Extra headroom", parameters: { ceilingDb: -6 } },
  ],
};

export const matchingEffectPreset = (
  pluginId: string,
  parameters: Record<string, ParameterValue>,
): string =>
  effectPresets[pluginId]?.find((preset) =>
    Object.entries(preset.parameters).every(([key, value]) => {
      const current = parameters[key];
      return typeof current === "number" && typeof value === "number"
        ? Math.abs(current - value) < 0.0001
        : current === value;
    }),
  )?.id ?? "";
