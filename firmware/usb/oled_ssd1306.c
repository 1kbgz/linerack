#include "oled_ssd1306.h"

#include <string.h>

bool LineRackOledInit(const LineRackOledTransport *transport)
{
    static const uint8_t commands[] = {
        0x00U,
        0xaeU,
        0xd5U, 0x80U,
        0xa8U, 0x1fU,
        0xd3U, 0x00U,
        0x40U,
        0x8dU, 0x14U,
        0x20U, 0x02U,
        0xa1U,
        0xc8U,
        0xdaU, 0x02U,
        0x81U, 0x8fU,
        0xd9U, 0xf1U,
        0xdbU, 0x40U,
        0x2eU,
        0xa4U,
        0xa6U,
        0xafU,
    };
    return transport != NULL && transport->write != NULL
           && transport->write(
               transport->context, commands, sizeof(commands));
}

bool LineRackOledSetEnabled(const LineRackOledTransport *transport,
                            bool                         enabled)
{
    const uint8_t command[] = {0x00U, enabled ? 0xafU : 0xaeU};
    return transport != NULL && transport->write != NULL
           && transport->write(
               transport->context, command, sizeof(command));
}

bool LineRackOledWriteFrame(const LineRackOledTransport *transport,
                            const LineRackDisplayFrame   *frame)
{
    if(transport == NULL || transport->write == NULL || frame == NULL)
        return false;
    for(uint8_t page = 0U; page < LINERACK_DISPLAY_HEIGHT / 8U; ++page)
    {
        const uint8_t commands[] = {
            0x00U,
            (uint8_t)(0xb0U + page),
            0x00U,
            0x10U,
        };
        uint8_t data[LINERACK_DISPLAY_WIDTH + 1U];
        data[0] = 0x40U;
        memcpy(&data[1],
               &frame->data[(size_t)page * LINERACK_DISPLAY_WIDTH],
               LINERACK_DISPLAY_WIDTH);
        if(!transport->write(
               transport->context, commands, sizeof(commands))
           || !transport->write(transport->context, data, sizeof(data)))
            return false;
    }
    return true;
}
