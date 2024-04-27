#ifndef ATA_H
#define ATA_H

#include <stddef.h>
#include <stdint.h>

void init_disk();
size_t disk_identify();
void software_reset();
void disk_status();
size_t disk_read(uint32_t lba);
size_t disk_write(uint32_t lba);

#endif // ATA_H
