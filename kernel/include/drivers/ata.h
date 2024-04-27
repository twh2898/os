#ifndef ATA_H
#define ATA_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

void init_disk();
size_t disk_identify();
void software_reset();
bool disk_status();
size_t disk_read(uint8_t * buff, size_t count, uint32_t lba);
size_t disk_write(uint8_t * buff, size_t count, uint32_t lba);

#endif // ATA_H
