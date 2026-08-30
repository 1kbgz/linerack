import { describe, expect, it, vi } from "vitest";
import { defaultPresetLimits, createDefaultPresetFile } from "./presets";
import { HidPort, HidRpcClient } from "./hid-rpc";
import {
  Command,
  encodeMessage,
  HID_REPORT_ID,
  MessageAssembler,
  MessageKind,
  ProtocolMessage,
} from "./hid-protocol";
import { WebHidLineRackDevice } from "./webhid-device";

class LoopbackHidPort implements HidPort {
  private readonly assembler = new MessageAssembler();
  private readonly listeners = new Set<(reportId: number, data: Uint8Array) => void>();
  opened = false;

  constructor(
    private readonly respond: (message: ProtocolMessage) => ProtocolMessage | undefined,
  ) {}

  async open(): Promise<void> {
    this.opened = true;
  }

  async close(): Promise<void> {
    this.opened = false;
  }

  async sendReport(reportId: number, data: Uint8Array): Promise<void> {
    if (!this.opened) throw new Error("Port is closed");
    if (reportId !== HID_REPORT_ID) return;
    const message = this.assembler.accept(data);
    if (!message) return;
    const response = this.respond(message);
    if (response) this.emit(response);
  }

  onInputReport(listener: (reportId: number, data: Uint8Array) => void): () => void {
    this.listeners.add(listener);
    return () => this.listeners.delete(listener);
  }

  emit(message: ProtocolMessage): void {
    for (const report of encodeMessage(message)) {
      this.listeners.forEach((listener) => listener(HID_REPORT_ID, report));
    }
  }
}

describe("HID RPC", () => {
  it("correlates a chunked request and response", async () => {
    const port = new LoopbackHidPort((request) => ({
      kind: MessageKind.Response,
      command: request.command,
      requestId: request.requestId,
      payload: { echoed: request.payload },
    }));
    const client = new HidRpcClient(port);
    await client.open();

    const payload = { values: Array.from({ length: 80 }, (_, index) => `Value ${index}`) };
    await expect(client.request(Command.WritePresets, payload)).resolves.toEqual({
      echoed: payload,
    });
  });

  it("surfaces device errors", async () => {
    const port = new LoopbackHidPort((request) => ({
      kind: MessageKind.Error,
      command: request.command,
      requestId: request.requestId,
      payload: { message: "Preset is incompatible" },
    }));
    const client = new HidRpcClient(port);
    await client.open();

    await expect(client.request(Command.WritePresets, {})).rejects.toThrow(
      "Preset is incompatible",
    );
  });

  it("serializes requests so firmware receives only one transaction at a time", async () => {
    const requests: ProtocolMessage[] = [];
    const port = new LoopbackHidPort((request) => {
      requests.push(request);
      return requests.length === 1
        ? undefined
        : {
            kind: MessageKind.Response,
            command: request.command,
            requestId: request.requestId,
            payload: { order: requests.length },
          };
    });
    const client = new HidRpcClient(port);
    await client.open();

    const first = client.request<{ order: number }>(Command.Hello, {});
    const second = client.request<{ order: number }>(Command.GetStatus, {});
    await vi.waitFor(() => expect(requests).toHaveLength(1));

    port.emit({
      kind: MessageKind.Response,
      command: requests[0].command,
      requestId: requests[0].requestId,
      payload: { order: 1 },
    });

    await expect(first).resolves.toEqual({ order: 1 });
    await expect(second).resolves.toEqual({ order: 2 });
    expect(requests).toHaveLength(2);
  });
});

describe("WebHID LineRack device", () => {
  it("loads a snapshot and forwards status events", async () => {
    const presets = createDefaultPresetFile();
    const capabilities = {
      product: "LineRack Development Board",
      firmwareVersion: "0.1.0",
      engineVersion: "1",
      ...defaultPresetLimits,
    };
    const requests: ProtocolMessage[] = [];
    const port = new LoopbackHidPort((request) => {
      requests.push(request);
      const payload =
        request.command === Command.Hello
          ? { capabilities, status: { connected: true, activeSlot: 1 } }
          : presets;
      return {
        kind: MessageKind.Response,
        command: request.command,
        requestId: request.requestId,
        payload,
      };
    });
    const device = new WebHidLineRackDevice(port);
    const onStatus = vi.fn();
    device.onStatusChange(onStatus);

    await expect(device.connect()).resolves.toEqual({
      capabilities,
      presets,
      status: { connected: true, activeSlot: 1 },
    });
    await expect(device.wakeDisplay()).resolves.toBeUndefined();
    expect(requests.at(-1)?.command).toBe(Command.WakeDisplay);

    port.emit({
      kind: MessageKind.Event,
      command: Command.StatusChanged,
      requestId: 0,
      payload: { connected: true, activeSlot: 2 },
    });
    expect(onStatus).toHaveBeenCalledWith({ connected: true, activeSlot: 2 });
  });

  it("normalizes legacy read and write responses without display settings", async () => {
    const presets = createDefaultPresetFile();
    const legacyPresets = { ...presets, display: undefined, routing: undefined };
    const capabilities = {
      product: "LineRack Development Board",
      firmwareVersion: "0.1.0",
      engineVersion: "1",
      ...defaultPresetLimits,
    };
    const port = new LoopbackHidPort((request) => ({
      kind: MessageKind.Response,
      command: request.command,
      requestId: request.requestId,
      payload:
        request.command === Command.Hello
          ? { capabilities, status: { connected: true, activeSlot: 1 } }
          : legacyPresets,
    }));
    const device = new WebHidLineRackDevice(port);

    await expect(device.connect()).resolves.toMatchObject({
      presets: { display: { defaultMode: "preset" } },
    });
    await expect(device.writePresets(presets)).resolves.toMatchObject({
      display: { defaultMode: "preset" },
      routing: { sourceMode: "usb", usbTrimDb: 0, analogTrimDb: 0 },
    });
  });
});
