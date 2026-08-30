#pragma once

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define LINERACK_BUTTON_DOUBLE_TAP_MS 350U
#define LINERACK_BUTTON_LONG_HOLD_MS 5000U
#define LINERACK_BUTTON_RESERVED_HOLD_MS 10000U

typedef enum
{
    LINERACK_BUTTON_NONE = 0,
    LINERACK_BUTTON_WAKE_DISPLAY,
    LINERACK_BUTTON_CYCLE_PRESET,
    LINERACK_BUTTON_CYCLE_DISPLAY,
    LINERACK_BUTTON_RESERVED,
} LineRackButtonAction;

typedef struct
{
    uint32_t pressed_at_ms;
    uint32_t first_tap_released_at_ms;
    bool     pressed;
    bool     tap_pending;
} LineRackButtonGesture;

void LineRackButtonGestureInit(LineRackButtonGesture *gesture);
LineRackButtonAction LineRackButtonGestureUpdate(LineRackButtonGesture *gesture,
                                                  bool pressed,
                                                  uint32_t now_ms);

#ifdef __cplusplus
}
#endif
