#ifndef DRIVER_RAMDISK_H
#define DRIVER_RAMDISK_H

#include <stddef.h>

#include "driver.h"

#define DRV_RAMDISK_MAX_DEVICES 8

/**
 * @brief Register ramdisk driver and initialize emtpy devices.
 *
 * @return int < 0 if registration fails
 */
int drv_ramdisk_init();

/**
 * @brief Create a ramdisk device of size.
 *
 * @param size disk size in bytes
 * @return int id of new device or < 0 for fail
 */
int drv_ramdisk_create_device(size_t size);

/**
 * @brief Open ramdisk device by id.
 *
 * @param id device id returned from drv_ramdisk_create_device
 * @return driver_disk_t* disk driver object
 */
driver_disk_t * drv_ramdisk_open(int id);

/**
 * @brief Close ramdisk device and disk driver.
 *
 * @return int < 0 for fail
 */
int drv_ramdisk_close(driver_disk_t *);

/**
 * @brief Get statistics of the disk.
 *
 * @param disk driver object
 * @param stat pointer to output struct
 * @return int < 0 for fail
 */
int drv_ramdisk_stat(driver_disk_t * disk, disk_stat_t * stat);

/**
 * @brief Read bytes from the disk.
 *
 * @param disk driver object
 * @param buff output buffer
 * @param count bytes to read
 * @param addr offset into disk
 * @return int number of bytes read or < 0 for fail
 */
int drv_ramdisk_read(driver_disk_t * disk, char * buff, size_t count, size_t addr);

/**
 * @brief Write bytes to the disk.
 *
 * @param disk driver object
 * @param buff input buffer
 * @param count number of bytes to write
 * @param addr offset into disk
 * @return int number of bytes written or < 0 for fail
 */
int drv_ramdisk_write(driver_disk_t * disk, const char * buff, size_t count, size_t addr);

#endif // DRIVER_RAMDISK_H
