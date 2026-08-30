#include "audio_descriptors.h"

#include "usbd_ctlreq.h"

#define LINERACK_USB_VID 0xCAFEU
#define LINERACK_USB_PID 0x4C52U

static uint8_t *DeviceDescriptor(USBD_SpeedTypeDef speed, uint16_t *length);
static uint8_t *LanguageDescriptor(USBD_SpeedTypeDef speed, uint16_t *length);
static uint8_t *ManufacturerDescriptor(USBD_SpeedTypeDef speed,
                                       uint16_t         *length);
static uint8_t *ProductDescriptor(USBD_SpeedTypeDef speed, uint16_t *length);
static uint8_t *SerialDescriptor(USBD_SpeedTypeDef speed, uint16_t *length);
static uint8_t *ConfigurationDescriptor(USBD_SpeedTypeDef speed,
                                        uint16_t         *length);
static uint8_t *InterfaceDescriptor(USBD_SpeedTypeDef speed,
                                    uint16_t         *length);

USBD_DescriptorsTypeDef LineRackUsbAudioDescriptors = {
    DeviceDescriptor,
    LanguageDescriptor,
    ManufacturerDescriptor,
    ProductDescriptor,
    SerialDescriptor,
    ConfigurationDescriptor,
    InterfaceDescriptor,
};

__ALIGN_BEGIN static uint8_t device_descriptor[USB_LEN_DEV_DESC] __ALIGN_END = {
    USB_LEN_DEV_DESC,
    USB_DESC_TYPE_DEVICE,
    0x00,
    0x02,
#ifdef USE_USBD_COMPOSITE
    0xEF,
    0x02,
    0x01,
#else
    0x00,
    0x00,
    0x00,
#endif
    USB_MAX_EP0_SIZE,
    LOBYTE(LINERACK_USB_VID),
    HIBYTE(LINERACK_USB_VID),
    LOBYTE(LINERACK_USB_PID),
    HIBYTE(LINERACK_USB_PID),
#ifdef USE_USBD_COMPOSITE
    0x03,
#else
    0x02,
#endif
    0x00,
    USBD_IDX_MFC_STR,
    USBD_IDX_PRODUCT_STR,
    USBD_IDX_SERIAL_STR,
    0x01,
};

__ALIGN_BEGIN static uint8_t language_descriptor[USB_LEN_LANGID_STR_DESC]
    __ALIGN_END = {USB_LEN_LANGID_STR_DESC,
                   USB_DESC_TYPE_STRING,
                   LOBYTE(0x0409U),
                   HIBYTE(0x0409U)};

__ALIGN_BEGIN static uint8_t string_descriptor[USBD_MAX_STR_DESC_SIZ]
    __ALIGN_END;

static uint8_t *DeviceDescriptor(USBD_SpeedTypeDef speed, uint16_t *length)
{
    (void)speed;
    *length = sizeof(device_descriptor);
    return device_descriptor;
}

static uint8_t *LanguageDescriptor(USBD_SpeedTypeDef speed, uint16_t *length)
{
    (void)speed;
    *length = sizeof(language_descriptor);
    return language_descriptor;
}

static uint8_t *StringDescriptor(const char *value, uint16_t *length)
{
    USBD_GetString((uint8_t *)value, string_descriptor, length);
    return string_descriptor;
}

static uint8_t *ManufacturerDescriptor(USBD_SpeedTypeDef speed,
                                       uint16_t         *length)
{
    (void)speed;
    return StringDescriptor("LineRack Development", length);
}

static uint8_t *ProductDescriptor(USBD_SpeedTypeDef speed, uint16_t *length)
{
    (void)speed;
    return StringDescriptor("LineRack USB Audio Dev", length);
}

static uint8_t *SerialDescriptor(USBD_SpeedTypeDef speed, uint16_t *length)
{
    static const char hex_digits[] = "0123456789ABCDEF";
    static char       serial[25];
    const uint32_t    words[] = {HAL_GetUIDw0(), HAL_GetUIDw1(), HAL_GetUIDw2()};
    (void)speed;
    for(uint32_t word = 0U; word < 3U; ++word)
    {
        for(uint32_t digit = 0U; digit < 8U; ++digit)
        {
            const uint32_t shift = 28U - digit * 4U;
            serial[word * 8U + digit] =
                hex_digits[(words[word] >> shift) & 0x0FU];
        }
    }
    serial[24] = '\0';
    return StringDescriptor(serial, length);
}

static uint8_t *ConfigurationDescriptor(USBD_SpeedTypeDef speed,
                                        uint16_t         *length)
{
    (void)speed;
    return StringDescriptor("UAC1 Playback", length);
}

static uint8_t *InterfaceDescriptor(USBD_SpeedTypeDef speed,
                                    uint16_t         *length)
{
    (void)speed;
    return StringDescriptor("Stereo Output", length);
}
