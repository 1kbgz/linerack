#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "preset_model.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef bool (*LineRackFlashRead)(void    *context,
                                  uint32_t address,
                                  uint8_t *data,
                                  size_t   size);
typedef bool (*LineRackFlashErase)(void *context, uint32_t address);
typedef bool (*LineRackFlashWrite)(void          *context,
                                   uint32_t       address,
                                   const uint8_t *data,
                                   size_t         size);

typedef struct
{
    void              *context;
    LineRackFlashRead   read;
    LineRackFlashErase  erase_sector;
    LineRackFlashWrite  write;
    uint32_t            sector_address[2];
} LineRackPresetStorage;

bool LineRackPresetStorageLoad(const LineRackPresetStorage *storage,
                               LineRackPresetBank          *bank);
bool LineRackPresetStorageSave(const LineRackPresetStorage *storage,
                               const LineRackPresetBank    *bank);

#ifdef __cplusplus
}
#endif
