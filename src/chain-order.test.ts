import { describe, expect, it } from "vitest";
import { createChainSlots, insertEffectAt, moveEffectTo } from "./chain-order";
import type { PluginInstance } from "./presets";

const plugin = (id: string, pluginId = id): PluginInstance => ({
  id,
  pluginId,
  pluginVersion: 1,
  enabled: true,
  parameters: {},
});

describe("signal-chain ordering", () => {
  it("renders fixed endpoints around eight effect positions", () => {
    const slots = createChainSlots([plugin("gain"), plugin("limit", "limiter")], 10);

    expect(slots).toHaveLength(10);
    expect(slots[0]?.pluginId).toBe("gain");
    expect(slots.slice(1, 9).every((slot) => slot === null)).toBe(true);
    expect(slots[9]?.pluginId).toBe("limiter");
  });

  it("inserts a catalog effect at a requested slot before the limiter", () => {
    const result = insertEffectAt(
      [plugin("gain"), plugin("eq", "parametric-eq"), plugin("limit", "limiter")],
      plugin("filter", "high-pass"),
      0,
    );

    expect(result.map(({ id }) => id)).toEqual(["gain", "filter", "eq", "limit"]);
  });

  it("moves an existing effect while keeping the limiter last", () => {
    const result = moveEffectTo(
      [
        plugin("gain"),
        plugin("eq", "parametric-eq"),
        plugin("filter", "low-pass"),
        plugin("limit", "limiter"),
      ],
      "eq",
      1,
    );

    expect(result.map(({ id }) => id)).toEqual(["gain", "filter", "eq", "limit"]);
  });

  it("does not move either fixed endpoint", () => {
    const plugins = [plugin("gain"), plugin("eq", "parametric-eq"), plugin("limit", "limiter")];

    expect(moveEffectTo(plugins, "gain", 1)).toBe(plugins);
    expect(moveEffectTo(plugins, "limit", 0)).toBe(plugins);
  });
});
