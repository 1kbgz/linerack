import { describe, expect, it, vi } from "vitest";
import {
  findAuthorizedLineRackDevice,
  isLineRackHidDevice,
  LINERACK_HID_FILTER,
  requestLineRackDevice,
} from "./webhid-presence";

const hidDevice = (vendorId: number, productId: number): HIDDevice =>
  ({ vendorId, productId }) as HIDDevice;

describe("LineRack WebHID presence", () => {
  it("matches only the development USB identity", () => {
    expect(isLineRackHidDevice(hidDevice(0xcafe, 0x4c52))).toBe(true);
    expect(isLineRackHidDevice(hidDevice(0xcafe, 0x0001))).toBe(false);
    expect(isLineRackHidDevice(hidDevice(0x0001, 0x4c52))).toBe(false);
  });

  it("finds a previously authorized LineRack", async () => {
    const lineRack = hidDevice(0xcafe, 0x4c52);
    const hid = {
      getDevices: vi.fn().mockResolvedValue([hidDevice(0x1234, 0x5678), lineRack]),
    } as unknown as HID;

    await expect(findAuthorizedLineRackDevice(hid)).resolves.toBe(lineRack);
  });

  it("requests only the LineRack development identity", async () => {
    const lineRack = hidDevice(0xcafe, 0x4c52);
    const requestDevice = vi.fn().mockResolvedValue([lineRack]);
    const hid = { requestDevice } as unknown as HID;

    await expect(requestLineRackDevice(hid)).resolves.toBe(lineRack);
    expect(requestDevice).toHaveBeenCalledWith({ filters: [LINERACK_HID_FILTER] });
  });
});
