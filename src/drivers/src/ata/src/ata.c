#include "drivers/ata.h"

#include <stdint.h>

#include "drivers/_ata/defs.h"
#include "drivers/_ata/support.h"
#include "libc/memory.h"
#include "libc/string.h"

static driver_register_t drv_register;

int drv_ata_init() {
    drv_register.type          = DRIVER_DEVICE_TYPE_DISK;
    drv_register.disk.fn_open  = drv_ata_open;
    drv_register.disk.fn_close = drv_ata_close;
    drv_register.disk.fn_stat  = drv_ata_stat;
    drv_register.disk.fn_read  = drv_ata_read;
    drv_register.disk.fn_write = drv_ata_write;

    // TODO register interrupts? Or does the kernel get a callback fn?
    // register_interrupt_handler(IRQ14, drv_ata_irq_callback);

    int res = register_driver(&drv_register);
    return res;
}

driver_disk_t * drv_ata_open(int id) {
    if (id != 0) {
        return 0;
    }

    // TODO prevent disk from being opened multiple times

    drv_ata_t * device = kmalloc(sizeof(drv_ata_t));
    if (!device) {
        return 0;
    }

    device->io_base = ATA_BUS_0_IO_BASE;
    device->ct_base = ATA_BUS_0_CTL_BASE;

    // will set sector count
    if (!drv_ata_identify(device)) {
        kfree(device);
        return 0;
    }

    driver_disk_t * disk = kmalloc(sizeof(driver_disk_t));
    if (!disk) {
        kfree(device);
        return 0;
    }

    disk->id         = id;
    disk->stat.size  = device->sect_count * ATA_SECTOR_BYTES;
    disk->stat.state = DRIVER_DISK_STATE_IDLE;
    disk->drv_data   = device;

    return disk;
}

int drv_ata_close(driver_disk_t * disk) {
    if (!disk) {
        return -1;
    }

    if (disk->drv_data) {
        kfree(disk->drv_data);
    }

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

    if (disk->id != 0) {
        return -1;
    }

    if (count == 0) {
        return 0;
    }

    size_t size = disk->stat.size;
    if (addr >= size) {
        return 0;
    }

    if (size - addr < count) {
        count = size - addr;
    }

    drv_ata_t * device = disk->drv_data;

    size_t lba          = addr / ATA_SECTOR_BYTES;
    size_t sect_to_read = count / ATA_SECTOR_BYTES;

    if (count % ATA_SECTOR_BYTES)
        sect_to_read++;

    // TODO curr impl should fail to read more than driver buffer, need fix

    if (drv_ata_sect_read(device, sect_to_read, lba) != sect_to_read) {
        return 0;
    }

    size_t pos_in_buff = addr % ATA_SECTOR_BYTES;
    kmemcpy(buff, device->buff + pos_in_buff, count);

    return count;
}

int drv_ata_write(driver_disk_t * disk, const char * buff, size_t count, size_t addr) {
    if (!disk || !buff) {
        return -1;
    }

    if (disk->id != 0) {
        return -1;
    }

    if (count == 0) {
        return 0;
    }

    size_t size = disk->stat.size;
    if (addr >= size) {
        return 0;
    }

    if (size - addr < count) {
        count = size - addr;
    }

    drv_ata_t * device = disk->drv_data;

    size_t lba           = addr / ATA_SECTOR_BYTES;
    size_t sect_to_write = count / ATA_SECTOR_BYTES;

    if (count % ATA_SECTOR_BYTES) {
        if (!drv_ata_sect_read(device, 1, lba))
            return 0;
        sect_to_write++;
    }

    kmemcpy(device->buff, buff, count);

    if (drv_ata_sect_write(device, sect_to_write, lba) != sect_to_write) {
        return 0;
    }

    return count;
}
