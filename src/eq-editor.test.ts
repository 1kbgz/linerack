import { createElement } from "react";
import { renderToStaticMarkup } from "react-dom/server";
import { describe, expect, it } from "vitest";
import { EqResponseGraph } from "./eq-editor";
import type { PluginInstance } from "./presets";

const plugin = (
  id: string,
  pluginId: string,
  parameters: Record<string, number>,
): PluginInstance => ({
  id,
  pluginId,
  pluginVersion: 1,
  enabled: true,
  parameters,
});

describe("EQ response graph", () => {
  it("draws the current block separately from the net chain", () => {
    const current = plugin("eq", "parametric-eq", {
      frequencyHz: 1000,
      gainDb: 6,
      q: 1,
    });
    const highPass = plugin("hp", "high-pass", {
      cutoffHz: 500,
      slopeDbPerOct: 24,
    });
    const markup = renderToStaticMarkup(
      createElement(EqResponseGraph, {
        chainPlugins: [current, highPass],
        plugin: current,
        sampleRate: 48000,
      }),
    );

    const blockPoints = markup.match(/class="eq-curve" points="([^"]+)"/)?.[1];
    const netPoints = markup.match(/class="eq-net-curve" points="([^"]+)"/)?.[1];
    expect(blockPoints).toBeDefined();
    expect(netPoints).toBeDefined();
    expect(blockPoints).not.toBe(netPoints);
    expect(markup).toContain("This block");
    expect(markup).toContain("Net chain");
  });

  it("omits the net curve when no other response block is enabled", () => {
    const current = plugin("lp", "low-pass", {
      cutoffHz: 12000,
      slopeDbPerOct: 12,
    });
    const markup = renderToStaticMarkup(
      createElement(EqResponseGraph, {
        chainPlugins: [current],
        plugin: current,
        sampleRate: 48000,
      }),
    );

    expect(markup).toContain('class="eq-curve"');
    expect(markup).not.toContain("eq-net-curve");
  });
});
