import {
  Command,
  encodeMessage,
  HID_REPORT_ID,
  MessageAssembler,
  MessageKind,
  ProtocolMessage,
} from "./hid-protocol";

export interface HidPort {
  open(): Promise<void>;
  close(): Promise<void>;
  sendReport(reportId: number, data: Uint8Array): Promise<void>;
  onInputReport(listener: (reportId: number, data: Uint8Array) => void): () => void;
}

interface PendingRequest {
  command: Command;
  resolve: (payload: unknown) => void;
  reject: (error: Error) => void;
  timeout: ReturnType<typeof setTimeout>;
}

type EventListener = (message: ProtocolMessage) => void;

export class HidRpcClient {
  private readonly assembler = new MessageAssembler();
  private readonly pending = new Map<number, PendingRequest>();
  private readonly eventListeners = new Set<EventListener>();
  private nextRequestId = 1;
  private removeInputListener: (() => void) | undefined;
  private requestQueue: Promise<void> = Promise.resolve();

  constructor(private readonly port: HidPort) {}

  async open(): Promise<void> {
    if (this.removeInputListener) return;
    this.removeInputListener = this.port.onInputReport((reportId, data) => {
      if (reportId === HID_REPORT_ID) this.handleReport(data);
    });
    try {
      await this.port.open();
    } catch (error) {
      this.removeInputListener();
      this.removeInputListener = undefined;
      throw error;
    }
  }

  async close(): Promise<void> {
    this.removeInputListener?.();
    this.removeInputListener = undefined;
    for (const request of this.pending.values()) {
      clearTimeout(request.timeout);
      request.reject(new Error("HID connection closed"));
    }
    this.pending.clear();
    await this.port.close();
  }

  request<TResponse>(command: Command, payload: unknown, timeoutMs = 3000): Promise<TResponse> {
    const operation = this.requestQueue.then(() =>
      this.performRequest<TResponse>(command, payload, timeoutMs),
    );
    this.requestQueue = operation.then(
      () => undefined,
      () => undefined,
    );
    return operation;
  }

  private async performRequest<TResponse>(
    command: Command,
    payload: unknown,
    timeoutMs: number,
  ): Promise<TResponse> {
    if (!this.removeInputListener) throw new Error("HID connection is not open");
    const requestId = this.allocateRequestId();
    const response = new Promise<TResponse>((resolve, reject) => {
      const timeout = setTimeout(() => {
        this.pending.delete(requestId);
        reject(new Error(`HID command ${Command[command]} timed out`));
      }, timeoutMs);
      this.pending.set(requestId, {
        command,
        resolve: (value) => resolve(value as TResponse),
        reject,
        timeout,
      });
    });

    try {
      const reports = encodeMessage({ kind: MessageKind.Request, command, requestId, payload });
      for (const report of reports) await this.port.sendReport(HID_REPORT_ID, report);
    } catch (error) {
      const request = this.pending.get(requestId);
      if (request) {
        clearTimeout(request.timeout);
        this.pending.delete(requestId);
        request.reject(error instanceof Error ? error : new Error(String(error)));
      }
    }

    return response;
  }

  onEvent(listener: EventListener): () => void {
    this.eventListeners.add(listener);
    return () => this.eventListeners.delete(listener);
  }

  private allocateRequestId(): number {
    for (let attempts = 0; attempts < 0xffff; attempts += 1) {
      const requestId = this.nextRequestId;
      this.nextRequestId = requestId === 0xffff ? 1 : requestId + 1;
      if (!this.pending.has(requestId)) return requestId;
    }
    throw new Error("No HID request IDs are available");
  }

  private handleReport(report: Uint8Array): void {
    const message = this.assembler.accept(report);
    if (!message) return;
    if (message.kind === MessageKind.Event) {
      this.eventListeners.forEach((listener) => listener(message));
      return;
    }
    if (message.kind !== MessageKind.Response && message.kind !== MessageKind.Error) return;

    const request = this.pending.get(message.requestId);
    if (!request || request.command !== message.command) return;
    clearTimeout(request.timeout);
    this.pending.delete(message.requestId);

    if (message.kind === MessageKind.Error) {
      const payload = message.payload as { message?: unknown };
      request.reject(
        new Error(
          typeof payload?.message === "string" ? payload.message : "Device rejected HID command",
        ),
      );
    } else {
      request.resolve(message.payload);
    }
  }
}
