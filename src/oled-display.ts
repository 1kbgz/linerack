import type { EqResponsePoint } from "./eq";
import type { DisplayMode, PresetSlot } from "./presets";

export const OLED_WIDTH = 128;
export const OLED_HEIGHT = 32;

export const PRESET_CHANGE_DISPLAY_MS = 5000;

export const resolveOledDisplayMode = (
  preferredMode: DisplayMode,
  presetDisplayUntilMs: number,
  nowMs: number,
): DisplayMode => (nowMs < presetDisplayUntilMs ? "preset" : preferredMode);

export interface OledFrame {
  pixels: Uint8Array;
  width: typeof OLED_WIDTH;
  height: typeof OLED_HEIGHT;
}

const FONT: Record<string, string[]> = Object.fromEntries(
  Object.entries({
    " ": "00000/00000/00000/00000/00000/00000/00000",
    "-": "00000/00000/00000/11111/00000/00000/00000",
    ".": "00000/00000/00000/00000/00000/00110/00110",
    "?": "01110/10001/00001/00010/00100/00000/00100",
    "0": "01110/10001/10011/10101/11001/10001/01110",
    "1": "00100/01100/00100/00100/00100/00100/01110",
    "2": "01110/10001/00001/00010/00100/01000/11111",
    "3": "11110/00001/00001/01110/00001/00001/11110",
    "4": "00010/00110/01010/10010/11111/00010/00010",
    "5": "11111/10000/10000/11110/00001/00001/11110",
    "6": "01110/10000/10000/11110/10001/10001/01110",
    "7": "11111/00001/00010/00100/01000/01000/01000",
    "8": "01110/10001/10001/01110/10001/10001/01110",
    "9": "01110/10001/10001/01111/00001/00001/01110",
    A: "01110/10001/10001/11111/10001/10001/10001",
    B: "11110/10001/10001/11110/10001/10001/11110",
    C: "01111/10000/10000/10000/10000/10000/01111",
    D: "11110/10001/10001/10001/10001/10001/11110",
    E: "11111/10000/10000/11110/10000/10000/11111",
    F: "11111/10000/10000/11110/10000/10000/10000",
    G: "01111/10000/10000/10111/10001/10001/01111",
    H: "10001/10001/10001/11111/10001/10001/10001",
    I: "01110/00100/00100/00100/00100/00100/01110",
    J: "00001/00001/00001/00001/10001/10001/01110",
    K: "10001/10010/10100/11000/10100/10010/10001",
    L: "10000/10000/10000/10000/10000/10000/11111",
    M: "10001/11011/10101/10101/10001/10001/10001",
    N: "10001/11001/10101/10011/10001/10001/10001",
    O: "01110/10001/10001/10001/10001/10001/01110",
    P: "11110/10001/10001/11110/10000/10000/10000",
    Q: "01110/10001/10001/10001/10101/10010/01101",
    R: "11110/10001/10001/11110/10100/10010/10001",
    S: "01111/10000/10000/01110/00001/00001/11110",
    T: "11111/00100/00100/00100/00100/00100/00100",
    U: "10001/10001/10001/10001/10001/10001/01110",
    V: "10001/10001/10001/10001/10001/01010/00100",
    W: "10001/10001/10001/10101/10101/10101/01010",
    X: "10001/10001/01010/00100/01010/10001/10001",
    Y: "10001/10001/01010/00100/00100/00100/00100",
    Z: "11111/00001/00010/00100/01000/10000/11111",
  }).map(([character, rows]) => [character, rows.split("/")]),
);

const createFrame = (): OledFrame => ({
  pixels: new Uint8Array(OLED_WIDTH * OLED_HEIGHT),
  width: OLED_WIDTH,
  height: OLED_HEIGHT,
});

const setPixel = (frame: OledFrame, x: number, y: number) => {
  if (x < 0 || x >= frame.width || y < 0 || y >= frame.height) return;
  frame.pixels[y * frame.width + x] = 1;
};

const drawLine = (frame: OledFrame, startX: number, startY: number, endX: number, endY: number) => {
  let x = startX;
  let y = startY;
  const deltaX = Math.abs(endX - startX);
  const deltaY = -Math.abs(endY - startY);
  const stepX = startX < endX ? 1 : -1;
  const stepY = startY < endY ? 1 : -1;
  let error = deltaX + deltaY;

  while (true) {
    setPixel(frame, x, y);
    if (x === endX && y === endY) break;
    const doubledError = 2 * error;
    if (doubledError >= deltaY) {
      error += deltaY;
      x += stepX;
    }
    if (doubledError <= deltaX) {
      error += deltaX;
      y += stepY;
    }
  }
};

const drawText = (frame: OledFrame, text: string, x: number, y: number, scale = 1) => {
  let cursor = x;
  for (const character of text.toUpperCase()) {
    const glyph = FONT[character] ?? FONT["?"];
    glyph.forEach((row, rowIndex) => {
      [...row].forEach((pixel, columnIndex) => {
        if (pixel !== "1") return;
        for (let scaleY = 0; scaleY < scale; scaleY += 1) {
          for (let scaleX = 0; scaleX < scale; scaleX += 1) {
            setPixel(frame, cursor + columnIndex * scale + scaleX, y + rowIndex * scale + scaleY);
          }
        }
      });
    });
    cursor += 6 * scale;
  }
};

const createPresetFrame = (slot: PresetSlot, active: boolean): OledFrame => {
  const frame = createFrame();
  const number = String(slot.number);
  drawText(frame, number, 2, number.length === 1 ? 5 : 9, number.length === 1 ? 3 : 2);
  drawLine(frame, 22, 2, 22, 29);
  drawText(frame, slot.name.slice(0, 12), 28, 3);
  const blockCount = slot.plugins.filter((plugin) => plugin.enabled).length;
  drawText(frame, `${blockCount} BLOCKS`, 28, 21);
  drawText(frame, active ? "LIVE" : "EDIT", 100, 21);
  return frame;
};

const createEqFrame = (response: EqResponsePoint[]): OledFrame => {
  const frame = createFrame();
  for (let x = 0; x < OLED_WIDTH; x += 2) setPixel(frame, x, 16);

  response.forEach((point, index) => {
    const x = response.length === 1 ? 0 : Math.round((index / (response.length - 1)) * 127);
    const gainDb = Math.round(Math.max(-18, Math.min(18, point.gainDb)) * 4) / 4;
    const y = Math.round(16 - (gainDb / 18) * 13);
    if (index === 0) {
      setPixel(frame, x, y);
      return;
    }
    const previous = response[index - 1];
    const previousX = Math.round(((index - 1) / (response.length - 1)) * 127);
    const previousGainDb = Math.round(Math.max(-18, Math.min(18, previous.gainDb)) * 4) / 4;
    const previousY = Math.round(16 - (previousGainDb / 18) * 13);
    drawLine(frame, previousX, previousY, x, y);
  });

  drawText(frame, "EQ", 2, 2);
  drawText(frame, "20", 2, 23);
  drawText(frame, "20K", 108, 23);
  return frame;
};

const drawMeter = (frame: OledFrame, label: string, percent: number, top: number) => {
  const boundedPercent = Math.max(0, Math.min(100, percent));
  drawText(frame, label, 2, top + 1);
  drawLine(frame, 11, top, 126, top);
  drawLine(frame, 11, top + 9, 126, top + 9);
  drawLine(frame, 11, top, 11, top + 9);
  drawLine(frame, 126, top, 126, top + 9);
  const fillWidth = Math.floor((boundedPercent * 112) / 100);
  for (let x = 13; x < 13 + fillWidth; x += 1) {
    for (let y = top + 2; y <= top + 7; y += 1) setPixel(frame, x, y);
  }
};

const createVisualizerFrame = (leftPercent: number, rightPercent: number): OledFrame => {
  const frame = createFrame();
  drawMeter(frame, "L", leftPercent, 3);
  drawMeter(frame, "R", rightPercent, 20);
  return frame;
};

export const createOledFrame = (
  mode: DisplayMode,
  slot: PresetSlot,
  active: boolean,
  response: EqResponsePoint[],
  levels = { left: 0, right: 0 },
): OledFrame =>
  mode === "preset"
    ? createPresetFrame(slot, active)
    : mode === "eq-response"
      ? createEqFrame(response)
      : createVisualizerFrame(levels.left, levels.right);

export const oledPixel = (frame: OledFrame, x: number, y: number): boolean =>
  frame.pixels[y * frame.width + x] === 1;
