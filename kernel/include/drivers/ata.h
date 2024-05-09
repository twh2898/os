#ifndef ATA_H
#define ATA_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define ATA_SECTOR_WORDS 256
#define ATA_SECTOR_BYTES (ATA_SECTOR_WORDS * 2)

typedef struct _disk disk_t;

void init_disk();

disk_t * disk_open(uint8_t id);
void disk_close(disk_t * disk);

size_t disk_size(disk_t * disk);
size_t disk_sector_count(disk_t * disk);
bool disk_status(disk_t * disk);

// Read / write whole sector block (read / write bytes in file system driver)
// buff must be of size sect_count * ATA_SECTOR_BYTES
size_t disk_sect_read(disk_t * disk, uint8_t * buff, size_t sect_count, uint32_t lba);
size_t disk_sect_write(disk_t * disk, uint8_t * buff, size_t sect_count, uint32_t lba);

#endif // ATA_H
