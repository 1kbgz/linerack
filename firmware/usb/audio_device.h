#pragma once

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

int      LineRackUsbAudioStart(void);
bool     LineRackUsbAudioConfigured(void);
bool     LineRackUsbAudioReadFrame(float *left, float *right);
void     LineRackUsbAudioApplyHostVolume(float *left, float *right);
uint8_t  LineRackUsbAudioHostVolumePercent(void);
bool     LineRackUsbAudioHostMuted(void);
uint32_t LineRackUsbAudioPacketCount(void);
uint32_t LineRackUsbAudioUnderrunCount(void);
uint32_t LineRackUsbAudioOverrunCount(void);
uint32_t LineRackUsbAudioBufferFill(void);
uint32_t LineRackUsbHidReportCount(void);
bool     LineRackUsbHidSendReport(const uint8_t *report, uint16_t size);

#ifdef __cplusplus
}
#endif
