import {
  createDefaultPresetFile,
  defaultPresetLimits,
  findCompatibilityIssues,
  PluginDefinition,
  PresetFile,
  PresetLimits,
} from "./presets";

export interface DeviceCapabilities extends PresetLimits {
  product: string;
  firmwareVersion: string;
  engineVersion: string;
  displayWake?: boolean;
}

export interface DeviceStatus {
  connected: boolean;
  activeSlot: number;
  diagnostics?: {
    usbPackets: number;
    underruns: number;
    overruns: number;
    bufferFillFrames: number;
  };
}

export interface DeviceSnapshot {
  capabilities: DeviceCapabilities;
  presets: PresetFile;
  status: DeviceStatus;
}

export interface LineRackDevice {
  connect(): Promise<DeviceSnapshot>;
  readPresets(): Promise<PresetFile>;
  writePresets(presets: PresetFile): Promise<PresetFile>;
  activateSlot(slotNumber: number): Promise<DeviceStatus>;
  cyclePreset(): Promise<DeviceStatus>;
  getStatus(): Promise<DeviceStatus>;
  wakeDisplay(): Promise<void>;
  onStatusChange(listener: (status: DeviceStatus) => void): () => void;
}

const clone = <T>(value: T): T => structuredClone(value);

export class SimulatedLineRackDevice implements LineRackDevice {
  private connected = false;
  private activeSlot = 1;
  private presets = createDefaultPresetFile();
  private usbPackets = 0;
  private readonly statusListeners = new Set<(status: DeviceStatus) => void>();

  private readonly capabilities: DeviceCapabilities = {
    product: "LineRack Simulator",
    firmwareVersion: "0.0.0-sim",
    engineVersion: "1",
    ...defaultPresetLimits,
    displayWake: true,
    sourceModes: [...(defaultPresetLimits.sourceModes ?? [])],
    plugins: clone(defaultPresetLimits.plugins),
  };

  async connect(): Promise<DeviceSnapshot> {
    this.connected = true;
    return {
      capabilities: clone(this.capabilities),
      presets: clone(this.presets),
      status: this.status(),
    };
  }

  async readPresets(): Promise<PresetFile> {
    this.requireConnection();
    return clone(this.presets);
  }

  async writePresets(presets: PresetFile): Promise<PresetFile> {
    this.requireConnection();
    const issues = findCompatibilityIssues(presets, this.capabilities);
    if (issues.length > 0) {
      throw new Error(issues.join("; "));
    }
    this.presets = clone(presets);
    return clone(this.presets);
  }

  async activateSlot(slotNumber: number): Promise<DeviceStatus> {
    this.requireConnection();
    if (!this.presets.slots.some((slot) => slot.number === slotNumber)) {
      throw new Error(`Preset slot ${slotNumber} does not exist`);
    }
    this.activeSlot = slotNumber;
    return this.emitStatus();
  }

  async cyclePreset(): Promise<DeviceStatus> {
    this.requireConnection();
    const slotNumbers = this.presets.slots.map((slot) => slot.number).sort((a, b) => a - b);
    const currentIndex = slotNumbers.indexOf(this.activeSlot);
    this.activeSlot = slotNumbers[(currentIndex + 1) % slotNumbers.length];
    return this.emitStatus();
  }

  async getStatus(): Promise<DeviceStatus> {
    this.requireConnection();
    this.usbPackets += 48;
    return this.status();
  }

  async wakeDisplay(): Promise<void> {
    this.requireConnection();
  }

  onStatusChange(listener: (status: DeviceStatus) => void): () => void {
    this.statusListeners.add(listener);
    return () => this.statusListeners.delete(listener);
  }

  private requireConnection() {
    if (!this.connected) throw new Error("Device is not connected");
  }

  private status(): DeviceStatus {
    return {
      connected: this.connected,
      activeSlot: this.activeSlot,
      diagnostics: {
        usbPackets: this.usbPackets,
        underruns: 0,
        overruns: 0,
        bufferFillFrames: this.connected ? 96 : 0,
      },
    };
  }

  private emitStatus(): DeviceStatus {
    const status = this.status();
    this.statusListeners.forEach((listener) => listener(status));
    return status;
  }
}

export const findCapabilityPlugin = (
  capabilities: DeviceCapabilities,
  pluginId: string,
): PluginDefinition | undefined => capabilities.plugins.find((plugin) => plugin.id === pluginId);
