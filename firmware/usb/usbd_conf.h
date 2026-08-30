#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "stm32h7xx.h"
#include "stm32h7xx_hal.h"

#define USBD_MAX_NUM_INTERFACES 3U
#define USBD_MAX_NUM_CONFIGURATION 1U
#define USBD_MAX_STR_DESC_SIZ 512U
#define USBD_SUPPORT_USER_STRING_DESC 1U
#define USBD_DEBUG_LEVEL 0U
#define USBD_LPM_ENABLED 0U
#define USBD_SELF_POWERED 0U
#define USBD_MAX_POWER 0xFAU

#define DEVICE_FS 0
#define DEVICE_HS 1

#define USBD_malloc malloc
#define USBD_free free
#define USBD_memset memset
#define USBD_memcpy memcpy
#define USBD_Delay HAL_Delay

#define USBD_UsrLog(...)
#define USBD_ErrLog(...)
#define USBD_DbgLog(...)
