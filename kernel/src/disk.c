#include "disk.h"

#include "drivers/ata.h"
#include "libc/mem.h"
#include "libc/string.h"

#define DISK_BUFFER_SIZE 4096

typedef size_t (*disk_io)(disk_t * disk, uint8_t * buff, size_t count, size_t pos);

struct _disk {
    enum DISK_DRIVER driver;

    char * buff;
    size_t buff_size;

    union {
        ata_t * ata;
        // TODO floppy
        // TODO ram
    } device;

    size_t size;
    disk_io fn_read;
    disk_io fn_write;
};

static size_t disk_ata_read(disk_t * disk, uint8_t * buff, size_t count, size_t pos);
static size_t disk_ata_write(disk_t * disk, uint8_t * buff, size_t count, size_t pos);

disk_t * disk_open(int id, enum DISK_DRIVER driver) {
    disk_t * disk = malloc(sizeof(disk_t));
    if (disk) {
        disk->driver = driver;

        disk->buff_size = DISK_BUFFER_SIZE;
        disk->buff = malloc(disk->buff_size);
        if (!disk->buff) {
            free(disk);
            return 0;
        }

        switch (driver) {
            case DISK_DRIVER_ATA: {
                disk->device.ata = ata_open(id);
                if (!disk->device.ata) {
                    free(disk->buff);
                    free(disk);
                    return 0;
                }

                disk->size = ata_size(disk->device.ata);
                disk->fn_read = disk_ata_read;
                disk->fn_write = disk_ata_write;
            } break;
            case DISK_DRIVER_FLOPPY: {
                // TODO open floppy driver
                free(disk->buff);
                free(disk);
                return 0;
            } break;
            case DISK_DRIVER_RAM_DISK: {
                // TODO open ram disk driver
                free(disk->buff);
                free(disk);
                return 0;
            } break;
            default: {
                return 0;
            } break;
        }
    }
    return disk;
}

void disk_close(disk_t * disk) {
    switch (disk->driver) {
        case DISK_DRIVER_ATA: {
            ata_close(disk->device.ata);
        } break;
        case DISK_DRIVER_FLOPPY: {

        } break;
        case DISK_DRIVER_RAM_DISK: {

        } break;
        default: {

        } break;
    }
    free(disk->buff);
    free(disk);
}

size_t disk_size(disk_t * disk) {
    return disk->size;
}

size_t disk_read(disk_t * disk, uint8_t * buff, size_t count, size_t pos) {
    size_t steps = count / disk->buff_size;
    if (count % disk->buff_size)
        steps++;

    size_t o_len = 0;
    for (size_t i = 0; i < steps; i++) {
        size_t offset = i * disk->buff_size;
        size_t to_write = disk->buff_size;
        if (count < to_write)
            to_write = count;
        o_len += disk->fn_read(disk, buff + offset, to_write, pos + offset);
    }
    return o_len;
}

size_t disk_write(disk_t * disk, uint8_t * buff, size_t count, size_t pos) {
    size_t steps = count / disk->buff_size;
    if (count % disk->buff_size)
        steps++;

    size_t o_len = 0;
    for (size_t i = 0; i < steps; i++) {
        size_t offset = i * disk->buff_size;
        size_t to_write = disk->buff_size;
        if (count < to_write)
            to_write = count;
        o_len += disk->fn_write(disk, buff + offset, to_write, pos + offset);
    }
    return o_len;
}

static size_t disk_ata_read(disk_t * disk, uint8_t * buff, size_t count, size_t pos) {
    // TODO currently assume count < buff_size, handle bigger with loop
    if (count > disk->buff_size)
        return 0;

    if (disk->size - pos < count)
        count = disk->size - pos;

    size_t lba = pos / ATA_SECTOR_BYTES;
    size_t sect_to_read = count / ATA_SECTOR_BYTES;

    if (count % ATA_SECTOR_BYTES)
        sect_to_read++;

    if (ata_sect_read(disk->device.ata, disk->buff, sect_to_read, lba)
        != sect_to_read) {
        return 0;
    }

    size_t pos_in_buff = pos % ATA_SECTOR_BYTES;
    memcpy(buff, disk->buff + pos_in_buff, count);
    return count;
}

static size_t disk_ata_write(disk_t * disk, uint8_t * buff, size_t count, size_t pos) {
    // TODO currently assume count < buff_size, handle bigger with loop
    if (count > disk->buff_size)
        return 0;

    if (disk->size - pos < count)
        count = disk->size - pos;

    size_t lba = pos / ATA_SECTOR_BYTES;
    size_t sect_to_write = count / ATA_SECTOR_BYTES;

    if (count % ATA_SECTOR_BYTES) {
        if (!ata_sect_read(disk->device.ata, disk->buff, 1, lba))
            return 0;
        sect_to_write++;
    }

    memcpy(disk->buff, buff, count);

    if (ata_sect_write(disk->device.ata, disk->buff, sect_to_write, lba)
        != sect_to_write) {
        return 0;
    }

    return count;
}
