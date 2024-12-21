#include "drivers/ata.h"

#include "libc/memory.h"
#include "libc/string.h"

typedef struct _drv_ata {
    int    id;
    size_t size;
    char * data;

    driver_disk_t * driver;
} drv_ata_t;

driver_register_t drv_register;

int drv_ata_init() {
    drv_register.type          = DRIVER_DEVICE_TYPE_DISK;
    drv_register.disk.fn_open  = drv_ata_open;
    drv_register.disk.fn_close = drv_ata_close;
    drv_register.disk.fn_stat  = drv_ata_stat;
    drv_register.disk.fn_read  = drv_ata_read;
    drv_register.disk.fn_write = drv_ata_write;

    int res = register_driver(&drv_register);
    return res;
}

driver_disk_t * drv_ata_open(int id) {
    if (id < 0) {
        return 0;
    }

    // TODO open ATA device

    driver_disk_t * disk = kmalloc(sizeof(driver_disk_t));
    if (disk) {
        disk->id         = id;
        disk->stat.size  = 0;
        disk->stat.state = DRIVER_DISK_STATE_IDLE;
    }
    return disk;
}

int drv_ata_close(driver_disk_t * disk) {
    if (!disk) {
        return -1;
    }

    // TODO close ATA device

    kfree(disk);
    return 0;
}

int drv_ata_stat(driver_disk_t * disk, disk_stat_t * stat) {
    if (!disk || !stat) {
        return -1;
    }

    if (!kmemcpy(stat, &disk->stat, sizeof(disk_stat_t))) {
        return -1;
    }

    return 0;
}

int drv_ata_read(driver_disk_t * disk, char * buff, size_t count, size_t addr) {
    if (!disk || !buff) {
        return -1;
    }

    if (count == 0) {
        return 0;
    }

    // size_t size = device->size;
    // if (addr >= size) {
    //     return 0;
    // }

    // if (size - addr < count) {
    //     count = size - addr;
    // }

    // TODO read ATA data

    return count;
}

int drv_ata_write(driver_disk_t * disk, const char * buff, size_t count, size_t addr) {
    if (!disk || !buff) {
        return -1;
    }

    if (count == 0) {
        return 0;
    }

    // size_t size = device->size;
    // if (addr >= size) {
    //     return 0;
    // }

    // if (size - addr < count) {
    //     count = size - addr;
    // }

    // TODO write ATA data

    return count;
}
