#include "hid_rpc.h"

#include "preset_cbor.h"

#include <array>
#include <cassert>
#include <cstring>
#include <vector>

namespace
{
LineRackPresetBank stored;
uint8_t active_slot = 1U;
uint8_t display_wakes = 0U;
std::vector<std::array<uint8_t, 64>> sent_reports;

uint8_t ActiveSlot(void *) { return active_slot; }

bool ActivateSlot(void *, uint8_t slot)
{
    if(slot < 1U || slot > LINERACK_PRESET_COUNT) return false;
    active_slot = slot;
    return true;
}

bool CyclePreset(void *)
{
    active_slot = static_cast<uint8_t>(active_slot % LINERACK_PRESET_COUNT + 1U);
    return true;
}

void ReadPresets(void *, LineRackPresetBank *bank) { *bank = stored; }

bool WritePresets(void *, const LineRackPresetBank *bank)
{
    stored = *bank;
    return true;
}

void WakeDisplay(void *) { ++display_wakes; }

void SendRequest(uint8_t command,
                 uint16_t request_id,
                 const uint8_t *payload,
                 size_t size)
{
    const uint8_t chunk_count = static_cast<uint8_t>((size + 51U) / 52U);
    for(uint8_t chunk = 0U; chunk < chunk_count; ++chunk)
    {
        uint8_t report[64] = {};
        const size_t offset = static_cast<size_t>(chunk) * 52U;
        const size_t remaining = size - offset;
        const uint8_t chunk_size = static_cast<uint8_t>(remaining < 52U ? remaining : 52U);
        report[0] = 'L'; report[1] = 'R'; report[2] = 1U;
        report[3] = 1U; report[4] = command;
        report[6] = static_cast<uint8_t>(request_id);
        report[7] = static_cast<uint8_t>(request_id >> 8U);
        report[8] = chunk;
        report[10] = chunk_count;
        report[11] = chunk_size;
        std::memcpy(report + 12U, payload + offset, chunk_size);
        LineRackHidRpcAcceptReport(report);
    }
}

void SendPartialRequest(uint8_t command, uint16_t request_id)
{
    uint8_t report[64] = {};
    report[0] = 'L'; report[1] = 'R'; report[2] = 1U;
    report[3] = 1U; report[4] = command;
    report[6] = static_cast<uint8_t>(request_id);
    report[7] = static_cast<uint8_t>(request_id >> 8U);
    report[10] = 2U;
    report[11] = 52U;
    LineRackHidRpcAcceptReport(report);
}

void Drain()
{
    for(size_t poll = 0U; poll < 200U; ++poll)
        LineRackHidRpcPoll();
}
} // namespace

extern "C" bool LineRackUsbHidSendReport(const uint8_t *report, uint16_t size)
{
    assert(size == 64U);
    std::array<uint8_t, 64> copy;
    std::memcpy(copy.data(), report, copy.size());
    sent_reports.push_back(copy);
    return true;
}

extern "C" uint32_t LineRackUsbAudioPacketCount(void) { return 1234U; }
extern "C" uint32_t LineRackUsbAudioUnderrunCount(void) { return 2U; }
extern "C" uint32_t LineRackUsbAudioOverrunCount(void) { return 3U; }
extern "C" uint32_t LineRackUsbAudioBufferFill(void) { return 4U; }

int main()
{
    LineRackPresetBankDefaults(&stored);
    const LineRackHidRpcCallbacks callbacks = {
        ActiveSlot,
        ActivateSlot,
        CyclePreset,
        ReadPresets,
        WritePresets,
        WakeDisplay,
        nullptr,
    };
    LineRackHidRpcInit(&callbacks);

    const uint8_t empty_map[] = {0xa0U};
    SendRequest(1U, 7U, empty_map, sizeof(empty_map));
    Drain();
    assert(sent_reports.size() > 1U);
    assert(sent_reports.front()[3] == 2U);
    assert(sent_reports.front()[4] == 1U);
    assert(sent_reports.front()[6] == 7U);

    sent_reports.clear();
    SendRequest(5U, 16U, empty_map, sizeof(empty_map));
    Drain();
    assert(!sent_reports.empty());
    for(const auto &report : sent_reports)
        assert(report[3] == 2U);

    sent_reports.clear();
    LineRackPresetBank changed = stored;
    std::strcpy(changed.presets[0].name, "New name");
    uint8_t payload[4096];
    const size_t size = LineRackCborEncodePresets(payload, sizeof(payload), &changed);
    SendRequest(3U, 8U, payload, size);
    Drain();
    assert(LineRackPresetBankEqual(&stored, &changed));
    assert(sent_reports.front()[3] == 2U);
    assert(sent_reports.front()[4] == 3U);

    sent_reports.clear();
    SendRequest(8U, 15U, empty_map, sizeof(empty_map));
    Drain();
    assert(display_wakes == 1U);
    assert(sent_reports.front()[3] == 2U);
    assert(sent_reports.front()[4] == 8U);

    sent_reports.clear();
    const uint8_t slot_request[] = {0xa1U, 0x6aU, 's', 'l', 'o', 't', 'N',
                                    'u',   'm',   'b', 'e', 'r', 0x04U};
    SendRequest(4U, 9U, slot_request, sizeof(slot_request));
    Drain();
    assert(active_slot == 4U);
    assert(sent_reports.front()[3] == 2U);

    sent_reports.clear();
    LineRackHidRpcNotifyStatusChanged();
    Drain();
    assert(sent_reports.front()[3] == 3U);
    assert(sent_reports.front()[4] == 7U);

    sent_reports.clear();
    SendPartialRequest(1U, 10U);
    SendRequest(1U, 11U, empty_map, sizeof(empty_map));
    Drain();
    assert(!sent_reports.empty());
    assert(sent_reports.front()[3] == 2U);
    assert(sent_reports.front()[6] == 11U);

    sent_reports.clear();
    SendPartialRequest(1U, 12U);
    for(size_t poll = 0U; poll < 3000U; ++poll)
        LineRackHidRpcPoll();
    SendRequest(1U, 13U, empty_map, sizeof(empty_map));
    Drain();
    assert(!sent_reports.empty());
    assert(sent_reports.front()[3] == 2U);
    assert(sent_reports.front()[6] == 13U);

    sent_reports.clear();
    LineRackHidRpcNotifyStatusChanged();
    LineRackHidRpcPoll();
    SendRequest(1U, 14U, empty_map, sizeof(empty_map));
    Drain();
    bool saw_event = false;
    bool saw_response = false;
    for(const auto &report : sent_reports)
    {
        saw_event = saw_event || report[3] == 3U;
        saw_response = saw_response
                       || (report[3] == 2U && report[6] == 14U);
    }
    assert(saw_event);
    assert(saw_response);
    return 0;
}
