import type { PluginInstance } from "./presets";

export const GRAPHIC_EQ_BANDS = [
  { id: "gain63Hz", label: "63", frequencyHz: 63 },
  { id: "gain125Hz", label: "125", frequencyHz: 125 },
  { id: "gain250Hz", label: "250", frequencyHz: 250 },
  { id: "gain500Hz", label: "500", frequencyHz: 500 },
  { id: "gain1kHz", label: "1k", frequencyHz: 1000 },
  { id: "gain2kHz", label: "2k", frequencyHz: 2000 },
  { id: "gain4kHz", label: "4k", frequencyHz: 4000 },
  { id: "gain8kHz", label: "8k", frequencyHz: 8000 },
  { id: "gain16kHz", label: "16k", frequencyHz: 16000 },
] as const;

const graphicEqParameters = (gains: number[]): Record<string, number> =>
  Object.fromEntries(GRAPHIC_EQ_BANDS.map((band, index) => [band.id, gains[index]]));

export const GRAPHIC_EQ_RECIPES = [
  { name: "Flat", parameters: graphicEqParameters([0, 0, 0, 0, 0, 0, 0, 0, 0]) },
  { name: "Bass", parameters: graphicEqParameters([6, 5, 3, 1, 0, 0, 0, 0, 0]) },
  { name: "Clarity", parameters: graphicEqParameters([-2, -1, 0, 1, 3, 3, 2, 1, 0]) },
  { name: "Bright", parameters: graphicEqParameters([0, 0, 0, 0, 1, 2, 3, 4, 4]) },
  { name: "Loudness", parameters: graphicEqParameters([5, 4, 2, 0, -1, 0, 2, 3, 4]) },
] as const;

export interface EqResponsePoint {
  frequencyHz: number;
  gainDb: number;
}

const numericParameter = (plugin: PluginInstance, id: string, fallback: number): number => {
  const value = plugin.parameters[id];
  return typeof value === "number" && Number.isFinite(value) ? value : fallback;
};

interface BiquadCoefficients {
  b0: number;
  b1: number;
  b2: number;
  a1: number;
  a2: number;
}

const biquadMagnitudeDb = (
  frequencyHz: number,
  sampleRate: number,
  coefficients: BiquadCoefficients,
): number => {
  const radians = (2 * Math.PI * frequencyHz) / sampleRate;
  const cos1 = Math.cos(radians);
  const sin1 = Math.sin(radians);
  const cos2 = Math.cos(2 * radians);
  const sin2 = Math.sin(2 * radians);
  const numeratorReal = coefficients.b0 + coefficients.b1 * cos1 + coefficients.b2 * cos2;
  const numeratorImaginary = -coefficients.b1 * sin1 - coefficients.b2 * sin2;
  const denominatorReal = 1 + coefficients.a1 * cos1 + coefficients.a2 * cos2;
  const denominatorImaginary = -coefficients.a1 * sin1 - coefficients.a2 * sin2;
  const numeratorPower = numeratorReal ** 2 + numeratorImaginary ** 2;
  const denominatorPower = denominatorReal ** 2 + denominatorImaginary ** 2;
  return 10 * Math.log10(numeratorPower / denominatorPower);
};

const peakMagnitudeDb = (
  frequencyHz: number,
  centerHz: number,
  gainDb: number,
  q: number,
  sampleRate: number,
): number => {
  if (gainDb === 0) return 0;

  const amplitude = 10 ** (gainDb / 40);
  const centerRadians = (2 * Math.PI * centerHz) / sampleRate;
  const alpha = Math.sin(centerRadians) / (2 * q);
  const a0 = 1 + alpha / amplitude;
  const b0 = (1 + alpha * amplitude) / a0;
  const b1 = (-2 * Math.cos(centerRadians)) / a0;
  const b2 = (1 - alpha * amplitude) / a0;
  const a1 = (-2 * Math.cos(centerRadians)) / a0;
  const a2 = (1 - alpha / amplitude) / a0;
  return biquadMagnitudeDb(frequencyHz, sampleRate, { b0, b1, b2, a1, a2 });
};

const filterMagnitudeDb = (
  frequencyHz: number,
  cutoffHz: number,
  q: number,
  sampleRate: number,
  highPass: boolean,
): number => {
  const cutoffRadians = (2 * Math.PI * cutoffHz) / sampleRate;
  const cosine = Math.cos(cutoffRadians);
  const alpha = Math.sin(cutoffRadians) / (2 * q);
  const a0 = 1 + alpha;
  const b0 = (highPass ? 1 + cosine : 1 - cosine) / 2 / a0;
  const b1 = (highPass ? -(1 + cosine) : 1 - cosine) / a0;
  const b2 = b0;
  const a1 = (-2 * cosine) / a0;
  const a2 = (1 - alpha) / a0;
  return biquadMagnitudeDb(frequencyHz, sampleRate, { b0, b1, b2, a1, a2 });
};

const slopeMagnitudeDb = (
  plugin: PluginInstance,
  frequencyHz: number,
  sampleRate: number,
): number => {
  const cutoffHz = numericParameter(plugin, "cutoffHz", 1000);
  const highPass = plugin.pluginId === "high-pass";
  if (numericParameter(plugin, "slopeDbPerOct", 12) === 24) {
    return (
      filterMagnitudeDb(frequencyHz, cutoffHz, 0.541196, sampleRate, highPass) +
      filterMagnitudeDb(frequencyHz, cutoffHz, 1.306563, sampleRate, highPass)
    );
  }
  return filterMagnitudeDb(frequencyHz, cutoffHz, Math.SQRT1_2, sampleRate, highPass);
};

export const frequencyAtPosition = (position: number, minimum = 20, maximum = 20000): number =>
  minimum * (maximum / minimum) ** position;

export const calculateEqResponse = (
  plugins: PluginInstance[],
  sampleRate: number,
  pointCount: number,
): EqResponsePoint[] => {
  const enabledEq = plugins.filter(
    (plugin) =>
      plugin.enabled &&
      ["parametric-eq", "graphic-eq", "high-pass", "low-pass"].includes(plugin.pluginId),
  );

  return Array.from({ length: pointCount }, (_, index) => {
    const position = pointCount === 1 ? 0 : index / (pointCount - 1);
    const frequencyHz = frequencyAtPosition(position);
    let gainDb = 0;

    for (const plugin of enabledEq) {
      if (plugin.pluginId === "parametric-eq") {
        gainDb += peakMagnitudeDb(
          frequencyHz,
          numericParameter(plugin, "frequencyHz", 1000),
          numericParameter(plugin, "gainDb", 0),
          numericParameter(plugin, "q", 1),
          sampleRate,
        );
      } else if (plugin.pluginId === "graphic-eq") {
        for (const band of GRAPHIC_EQ_BANDS) {
          gainDb += peakMagnitudeDb(
            frequencyHz,
            band.frequencyHz,
            numericParameter(plugin, band.id, 0),
            1.4,
            sampleRate,
          );
        }
      } else {
        gainDb += slopeMagnitudeDb(plugin, frequencyHz, sampleRate);
      }
    }

    return { frequencyHz, gainDb };
  });
};
