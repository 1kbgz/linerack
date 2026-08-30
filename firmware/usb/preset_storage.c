#include "preset_storage.h"

#include <string.h>

#define LINERACK_STORAGE_MAGIC 0x4b52504cU
#define LINERACK_STORAGE_COMMIT 0x4d43524cU

typedef struct
{
    uint32_t magic;
    uint32_t schema_version;
    uint32_t generation;
    uint32_t payload_size;
    uint32_t payload_crc;
    uint32_t commit;
} StorageHeader;

typedef struct
{
    bool               valid;
    uint32_t           generation;
    LineRackPresetBank bank;
} StorageRecord;

static uint32_t Crc32(const uint8_t *data, size_t size)
{
    uint32_t crc = 0xffffffffU;
    for(size_t index = 0U; index < size; ++index)
    {
        crc ^= data[index];
        for(uint8_t bit = 0U; bit < 8U; ++bit)
            crc = (crc >> 1U) ^ (0xedb88320U & (0U - (crc & 1U)));
    }
    return ~crc;
}

static StorageRecord ReadRecord(const LineRackPresetStorage *storage,
                                uint8_t                      sector)
{
    StorageRecord record;
    StorageHeader header;
    memset(&record, 0, sizeof(record));
    if(!storage->read(storage->context,
                      storage->sector_address[sector],
                      (uint8_t *)&header,
                      sizeof(header)))
        return record;
    if(header.magic != LINERACK_STORAGE_MAGIC || header.schema_version != 1U
       || header.payload_size != sizeof(record.bank)
       || header.commit != LINERACK_STORAGE_COMMIT)
        return record;
    if(!storage->read(storage->context,
                      storage->sector_address[sector] + sizeof(header),
                      (uint8_t *)&record.bank,
                      sizeof(record.bank)))
        return record;
    if(Crc32((const uint8_t *)&record.bank, sizeof(record.bank))
           != header.payload_crc
       || !LineRackPresetBankValid(&record.bank))
        return record;
    record.valid = true;
    record.generation = header.generation;
    return record;
}

static bool StorageValid(const LineRackPresetStorage *storage)
{
    return storage != NULL && storage->read != NULL
           && storage->erase_sector != NULL && storage->write != NULL
           && storage->sector_address[0] != storage->sector_address[1];
}

static uint8_t NewestRecord(const StorageRecord records[2])
{
    if(records[0].valid && records[1].valid)
        return (int32_t)(records[1].generation - records[0].generation) > 0
                   ? 1U
                   : 0U;
    return records[1].valid ? 1U : 0U;
}

bool LineRackPresetStorageLoad(const LineRackPresetStorage *storage,
                               LineRackPresetBank          *bank)
{
    if(!StorageValid(storage) || bank == NULL)
        return false;
    const StorageRecord records[2] = {
        ReadRecord(storage, 0U),
        ReadRecord(storage, 1U),
    };
    if(!records[0].valid && !records[1].valid)
        return false;
    *bank = records[NewestRecord(records)].bank;
    return true;
}

bool LineRackPresetStorageSave(const LineRackPresetStorage *storage,
                               const LineRackPresetBank    *bank)
{
    if(!StorageValid(storage) || !LineRackPresetBankValid(bank))
        return false;
    const StorageRecord records[2] = {
        ReadRecord(storage, 0U),
        ReadRecord(storage, 1U),
    };
    uint8_t target = 0U;
    uint32_t generation = 1U;
    if(records[0].valid || records[1].valid)
    {
        const uint8_t newest = NewestRecord(records);
        target = (uint8_t)(newest ^ 1U);
        generation = records[newest].generation + 1U;
    }

    StorageHeader header = {
        LINERACK_STORAGE_MAGIC,
        1U,
        generation,
        sizeof(*bank),
        Crc32((const uint8_t *)bank, sizeof(*bank)),
        0xffffffffU,
    };
    const uint32_t address = storage->sector_address[target];
    if(!storage->erase_sector(storage->context, address)
       || !storage->write(storage->context,
                          address,
                          (const uint8_t *)&header,
                          sizeof(header))
       || !storage->write(storage->context,
                          address + sizeof(header),
                          (const uint8_t *)bank,
                          sizeof(*bank)))
        return false;
    const uint32_t commit = LINERACK_STORAGE_COMMIT;
    return storage->write(storage->context,
                          address + offsetof(StorageHeader, commit),
                          (const uint8_t *)&commit,
                          sizeof(commit));
}
