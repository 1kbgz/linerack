import { Decoder, Encoder } from "cbor-x";

export const HID_REPORT_ID = 0;
export const HID_REPORT_BYTES = 64;
export const HID_HEADER_BYTES = 12;
export const HID_PAYLOAD_BYTES = HID_REPORT_BYTES - HID_HEADER_BYTES;
export const HID_PROTOCOL_VERSION = 1;
export const HID_USAGE_PAGE = 0xff00;
export const HID_USAGE = 0x01;

const MAGIC_0 = 0x4c;
const MAGIC_1 = 0x52;
const cborEncoder = new Encoder({ useRecords: false, variableMapSize: true });
const cborDecoder = new Decoder({ mapsAsObjects: true });

export enum MessageKind {
  Request = 1,
  Response = 2,
  Event = 3,
  Error = 4,
}

export enum Command {
  Hello = 1,
  ReadPresets = 2,
  WritePresets = 3,
  ActivateSlot = 4,
  GetStatus = 5,
  CyclePreset = 6,
  StatusChanged = 7,
  WakeDisplay = 8,
}

export interface ProtocolMessage<T = unknown> {
  kind: MessageKind;
  command: Command;
  requestId: number;
  payload: T;
}

interface DecodedFrame {
  kind: MessageKind;
  command: Command;
  requestId: number;
  chunkIndex: number;
  chunkCount: number;
  payload: Uint8Array;
}

const encodePayload = (payload: unknown): Uint8Array => new Uint8Array(cborEncoder.encode(payload));

const decodePayload = <T>(payload: Uint8Array): T => cborDecoder.decode(payload) as T;

export const encodeMessage = (message: ProtocolMessage): Uint8Array[] => {
  if (!Number.isInteger(message.requestId) || message.requestId < 0 || message.requestId > 0xffff) {
    throw new Error("Request ID must be an unsigned 16-bit integer");
  }

  const payload = encodePayload(message.payload);
  const chunkCount = Math.max(1, Math.ceil(payload.byteLength / HID_PAYLOAD_BYTES));
  if (chunkCount > 0xff) throw new Error("Message exceeds the HID message limit");

  return Array.from({ length: chunkCount }, (_, chunkIndex) => {
    const payloadOffset = chunkIndex * HID_PAYLOAD_BYTES;
    const chunk = payload.subarray(payloadOffset, payloadOffset + HID_PAYLOAD_BYTES);
    const report = new Uint8Array(HID_REPORT_BYTES);
    const view = new DataView(report.buffer);
    report[0] = MAGIC_0;
    report[1] = MAGIC_1;
    report[2] = HID_PROTOCOL_VERSION;
    report[3] = message.kind;
    report[4] = message.command;
    report[5] = 0;
    view.setUint16(6, message.requestId, true);
    view.setUint16(8, chunkIndex, true);
    report[10] = chunkCount;
    report[11] = chunk.byteLength;
    report.set(chunk, HID_HEADER_BYTES);
    return report;
  });
};

export const decodeFrame = (report: Uint8Array): DecodedFrame => {
  if (report.byteLength !== HID_REPORT_BYTES) {
    throw new Error(`HID report must be ${HID_REPORT_BYTES} bytes`);
  }
  if (report[0] !== MAGIC_0 || report[1] !== MAGIC_1) throw new Error("Invalid HID message magic");
  if (report[2] !== HID_PROTOCOL_VERSION) throw new Error("Unsupported HID protocol version");

  const view = new DataView(report.buffer, report.byteOffset, report.byteLength);
  const chunkIndex = view.getUint16(8, true);
  const chunkCount = report[10];
  const payloadLength = report[11];
  if (chunkCount === 0 || chunkIndex >= chunkCount) throw new Error("Invalid HID chunk index");
  if (payloadLength > HID_PAYLOAD_BYTES) throw new Error("Invalid HID chunk length");

  return {
    kind: report[3] as MessageKind,
    command: report[4] as Command,
    requestId: view.getUint16(6, true),
    chunkIndex,
    chunkCount,
    payload: report.slice(HID_HEADER_BYTES, HID_HEADER_BYTES + payloadLength),
  };
};

interface PartialMessage {
  kind: MessageKind;
  command: Command;
  requestId: number;
  chunkCount: number;
  chunks: Array<Uint8Array | undefined>;
}

export class MessageAssembler {
  private readonly pending = new Map<string, PartialMessage>();

  accept(report: Uint8Array): ProtocolMessage | undefined {
    const frame = decodeFrame(report);
    const key = `${frame.kind}:${frame.command}:${frame.requestId}`;
    let partial = this.pending.get(key);

    if (!partial) {
      partial = {
        kind: frame.kind,
        command: frame.command,
        requestId: frame.requestId,
        chunkCount: frame.chunkCount,
        chunks: Array.from({ length: frame.chunkCount }),
      };
      this.pending.set(key, partial);
    } else if (partial.chunkCount !== frame.chunkCount) {
      this.pending.delete(key);
      throw new Error("HID chunk count changed within a message");
    }

    partial.chunks[frame.chunkIndex] = frame.payload;
    if (partial.chunks.some((chunk) => chunk === undefined)) return undefined;

    this.pending.delete(key);
    const payloadLength = partial.chunks.reduce((total, chunk) => total + chunk!.byteLength, 0);
    const payload = new Uint8Array(payloadLength);
    let offset = 0;
    for (const chunk of partial.chunks) {
      payload.set(chunk!, offset);
      offset += chunk!.byteLength;
    }

    return {
      kind: partial.kind,
      command: partial.command,
      requestId: partial.requestId,
      payload: decodePayload(payload),
    };
  }
}
