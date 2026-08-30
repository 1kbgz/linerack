import { ChangeEvent, DragEvent, useCallback, useEffect, useMemo, useRef, useState } from "react";
import { AudioAudition } from "./audio-audition";
import { createChainSlots, insertEffectAt, moveEffectTo } from "./chain-order";
import { CollapsibleSection } from "./collapsible-section";
import {
  DeviceCapabilities,
  DeviceStatus,
  findCapabilityPlugin,
  LineRackDevice,
  SimulatedLineRackDevice,
} from "./device";
import {
  AudioSourceMode,
  createDefaultPresetFile,
  createPluginInstance,
  createSharedPresetFile,
  DisplayMode,
  findCompatibilityIssues,
  ParameterValue,
  parsePresetFile,
  parseSharedPresetFile,
  PluginInstance,
  PresetFile,
  PresetSlot,
} from "./presets";
import { createPresetFromRecipe, presetLibrary, PresetRecipe } from "./preset-library";
import {
  findAuthorizedLineRackDevice,
  isLineRackHidDevice,
  requestLineRackDevice,
} from "./webhid-presence";
import { BrowserHidPort, WebHidLineRackDevice } from "./webhid-device";
import { EqResponseGraph, GraphicEqEditor } from "./eq-editor";
import { effectPresets, matchingEffectPreset } from "./effect-presets";
import { OledPreview } from "./oled-preview";
import {
  formatParameterValue,
  logarithmicSliderPosition,
  logarithmicSliderValue,
} from "./parameter-format";
import { SiteNav } from "./site-nav";
import { SITE_ORIGIN } from "./site-url";

type HardwareConnectionState =
  | "checking"
  | "authorization-needed"
  | "authorizing"
  | "connected"
  | "disconnected"
  | "unsupported"
  | "error";

type DraggedEffect = { kind: "catalog"; pluginId: string } | { kind: "chain"; instanceId: string };
type WorkspaceView = "global" | "presets";

const downloadJsonFile = (value: unknown, filename: string) => {
  const blob = new Blob([`${JSON.stringify(value, null, 2)}\n`], {
    type: "application/json",
  });
  const url = URL.createObjectURL(blob);
  const link = document.createElement("a");
  link.href = url;
  link.download = filename;
  link.click();
  URL.revokeObjectURL(url);
};

const downloadPresetFile = (presetFile: PresetFile) =>
  downloadJsonFile(presetFile, "linerack-presets.linerack.json");

const presetFilename = (name: string) => {
  const slug = name
    .toLowerCase()
    .replace(/[^a-z0-9]+/g, "-")
    .replace(/(^-|-$)/g, "");
  return `linerack-${slug || "preset"}.linerack-preset.json`;
};

export default function App() {
  const [capabilities, setCapabilities] = useState<DeviceCapabilities>();
  const [deviceStatus, setDeviceStatus] = useState<DeviceStatus>();
  const [presetFile, setPresetFile] = useState<PresetFile>();
  const [selectedSlotNumber, setSelectedSlotNumber] = useState(1);
  const [dirty, setDirty] = useState(false);
  const [applying, setApplying] = useState(false);
  const [notice, setNotice] = useState("Connecting to simulator…");
  const [hardwareState, setHardwareState] = useState<HardwareConnectionState>("checking");
  const [hardwareProduct, setHardwareProduct] = useState("LineRack");
  const [hardwareError, setHardwareError] = useState("");
  const [showHardwarePrompt, setShowHardwarePrompt] = useState(false);
  const [showPresetLibrary, setShowPresetLibrary] = useState(false);
  const [workspaceView, setWorkspaceView] = useState<WorkspaceView>("presets");
  const [diagnosticsOpen, setDiagnosticsOpen] = useState(false);
  const [editingHardware, setEditingHardware] = useState(false);
  const [draggedEffect, setDraggedEffect] = useState<DraggedEffect>();
  const [dropSlotIndex, setDropSlotIndex] = useState<number>();
  const [insertSlotIndex, setInsertSlotIndex] = useState<number>();
  const fileInput = useRef<HTMLInputElement>(null);
  const presetInput = useRef<HTMLInputElement>(null);
  const simulator = useRef(new SimulatedLineRackDevice());
  const activeDevice = useRef<LineRackDevice>(simulator.current);
  const removeStatusListener = useRef<(() => void) | undefined>(undefined);
  const dirtyRef = useRef(false);
  const draftRevision = useRef(0);
  const applyingRef = useRef(false);
  const displayWakeInFlight = useRef(false);
  const lastDisplayWakeMs = useRef(0);

  useEffect(() => {
    const configuratorUrl = new URL("/configure", SITE_ORIGIN).href;
    document.title = "LineRack Configurator";
    document
      .querySelector('meta[name="description"]')
      ?.setAttribute("content", "Build and load effect chains for a connected LineRack.");
    document.querySelector('meta[property="og:title"]')?.setAttribute("content", document.title);
    document
      .querySelector('meta[property="og:description"]')
      ?.setAttribute("content", "Build and load effect chains for a connected LineRack.");
    document.querySelector('meta[property="og:url"]')?.setAttribute("content", configuratorUrl);
    document.querySelector('link[rel="canonical"]')?.setAttribute("href", configuratorUrl);
  }, []);

  const wakeDisplayForEdit = useCallback(() => {
    if (!editingHardware || capabilities?.displayWake !== true) return;
    const nowMs = Date.now();
    if (displayWakeInFlight.current || nowMs - lastDisplayWakeMs.current < 500) return;
    displayWakeInFlight.current = true;
    lastDisplayWakeMs.current = nowMs;
    void activeDevice.current
      .wakeDisplay()
      .catch(() => undefined)
      .finally(() => {
        displayWakeInFlight.current = false;
      });
  }, [capabilities?.displayWake, editingHardware]);

  const markDirty = useCallback(() => {
    dirtyRef.current = true;
    draftRevision.current += 1;
    setDirty(true);
    wakeDisplayForEdit();
  }, [wakeDisplayForEdit]);

  const markClean = useCallback(() => {
    dirtyRef.current = false;
    setDirty(false);
  }, []);

  const loadDevice = useCallback(
    async (nextDevice: LineRackDevice, hardware: boolean) => {
      const snapshot = await nextDevice.connect();
      const preserveDraft = dirtyRef.current;
      removeStatusListener.current?.();
      activeDevice.current = nextDevice;
      removeStatusListener.current = nextDevice.onStatusChange((status) => {
        setDeviceStatus(status);
        setSelectedSlotNumber(status.activeSlot);
      });
      setCapabilities(snapshot.capabilities);
      setDeviceStatus(snapshot.status);
      if (!preserveDraft) {
        setPresetFile(snapshot.presets);
        setSelectedSlotNumber(snapshot.status.activeSlot);
        markClean();
      }
      setEditingHardware(hardware);
      setNotice(
        `${snapshot.capabilities.product} connected${preserveDraft ? ". Unapplied draft preserved." : ""}`,
      );
      return snapshot;
    },
    [markClean],
  );

  const loadSimulator = useCallback(async () => {
    await loadDevice(simulator.current, false);
  }, [loadDevice]);

  const openHardwareDevice = useCallback(
    async (selectedDevice: HIDDevice) => {
      setHardwareState("authorizing");
      setHardwareError("");
      try {
        const hardwareDevice = new WebHidLineRackDevice(new BrowserHidPort(selectedDevice));
        const snapshot = await loadDevice(hardwareDevice, true);
        setHardwareProduct(
          snapshot.capabilities.product || selectedDevice.productName || "LineRack",
        );
        setHardwareState("connected");
        setShowHardwarePrompt(false);
      } catch (error) {
        setHardwareState("error");
        setHardwareError(
          error instanceof Error ? error.message : "Could not communicate with LineRack",
        );
        await loadSimulator();
      }
    },
    [loadDevice, loadSimulator],
  );

  const authorizeHardware = useCallback(async () => {
    if (!("hid" in navigator)) {
      setHardwareState("unsupported");
      return;
    }

    setHardwareState("authorizing");
    setHardwareError("");
    try {
      const selectedDevice = await requestLineRackDevice(navigator.hid);
      if (!selectedDevice) {
        setHardwareState("authorization-needed");
        return;
      }
      await openHardwareDevice(selectedDevice);
    } catch (error) {
      setHardwareState("error");
      setHardwareError(error instanceof Error ? error.message : "LineRack authorization failed");
    }
  }, [openHardwareDevice]);

  useEffect(() => {
    void loadSimulator();
    return () => removeStatusListener.current?.();
  }, [loadSimulator]);

  useEffect(() => setInsertSlotIndex(undefined), [selectedSlotNumber]);

  useEffect(() => {
    if (workspaceView !== "global" || !diagnosticsOpen) return;

    let disposed = false;
    const refresh = async () => {
      try {
        const status = await activeDevice.current.getStatus();
        if (!disposed) setDeviceStatus(status);
      } catch {
        // Connection handling owns user-visible device errors.
      }
    };
    void refresh();
    const interval = window.setInterval(() => void refresh(), 1000);
    return () => {
      disposed = true;
      window.clearInterval(interval);
    };
  }, [diagnosticsOpen, workspaceView]);

  useEffect(() => {
    if (!dirty) return;

    const warnBeforeUnload = (event: BeforeUnloadEvent) => {
      event.preventDefault();
      event.returnValue = true;
    };
    window.addEventListener("beforeunload", warnBeforeUnload);
    return () => window.removeEventListener("beforeunload", warnBeforeUnload);
  }, [dirty]);

  useEffect(() => {
    if (!("hid" in navigator)) {
      setHardwareState("unsupported");
      return;
    }

    const hid = navigator.hid;
    let disposed = false;
    const handleConnect = (event: HIDConnectionEvent) => {
      if (isLineRackHidDevice(event.device)) void openHardwareDevice(event.device);
    };
    const handleDisconnect = (event: HIDConnectionEvent) => {
      if (!isLineRackHidDevice(event.device)) return;
      setHardwareProduct(event.device.productName || "LineRack");
      setHardwareState("disconnected");
      void loadSimulator();
      setNotice("LineRack disconnected. Editing simulator.");
    };

    hid.addEventListener("connect", handleConnect);
    hid.addEventListener("disconnect", handleDisconnect);
    void findAuthorizedLineRackDevice(hid)
      .then((authorizedDevice) => {
        if (disposed) return;
        if (authorizedDevice) {
          void openHardwareDevice(authorizedDevice);
        } else {
          setHardwareState("authorization-needed");
          setShowHardwarePrompt(true);
        }
      })
      .catch((error: unknown) => {
        if (disposed) return;
        setHardwareState("error");
        setHardwareError(error instanceof Error ? error.message : "Could not inspect HID devices");
        setShowHardwarePrompt(true);
      });

    return () => {
      disposed = true;
      hid.removeEventListener("connect", handleConnect);
      hid.removeEventListener("disconnect", handleDisconnect);
    };
  }, [loadSimulator, openHardwareDevice]);

  const selectedSlot = useMemo(
    () =>
      presetFile?.slots.find((slot) => slot.number === selectedSlotNumber) ?? presetFile?.slots[0],
    [presetFile, selectedSlotNumber],
  );

  if (!capabilities || !deviceStatus || !presetFile || !selectedSlot) {
    return <div className="loading">Connecting to LineRack simulator…</div>;
  }

  const updateSlot = (updater: (slot: PresetSlot) => PresetSlot) => {
    setPresetFile((current) => {
      if (!current) return current;
      return {
        ...current,
        slots: current.slots.map((slot) =>
          slot.number === selectedSlot.number ? updater(slot) : slot,
        ),
      };
    });
    markDirty();
  };

  const updateDisplayMode = (defaultMode: DisplayMode) => {
    setPresetFile(
      (current) =>
        current && {
          ...current,
          display: { ...current.display, defaultMode },
        },
    );
    markDirty();
  };

  const updateDisplayBlanking = (blankingEnabled: boolean) => {
    setPresetFile(
      (current) =>
        current && {
          ...current,
          display: { ...current.display, blankingEnabled },
        },
    );
    markDirty();
  };

  const updateSourceMode = (sourceMode: AudioSourceMode) => {
    setPresetFile(
      (current) =>
        current && {
          ...current,
          routing: {
            ...current.routing,
            sourceMode,
            usbTrimDb:
              sourceMode === "mix"
                ? Math.min(current.routing.usbTrimDb, -6.5)
                : current.routing.usbTrimDb,
            analogTrimDb:
              sourceMode === "mix"
                ? Math.min(current.routing.analogTrimDb, -6.5)
                : current.routing.analogTrimDb,
          },
        },
    );
    markDirty();
  };

  const updateSourceTrim = (source: "usb" | "analog", trimDb: number) => {
    setPresetFile(
      (current) =>
        current && {
          ...current,
          routing: {
            ...current.routing,
            [source === "usb" ? "usbTrimDb" : "analogTrimDb"]: trimDb,
          },
        },
    );
    markDirty();
  };

  const addPlugin = (pluginId: string, targetIndex = Number.MAX_SAFE_INTEGER) => {
    const definition = findCapabilityPlugin(capabilities, pluginId);
    if (!definition || definition.id === "gain" || definition.id === "limiter") return;
    if (selectedSlot.plugins.length >= capabilities.maxPluginsPerSlot) {
      setNotice(`This device supports ${capabilities.maxPluginsPerSlot - 2} effects per preset`);
      return;
    }
    updateSlot((slot) => {
      const plugins = insertEffectAt(slot.plugins, createPluginInstance(definition), targetIndex);
      return { ...slot, plugins };
    });
    setNotice(`${definition.name} added to slot ${selectedSlot.number}`);
  };

  const updatePlugin = (
    instanceId: string,
    updater: (plugin: PluginInstance) => PluginInstance,
  ) => {
    updateSlot((slot) => ({
      ...slot,
      plugins: slot.plugins.map((plugin) => (plugin.id === instanceId ? updater(plugin) : plugin)),
    }));
  };

  const movePlugin = (instanceId: string, index: number, direction: -1 | 1) => {
    updateSlot((slot) => ({
      ...slot,
      plugins: moveEffectTo(slot.plugins, instanceId, index + direction),
    }));
  };

  const startCatalogDrag = (event: DragEvent, pluginId: string) => {
    event.dataTransfer.effectAllowed = "copy";
    event.dataTransfer.setData("text/plain", `catalog:${pluginId}`);
    setDraggedEffect({ kind: "catalog", pluginId });
  };

  const startChainDrag = (event: DragEvent, instanceId: string) => {
    event.dataTransfer.effectAllowed = "move";
    event.dataTransfer.setData("text/plain", `chain:${instanceId}`);
    setDraggedEffect({ kind: "chain", instanceId });
  };

  const finishDrag = () => {
    setDraggedEffect(undefined);
    setDropSlotIndex(undefined);
  };

  const allowEffectDrop = (event: DragEvent, targetIndex: number, displayIndex: number) => {
    if (!draggedEffect) return;
    event.preventDefault();
    event.dataTransfer.dropEffect = draggedEffect.kind === "catalog" ? "copy" : "move";
    setDropSlotIndex(displayIndex);
  };

  const dropEffect = (event: DragEvent, targetIndex: number) => {
    event.preventDefault();
    if (draggedEffect?.kind === "catalog") {
      addPlugin(draggedEffect.pluginId, targetIndex);
    } else if (draggedEffect?.kind === "chain") {
      updateSlot((slot) => ({
        ...slot,
        plugins: moveEffectTo(slot.plugins, draggedEffect.instanceId, targetIndex),
      }));
      setNotice(`Effect moved to slot ${targetIndex + 1}`);
    }
    finishDrag();
  };

  const importPresetFile = async (event: ChangeEvent<HTMLInputElement>) => {
    const file = event.target.files?.[0];
    event.target.value = "";
    if (!file) return;

    try {
      const imported = parsePresetFile(await file.text());
      const issues = findCompatibilityIssues(imported, capabilities);
      if (issues.length > 0) throw new Error(issues.join("; "));
      setPresetFile(imported);
      setSelectedSlotNumber(imported.slots[0].number);
      markDirty();
      setNotice(`Imported ${file.name}. Apply to save it on the device.`);
    } catch (error) {
      setNotice(error instanceof Error ? error.message : "Import failed");
    }
  };

  const replaceSelectedPreset = (replacement: PresetSlot, source: string) => {
    const nextPresetFile = {
      ...presetFile,
      slots: presetFile.slots.map((slot) =>
        slot.number === selectedSlot.number
          ? {
              ...replacement,
              number: selectedSlot.number,
              plugins: replacement.plugins.map((plugin) => ({
                ...plugin,
                id: crypto.randomUUID(),
              })),
            }
          : slot,
      ),
    };
    const issues = findCompatibilityIssues(nextPresetFile, capabilities);
    if (issues.length > 0) {
      setNotice(issues.join("; "));
      return false;
    }
    setPresetFile(nextPresetFile);
    markDirty();
    setNotice(`${source}. Apply to save it on the device.`);
    return true;
  };

  const importSharedPresetFile = async (event: ChangeEvent<HTMLInputElement>) => {
    const file = event.target.files?.[0];
    event.target.value = "";
    if (!file) return;

    try {
      const imported = parseSharedPresetFile(await file.text());
      if (
        imported.engine.sampleRate !== presetFile.engine.sampleRate ||
        imported.engine.channels !== presetFile.engine.channels
      ) {
        throw new Error("Preset audio engine does not match this device");
      }
      replaceSelectedPreset(
        { ...imported.preset, number: selectedSlot.number },
        `Imported ${file.name} into slot ${selectedSlot.number}`,
      );
    } catch (error) {
      setNotice(error instanceof Error ? error.message : "Preset import failed");
    }
  };

  const usePresetRecipe = (recipe: PresetRecipe) => {
    if (
      replaceSelectedPreset(
        createPresetFromRecipe(recipe, selectedSlot.number),
        `${recipe.name} loaded into slot ${selectedSlot.number}`,
      )
    ) {
      setShowPresetLibrary(false);
    }
  };

  const restoreStarterSetup = () => {
    const presets = createDefaultPresetFile();
    const issues = findCompatibilityIssues(presets, capabilities);
    if (issues.length > 0) {
      setNotice(issues.join("; "));
      return;
    }
    setPresetFile(presets);
    setSelectedSlotNumber(presets.slots[0].number);
    markDirty();
    setNotice("Four-preset starter setup loaded. Apply to save it on the device.");
  };

  const applyToDevice = async () => {
    if (applyingRef.current) return;
    const issues = findCompatibilityIssues(presetFile, capabilities);
    if (issues.length > 0) {
      setNotice(issues.join("; "));
      return;
    }

    const revision = draftRevision.current;
    const targetDevice = activeDevice.current;
    applyingRef.current = true;
    setApplying(true);
    try {
      const stored = await targetDevice.writePresets(presetFile);
      if (activeDevice.current === targetDevice && draftRevision.current === revision) {
        setPresetFile(stored);
        markClean();
        setNotice(`All four presets stored on ${editingHardware ? "LineRack" : "the simulator"}`);
      } else {
        setNotice("Earlier draft stored. Current unapplied edits were preserved.");
      }
    } catch (error) {
      setNotice(error instanceof Error ? error.message : "Device rejected the setup");
    } finally {
      applyingRef.current = false;
      setApplying(false);
    }
  };

  const activateSelectedSlot = async () => {
    try {
      const status = await activeDevice.current.activateSlot(selectedSlot.number);
      setDeviceStatus(status);
      setNotice(`Preset ${status.activeSlot} is active`);
    } catch (error) {
      setNotice(error instanceof Error ? error.message : "Preset activation failed");
    }
  };

  const pressDeviceButton = async () => {
    const status = await activeDevice.current.cyclePreset();
    setDeviceStatus(status);
    setSelectedSlotNumber(status.activeSlot);
    setNotice(`Simulated double tap selected preset ${status.activeSlot}`);
  };

  const hardwareLabel = {
    checking: "Checking for LineRack…",
    "authorization-needed": "No LineRack authorized",
    authorizing: "Connecting to LineRack…",
    connected: `${hardwareProduct} connected`,
    disconnected: `${hardwareProduct} disconnected`,
    unsupported: "WebHID unavailable",
    error: "LineRack connection failed",
  }[hardwareState];

  const effectPlugins = selectedSlot.plugins.filter(
    (plugin) => plugin.pluginId !== "gain" && plugin.pluginId !== "limiter",
  );
  const effectCapacity = Math.max(0, capabilities.maxPluginsPerSlot - 2);
  const chainSlots = createChainSlots(selectedSlot.plugins, capabilities.maxPluginsPerSlot);
  const effectDefinitions = capabilities.plugins.filter(
    (plugin) => plugin.id !== "gain" && plugin.id !== "limiter",
  );
  const nextInsertSlotIndex = effectPlugins.length + 1;
  const sourceModes = capabilities.sourceModes ?? [];
  const sourceLabel = {
    usb: "USB input",
    analog: "analog input",
    mix: "USB + analog mix",
  }[presetFile.routing.sourceMode];
  const presetCategories = [...new Set(presetLibrary.map((recipe) => recipe.category))];

  return (
    <div className="app-shell">
      {showHardwarePrompt && hardwareState !== "connected" && (
        <div className="connection-backdrop">
          <section
            aria-describedby="connection-description"
            aria-labelledby="connection-title"
            aria-modal="true"
            className="connection-prompt"
            role="dialog"
          >
            <p className="section-label">Hardware connection</p>
            <h2 id="connection-title">Authorize your LineRack</h2>
            <p id="connection-description">
              Choose LineRack in Chrome's device prompt. Chrome reconnects automatically after the
              first authorization.
            </p>
            {hardwareError && <p className="connection-error">{hardwareError}</p>}
            <div className="connection-actions">
              <button
                className="primary"
                disabled={hardwareState === "authorizing"}
                onClick={authorizeHardware}
              >
                {hardwareState === "authorizing" ? "Connecting…" : "Authorize LineRack"}
              </button>
              <button className="secondary" onClick={() => setShowHardwarePrompt(false)}>
                Continue with simulator
              </button>
            </div>
          </section>
        </div>
      )}

      {showPresetLibrary && (
        <div className="connection-backdrop">
          <section
            aria-labelledby="preset-library-title"
            aria-modal="true"
            className="preset-library-dialog"
            role="dialog"
          >
            <div className="preset-library-heading">
              <div>
                <p className="section-label">Selected slot {selectedSlot.number}</p>
                <h2 id="preset-library-title">Preset library</h2>
              </div>
              <button className="secondary compact" onClick={() => setShowPresetLibrary(false)}>
                Close
              </button>
            </div>
            <p className="preset-library-intro">
              Choose a starting point. You can adjust the chain before applying it.
            </p>
            {presetCategories.map((category) => (
              <section className="preset-library-category" key={category}>
                <h3>{category}</h3>
                <div className="preset-library-grid">
                  {presetLibrary
                    .filter((recipe) => recipe.category === category)
                    .map((recipe) => {
                      const effectNames = recipe.blocks
                        .filter(
                          (block) => block.pluginId !== "gain" && block.pluginId !== "limiter",
                        )
                        .map(
                          (block) =>
                            capabilities.plugins.find((plugin) => plugin.id === block.pluginId)
                              ?.name ?? block.pluginId,
                        );
                      const supported = recipe.blocks.every((block) =>
                        capabilities.plugins.some((plugin) => plugin.id === block.pluginId),
                      );
                      return (
                        <article className="preset-library-card" key={recipe.id}>
                          <h4>{recipe.name}</h4>
                          <p>{recipe.description}</p>
                          <small>{effectNames.join(" · ") || "No effects"}</small>
                          <button
                            className="primary"
                            disabled={!supported}
                            onClick={() => usePresetRecipe(recipe)}
                          >
                            {supported ? `Use in slot ${selectedSlot.number}` : "Not supported"}
                          </button>
                        </article>
                      );
                    })}
                </div>
              </section>
            ))}
          </section>
        </div>
      )}

      <header className="topbar">
        <div className="brand-block">
          <p className="eyebrow">Portable audio DSP</p>
          <h1>LineRack</h1>
        </div>
        <SiteNav current="configurator" />
        <div className="device-controls">
          {hardwareState !== "connected" && hardwareState !== "unsupported" && (
            <button className="secondary compact" onClick={authorizeHardware}>
              Connect LineRack
            </button>
          )}
          {!editingHardware && (
            <button className="secondary compact simulate-button" onClick={pressDeviceButton}>
              Simulate double tap
            </button>
          )}
          <div className={`device-state ${hardwareState}`}>
            <span className="status-dot" />
            <span className="device-copy">
              <strong>{hardwareLabel}</strong>
              <small>
                Editing {editingHardware ? "hardware" : "simulator"} · preset{" "}
                {deviceStatus.activeSlot}
              </small>
            </span>
          </div>
        </div>
      </header>

      <nav aria-label="Configurator sections" className="workspace-tabs">
        <button
          aria-current={workspaceView === "global" ? "page" : undefined}
          onClick={() => setWorkspaceView("global")}
        >
          Global
          <small>Source, listening, display</small>
        </button>
        <button
          aria-current={workspaceView === "presets" ? "page" : undefined}
          onClick={() => setWorkspaceView("presets")}
        >
          Presets
          <small>Slots, recipes, signal chain</small>
        </button>
      </nav>

      <aside className="sidebar" hidden={workspaceView !== "presets"}>
        <section>
          <p className="section-label">Preset slots</p>
          <div className="slot-list">
            {presetFile.slots.map((slot) => {
              const effectCount = slot.plugins.filter(
                (plugin) => plugin.pluginId !== "gain" && plugin.pluginId !== "limiter",
              ).length;
              return (
                <button
                  className={slot.number === selectedSlot.number ? "slot active" : "slot"}
                  key={slot.number}
                  onClick={() => setSelectedSlotNumber(slot.number)}
                >
                  <span>{slot.number}</span>
                  <strong>{slot.name}</strong>
                  <small>
                    {effectCount} effects
                    {slot.number === deviceStatus.activeSlot ? " · active" : ""}
                  </small>
                </button>
              );
            })}
          </div>
        </section>

        <section className="file-actions">
          <input
            accept=".json,.linerack.json,application/json"
            hidden
            onChange={importPresetFile}
            ref={fileInput}
            type="file"
          />
          <input
            accept=".json,.linerack-preset.json,application/json"
            hidden
            onChange={importSharedPresetFile}
            ref={presetInput}
            type="file"
          />
          <p className="section-label">Share this preset</p>
          <button className="secondary" onClick={() => presetInput.current?.click()}>
            Import preset
          </button>
          <button
            className="secondary"
            onClick={() =>
              downloadJsonFile(
                createSharedPresetFile(selectedSlot, presetFile.engine),
                presetFilename(selectedSlot.name),
              )
            }
          >
            Download preset
          </button>
          <p className="section-label">Complete setup</p>
          <button className="secondary" onClick={() => fileInput.current?.click()}>
            Import setup
          </button>
          <button className="secondary" onClick={() => downloadPresetFile(presetFile)}>
            Download setup
          </button>
        </section>
      </aside>

      <main className={workspaceView === "global" ? "global-main" : undefined}>
        {workspaceView === "global" && (
          <section className="workspace-header">
            <div>
              <p className="section-label">Global settings</p>
              <h2>Settings for every preset</h2>
              <p>Apply saves the complete setup to LineRack.</p>
            </div>
            <button className="primary" disabled={!dirty || applying} onClick={applyToDevice}>
              {applying ? "Applying…" : "Apply to device"}
            </button>
          </section>
        )}

        <section className="preset-header" hidden={workspaceView !== "presets"}>
          <div className="preset-number">{selectedSlot.number}</div>
          <div>
            <p className="section-label">Selected preset</p>
            <input
              aria-label="Preset name"
              className="preset-name"
              maxLength={capabilities.presetNameMaxLength ?? 32}
              onChange={(event) => updateSlot((slot) => ({ ...slot, name: event.target.value }))}
              value={selectedSlot.name}
            />
          </div>
          <div className="preset-actions">
            <button
              className="secondary"
              disabled={dirty || selectedSlot.number === deviceStatus.activeSlot}
              onClick={activateSelectedSlot}
            >
              {selectedSlot.number === deviceStatus.activeSlot ? "Active" : "Activate slot"}
            </button>
            <button className="primary" disabled={!dirty || applying} onClick={applyToDevice}>
              {applying ? "Applying…" : "Apply to device"}
            </button>
          </div>
        </section>

        {workspaceView === "global" && sourceModes.length > 0 && (
          <CollapsibleSection
            badge="Device-wide"
            className="routing-panel"
            defaultOpen
            label="Audio source"
            title="Choose the audio source"
          >
            <div className="routing-controls">
              <label className="routing-mode">
                <span>Source</span>
                <select
                  aria-label="Audio source mode"
                  onChange={(event) => updateSourceMode(event.target.value as AudioSourceMode)}
                  value={presetFile.routing.sourceMode}
                >
                  {sourceModes.map((mode) => (
                    <option key={mode} value={mode}>
                      {{ usb: "USB", analog: "Analog", mix: "USB + Analog" }[mode]}
                    </option>
                  ))}
                </select>
              </label>
              {(["usb", "analog"] as const).map((source) => {
                const value = presetFile.routing[source === "usb" ? "usbTrimDb" : "analogTrimDb"];
                return (
                  <label className="parameter" key={source}>
                    <span>{source === "usb" ? "USB trim" : "Analog trim"}</span>
                    <input
                      max={presetFile.routing.sourceMode === "mix" ? -6.5 : 0}
                      min={-24}
                      onChange={(event) => updateSourceTrim(source, event.target.valueAsNumber)}
                      step={0.5}
                      type="range"
                      value={value}
                    />
                    <output>{value.toFixed(1)} dB</output>
                  </label>
                );
              })}
            </div>
            {presetFile.routing.sourceMode === "mix" && (
              <p className="routing-note">
                To prevent clipping, mixed mode caps both trims at −6.5 dB.
              </p>
            )}
          </CollapsibleSection>
        )}

        {workspaceView === "global" && (
          <CollapsibleSection
            badge="Local only"
            className="audition-panel"
            defaultOpen
            label="Listen through LineRack"
            title="Test presets with your own audio"
          >
            <AudioAudition />
          </CollapsibleSection>
        )}

        {workspaceView === "global" && (
          <CollapsibleSection
            badge="Device-wide"
            className="display-panel"
            defaultOpen
            label="Device display"
            title="Choose what the OLED shows"
          >
            <OledPreview
              active={selectedSlot.number === deviceStatus.activeSlot}
              activeSlotNumber={deviceStatus.activeSlot}
              defaultMode={presetFile.display.defaultMode}
              blankingEnabled={presetFile.display.blankingEnabled}
              blankingSupported={capabilities.displayBlanking === true}
              onBlankingEnabledChange={updateDisplayBlanking}
              onDefaultModeChange={updateDisplayMode}
              sampleRate={presetFile.engine.sampleRate}
              slot={selectedSlot}
            />
          </CollapsibleSection>
        )}

        {workspaceView === "global" && (
          <CollapsibleSection
            badge={
              deviceStatus.diagnostics
                ? deviceStatus.diagnostics.underruns + deviceStatus.diagnostics.overruns === 0
                  ? "Healthy"
                  : "Attention"
                : "Unavailable"
            }
            className="diagnostics-panel"
            label="Diagnostics"
            onOpenChange={setDiagnosticsOpen}
            title="Check USB audio"
          >
            {deviceStatus.diagnostics ? (
              <>
                <div className="diagnostics-grid">
                  <article>
                    <span>USB packets</span>
                    <strong>{deviceStatus.diagnostics.usbPackets.toLocaleString()}</strong>
                  </article>
                  <article>
                    <span>Buffer fill</span>
                    <strong>{deviceStatus.diagnostics.bufferFillFrames} frames</strong>
                  </article>
                  <article>
                    <span>Underruns</span>
                    <strong>{deviceStatus.diagnostics.underruns.toLocaleString()}</strong>
                  </article>
                  <article>
                    <span>Overruns</span>
                    <strong>{deviceStatus.diagnostics.overruns.toLocaleString()}</strong>
                  </article>
                </div>
                <p className="diagnostics-note">
                  Counters update once per second while this panel is open. They reset when LineRack
                  restarts.
                </p>
              </>
            ) : (
              <p className="diagnostics-note">
                This firmware does not expose live audio diagnostics.
              </p>
            )}
          </CollapsibleSection>
        )}

        <CollapsibleSection
          badge={`Slot ${selectedSlot.number}`}
          className="preset-starters-panel"
          defaultOpen
          hidden={workspaceView !== "presets"}
          label="Preset library"
          title="Choose a starting preset"
        >
          <div className="preset-starter-actions">
            <article>
              <h3>Replace this preset</h3>
              <p>Load a preset into slot {selectedSlot.number}, then adjust it as needed.</p>
              <button className="primary" onClick={() => setShowPresetLibrary(true)}>
                Browse preset library
              </button>
            </article>
            <article>
              <h3>Restore starter presets</h3>
              <p>Replace all four slots with Clean, Punch, Small Room, and Wide Hall.</p>
              <button className="secondary" onClick={restoreStarterSetup}>
                Load starter set
              </button>
            </article>
          </div>
        </CollapsibleSection>

        <CollapsibleSection
          badge={`${effectPlugins.length} / ${effectCapacity} effects`}
          className="chain-panel"
          defaultOpen
          hidden={workspaceView !== "presets"}
          label="Signal chain"
          title={`${sourceLabel} → Gain → effects → Limiter → output`}
        >
          <div className="chain">
            {chainSlots.map((instance, index) => {
              if (!instance) {
                const insertable = index === nextInsertSlotIndex;
                const slotPickerOpen = insertSlotIndex === index;
                if (slotPickerOpen) {
                  return (
                    <div
                      className={`plugin-slot-skeleton slot-picker${dropSlotIndex === index ? " drop-target" : ""}`}
                      key={`empty-${index}`}
                      onDragOver={(event) => allowEffectDrop(event, index - 1, index)}
                      onDrop={(event) => dropEffect(event, index - 1)}
                    >
                      <div className="order-number">{index + 1}</div>
                      <label>
                        <span>Add effect</span>
                        <select
                          aria-label={`Effect for slot ${index + 1}`}
                          autoFocus
                          defaultValue=""
                          onChange={(event) => {
                            addPlugin(event.target.value, index - 1);
                            setInsertSlotIndex(undefined);
                          }}
                        >
                          <option disabled value="">
                            Choose an effect…
                          </option>
                          {effectDefinitions.map((plugin) => (
                            <option key={plugin.id} value={plugin.id}>
                              {plugin.name}
                            </option>
                          ))}
                        </select>
                      </label>
                      <button
                        aria-label={`Cancel adding effect to slot ${index + 1}`}
                        className="slot-picker-cancel"
                        onClick={() => setInsertSlotIndex(undefined)}
                        type="button"
                      >
                        ×
                      </button>
                    </div>
                  );
                }
                if (!insertable) {
                  return (
                    <div className="plugin-slot-skeleton" key={`empty-${index}`}>
                      <div className="order-number">{index + 1}</div>
                      <span>Empty effect slot</span>
                    </div>
                  );
                }
                return (
                  <button
                    className={`plugin-slot-skeleton slot-add${dropSlotIndex === index ? " drop-target" : ""}`}
                    key={`empty-${index}`}
                    onClick={() => setInsertSlotIndex(index)}
                    onDragOver={(event) => allowEffectDrop(event, index - 1, index)}
                    onDrop={(event) => dropEffect(event, index - 1)}
                    type="button"
                  >
                    <div className="order-number">{index + 1}</div>
                    <span className="slot-add-copy">
                      <strong aria-hidden="true">+</strong> Add effect
                    </span>
                    <small>Tap to choose or drop here</small>
                  </button>
                );
              }
              const definition = findCapabilityPlugin(capabilities, instance.pluginId);
              if (!definition) return null;
              const processorPresets = effectPresets[instance.pluginId] ?? [];
              const selectedProcessorPreset = matchingEffectPreset(
                instance.pluginId,
                instance.parameters,
              );
              const lockedEndpoint =
                instance.pluginId === "gain" || instance.pluginId === "limiter";
              const effectIndex = effectPlugins.findIndex((plugin) => plugin.id === instance.id);
              const canMoveDown = !lockedEndpoint && effectIndex < effectPlugins.length - 1;
              return (
                <article
                  className={`${instance.enabled ? "plugin-card" : "plugin-card disabled"}${lockedEndpoint ? " endpoint-card" : ""}${dropSlotIndex === index ? " drop-target" : ""}`}
                  key={instance.id}
                  onDragOver={
                    lockedEndpoint
                      ? undefined
                      : (event) => allowEffectDrop(event, effectIndex, index)
                  }
                  onDrop={lockedEndpoint ? undefined : (event) => dropEffect(event, effectIndex)}
                >
                  <div
                    className="plugin-heading"
                    draggable={!lockedEndpoint}
                    onDragEnd={finishDrag}
                    onDragStart={(event) => startChainDrag(event, instance.id)}
                  >
                    <div className="order-number">{index + 1}</div>
                    <div>
                      <h3>{definition.name}</h3>
                      <p>{definition.description}</p>
                    </div>
                    <div className="plugin-heading-actions">
                      {lockedEndpoint && (
                        <span className="endpoint-lock">
                          {instance.pluginId === "gain" ? "Fixed input" : "Fixed output"}
                        </span>
                      )}
                      <label className="effect-preset-picker">
                        <span className="visually-hidden">{definition.name} preset</span>
                        <select
                          aria-label={`${definition.name} preset`}
                          onChange={(event) => {
                            const preset = processorPresets.find(
                              (candidate) => candidate.id === event.target.value,
                            );
                            if (!preset) return;
                            updatePlugin(instance.id, (plugin) => ({
                              ...plugin,
                              parameters: { ...plugin.parameters, ...preset.parameters },
                            }));
                          }}
                          onPointerDown={(event) => event.stopPropagation()}
                          value={selectedProcessorPreset}
                        >
                          <option value="">Custom</option>
                          {processorPresets.map((preset) => (
                            <option key={preset.id} value={preset.id}>
                              {preset.name}
                            </option>
                          ))}
                        </select>
                      </label>
                      <label className="bypass-toggle">
                        <input
                          checked={instance.enabled}
                          onChange={(event) =>
                            updatePlugin(instance.id, (plugin) => ({
                              ...plugin,
                              enabled: event.target.checked,
                            }))
                          }
                          type="checkbox"
                        />
                        Enabled
                      </label>
                      {!lockedEndpoint && (
                        <div className="block-actions">
                          <span aria-hidden="true" className="drag-handle" title="Drag to reorder">
                            ⠿
                          </span>
                          <button
                            aria-label={`Move ${definition.name} up`}
                            disabled={effectIndex === 0}
                            onClick={() => movePlugin(instance.id, effectIndex, -1)}
                            title="Move up"
                            type="button"
                          >
                            ↑
                          </button>
                          <button
                            aria-label={`Move ${definition.name} down`}
                            disabled={!canMoveDown}
                            onClick={() => movePlugin(instance.id, effectIndex, 1)}
                            title="Move down"
                            type="button"
                          >
                            ↓
                          </button>
                          <button
                            aria-label={`Remove ${definition.name}`}
                            className="remove"
                            onClick={() =>
                              updateSlot((slot) => ({
                                ...slot,
                                plugins: slot.plugins.filter((plugin) => plugin.id !== instance.id),
                              }))
                            }
                            title="Remove"
                            type="button"
                          >
                            ×
                          </button>
                        </div>
                      )}
                    </div>
                  </div>

                  {["parametric-eq", "graphic-eq", "high-pass", "low-pass"].includes(
                    instance.pluginId,
                  ) && (
                    <EqResponseGraph
                      chainPlugins={selectedSlot.plugins}
                      plugin={instance}
                      sampleRate={presetFile.engine.sampleRate}
                    />
                  )}

                  {instance.pluginId === "graphic-eq" ? (
                    <GraphicEqEditor
                      onChange={(parameterId, value) =>
                        updatePlugin(instance.id, (plugin) => ({
                          ...plugin,
                          parameters: { ...plugin.parameters, [parameterId]: value },
                        }))
                      }
                      plugin={instance}
                    />
                  ) : (
                    <div className="parameters">
                      {definition.parameters.map((parameter) => {
                        const value = Number(instance.parameters[parameter.id]);
                        const logarithmic = ["frequencyHz", "cutoffHz", "q"].includes(parameter.id);
                        const filterSlope = parameter.id === "slopeDbPerOct";
                        const sliderValue = logarithmic ? logarithmicSliderPosition(value) : value;
                        const sliderMinimum = logarithmic
                          ? logarithmicSliderPosition(parameter.min)
                          : parameter.min;
                        const sliderMaximum = logarithmic
                          ? logarithmicSliderPosition(parameter.max)
                          : parameter.max;
                        const setValue = (nextValue: ParameterValue) =>
                          updatePlugin(instance.id, (plugin) => ({
                            ...plugin,
                            parameters: { ...plugin.parameters, [parameter.id]: nextValue },
                          }));
                        return (
                          <label className="parameter" key={parameter.id}>
                            <span>{parameter.label}</span>
                            {filterSlope ? (
                              <select
                                aria-label={`${definition.name} slope`}
                                onChange={(event) => setValue(Number(event.target.value))}
                                value={value}
                              >
                                <option value={12}>12 dB/oct</option>
                                <option value={24}>24 dB/oct</option>
                              </select>
                            ) : (
                              <input
                                max={sliderMaximum}
                                min={sliderMinimum}
                                onChange={(event) =>
                                  setValue(
                                    logarithmic
                                      ? logarithmicSliderValue(
                                          event.target.valueAsNumber,
                                          parameter.min,
                                          parameter.max,
                                          parameter.step,
                                        )
                                      : event.target.valueAsNumber,
                                  )
                                }
                                step={logarithmic ? 0.001 : parameter.step}
                                type="range"
                                value={sliderValue}
                              />
                            )}
                            <output>
                              {formatParameterValue(value, parameter.step)} {parameter.unit}
                            </output>
                          </label>
                        );
                      })}
                    </div>
                  )}
                </article>
              );
            })}
          </div>
        </CollapsibleSection>

        <CollapsibleSection
          className="catalog-panel"
          hidden={workspaceView !== "presets"}
          label="Built-in effects"
          title="Add another effect"
        >
          <div className="catalog">
            {effectDefinitions.map((plugin) => {
              const unavailable = selectedSlot.plugins.length >= capabilities.maxPluginsPerSlot;
              return (
                <button
                  disabled={unavailable}
                  draggable={!unavailable}
                  key={plugin.id}
                  onClick={() => addPlugin(plugin.id)}
                  onDragEnd={finishDrag}
                  onDragStart={(event) => startCatalogDrag(event, plugin.id)}
                  title="Click to append or drag into a chain slot"
                >
                  <strong>{plugin.name}</strong>
                  <span>{plugin.description}</span>
                </button>
              );
            })}
          </div>
        </CollapsibleSection>
      </main>

      <footer>
        <span>{notice}</span>
        {dirty && <strong>Unapplied changes</strong>}
      </footer>
    </div>
  );
}
