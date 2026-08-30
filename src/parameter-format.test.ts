import { describe, expect, it } from "vitest";
import {
  formatParameterValue,
  logarithmicSliderPosition,
  logarithmicSliderValue,
} from "./parameter-format";

describe("parameter value formatting", () => {
  it("hides float32 noise at the parameter's declared precision", () => {
    expect(formatParameterValue(0.800000011920929, 0.1)).toBe("0.8");
    expect(formatParameterValue(0.800000011920929, 0.10000000149011612)).toBe("0.8");
  });

  it("preserves meaningful step precision", () => {
    expect(formatParameterValue(-1, 0.5)).toBe("-1.0");
    expect(formatParameterValue(120.00001, 1)).toBe("120");
  });

  it("maps equal Q ratios to equal slider distances", () => {
    const lowOctave = logarithmicSliderPosition(0.2) - logarithmicSliderPosition(0.1);
    const middleOctave = logarithmicSliderPosition(2) - logarithmicSliderPosition(1);

    expect(lowOctave).toBeCloseTo(middleOctave);
    expect(logarithmicSliderValue(logarithmicSliderPosition(0.1), 0.1, 18, 0.01)).toBe(0.1);
    expect(logarithmicSliderValue(logarithmicSliderPosition(1.23), 0.1, 18, 0.01)).toBe(1.23);
    expect(logarithmicSliderValue(logarithmicSliderPosition(18), 0.1, 18, 0.01)).toBe(18);
  });
});
