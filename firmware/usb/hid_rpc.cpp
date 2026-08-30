#include "hid_rpc.h"

#include "audio_device.h"
#include "preset_cbor.h"

#include <cstring>

namespace
{
constexpr size_t kReportBytes = 64U;
constexpr size_t kHeaderBytes = 12U;
constexpr size_t kChunkBytes = kReportBytes - kHeaderBytes;
constexpr size_t kMaximumChunks = 255U;
constexpr size_t kMaximumRequestBytes = kChunkBytes * kMaximumChunks;
constexpr size_t kResponseBytes = 8192U;
constexpr uint32_t kRequestIdlePollLimit = 3000U;

enum MessageKind : uint8_t
{
    kRequest = 1U,
    kResponse = 2U,
    kEvent = 3U,
    kError = 4U,
};

enum Command : uint8_t
{
    kHello = 1U,
    kReadPresets = 2U,
    kWritePresets = 3U,
    kActivateSlot = 4U,
    kGetStatus = 5U,
    kCyclePreset = 6U,
    kStatusChanged = 7U,
    kWakeDisplay = 8U,
};

LineRackHidRpcCallbacks callbacks;
uint8_t request_payload[kMaximumRequestBytes];
uint8_t response_payload[kResponseBytes];
uint8_t outgoing_report[kReportBytes];
uint8_t received_chunks[32];
volatile bool request_active;
volatile bool request_ready;
volatile bool status_event_pending;
uint8_t request_command;
uint16_t request_id;
uint8_t request_chunk_count;
uint8_t received_count;
uint8_t final_chunk_size;
volatile uint32_t request_idle_polls;

bool response_active;
uint8_t response_kind;
uint8_t response_command;
uint16_t response_request_id;
size_t response_size;
uint8_t response_chunk_count;
uint8_t response_next_chunk;
bool response_resets_request;

uint16_t ReadLe16(const uint8_t *data)
{
    return static_cast<uint16_t>(data[0])
           | static_cast<uint16_t>(data[1] << 8U);
}

void WriteLe16(uint8_t *data, uint16_t value)
{
    data[0] = static_cast<uint8_t>(value);
    data[1] = static_cast<uint8_t>(value >> 8U);
}

void ResetRequest()
{
    request_active = false;
    request_ready = false;
    received_count = 0U;
    final_chunk_size = 0U;
    request_idle_polls = 0U;
    std::memset(received_chunks, 0, sizeof(received_chunks));
}

bool ChunkReceived(uint8_t index)
{
    return (received_chunks[index / 8U] & (1U << (index % 8U))) != 0U;
}

void MarkChunkReceived(uint8_t index)
{
    received_chunks[index / 8U] |= (1U << (index % 8U));
}

void StartResponse(uint8_t kind,
                   uint8_t command,
                   uint16_t id,
                   size_t size)
{
    response_resets_request = kind != kEvent;
    if(size == 0U || size > sizeof(response_payload))
    {
        kind = kError;
        size = LineRackCborEncodeError(
            response_payload, sizeof(response_payload), "Response too large");
    }
    response_kind = kind;
    response_command = command;
    response_request_id = id;
    response_size = size;
    response_chunk_count = static_cast<uint8_t>((size + kChunkBytes - 1U) / kChunkBytes);
    response_next_chunk = 0U;
    response_active = true;
}

void ErrorResponse(const char *message)
{
    StartResponse(kError,
                  request_command,
                  request_id,
                  LineRackCborEncodeError(
                      response_payload, sizeof(response_payload), message));
}

size_t RequestSize()
{
    return static_cast<size_t>(request_chunk_count - 1U) * kChunkBytes
           + final_chunk_size;
}

LineRackDeviceDiagnostics Diagnostics()
{
    return {
        LineRackUsbAudioPacketCount(),
        LineRackUsbAudioUnderrunCount(),
        LineRackUsbAudioOverrunCount(),
        LineRackUsbAudioBufferFill(),
    };
}

size_t EncodeStatus(uint8_t active_slot)
{
    const LineRackDeviceDiagnostics diagnostics = Diagnostics();
    return LineRackCborEncodeStatus(
        response_payload, sizeof(response_payload), active_slot, &diagnostics);
}

void ProcessRequest()
{
    size_t encoded_size = 0U;
    switch(request_command)
    {
        case kHello:
        {
            const LineRackDeviceDiagnostics diagnostics = Diagnostics();
            encoded_size = LineRackCborEncodeHello(
                response_payload, sizeof(response_payload),
                callbacks.active_slot(callbacks.context), &diagnostics);
            break;
        }
        case kReadPresets:
        {
            LineRackPresetBank bank;
            callbacks.read_presets(callbacks.context, &bank);
            encoded_size = LineRackCborEncodePresets(
                response_payload, sizeof(response_payload), &bank);
            break;
        }
        case kWritePresets:
        {
            LineRackPresetBank bank;
            if(!LineRackCborDecodePresets(
                   request_payload, RequestSize(), &bank))
            {
                ErrorResponse("Invalid or unsupported preset setup");
                return;
            }
            if(!callbacks.write_presets(callbacks.context, &bank))
            {
                ErrorResponse("Could not persist preset setup");
                return;
            }
            encoded_size = LineRackCborEncodePresets(
                response_payload, sizeof(response_payload), &bank);
            break;
        }
        case kActivateSlot:
        {
            uint8_t slot;
            if(!LineRackCborDecodeSlotNumber(
                   request_payload, RequestSize(), &slot)
               || !callbacks.activate_slot(callbacks.context, slot))
            {
                ErrorResponse("Invalid preset slot");
                return;
            }
            encoded_size = EncodeStatus(slot);
            status_event_pending = true;
            break;
        }
        case kGetStatus:
            encoded_size = EncodeStatus(callbacks.active_slot(callbacks.context));
            break;
        case kCyclePreset:
            if(!callbacks.cycle_preset(callbacks.context))
            {
                ErrorResponse("Could not cycle preset");
                return;
            }
            encoded_size = EncodeStatus(callbacks.active_slot(callbacks.context));
            break;
        case kWakeDisplay:
            callbacks.wake_display(callbacks.context);
            encoded_size = EncodeStatus(callbacks.active_slot(callbacks.context));
            break;
        default:
            ErrorResponse("Unsupported command");
            return;
    }
    StartResponse(kResponse, request_command, request_id, encoded_size);
}

void SendNextReport()
{
    const size_t offset = static_cast<size_t>(response_next_chunk) * kChunkBytes;
    const size_t remaining = response_size - offset;
    const uint8_t chunk_size = static_cast<uint8_t>(remaining < kChunkBytes ? remaining : kChunkBytes);
    std::memset(outgoing_report, 0, sizeof(outgoing_report));
    outgoing_report[0] = 'L';
    outgoing_report[1] = 'R';
    outgoing_report[2] = 1U;
    outgoing_report[3] = response_kind;
    outgoing_report[4] = response_command;
    WriteLe16(outgoing_report + 6U, response_request_id);
    WriteLe16(outgoing_report + 8U, response_next_chunk);
    outgoing_report[10] = response_chunk_count;
    outgoing_report[11] = chunk_size;
    std::memcpy(outgoing_report + kHeaderBytes,
                response_payload + offset,
                chunk_size);
    if(!LineRackUsbHidSendReport(outgoing_report, sizeof(outgoing_report)))
        return;
    ++response_next_chunk;
    if(response_next_chunk == response_chunk_count)
    {
        response_active = false;
        if(response_resets_request)
            ResetRequest();
    }
}
} // namespace

extern "C" void LineRackHidRpcInit(const LineRackHidRpcCallbacks *configuration)
{
    callbacks = *configuration;
    response_active = false;
    status_event_pending = false;
    ResetRequest();
}

extern "C" void LineRackHidRpcAcceptReport(const uint8_t report[64])
{
    if(report == nullptr || request_ready || report[0] != 'L'
       || report[1] != 'R' || report[2] != 1U || report[3] != kRequest
       || report[10] == 0U || report[11] > kChunkBytes)
        return;
    const uint8_t command = report[4];
    const uint16_t id = ReadLe16(report + 6U);
    const uint16_t chunk_index = ReadLe16(report + 8U);
    const uint8_t chunk_count = report[10];
    if(chunk_index >= chunk_count)
        return;

    const bool different_request =
        request_active
        && (command != request_command || id != request_id
            || chunk_count != request_chunk_count);
    if(!request_active || (different_request && chunk_index == 0U))
    {
        request_active = true;
        request_command = command;
        request_id = id;
        request_chunk_count = chunk_count;
        received_count = 0U;
        final_chunk_size = 0U;
        request_idle_polls = 0U;
        std::memset(received_chunks, 0, sizeof(received_chunks));
    }
    else if(different_request)
        return;
    if(ChunkReceived(static_cast<uint8_t>(chunk_index)))
        return;
    if(chunk_index + 1U < chunk_count && report[11] != kChunkBytes)
        return;

    std::memcpy(request_payload + chunk_index * kChunkBytes,
                report + kHeaderBytes,
                report[11]);
    request_idle_polls = 0U;
    MarkChunkReceived(static_cast<uint8_t>(chunk_index));
    ++received_count;
    if(chunk_index + 1U == chunk_count)
        final_chunk_size = report[11];
    if(received_count == request_chunk_count && final_chunk_size > 0U)
        request_ready = true;
}

extern "C" void LineRackHidRpcPoll(void)
{
    if(response_active)
    {
        SendNextReport();
        return;
    }
    if(request_ready)
    {
        ProcessRequest();
        return;
    }
    if(request_active)
    {
        ++request_idle_polls;
        if(request_idle_polls >= kRequestIdlePollLimit)
            ResetRequest();
        return;
    }
    if(status_event_pending)
    {
        status_event_pending = false;
        StartResponse(kEvent,
                      kStatusChanged,
                      0U,
                      EncodeStatus(callbacks.active_slot(callbacks.context)));
    }
}

extern "C" void LineRackHidRpcNotifyStatusChanged(void)
{
    status_event_pending = true;
}
