#ifndef DRIVER_RAMDISK_H
#define DRIVER_RAMDISK_H

#include <stddef.h>

#include "driver.h"

#define DRV_RAMDISK_MAX_DEVICES 8

int drv_ramdisk_init();

int drv_ramdisk_create_device(size_t size);

driver_disk_t * drv_ramdisk_open(int id);
int             drv_ramdisk_close(driver_disk_t *);
int             drv_ramdisk_stat(driver_disk_t *, disk_stat_t *);
int             drv_ramdisk_read(driver_disk_t *, char * buff, size_t count, size_t addr);
int             drv_ramdisk_write(driver_disk_t *, const char * buff, size_t count, size_t addr);

#endif // DRIVER_RAMDISK_H
