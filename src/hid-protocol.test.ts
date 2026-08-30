import { describe, expect, it } from "vitest";
import {
  Command,
  decodeFrame,
  encodeMessage,
  HID_REPORT_BYTES,
  MessageAssembler,
  MessageKind,
} from "./hid-protocol";

describe("HID protocol", () => {
  it("round-trips a message through a single report", () => {
    const assembler = new MessageAssembler();
    const reports = encodeMessage({
      kind: MessageKind.Request,
      command: Command.ActivateSlot,
      requestId: 42,
      payload: { slotNumber: 3 },
    });

    expect(reports).toHaveLength(1);
    expect(reports[0]).toHaveLength(HID_REPORT_BYTES);
    expect(reports[0][12] >> 5).toBe(5);
    expect(assembler.accept(reports[0])).toEqual({
      kind: MessageKind.Request,
      command: Command.ActivateSlot,
      requestId: 42,
      payload: { slotNumber: 3 },
    });
  });

  it("reassembles a multi-report CBOR payload", () => {
    const assembler = new MessageAssembler();
    const message = {
      kind: MessageKind.Response,
      command: Command.ReadPresets,
      requestId: 7,
      payload: { names: Array.from({ length: 40 }, (_, index) => `Preset value ${index}`) },
    };
    const reports = encodeMessage(message);

    expect(reports.length).toBeGreaterThan(1);
    expect(reports.slice(0, -1).map((report) => assembler.accept(report))).toEqual(
      Array.from({ length: reports.length - 1 }),
    );
    expect(assembler.accept(reports.at(-1)!)).toEqual(message);
  });

  it("rejects reports with invalid framing", () => {
    const report = new Uint8Array(HID_REPORT_BYTES);

    expect(() => decodeFrame(report)).toThrow("Invalid HID message magic");
  });
});
