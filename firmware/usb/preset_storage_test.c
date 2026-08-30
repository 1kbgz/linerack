#include "preset_storage.h"

#include <assert.h>
#include <string.h>

#define FLASH_SIZE 8192U
#define SECTOR_SIZE 4096U

typedef struct
{
    uint8_t memory[FLASH_SIZE];
    int     writes_before_failure;
} FakeFlash;

static bool Read(void *context, uint32_t address, uint8_t *data, size_t size)
{
    FakeFlash *flash = context;
    if(address + size > sizeof(flash->memory)) return false;
    memcpy(data, flash->memory + address, size);
    return true;
}

static bool Erase(void *context, uint32_t address)
{
    FakeFlash *flash = context;
    if(address % SECTOR_SIZE != 0U || address + SECTOR_SIZE > sizeof(flash->memory)) return false;
    memset(flash->memory + address, 0xff, SECTOR_SIZE);
    return true;
}

static bool Write(void *context, uint32_t address, const uint8_t *data, size_t size)
{
    FakeFlash *flash = context;
    if(address + size > sizeof(flash->memory) || flash->writes_before_failure == 0) return false;
    if(flash->writes_before_failure > 0) --flash->writes_before_failure;
    for(size_t index = 0U; index < size; ++index)
        flash->memory[address + index] &= data[index];
    return true;
}

int main(void)
{
    FakeFlash flash;
    memset(&flash, 0xff, sizeof(flash));
    flash.writes_before_failure = -1;
    const LineRackPresetStorage storage = {
        &flash,
        Read,
        Erase,
        Write,
        {0U, SECTOR_SIZE},
    };
    LineRackPresetBank first;
    LineRackPresetBank second;
    LineRackPresetBank loaded;
    LineRackPresetBankDefaults(&first);

    assert(!LineRackPresetStorageLoad(&storage, &loaded));
    assert(LineRackPresetStorageSave(&storage, &first));
    assert(LineRackPresetStorageLoad(&storage, &loaded));
    assert(LineRackPresetBankEqual(&first, &loaded));

    second = first;
    strcpy(second.presets[0].name, "Changed");
    assert(LineRackPresetStorageSave(&storage, &second));
    assert(LineRackPresetStorageLoad(&storage, &loaded));
    assert(LineRackPresetBankEqual(&second, &loaded));

    flash.memory[SECTOR_SIZE + 32U] ^= 0x01U;
    assert(LineRackPresetStorageLoad(&storage, &loaded));
    assert(LineRackPresetBankEqual(&first, &loaded));

    flash.writes_before_failure = 2;
    assert(!LineRackPresetStorageSave(&storage, &second));
    assert(LineRackPresetStorageLoad(&storage, &loaded));
    assert(LineRackPresetBankEqual(&first, &loaded));
    return 0;
}
