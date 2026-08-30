import { describe, expect, it } from "vitest";
import appSource from "./App.tsx?raw";

describe("chain card drag boundary", () => {
  it("binds chain dragging to the heading instead of the whole card", () => {
    const cardClass = appSource.indexOf('instance.enabled ? "plugin-card"');
    const cardStart = appSource.lastIndexOf("<article", cardClass);
    const headingClass = appSource.indexOf('className="plugin-heading"', cardClass);
    const headingStart = appSource.lastIndexOf("<div", headingClass);
    const headingEnd = appSource.indexOf(">", headingStart);

    expect(cardStart).toBeGreaterThan(-1);
    expect(headingStart).toBeGreaterThan(cardStart);
    expect(appSource.slice(cardStart, headingStart)).not.toContain("draggable=");
    expect(appSource.slice(headingStart, headingEnd)).toContain("draggable={!lockedEndpoint}");
    expect(appSource.slice(headingStart, headingEnd)).toContain("onDragStart=");
  });

  it("offers slot-local effect insertion without drag and drop", () => {
    expect(appSource).toContain("const nextInsertSlotIndex = effectPlugins.length + 1");
    expect(appSource).toContain("const insertable = index === nextInsertSlotIndex");
    expect(appSource).toContain("onClick={() => setInsertSlotIndex(index)}");
    expect(appSource).toContain("aria-label={`Effect for slot ${index + 1}`}");
    expect(appSource).toContain("addPlugin(event.target.value, index - 1)");
  });
});
