import type { PluginInstance } from "./presets";

const splitEndpoints = (plugins: PluginInstance[]) => ({
  gain: plugins.find((plugin) => plugin.pluginId === "gain"),
  effects: plugins.filter((plugin) => plugin.pluginId !== "gain" && plugin.pluginId !== "limiter"),
  limiter: plugins.find((plugin) => plugin.pluginId === "limiter"),
});

const joinEndpoints = (
  effects: PluginInstance[],
  gain?: PluginInstance,
  limiter?: PluginInstance,
): PluginInstance[] => [...(gain ? [gain] : []), ...effects, ...(limiter ? [limiter] : [])];

export const createChainSlots = (
  plugins: PluginInstance[],
  maxPluginsPerSlot: number,
): Array<PluginInstance | null> => {
  const { effects, gain, limiter } = splitEndpoints(plugins);
  const effectCapacity = Math.max(0, maxPluginsPerSlot - 2);
  return [
    ...(gain ? [gain] : []),
    ...effects,
    ...Array.from({ length: Math.max(0, effectCapacity - effects.length) }, () => null),
    ...(limiter ? [limiter] : []),
  ];
};

export const insertEffectAt = (
  plugins: PluginInstance[],
  effect: PluginInstance,
  targetIndex: number,
): PluginInstance[] => {
  const { effects, gain, limiter } = splitEndpoints(plugins);
  effects.splice(Math.max(0, Math.min(targetIndex, effects.length)), 0, effect);
  return joinEndpoints(effects, gain, limiter);
};

export const moveEffectTo = (
  plugins: PluginInstance[],
  instanceId: string,
  targetIndex: number,
): PluginInstance[] => {
  const { effects, gain, limiter } = splitEndpoints(plugins);
  const sourceIndex = effects.findIndex((plugin) => plugin.id === instanceId);
  if (sourceIndex < 0) return plugins;
  const [effect] = effects.splice(sourceIndex, 1);
  effects.splice(Math.max(0, Math.min(targetIndex, effects.length)), 0, effect);
  return joinEndpoints(effects, gain, limiter);
};
