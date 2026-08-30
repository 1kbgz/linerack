#include "button_gesture.h"

#include <stddef.h>

void LineRackButtonGestureInit(LineRackButtonGesture *gesture)
{
    if(gesture == NULL)
        return;
    gesture->pressed_at_ms = 0U;
    gesture->first_tap_released_at_ms = 0U;
    gesture->pressed = false;
    gesture->tap_pending = false;
}

LineRackButtonAction LineRackButtonGestureUpdate(LineRackButtonGesture *gesture,
                                                  bool pressed,
                                                  uint32_t now_ms)
{
    if(gesture == NULL)
        return LINERACK_BUTTON_NONE;

    if(pressed && !gesture->pressed)
    {
        gesture->pressed = true;
        gesture->pressed_at_ms = now_ms;
        return LINERACK_BUTTON_NONE;
    }

    if(!pressed && gesture->pressed)
    {
        gesture->pressed = false;
        const uint32_t held_ms = now_ms - gesture->pressed_at_ms;
        if(held_ms >= LINERACK_BUTTON_RESERVED_HOLD_MS)
        {
            gesture->tap_pending = false;
            return LINERACK_BUTTON_RESERVED;
        }
        if(held_ms >= LINERACK_BUTTON_LONG_HOLD_MS)
        {
            gesture->tap_pending = false;
            return LINERACK_BUTTON_CYCLE_DISPLAY;
        }
        if(gesture->tap_pending
           && now_ms - gesture->first_tap_released_at_ms
                  <= LINERACK_BUTTON_DOUBLE_TAP_MS)
        {
            gesture->tap_pending = false;
            return LINERACK_BUTTON_CYCLE_PRESET;
        }
        gesture->tap_pending = true;
        gesture->first_tap_released_at_ms = now_ms;
        return LINERACK_BUTTON_NONE;
    }

    if(!pressed && gesture->tap_pending
       && now_ms - gesture->first_tap_released_at_ms
              > LINERACK_BUTTON_DOUBLE_TAP_MS)
    {
        gesture->tap_pending = false;
        return LINERACK_BUTTON_WAKE_DISPLAY;
    }

    return LINERACK_BUTTON_NONE;
}
