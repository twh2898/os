#ifndef DRIVER_H
#define DRIVER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// Import driver structs and defs
#include "_driver/disk.h"
#include "_driver/fs.h"

typedef struct _driver_device_hmi {
} driver_device_hmi_t;

typedef enum DRIVER_DEVICE_TYPE {
    DRIVER_DEVICE_TYPE_NULL = 0,
    DRIVER_DEVICE_TYPE_DISK,
    DRIVER_DEVICE_TYPE_FS,
    DRIVER_DEVICE_TYPE_HMI,
} driver_device_type_t;

typedef struct _driver_device_disk driver_device_disk_t;
typedef struct _driver_device_fs   driver_device_fs_t;
typedef struct _driver_device_hmi  driver_device_hmi_t;

typedef struct _driver_register {
    uint32_t                id;
    enum DRIVER_DEVICE_TYPE type;

    union {
        struct _driver_device_disk disk;
        struct _driver_device_fs   fs;
        struct _driver_device_hmi  hmi;
    };
} driver_register_t;

void init_drivers();

// TODO assign id field
int register_driver(driver_register_t * reg);
int unregister_driver(driver_register_t * reg);

driver_device_disk_t * driver_get_disk(const char * type);
driver_device_fs_t *   driver_get_fs(const char * format);

driver_disk_t * driver_disk_open(driver_device_disk_t * drv, int id);
int             driver_disk_close(driver_disk_t * disk);
int             driver_disk_stat(driver_disk_t * disk, disk_stat_t * stat);
int             driver_disk_read(driver_disk_t * disk, char * buff, size_t count, size_t addr);
int             driver_disk_write(driver_disk_t * disk, const char * buff, size_t count, size_t addr);

#endif // DRIVER_H
