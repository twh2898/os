#include "drivers/ramdisk.h"

#include "libc/memory.h"
#include "libc/string.h"

typedef struct _drv_ramdisk {
    int    id;
    size_t size;
    char * data;

    driver_disk_t * driver;
} drv_ramdisk_t;

static drv_ramdisk_t devices[DRV_RAMDISK_MAX_DEVICES];
static size_t        device_count;

static driver_register_t drv_register;

int drv_ramdisk_init() {
    device_count = 0;

    drv_register.type          = DRIVER_DEVICE_TYPE_DISK;
    drv_register.disk.type     = "ramdisk";
    drv_register.disk.fn_open  = drv_ramdisk_open;
    drv_register.disk.fn_close = drv_ramdisk_close;
    drv_register.disk.fn_stat  = drv_ramdisk_stat;
    drv_register.disk.fn_read  = drv_ramdisk_read;
    drv_register.disk.fn_write = drv_ramdisk_write;

    int res = register_driver(&drv_register);
    return res;
}

int drv_ramdisk_create_device(size_t size) {
    if (size == 0 || device_count >= DRV_RAMDISK_MAX_DEVICES) {
        return -1;
    }

    void * data = kmalloc(size);
    if (!data) {
        return -1;
    }

    devices[device_count].id     = device_count;
    devices[device_count].size   = size;
    devices[device_count].data   = data;
    devices[device_count].driver = 0;

    return device_count++;
}

driver_disk_t * drv_ramdisk_open(int id) {
    if (id < 0 || id >= device_count) {
        return 0;
    }

    drv_ramdisk_t * device = &devices[id];

    if (device->driver) {
        return 0;
    }

    driver_disk_t * disk = kmalloc(sizeof(driver_disk_t));
    if (disk) {
        device->driver = disk;

        disk->id         = device->id;
        disk->stat.size  = device->size;
        disk->stat.state = DRIVER_DISK_STATE_IDLE;
    }
    return disk;
}

int drv_ramdisk_close(driver_disk_t * disk) {
    if (!disk) {
        return -1;
    }

    int id = disk->id;
    if (id < 0 || id >= device_count) {
        kfree(disk);
        return -1;
    }

    devices[id].driver = 0;
    kfree(disk);

    return 0;
}

int drv_ramdisk_stat(driver_disk_t * disk, driver_disk_stat_t * stat) {
    if (!disk || !stat) {
        return -1;
    }

    if (!kmemcpy(stat, &disk->stat, sizeof(driver_disk_stat_t))) {
        return -1;
    }

    return 0;
}

int drv_ramdisk_read(driver_disk_t * disk, char * buff, size_t count, size_t addr) {
    if (!disk || !buff) {
        return -1;
    }

    if (disk->id < 0 || disk->id >= device_count) {
        return -1;
    }

    if (count == 0) {
        return 0;
    }

    drv_ramdisk_t * device = &devices[disk->id];

    size_t size = device->size;
    if (addr >= size) {
        return 0;
    }

    if (size - addr < count) {
        count = size - addr;
    }

    if (!kmemcpy(buff, device->data + addr, count)) {
        return -1;
    }

    return count;
}

int drv_ramdisk_write(driver_disk_t * disk, const char * buff, size_t count, size_t addr) {
    if (!disk || !buff) {
        return -1;
    }

    if (disk->id < 0 || disk->id >= device_count) {
        return -1;
    }

    if (count == 0) {
        return 0;
    }

    drv_ramdisk_t * device = &devices[disk->id];

    size_t size = device->size;
    if (addr >= size) {
        return 0;
    }

    if (size - addr < count) {
        count = size - addr;
    }

    if (!kmemcpy(device->data + addr, buff, count)) {
        return -1;
    }

    return count;
}
