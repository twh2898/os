#ifndef ATA_H
#define ATA_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define ATA_SECTOR_WORDS 256
#define ATA_SECTOR_BYTES (ATA_SECTOR_WORDS * 2)

void init_disk();
// size in bytes = # of sectors * ATA_SECTOR_BYTES
size_t disk_size();
bool disk_status();

// Read / write whole sector block (read / write bytes in file system driver)
// buff must be of size sect_count * ATA_SECTOR_BYTES
size_t disk_sect_read(uint8_t * buff, size_t sect_count, uint32_t lba);
size_t disk_sect_write(uint8_t * buff, size_t sect_count, uint32_t lba);

#endif // ATA_H
