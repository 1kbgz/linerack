import { DeviceCapabilities, DeviceSnapshot, DeviceStatus, LineRackDevice } from "./device";
import { Command, HID_PROTOCOL_VERSION, MessageKind } from "./hid-protocol";
import { HidPort, HidRpcClient } from "./hid-rpc";
import { normalizePresetFile, PresetFile } from "./presets";

interface HelloResponse {
  capabilities: DeviceCapabilities;
  status: DeviceStatus;
}

export class BrowserHidPort implements HidPort {
  constructor(private readonly device: HIDDevice) {}

  async open(): Promise<void> {
    if (!this.device.opened) await this.device.open();
  }

  async close(): Promise<void> {
    if (this.device.opened) await this.device.close();
  }

  async sendReport(reportId: number, data: Uint8Array): Promise<void> {
    await this.device.sendReport(reportId, data.slice().buffer);
  }

  onInputReport(listener: (reportId: number, data: Uint8Array) => void): () => void {
    const browserListener = (event: HIDInputReportEvent) => {
      const data = new Uint8Array(
        event.data.buffer,
        event.data.byteOffset,
        event.data.byteLength,
      ).slice();
      listener(event.reportId, data);
    };
    this.device.addEventListener("inputreport", browserListener);
    return () => this.device.removeEventListener("inputreport", browserListener);
  }
}

export class WebHidLineRackDevice implements LineRackDevice {
  private readonly rpc: HidRpcClient;
  private readonly statusListeners = new Set<(status: DeviceStatus) => void>();
  private removeEventListener: (() => void) | undefined;

  constructor(port: HidPort) {
    this.rpc = new HidRpcClient(port);
  }

  async connect(): Promise<DeviceSnapshot> {
    await this.rpc.open();
    this.removeEventListener ??= this.rpc.onEvent((message) => {
      if (message.command !== Command.StatusChanged || message.kind !== MessageKind.Event) return;
      const status = message.payload as DeviceStatus;
      this.statusListeners.forEach((listener) => listener(status));
    });
    const hello = await this.rpc.request<HelloResponse>(Command.Hello, {
      client: "linerack-configurator",
      protocolVersion: HID_PROTOCOL_VERSION,
    });
    const presets = await this.readPresets();
    return { capabilities: hello.capabilities, presets, status: hello.status };
  }

  async readPresets(): Promise<PresetFile> {
    return normalizePresetFile(await this.rpc.request(Command.ReadPresets, {}));
  }

  async writePresets(presets: PresetFile): Promise<PresetFile> {
    return normalizePresetFile(await this.rpc.request(Command.WritePresets, presets));
  }

  activateSlot(slotNumber: number): Promise<DeviceStatus> {
    return this.rpc.request(Command.ActivateSlot, { slotNumber });
  }

  cyclePreset(): Promise<DeviceStatus> {
    return this.rpc.request(Command.CyclePreset, {});
  }

  getStatus(): Promise<DeviceStatus> {
    return this.rpc.request(Command.GetStatus, {});
  }

  async wakeDisplay(): Promise<void> {
    await this.rpc.request(Command.WakeDisplay, {});
  }

  onStatusChange(listener: (status: DeviceStatus) => void): () => void {
    this.statusListeners.add(listener);
    return () => this.statusListeners.delete(listener);
  }
}

export const requestWebHidLineRackDevice = async (
  filters: HIDDeviceFilter[],
): Promise<WebHidLineRackDevice> => {
  if (filters.length === 0) throw new Error("At least one LineRack USB device filter is required");
  if (!("hid" in navigator)) throw new Error("WebHID is not available in this browser");

  const devices = await navigator.hid.requestDevice({ filters });
  const device = devices[0];
  if (!device) throw new Error("No LineRack device was selected");
  return new WebHidLineRackDevice(new BrowserHidPort(device));
};
