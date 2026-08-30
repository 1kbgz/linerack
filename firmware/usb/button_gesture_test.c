#include "button_gesture.h"

#include <assert.h>
#include <stdint.h>

static LineRackButtonAction Update(LineRackButtonGesture *gesture,
                                   bool pressed,
                                   uint32_t now_ms)
{
    return LineRackButtonGestureUpdate(gesture, pressed, now_ms);
}

int main(void)
{
    LineRackButtonGesture gesture;
    LineRackButtonGestureInit(&gesture);

    assert(Update(&gesture, true, 100U) == LINERACK_BUTTON_NONE);
    assert(Update(&gesture, false, 180U) == LINERACK_BUTTON_NONE);
    assert(Update(&gesture, false, 530U) == LINERACK_BUTTON_NONE);
    assert(Update(&gesture, false, 531U) == LINERACK_BUTTON_WAKE_DISPLAY);

    assert(Update(&gesture, true, 1000U) == LINERACK_BUTTON_NONE);
    assert(Update(&gesture, false, 1070U) == LINERACK_BUTTON_NONE);
    assert(Update(&gesture, true, 1200U) == LINERACK_BUTTON_NONE);
    assert(Update(&gesture, false, 1280U) == LINERACK_BUTTON_CYCLE_PRESET);

    assert(Update(&gesture, true, 2000U) == LINERACK_BUTTON_NONE);
    assert(Update(&gesture, false, 7000U) == LINERACK_BUTTON_CYCLE_DISPLAY);

    assert(Update(&gesture, true, 8000U) == LINERACK_BUTTON_NONE);
    assert(Update(&gesture, false, 18000U) == LINERACK_BUTTON_RESERVED);

    LineRackButtonGestureInit(&gesture);
    assert(Update(&gesture, true, UINT32_MAX - 40U) == LINERACK_BUTTON_NONE);
    assert(Update(&gesture, false, 20U) == LINERACK_BUTTON_NONE);
    assert(Update(&gesture, false, 371U) == LINERACK_BUTTON_WAKE_DISPLAY);
    return 0;
}
