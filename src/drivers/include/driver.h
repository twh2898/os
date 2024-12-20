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

int register_driver(driver_register_t * reg);
int unregister_driver(driver_register_t * reg);

#endif // DRIVER_H
