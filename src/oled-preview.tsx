import { useEffect, useMemo, useRef, useState } from "react";
import { calculateEqResponse } from "./eq";
import { createOledFrame, PRESET_CHANGE_DISPLAY_MS, resolveOledDisplayMode } from "./oled-display";
import type { DisplayMode, PresetSlot } from "./presets";

interface OledPreviewProps {
  active: boolean;
  activeSlotNumber: number;
  blankingEnabled: boolean;
  blankingSupported: boolean;
  defaultMode: DisplayMode;
  onBlankingEnabledChange: (enabled: boolean) => void;
  onDefaultModeChange: (mode: DisplayMode) => void;
  sampleRate: number;
  slot: PresetSlot;
}

const framePath = (pixels: Uint8Array, width: number): string => {
  let path = "";
  pixels.forEach((pixel, index) => {
    if (pixel === 0) return;
    const x = index % width;
    const y = Math.floor(index / width);
    path += `M${x} ${y}h1v1h-1z`;
  });
  return path;
};

export function OledPreview({
  active,
  activeSlotNumber,
  blankingEnabled,
  blankingSupported,
  defaultMode,
  onBlankingEnabledChange,
  onDefaultModeChange,
  sampleRate,
  slot,
}: OledPreviewProps) {
  const [presetDisplayUntil, setPresetDisplayUntil] = useState(0);
  const [now, setNow] = useState(() => Date.now());
  const previousActiveSlot = useRef(activeSlotNumber);

  useEffect(() => {
    if (previousActiveSlot.current === activeSlotNumber) return;
    previousActiveSlot.current = activeSlotNumber;
    const displayUntil = Date.now() + PRESET_CHANGE_DISPLAY_MS;
    setPresetDisplayUntil(displayUntil);
    setNow(Date.now());
    const timeout = window.setTimeout(() => setNow(Date.now()), PRESET_CHANGE_DISPLAY_MS);
    return () => window.clearTimeout(timeout);
  }, [activeSlotNumber]);

  useEffect(() => {
    if (defaultMode !== "visualizer") return;
    const interval = window.setInterval(() => setNow(Date.now()), 100);
    return () => window.clearInterval(interval);
  }, [defaultMode]);

  const mode = resolveOledDisplayMode(defaultMode, presetDisplayUntil, now);
  const presetOverrideActive = mode === "preset" && defaultMode !== "preset";
  const response = useMemo(
    () => calculateEqResponse(slot.plugins, sampleRate, 128),
    [sampleRate, slot.plugins],
  );
  const frame = useMemo(
    () =>
      createOledFrame(mode, slot, active, response, {
        left: 25 + Math.abs(Math.sin(now / 310)) * 70,
        right: 20 + Math.abs(Math.sin(now / 370 + 0.8)) * 75,
      }),
    [active, mode, now, response, slot],
  );
  const path = useMemo(() => framePath(frame.pixels, frame.width), [frame]);

  return (
    <div className="display-content">
      <div className="display-modes" aria-label="Default device display mode">
        <button
          className={defaultMode === "preset" ? "selected" : ""}
          onClick={() => onDefaultModeChange("preset")}
        >
          Preset
        </button>
        <button
          className={defaultMode === "eq-response" ? "selected" : ""}
          onClick={() => onDefaultModeChange("eq-response")}
        >
          EQ response
        </button>
        <button
          className={defaultMode === "visualizer" ? "selected" : ""}
          onClick={() => onDefaultModeChange("visualizer")}
        >
          Visualizer
        </button>
      </div>
      {blankingSupported && (
        <div className="display-power-setting">
          <label className="display-blanking-toggle">
            <input
              checked={blankingEnabled}
              onChange={(event) => onBlankingEnabledChange(event.target.checked)}
              type="checkbox"
            />
            <span>
              <strong>Blank OLED after 20 seconds</strong>
              <small>Editing, applying, or pressing the device button wakes it.</small>
            </span>
          </label>
          {!blankingEnabled && (
            <div className="display-burn-warning" role="alert">
              <strong>OLED burn-in risk</strong>
              <span>
                A static image can burn into the OLED. Keep blanking on unless you need the display
                to stay lit.
              </span>
            </div>
          )}
        </div>
      )}
      {presetOverrideActive && (
        <div className="display-override" role="status">
          <strong>Temporary Preset view</strong>
          <span>
            {defaultMode === "eq-response" ? "EQ response" : "Visualizer"} remains saved. It returns
            after five seconds.
          </span>
        </div>
      )}
      <div className="oled-bezel">
        <svg
          aria-label={`${mode === "preset" ? "Preset" : mode === "eq-response" ? "EQ response" : "Visualizer"} OLED preview`}
          className="oled-screen"
          role="img"
          shapeRendering="crispEdges"
          viewBox="0 0 128 32"
        >
          <rect width="128" height="32" />
          <path d={path} />
        </svg>
      </div>
      <p className="display-caption">
        {presetOverrideActive
          ? "Showing the preset briefly after a preset change"
          : `Default: ${
              defaultMode === "preset"
                ? "Preset"
                : defaultMode === "eq-response"
                  ? "EQ response"
                  : "Visualizer"
            }`}
        {` · ${active ? "currently active" : "editing inactive slot"}`}
      </p>
    </div>
  );
}
