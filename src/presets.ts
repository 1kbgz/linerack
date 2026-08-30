import { GRAPHIC_EQ_BANDS } from "./eq";

export type ParameterValue = number | boolean | string;

export interface ParameterDefinition {
  id: string;
  label: string;
  kind: "number";
  default: number;
  min: number;
  max: number;
  step: number;
  unit?: string;
}

export interface PluginDefinition {
  id: string;
  name: string;
  version: number;
  description: string;
  parameters: ParameterDefinition[];
}

export interface PluginInstance {
  id: string;
  pluginId: string;
  pluginVersion: number;
  enabled: boolean;
  parameters: Record<string, ParameterValue>;
}

export interface PresetSlot {
  number: number;
  name: string;
  plugins: PluginInstance[];
}

export interface SharedPresetFile {
  format: "linerack-preset";
  schemaVersion: 1;
  engine: PresetFile["engine"];
  preset: Omit<PresetSlot, "number">;
}

export type DisplayMode = "preset" | "eq-response" | "visualizer";
export type AudioSourceMode = "usb" | "analog" | "mix";

export interface PresetFile {
  format: "linerack-presets";
  schemaVersion: 1;
  engine: {
    sampleRate: number;
    channels: number;
  };
  display: {
    defaultMode: DisplayMode;
    blankingEnabled: boolean;
  };
  routing: {
    sourceMode: AudioSourceMode;
    usbTrimDb: number;
    analogTrimDb: number;
  };
  slots: PresetSlot[];
}

export interface PresetLimits {
  slotCount: number;
  sampleRate: number;
  channels: number;
  maxPluginsPerSlot: number;
  presetNameMaxLength?: number;
  displayBlanking?: boolean;
  sourceModes?: AudioSourceMode[];
  plugins: PluginDefinition[];
}

const numberParameter = (
  id: string,
  label: string,
  defaultValue: number,
  min: number,
  max: number,
  step: number,
  unit?: string,
): ParameterDefinition => ({
  id,
  label,
  kind: "number",
  default: defaultValue,
  min,
  max,
  step,
  unit,
});

export const pluginCatalog: PluginDefinition[] = [
  {
    id: "gain",
    name: "Gain",
    version: 1,
    description: "Input headroom before the rest of the chain.",
    parameters: [numberParameter("gainDb", "Gain", 0, -24, 12, 0.5, "dB")],
  },
  {
    id: "parametric-eq",
    name: "Parametric EQ",
    version: 1,
    description: "One fully parametric bell filter. Add more instances for more bands.",
    parameters: [
      numberParameter("frequencyHz", "Frequency", 1000, 20, 20000, 1, "Hz"),
      numberParameter("gainDb", "Gain", 0, -18, 18, 0.5, "dB"),
      numberParameter("q", "Q", 1, 0.1, 18, 0.01),
    ],
  },
  {
    id: "graphic-eq",
    name: "9-band Graphic EQ",
    version: 1,
    description: "Fixed full-spectrum bands with a visual response editor.",
    parameters: GRAPHIC_EQ_BANDS.map((band) =>
      numberParameter(band.id, band.label, 0, -12, 12, 0.5, "dB"),
    ),
  },
  {
    id: "high-pass",
    name: "High-pass Filter",
    version: 1,
    description: "Remove low frequencies below the selected cutoff.",
    parameters: [
      numberParameter("cutoffHz", "Cutoff", 80, 20, 20000, 1, "Hz"),
      numberParameter("slopeDbPerOct", "Slope", 12, 12, 24, 12, "dB/oct"),
    ],
  },
  {
    id: "low-pass",
    name: "Low-pass Filter",
    version: 1,
    description: "Remove high frequencies above the selected cutoff.",
    parameters: [
      numberParameter("cutoffHz", "Cutoff", 18000, 20, 20000, 1, "Hz"),
      numberParameter("slopeDbPerOct", "Slope", 12, 12, 24, 12, "dB/oct"),
    ],
  },
  {
    id: "crossfeed",
    name: "Crossfeed",
    version: 1,
    description: "Blend a delayed, filtered signal between headphone channels.",
    parameters: [numberParameter("amount", "Amount", 20, 0, 100, 1, "%")],
  },
  {
    id: "compressor",
    name: "Compressor",
    version: 1,
    description: "Reduce dynamic range above a threshold.",
    parameters: [
      numberParameter("thresholdDb", "Threshold", -18, -60, 0, 1, "dB"),
      numberParameter("ratio", "Ratio", 3, 1, 20, 0.5, ":1"),
      numberParameter("attackMs", "Attack", 10, 0.1, 200, 0.1, "ms"),
      numberParameter("releaseMs", "Release", 100, 10, 2000, 1, "ms"),
    ],
  },
  {
    id: "noise-gate",
    name: "Noise Gate",
    version: 1,
    description: "Attenuate low-level noise when the signal falls below threshold.",
    parameters: [
      numberParameter("thresholdDb", "Threshold", -50, -80, 0, 1, "dB"),
      numberParameter("attackMs", "Attack", 2, 0.1, 100, 0.1, "ms"),
      numberParameter("holdMs", "Hold", 50, 0, 500, 1, "ms"),
      numberParameter("releaseMs", "Release", 150, 5, 2000, 1, "ms"),
      numberParameter("rangeDb", "Range", -60, -80, 0, 1, "dB"),
    ],
  },
  {
    id: "delay",
    name: "Delay",
    version: 1,
    description: "Stereo delay with feedback and wet/dry mix.",
    parameters: [
      numberParameter("timeMs", "Time", 180, 1, 1000, 1, "ms"),
      numberParameter("feedback", "Feedback", 20, 0, 90, 1, "%"),
      numberParameter("mix", "Mix", 15, 0, 100, 1, "%"),
    ],
  },
  {
    id: "reverb",
    name: "Reverb",
    version: 1,
    description: "Algorithmic room ambience.",
    parameters: [
      numberParameter("size", "Size", 35, 0, 100, 1, "%"),
      numberParameter("damping", "Damping", 50, 0, 100, 1, "%"),
      numberParameter("mix", "Mix", 10, 0, 100, 1, "%"),
    ],
  },
  {
    id: "limiter",
    name: "Limiter",
    version: 1,
    description: "Final safety ceiling for the headphone output.",
    parameters: [numberParameter("ceilingDb", "Ceiling", -1, -12, 0, 0.5, "dB")],
  },
];

export const findPluginDefinition = (id: string): PluginDefinition | undefined =>
  pluginCatalog.find((plugin) => plugin.id === id);

export const createPluginInstance = (definition: PluginDefinition): PluginInstance => ({
  id: crypto.randomUUID(),
  pluginId: definition.id,
  pluginVersion: definition.version,
  enabled: true,
  parameters: Object.fromEntries(
    definition.parameters.map((parameter) => [parameter.id, parameter.default]),
  ),
});

export const createConfiguredPluginInstance = (
  pluginId: string,
  parameters: Record<string, ParameterValue> = {},
): PluginInstance => {
  const definition = findPluginDefinition(pluginId);
  if (!definition) throw new Error(`Plugin ${pluginId} is missing from the catalog`);
  const instance = createPluginInstance(definition);
  return { ...instance, parameters: { ...instance.parameters, ...parameters } };
};

export const createDefaultPresetFile = (): PresetFile => {
  return {
    format: "linerack-presets",
    schemaVersion: 1,
    engine: { sampleRate: 48000, channels: 2 },
    display: { defaultMode: "preset", blankingEnabled: true },
    routing: { sourceMode: "usb", usbTrimDb: 0, analogTrimDb: 0 },
    slots: [
      {
        number: 1,
        name: "Clean",
        plugins: [
          createConfiguredPluginInstance("gain"),
          createConfiguredPluginInstance("parametric-eq"),
          createConfiguredPluginInstance("limiter"),
        ],
      },
      {
        number: 2,
        name: "Punch",
        plugins: [
          createConfiguredPluginInstance("gain", { gainDb: -3 }),
          createConfiguredPluginInstance("parametric-eq", {
            frequencyHz: 120,
            gainDb: 3,
            q: 0.8,
          }),
          createConfiguredPluginInstance("compressor", {
            thresholdDb: -18,
            ratio: 3,
            attackMs: 10,
            releaseMs: 120,
          }),
          createConfiguredPluginInstance("limiter"),
        ],
      },
      {
        number: 3,
        name: "Small Room",
        plugins: [
          createConfiguredPluginInstance("gain", { gainDb: -3 }),
          createConfiguredPluginInstance("compressor", {
            thresholdDb: -16,
            ratio: 2,
            attackMs: 15,
            releaseMs: 180,
          }),
          createConfiguredPluginInstance("reverb", { size: 30, damping: 55, mix: 15 }),
          createConfiguredPluginInstance("limiter"),
        ],
      },
      {
        number: 4,
        name: "Wide Hall",
        plugins: [
          createConfiguredPluginInstance("gain", { gainDb: -4 }),
          createConfiguredPluginInstance("compressor", {
            thresholdDb: -20,
            ratio: 2.5,
            attackMs: 25,
            releaseMs: 250,
          }),
          createConfiguredPluginInstance("reverb", { size: 80, damping: 40, mix: 28 }),
          createConfiguredPluginInstance("limiter"),
        ],
      },
    ],
  };
};

const isRecord = (value: unknown): value is Record<string, unknown> =>
  typeof value === "object" && value !== null && !Array.isArray(value);

const isParameterValue = (value: unknown): value is ParameterValue =>
  typeof value === "number" || typeof value === "boolean" || typeof value === "string";

export const validatePresetFile = (value: unknown): string[] => {
  const errors: string[] = [];

  if (!isRecord(value)) {
    return ["Preset document must be an object"];
  }
  if (value.format !== "linerack-presets") errors.push("Unsupported preset format");
  if (value.schemaVersion !== 1) errors.push("Unsupported schema version");

  if (!isRecord(value.engine)) {
    errors.push("Engine metadata is missing");
  } else {
    if (!Number.isInteger(value.engine.sampleRate) || Number(value.engine.sampleRate) <= 0) {
      errors.push("Engine sample rate must be a positive integer");
    }
    if (!Number.isInteger(value.engine.channels) || Number(value.engine.channels) <= 0) {
      errors.push("Engine channel count must be a positive integer");
    }
  }

  if (value.display !== undefined) {
    if (!isRecord(value.display)) {
      errors.push("Display settings must be an object");
    } else {
      if (
        value.display.defaultMode !== "preset" &&
        value.display.defaultMode !== "eq-response" &&
        value.display.defaultMode !== "visualizer"
      ) {
        errors.push("Display default mode must be preset, eq-response, or visualizer");
      }
      if (
        value.display.blankingEnabled !== undefined &&
        typeof value.display.blankingEnabled !== "boolean"
      ) {
        errors.push("Display blanking setting must be boolean");
      }
    }
  }

  if (value.routing !== undefined) {
    if (!isRecord(value.routing)) {
      errors.push("Routing settings must be an object");
    } else {
      if (!["usb", "analog", "mix"].includes(String(value.routing.sourceMode))) {
        errors.push("Audio source mode must be usb, analog, or mix");
      }
      for (const key of ["usbTrimDb", "analogTrimDb"] as const) {
        const trim = value.routing[key];
        if (
          typeof trim !== "number" ||
          !Number.isFinite(trim) ||
          trim < -24 ||
          trim > 0 ||
          !Number.isInteger(trim * 2)
        ) {
          errors.push(`${key} must be between -24 and 0 dB in 0.5 dB steps`);
        }
      }
    }
  }

  if (!Array.isArray(value.slots) || value.slots.length === 0) {
    errors.push("At least one preset slot is required");
    return errors;
  }

  const slotNumbers = new Set<number>();
  value.slots.forEach((slot, slotIndex) => {
    const path = `slots[${slotIndex}]`;
    if (!isRecord(slot)) {
      errors.push(`${path} must be an object`);
      return;
    }
    if (!Number.isInteger(slot.number) || Number(slot.number) < 1) {
      errors.push(`${path}.number must be a positive integer`);
    } else if (slotNumbers.has(Number(slot.number))) {
      errors.push(`${path}.number is duplicated`);
    } else {
      slotNumbers.add(Number(slot.number));
    }
    if (typeof slot.name !== "string" || slot.name.trim() === "") {
      errors.push(`${path}.name must not be empty`);
    }
    if (!Array.isArray(slot.plugins)) {
      errors.push(`${path}.plugins must be an array`);
      return;
    }
    slot.plugins.forEach((plugin, pluginIndex) => {
      const pluginPath = `${path}.plugins[${pluginIndex}]`;
      if (!isRecord(plugin)) {
        errors.push(`${pluginPath} must be an object`);
        return;
      }
      if (typeof plugin.id !== "string" || plugin.id === "")
        errors.push(`${pluginPath}.id is required`);
      if (typeof plugin.pluginId !== "string" || plugin.pluginId === "") {
        errors.push(`${pluginPath}.pluginId is required`);
      }
      if (!Number.isInteger(plugin.pluginVersion) || Number(plugin.pluginVersion) < 1) {
        errors.push(`${pluginPath}.pluginVersion must be a positive integer`);
      }
      if (typeof plugin.enabled !== "boolean") errors.push(`${pluginPath}.enabled must be boolean`);
      if (!isRecord(plugin.parameters)) {
        errors.push(`${pluginPath}.parameters must be an object`);
      } else if (!Object.values(plugin.parameters).every(isParameterValue)) {
        errors.push(`${pluginPath}.parameters contains an unsupported value`);
      }
    });
  });

  return errors;
};

export const normalizePresetFile = (value: unknown): PresetFile => {
  const errors = validatePresetFile(value);
  if (errors.length > 0) {
    throw new Error(errors.join("; "));
  }
  const presetFile = value as PresetFile;
  return {
    ...presetFile,
    display: {
      defaultMode: presetFile.display?.defaultMode ?? "preset",
      blankingEnabled: presetFile.display?.blankingEnabled ?? true,
    },
    routing: presetFile.routing ?? { sourceMode: "usb", usbTrimDb: 0, analogTrimDb: 0 },
  };
};

export const parsePresetFile = (text: string): PresetFile => {
  let value: unknown;
  try {
    value = JSON.parse(text);
  } catch {
    throw new Error("File is not valid JSON");
  }

  return normalizePresetFile(value);
};

export const createSharedPresetFile = (
  slot: PresetSlot,
  engine: PresetFile["engine"],
): SharedPresetFile => ({
  format: "linerack-preset",
  schemaVersion: 1,
  engine,
  preset: { name: slot.name, plugins: slot.plugins },
});

export const normalizeSharedPresetFile = (value: unknown): SharedPresetFile => {
  if (!isRecord(value) || value.format !== "linerack-preset") {
    throw new Error("Unsupported preset format");
  }
  const candidate = {
    format: "linerack-presets",
    schemaVersion: value.schemaVersion,
    engine: value.engine,
    display: { defaultMode: "preset", blankingEnabled: true },
    routing: { sourceMode: "usb", usbTrimDb: 0, analogTrimDb: 0 },
    slots: [isRecord(value.preset) ? { ...value.preset, number: 1 } : value.preset],
  };
  const errors = validatePresetFile(candidate);
  if (errors.length > 0) throw new Error(errors.join("; "));
  const normalized = candidate as PresetFile;
  const normalizedSlot = normalized.slots[0];
  return {
    format: "linerack-preset",
    schemaVersion: 1,
    engine: normalized.engine,
    preset: { name: normalizedSlot.name, plugins: normalizedSlot.plugins },
  };
};

export const parseSharedPresetFile = (text: string): SharedPresetFile => {
  let value: unknown;
  try {
    value = JSON.parse(text);
  } catch {
    throw new Error("File is not valid JSON");
  }
  return normalizeSharedPresetFile(value);
};

export const defaultPresetLimits: PresetLimits = {
  slotCount: 4,
  sampleRate: 48000,
  channels: 2,
  maxPluginsPerSlot: 10,
  presetNameMaxLength: 12,
  displayBlanking: true,
  sourceModes: ["usb", "analog", "mix"],
  plugins: pluginCatalog,
};

export const findCompatibilityIssues = (
  presetFile: PresetFile,
  limits: PresetLimits = defaultPresetLimits,
): string[] => {
  const issues: string[] = [];
  if (presetFile.slots.length !== limits.slotCount) {
    issues.push(`Device requires exactly ${limits.slotCount} preset slots`);
  }
  const expectedSlotNumbers = Array.from({ length: limits.slotCount }, (_, index) => index + 1);
  const actualSlotNumbers = presetFile.slots
    .map((slot) => slot.number)
    .sort((left, right) => left - right);
  if (actualSlotNumbers.some((slot, index) => slot !== expectedSlotNumbers[index])) {
    issues.push(`Device requires preset slot numbers 1 through ${limits.slotCount}`);
  }
  if (presetFile.engine.sampleRate !== limits.sampleRate) {
    issues.push(`Device requires a ${limits.sampleRate} Hz engine`);
  }
  if (presetFile.engine.channels !== limits.channels) {
    issues.push(`Device requires ${limits.channels} audio channels`);
  }
  if (!presetFile.display.blankingEnabled && !limits.displayBlanking) {
    issues.push("Device does not support disabling display blanking");
  }
  if (limits.sourceModes && !limits.sourceModes.includes(presetFile.routing.sourceMode)) {
    issues.push(`Device does not support ${presetFile.routing.sourceMode} audio routing`);
  }
  if (
    presetFile.routing.sourceMode === "mix" &&
    (presetFile.routing.usbTrimDb > -6.5 || presetFile.routing.analogTrimDb > -6.5)
  ) {
    issues.push("Mixed routing requires USB and analog trims at or below -6.5 dB");
  }

  for (const slot of presetFile.slots) {
    if (!/^[A-Za-z0-9 .?-]+$/.test(slot.name)) {
      issues.push(
        `Slot ${slot.number}: name may contain only letters, numbers, spaces, period, question mark, and hyphen`,
      );
    }
    if (
      limits.presetNameMaxLength !== undefined &&
      new TextEncoder().encode(slot.name).length > limits.presetNameMaxLength
    ) {
      issues.push(
        `Slot ${slot.number}: name must be at most ${limits.presetNameMaxLength} UTF-8 bytes`,
      );
    }
    if (slot.plugins.length > limits.maxPluginsPerSlot) {
      issues.push(`Slot ${slot.number}: maximum is ${limits.maxPluginsPerSlot} plugin instances`);
    }

    const gains = slot.plugins.filter((instance) => instance.pluginId === "gain");
    if (gains.length !== 1) {
      issues.push(`Slot ${slot.number}: exactly one input gain is required`);
    } else if (slot.plugins[0]?.pluginId !== "gain") {
      issues.push(`Slot ${slot.number}: input gain must be first`);
    }

    const limiters = slot.plugins.filter((instance) => instance.pluginId === "limiter");
    if (limiters.length !== 1) {
      issues.push(`Slot ${slot.number}: exactly one output limiter is required`);
    } else if (slot.plugins.at(-1)?.pluginId !== "limiter") {
      issues.push(`Slot ${slot.number}: output limiter must be last`);
    }

    for (const instance of slot.plugins) {
      const definition = limits.plugins.find((plugin) => plugin.id === instance.pluginId);
      if (!definition) {
        issues.push(`Slot ${slot.number}: unsupported plugin ${instance.pluginId}`);
      } else if (definition.version !== instance.pluginVersion) {
        issues.push(
          `Slot ${slot.number}: ${definition.name} version ${instance.pluginVersion} is unsupported`,
        );
      } else {
        const knownParameters = new Set(definition.parameters.map((parameter) => parameter.id));
        for (const parameter of definition.parameters) {
          const value = instance.parameters[parameter.id];
          if (typeof value !== "number" || !Number.isFinite(value)) {
            issues.push(`Slot ${slot.number}: ${definition.name}.${parameter.id} must be a number`);
          } else if (value < parameter.min || value > parameter.max) {
            issues.push(
              `Slot ${slot.number}: ${definition.name}.${parameter.id} must be between ${parameter.min} and ${parameter.max}`,
            );
          }
        }
        for (const parameterId of Object.keys(instance.parameters)) {
          if (!knownParameters.has(parameterId)) {
            issues.push(`Slot ${slot.number}: ${definition.name}.${parameterId} is unsupported`);
          }
        }
      }
    }
  }
  return issues;
};
