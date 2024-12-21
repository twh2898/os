#ifndef DRIVER_ATA_H
#define DRIVER_ATA_H

#include <stddef.h>

#include "driver.h"

int drv_ata_init();

/**
 * @brief Open ata device by id.
 *
 * @param id device id
 * @return driver_disk_t* disk driver object
 */
driver_disk_t * drv_ata_open(int id);

/**
 * @brief Close ata device and disk driver.
 *
 * @return int < 0 for fail
 */
int drv_ata_close(driver_disk_t *);

/**
 * @brief Get statistics of the disk.
 *
 * @param disk driver object
 * @param stat pointer to output struct
 * @return int < 0 for fail
 */
int drv_ata_stat(driver_disk_t * disk, disk_stat_t * stat);

/**
 * @brief Read bytes from the disk.
 *
 * @param disk driver object
 * @param buff output buffer
 * @param count bytes to read
 * @param addr offset into disk
 * @return int number of bytes read or < 0 for fail
 */
int drv_ata_read(driver_disk_t * disk, char * buff, size_t count, size_t addr);

/**
 * @brief Write bytes to the disk.
 *
 * @param disk driver object
 * @param buff input buffer
 * @param count number of bytes to write
 * @param addr offset into disk
 * @return int number of bytes written or < 0 for fail
 */
int drv_ata_write(driver_disk_t * disk, const char * buff, size_t count, size_t addr);

#endif // DRIVER_ATA_H
