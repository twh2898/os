#include "driver.h"

#include "libc/memory.h"
#include "libc/string.h"

typedef struct _driver_list {
    driver_register_t *   reg;
    struct _driver_list * next;
} driver_list_t;

static driver_list_t * list_of_drivers;

void init_drivers() {
    list_of_drivers = 0;
}

int register_driver(driver_register_t * reg) {
    driver_list_t * entry = kmalloc(sizeof(driver_list_t));
    if (!entry) {
        return -1;
    }

    entry->reg      = reg;
    entry->next     = list_of_drivers;
    list_of_drivers = entry;

    return 0;
}

int unregister_driver(driver_register_t * reg) {
    return -1;
}

driver_device_disk_t * driver_get_disk(const char * type) {
    driver_list_t * entry = list_of_drivers;

    while (entry) {
        if (entry->reg->type == DRIVER_DEVICE_TYPE_DISK) {
            if (kstrcmp(entry->reg->disk.type, type) == 0) {
                driver_register_t *    reg  = entry->reg;
                driver_device_disk_t * disk = &reg->disk;
                return disk;
            }
        }
        entry = entry->next;
    }

    return 0;
}

driver_device_fs_t * driver_get_fs(const char * format) {
    driver_list_t * entry = list_of_drivers;

    while (entry) {
        if (entry->reg->type == DRIVER_DEVICE_TYPE_FS) {
            if (kstrcmp(entry->reg->fs.format, format) == 0) {
                driver_register_t *  reg = entry->reg;
                driver_device_fs_t * fs  = &reg->fs;
                return fs;
            }
        }
        entry = entry->next;
    }

    return 0;
}

driver_disk_t * driver_disk_open(driver_device_disk_t * drv, int id) {
    if (!drv) {
        return 0;
    }

    return drv->fn_open(id);
}

int driver_disk_close(driver_disk_t * disk) {
    if (!disk) {
        return -1;
    }

    return disk->impl->fn_close(disk);
}

int driver_disk_stat(driver_disk_t * disk, driver_disk_stat_t * stat) {
    if (!disk) {
        return -1;
    }

    return disk->impl->fn_stat(disk, stat);
}

int driver_disk_read(driver_disk_t * disk, char * buff, size_t count, size_t addr) {
    if (!disk) {
        return -1;
    }

    return disk->impl->fn_read(disk, buff, count, addr);
}

int driver_disk_write(driver_disk_t * disk, const char * buff, size_t count, size_t addr) {
    if (!disk) {
        return -1;
    }

    return disk->impl->fn_write(disk, buff, count, addr);
}
