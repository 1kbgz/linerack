import {
  createConfiguredPluginInstance,
  ParameterValue,
  PluginInstance,
  PresetSlot,
} from "./presets";

interface RecipeBlock {
  pluginId: string;
  parameters?: Record<string, ParameterValue>;
}

export interface PresetRecipe {
  id: string;
  category: "Everyday" | "Movies" | "Spoken word" | "Space";
  name: string;
  description: string;
  blocks: RecipeBlock[];
}

const chain = (...effects: RecipeBlock[]): RecipeBlock[] => [
  { pluginId: "gain" },
  ...effects,
  { pluginId: "limiter" },
];

export const presetLibrary: PresetRecipe[] = [
  {
    id: "balanced",
    category: "Everyday",
    name: "Balanced",
    description: "Flat EQ with unity gain and a final limiter.",
    blocks: chain({ pluginId: "parametric-eq" }),
  },
  {
    id: "bass-cut",
    category: "Everyday",
    name: "Bass Cut",
    description: "Reduces deep bass and lowers the 110 Hz region by 4 dB.",
    blocks: chain(
      { pluginId: "high-pass", parameters: { cutoffHz: 35, slopeDbPerOct: 12 } },
      {
        pluginId: "parametric-eq",
        parameters: { frequencyHz: 110, gainDb: -4, q: 0.7 },
      },
    ),
  },
  {
    id: "late-night",
    category: "Everyday",
    name: "Late Night",
    description: "Adds bass and presence with moderate compression for quiet listening.",
    blocks: chain(
      {
        pluginId: "parametric-eq",
        parameters: { frequencyHz: 100, gainDb: 2, q: 0.7 },
      },
      {
        pluginId: "parametric-eq",
        parameters: { frequencyHz: 3500, gainDb: 2, q: 0.8 },
      },
      {
        pluginId: "compressor",
        parameters: { thresholdDb: -24, ratio: 3, attackMs: 15, releaseMs: 180 },
      },
    ),
  },
  {
    id: "treble-tamer",
    category: "Everyday",
    name: "Treble Tamer",
    description: "Cuts 6.5 kHz by 3 dB and rolls off above 18 kHz.",
    blocks: chain(
      {
        pluginId: "parametric-eq",
        parameters: { frequencyHz: 6500, gainDb: -3, q: 0.8 },
      },
      { pluginId: "low-pass", parameters: { cutoffHz: 18000, slopeDbPerOct: 12 } },
    ),
  },
  {
    id: "movie",
    category: "Movies",
    name: "Movie",
    description: "Adds low-end weight and dialogue presence with moderate compression.",
    blocks: chain(
      {
        pluginId: "parametric-eq",
        parameters: { frequencyHz: 90, gainDb: 2, q: 0.8 },
      },
      {
        pluginId: "parametric-eq",
        parameters: { frequencyHz: 2800, gainDb: 2.5, q: 0.9 },
      },
      {
        pluginId: "compressor",
        parameters: { thresholdDb: -20, ratio: 2.5, attackMs: 20, releaseMs: 220 },
      },
    ),
  },
  {
    id: "dialogue",
    category: "Movies",
    name: "Dialogue",
    description: "Cuts rumble and low mids. Raises the speech range.",
    blocks: chain(
      { pluginId: "high-pass", parameters: { cutoffHz: 65, slopeDbPerOct: 12 } },
      {
        pluginId: "parametric-eq",
        parameters: { frequencyHz: 180, gainDb: -2, q: 0.8 },
      },
      {
        pluginId: "parametric-eq",
        parameters: { frequencyHz: 2600, gainDb: 3, q: 0.9 },
      },
      {
        pluginId: "compressor",
        parameters: { thresholdDb: -22, ratio: 3, attackMs: 10, releaseMs: 160 },
      },
    ),
  },
  {
    id: "podcast-raw",
    category: "Spoken word",
    name: "Podcast Raw",
    description: "Cleans up untreated speech input with a gate, filter, EQ, and compressor.",
    blocks: chain(
      { pluginId: "high-pass", parameters: { cutoffHz: 75, slopeDbPerOct: 12 } },
      {
        pluginId: "noise-gate",
        parameters: {
          thresholdDb: -48,
          attackMs: 2,
          holdMs: 80,
          releaseMs: 180,
          rangeDb: -50,
        },
      },
      {
        pluginId: "parametric-eq",
        parameters: { frequencyHz: 3000, gainDb: 2.5, q: 0.8 },
      },
      {
        pluginId: "compressor",
        parameters: { thresholdDb: -20, ratio: 3.5, attackMs: 8, releaseMs: 140 },
      },
    ),
  },
  {
    id: "podcast-play",
    category: "Spoken word",
    name: "Podcast Play",
    description: "Raises vocal clarity and evens out recorded speech.",
    blocks: chain(
      {
        pluginId: "parametric-eq",
        parameters: { frequencyHz: 220, gainDb: -1.5, q: 0.8 },
      },
      {
        pluginId: "parametric-eq",
        parameters: { frequencyHz: 2800, gainDb: 2, q: 0.9 },
      },
      {
        pluginId: "compressor",
        parameters: { thresholdDb: -22, ratio: 2.5, attackMs: 12, releaseMs: 180 },
      },
    ),
  },
  {
    id: "live-room",
    category: "Space",
    name: "Live Room",
    description: "Adds a small bass lift and 13% room reverb.",
    blocks: chain(
      {
        pluginId: "parametric-eq",
        parameters: { frequencyHz: 140, gainDb: 1.5, q: 0.8 },
      },
      { pluginId: "reverb", parameters: { size: 45, damping: 55, mix: 13 } },
    ),
  },
  {
    id: "wide-hall",
    category: "Space",
    name: "Wide Hall",
    description: "Adds compression and 28% dark hall reverb.",
    blocks: chain(
      {
        pluginId: "compressor",
        parameters: { thresholdDb: -20, ratio: 2.5, attackMs: 25, releaseMs: 250 },
      },
      { pluginId: "reverb", parameters: { size: 80, damping: 40, mix: 28 } },
    ),
  },
];

export const createPresetFromRecipe = (recipe: PresetRecipe, number: number): PresetSlot => ({
  number,
  name: recipe.name,
  plugins: recipe.blocks.map(({ pluginId, parameters }): PluginInstance =>
    createConfiguredPluginInstance(pluginId, parameters),
  ),
});
