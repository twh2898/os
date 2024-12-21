#ifndef DRIVER_ATA_SUPPORT_H
#define DRIVER_ATA_SUPPORT_H

#include <stdbool.h>
#include <stddef.h>

#include "drivers/_ata/defs.h"

bool drv_ata_identify(drv_ata_t * device);
void drv_ata_software_reset(drv_ata_t * device);
bool drv_ata_status(drv_ata_t * device);

// Read one sector into device buffer
int drv_ata_sect_read(drv_ata_t * device, size_t sect_count, uint32_t lba);
int drv_ata_sect_write(drv_ata_t * device, size_t sect_count, uint32_t lba);

#endif // DRIVER_ATA_SUPPORT_H
