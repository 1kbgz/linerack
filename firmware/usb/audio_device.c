#include "audio_device.h"

#include "adaptive_playback.h"
#include "audio_descriptors.h"
#include "daisy_core.h"
#include "host_volume.h"
#include "pcm_ring.h"
#include "stm32h7xx_hal.h"
#include "usbd_audio.h"
#include "usbd_core.h"

#ifdef USE_USBD_COMPOSITE
#include "usbd_composite_builder.h"
#include "usbd_customhid.h"
#include "hid_rpc.h"
#endif

static USBD_HandleTypeDef usb_device;
static volatile uint32_t  packet_count;
static volatile uint32_t  last_packet_tick;
static volatile uint32_t  underrun_count;
static volatile uint32_t  overrun_count;
static AdaptivePlayback   playback;
static PcmRingBuffer      pcm_ring DMA_BUFFER_MEM_SECTION;
static LineRackHostVolume host_volume;

#ifdef USE_USBD_COMPOSITE
static uint8_t           hid_class_id;
static volatile uint32_t hid_report_count;

static uint8_t hid_report_descriptor[] = {
    0x06, 0x00, 0xFF,
    0x09, 0x01,
    0xA1, 0x01,
    0x15, 0x00,
    0x26, 0xFF, 0x00,
    0x75, 0x08,
    0x95, 0x40,
    0x09, 0x01,
    0x81, 0x02,
    0x95, 0x40,
    0x09, 0x01,
    0x91, 0x02,
    0xC0,
};
#endif

extern PCD_HandleTypeDef hpcd_USB_OTG_FS;

static int8_t AudioInit(uint32_t frequency,
                        uint32_t volume,
                        uint32_t options)
{
    (void)frequency;
    (void)volume;
    (void)options;
    return 0;
}

static int8_t AudioDeInit(uint32_t options)
{
    (void)options;
    return 0;
}

static int8_t AudioCommand(uint8_t *buffer, uint32_t size, uint8_t command)
{
    (void)buffer;
    (void)size;
    (void)command;
    return 0;
}

static int8_t VolumeControl(uint8_t volume)
{
    LineRackHostVolumeSetPercent(&host_volume, volume);
    return 0;
}

static int8_t MuteControl(uint8_t command)
{
    LineRackHostVolumeSetMute(&host_volume, command != 0U);
    return 0;
}

static int8_t PeriodicTransfer(uint8_t *buffer,
                               uint32_t size,
                               uint8_t  command)
{
    (void)command;
    ++packet_count;
    last_packet_tick = HAL_GetTick();
    const uint32_t frame_count = size / 4U;
    if(PcmRingWriteLe16(&pcm_ring, buffer, size) != frame_count)
        ++overrun_count;
    return 0;
}

static int8_t AudioState(void)
{
    return 0;
}

static USBD_AUDIO_ItfTypeDef audio_interface = {
    AudioInit,
    AudioDeInit,
    AudioCommand,
    VolumeControl,
    MuteControl,
    PeriodicTransfer,
    AudioState,
};

#ifdef USE_USBD_COMPOSITE
static int8_t HidInit(void)
{
    return 0;
}

static int8_t HidDeInit(void)
{
    return 0;
}

static int8_t HidOutEvent(uint8_t event_index, uint8_t state)
{
    (void)event_index;
    (void)state;
    ++hid_report_count;
    USBD_CUSTOM_HID_HandleTypeDef *hid =
        usb_device.pClassDataCmsit[hid_class_id];
    if(hid != NULL)
        LineRackHidRpcAcceptReport(hid->Report_buf);
    usb_device.classId = hid_class_id;
    return (int8_t)USBD_CUSTOM_HID_ReceivePacket(&usb_device);
}

static USBD_CUSTOM_HID_ItfTypeDef hid_interface = {
    hid_report_descriptor,
    HidInit,
    HidDeInit,
    HidOutEvent,
};
#endif

int LineRackUsbAudioStart(void)
{
    packet_count = 0;
    last_packet_tick = 0;
    underrun_count = 0;
    overrun_count = 0;
    AdaptivePlaybackReset(&playback);
    PcmRingReset(&pcm_ring);
    LineRackHostVolumeInit(&host_volume);

    if(USBD_Init(&usb_device, &LineRackUsbAudioDescriptors, DEVICE_FS)
       != USBD_OK)
        return -1;

#ifdef USE_USBD_COMPOSITE
    static uint8_t audio_endpoints[] = {0x01U};
    static uint8_t hid_endpoints[] = {0x81U, 0x02U};

    hid_report_count = 0U;
    if(USBD_RegisterClassComposite(&usb_device,
                                   USBD_AUDIO_CLASS,
                                   CLASS_TYPE_AUDIO,
                                   audio_endpoints)
       != USBD_OK)
        return -1;
    if(USBD_RegisterClassComposite(&usb_device,
                                   USBD_CUSTOM_HID_CLASS,
                                   CLASS_TYPE_CHID,
                                   hid_endpoints)
       != USBD_OK)
        return -1;

    if(USBD_CMPSIT_SetClassID(&usb_device, CLASS_TYPE_AUDIO, 0U) == 0xFFU
       || USBD_AUDIO_RegisterInterface(&usb_device, &audio_interface) != USBD_OK)
        return -1;
    if(USBD_CMPSIT_SetClassID(&usb_device, CLASS_TYPE_CHID, 0U) == 0xFFU)
        return -1;
    hid_class_id = (uint8_t)usb_device.classId;
    if(USBD_CUSTOM_HID_RegisterInterface(&usb_device, &hid_interface) != USBD_OK)
        return -1;
#else
    if(USBD_RegisterClass(&usb_device, USBD_AUDIO_CLASS) != USBD_OK)
        return -1;
    if(USBD_AUDIO_RegisterInterface(&usb_device, &audio_interface) != USBD_OK)
        return -1;
#endif

    HAL_PWREx_EnableUSBVoltageDetector();
    return USBD_Start(&usb_device) == USBD_OK ? 0 : -1;
}

bool LineRackUsbAudioConfigured(void)
{
    return usb_device.dev_state == USBD_STATE_CONFIGURED;
}

bool LineRackUsbAudioReadFrame(float *left, float *right)
{
    bool starved;
    const bool produced =
        AdaptivePlaybackRead(&playback, &pcm_ring, left, right, &starved);
    if(starved && HAL_GetTick() - last_packet_tick <= 2U)
        ++underrun_count;
    return produced;
}

void LineRackUsbAudioApplyHostVolume(float *left, float *right)
{
    LineRackHostVolumeProcess(&host_volume, left, right);
}

uint8_t LineRackUsbAudioHostVolumePercent(void)
{
    return LineRackHostVolumePercent(&host_volume);
}

bool LineRackUsbAudioHostMuted(void)
{
    return LineRackHostVolumeMuted(&host_volume);
}

uint32_t LineRackUsbAudioPacketCount(void)
{
    return packet_count;
}

uint32_t LineRackUsbAudioUnderrunCount(void)
{
    return underrun_count;
}

uint32_t LineRackUsbAudioOverrunCount(void)
{
    return overrun_count;
}

uint32_t LineRackUsbAudioBufferFill(void)
{
    return PcmRingFill(&pcm_ring);
}

uint32_t LineRackUsbHidReportCount(void)
{
#ifdef USE_USBD_COMPOSITE
    return hid_report_count;
#else
    return 0U;
#endif
}

bool LineRackUsbHidSendReport(const uint8_t *report, uint16_t size)
{
#ifdef USE_USBD_COMPOSITE
    if(usb_device.dev_state != USBD_STATE_CONFIGURED)
        return false;
    usb_device.classId = hid_class_id;
    return USBD_CUSTOM_HID_SendReport(
               &usb_device, (uint8_t *)report, size, hid_class_id)
           == USBD_OK;
#else
    (void)report;
    (void)size;
    return false;
#endif
}

void OTG_FS_EP1_OUT_IRQHandler(void)
{
    HAL_PCD_IRQHandler(&hpcd_USB_OTG_FS);
}

void OTG_FS_EP1_IN_IRQHandler(void)
{
    HAL_PCD_IRQHandler(&hpcd_USB_OTG_FS);
}

void OTG_FS_IRQHandler(void)
{
    HAL_PCD_IRQHandler(&hpcd_USB_OTG_FS);
}
