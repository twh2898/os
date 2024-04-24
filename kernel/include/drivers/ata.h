#ifndef ATA_H
#define ATA_H

#include <stdint.h>

void init_disk();
void disk_identify();
void disk_write(uint32_t lba);

#endif // ATA_H
