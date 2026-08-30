const decimalPlacesForStep = (step: number): number => {
  let decimalPlaces = 6;
  for (let places = 0; places <= 6; places += 1) {
    const roundedStep = Number(step.toFixed(places));
    const tolerance = Math.max(1e-7, Math.abs(step) * 1e-6);
    if (Math.abs(step - roundedStep) <= tolerance) {
      decimalPlaces = places;
      break;
    }
  }
  return decimalPlaces;
};

export const formatParameterValue = (value: number, step: number): string =>
  value.toFixed(decimalPlacesForStep(step));

export const logarithmicSliderPosition = (value: number): number => Math.log10(value);

export const logarithmicSliderValue = (
  position: number,
  minimum: number,
  maximum: number,
  step: number,
): number => {
  const rawValue = 10 ** position;
  const stepCount = Math.round((rawValue - minimum) / step);
  const snapped = minimum + stepCount * step;
  const clamped = Math.min(maximum, Math.max(minimum, snapped));
  return Number(clamped.toFixed(decimalPlacesForStep(step)));
};
