import { describe, expect, it } from "vitest";
import { SimulatedLineRackDevice } from "./device";

describe("simulated device", () => {
  it("reports capabilities, presets, and active slot on connection", async () => {
    const device = new SimulatedLineRackDevice();

    const snapshot = await device.connect();

    expect(snapshot.capabilities.slotCount).toBe(4);
    expect(snapshot.capabilities.plugins.length).toBeGreaterThan(0);
    expect(snapshot.capabilities.displayWake).toBe(true);
    expect(snapshot.presets.slots).toHaveLength(4);
    expect(snapshot.status).toMatchObject({ connected: true, activeSlot: 1 });
    expect(snapshot.status.diagnostics).toEqual({
      usbPackets: 0,
      underruns: 0,
      overruns: 0,
      bufferFillFrames: 96,
    });
  });

  it("writes valid presets without retaining the caller's mutable object", async () => {
    const device = new SimulatedLineRackDevice();
    const snapshot = await device.connect();
    snapshot.presets.slots[0].name = "Travel";

    await device.writePresets(snapshot.presets);
    snapshot.presets.slots[0].name = "Changed after write";

    expect((await device.readPresets()).slots[0].name).toBe("Travel");
  });

  it("rejects a chain without a final safety limiter", async () => {
    const device = new SimulatedLineRackDevice();
    const snapshot = await device.connect();
    snapshot.presets.slots[0].plugins.pop();

    await expect(device.writePresets(snapshot.presets)).rejects.toThrow(
      "exactly one output limiter is required",
    );
  });

  it("models the physical preset-cycle button", async () => {
    const device = new SimulatedLineRackDevice();
    await device.connect();

    expect((await device.cyclePreset()).activeSlot).toBe(2);
    expect((await device.cyclePreset()).activeSlot).toBe(3);
    expect((await device.cyclePreset()).activeSlot).toBe(4);
    expect((await device.cyclePreset()).activeSlot).toBe(1);
    await expect(device.wakeDisplay()).resolves.toBeUndefined();
  });

  it("reports live audio diagnostics", async () => {
    const device = new SimulatedLineRackDevice();
    await device.connect();

    expect((await device.getStatus()).diagnostics).toEqual({
      usbPackets: 48,
      underruns: 0,
      overruns: 0,
      bufferFillFrames: 96,
    });
    expect((await device.getStatus()).diagnostics?.usbPackets).toBe(96);
  });
});
