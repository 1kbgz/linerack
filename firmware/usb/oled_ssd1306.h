#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "display_model.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef bool (*LineRackOledWrite)(void          *context,
                                  const uint8_t *data,
                                  size_t         size);

typedef struct
{
    void              *context;
    LineRackOledWrite  write;
} LineRackOledTransport;

bool LineRackOledInit(const LineRackOledTransport *transport);
bool LineRackOledSetEnabled(const LineRackOledTransport *transport,
                            bool                         enabled);
bool LineRackOledWriteFrame(const LineRackOledTransport *transport,
                            const LineRackDisplayFrame   *frame);

#ifdef __cplusplus
}
#endif
