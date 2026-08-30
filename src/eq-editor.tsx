import { calculateEqResponse, GRAPHIC_EQ_BANDS } from "./eq";
import type { PluginInstance } from "./presets";
import { formatParameterValue } from "./parameter-format";

interface EqResponseGraphProps {
  chainPlugins: PluginInstance[];
  plugin: PluginInstance;
  sampleRate: number;
}

const responseCoordinates = (plugins: PluginInstance[], sampleRate: number) =>
  calculateEqResponse(plugins, sampleRate, 181)
    .map((point, index) => {
      const x = (index / 180) * 720;
      const y = 90 - (Math.max(-18, Math.min(18, point.gainDb)) / 18) * 72;
      return `${x.toFixed(1)},${y.toFixed(1)}`;
    })
    .join(" ");

const responsePluginIds = new Set(["parametric-eq", "graphic-eq", "high-pass", "low-pass"]);

export function EqResponseGraph({ chainPlugins, plugin, sampleRate }: EqResponseGraphProps) {
  const points = responseCoordinates([plugin], sampleRate);
  const responsePluginCount = chainPlugins.filter(
    (instance) => instance.enabled && responsePluginIds.has(instance.pluginId),
  ).length;
  const netPoints =
    responsePluginCount > 1 ? responseCoordinates(chainPlugins, sampleRate) : undefined;
  return (
    <div className="eq-response" aria-label="Block and net EQ frequency response">
      <svg role="img" viewBox="0 0 720 180">
        <title>Current block response from 20 Hz to 20 kHz, with optional net chain response</title>
        {[18, 54, 90, 126, 162].map((y) => (
          <line
            className={y === 90 ? "eq-zero" : "eq-grid"}
            key={y}
            x1="0"
            x2="720"
            y1={y}
            y2={y}
          />
        ))}
        {[0, 120, 240, 360, 480, 600, 720].map((x) => (
          <line className="eq-grid" key={x} x1={x} x2={x} y1="0" y2="180" />
        ))}
        {netPoints && <polyline className="eq-net-curve" points={netPoints} />}
        <polyline className="eq-curve" points={points} />
      </svg>
      <div className="eq-axis-labels" aria-hidden="true">
        <span>20</span>
        <span>100</span>
        <span>500</span>
        <span>2k</span>
        <span>10k</span>
        <span>20k Hz</span>
      </div>
      {netPoints && (
        <div className="eq-legend">
          <span className="eq-legend-block">This block</span>
          <span className="eq-legend-net">Net chain</span>
        </div>
      )}
    </div>
  );
}

interface GraphicEqEditorProps {
  plugin: PluginInstance;
  onChange: (parameterId: string, value: number) => void;
}

export function GraphicEqEditor({ plugin, onChange }: GraphicEqEditorProps) {
  return (
    <div className="graphic-eq-editor">
      <div className="graphic-eq-controls">
        {GRAPHIC_EQ_BANDS.map((band) => {
          const value = Number(plugin.parameters[band.id] ?? 0);
          return (
            <label key={band.id}>
              <output>
                {value > 0 ? "+" : ""}
                {formatParameterValue(value, 0.5)}
              </output>
              <input
                aria-label={`${band.label} Hz gain`}
                max="12"
                min="-12"
                onChange={(event) => onChange(band.id, event.target.valueAsNumber)}
                step="0.5"
                type="range"
                value={value}
              />
              <span>{band.label}</span>
            </label>
          );
        })}
      </div>
    </div>
  );
}
