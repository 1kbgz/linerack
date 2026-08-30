#include "oled_ssd1306.h"

#include <assert.h>
#include <string.h>

typedef struct
{
    uint8_t writes[11][LINERACK_DISPLAY_WIDTH + 1U];
    size_t  sizes[11];
    size_t  count;
    bool    succeed;
} FakeI2c;

static bool Write(void *context, const uint8_t *data, size_t size)
{
    FakeI2c *i2c = context;
    assert(i2c->count < 11U);
    assert(size <= sizeof(i2c->writes[0]));
    memcpy(i2c->writes[i2c->count], data, size);
    i2c->sizes[i2c->count] = size;
    ++i2c->count;
    return i2c->succeed;
}

int main(void)
{
    FakeI2c i2c = {.succeed = true};
    LineRackOledTransport transport = {&i2c, Write};
    LineRackDisplayFrame frame;
    for(size_t index = 0U; index < sizeof(frame.data); ++index)
        frame.data[index] = (uint8_t)index;

    assert(LineRackOledInit(&transport));
    assert(i2c.count == 1U);
    assert(i2c.writes[0][0] == 0x00U);
    assert(i2c.writes[0][1] == 0xaeU);
    assert(i2c.writes[0][i2c.sizes[0] - 1U] == 0xafU);

    assert(LineRackOledSetEnabled(&transport, false));
    assert(i2c.sizes[1] == 2U);
    assert(i2c.writes[1][1] == 0xaeU);
    assert(LineRackOledSetEnabled(&transport, true));
    assert(i2c.sizes[2] == 2U);
    assert(i2c.writes[2][1] == 0xafU);

    assert(LineRackOledWriteFrame(&transport, &frame));
    assert(i2c.count == 11U);
    for(uint8_t page = 0U; page < 4U; ++page)
    {
        const size_t command_index = 3U + (size_t)page * 2U;
        const size_t data_index = command_index + 1U;
        assert(i2c.sizes[command_index] == 4U);
        assert(i2c.writes[command_index][0] == 0x00U);
        assert(i2c.writes[command_index][1] == 0xb0U + page);
        assert(i2c.writes[command_index][2] == 0x00U);
        assert(i2c.writes[command_index][3] == 0x10U);
        assert(i2c.sizes[data_index] == LINERACK_DISPLAY_WIDTH + 1U);
        assert(i2c.writes[data_index][0] == 0x40U);
        assert(memcmp(&i2c.writes[data_index][1],
                      &frame.data[(size_t)page * LINERACK_DISPLAY_WIDTH],
                      LINERACK_DISPLAY_WIDTH) == 0);
    }

    i2c = (FakeI2c){.succeed = false};
    assert(!LineRackOledInit(&transport));
    return 0;
}
