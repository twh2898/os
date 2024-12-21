#include "drivers/ata.h"

#include <stdint.h>

#include "cpu/ports.h"
#include "libc/memory.h"
#include "libc/string.h"

#define MAX_RETRY       5000
#define ATA_BUFFER_SIZE 4096

#define ATA_SECTOR_WORDS 256
#define ATA_SECTOR_BYTES (ATA_SECTOR_WORDS * 2)

#define ATA_BUS_0_IO_BASE  0x1F0
#define ATA_BUS_0_CTL_BASE 0x3F6

enum ATA_IO {
    ATA_IO_DATA          = 0,                    // R/W
    ATA_IO_ERROR         = 1,                    // R
    ATA_IO_FEATURE       = 1,                    // W
    ATA_IO_SECTOR_COUNT  = 2,                    // R/W
    ATA_IO_SECTOR_NUMBER = 3,                    // R/W
    ATA_IO_LBA_LOW       = ATA_IO_SECTOR_NUMBER, // R/W
    ATA_IO_CYLINDER_LOW  = 4,                    // R/W
    ATA_IO_LBA_MID       = ATA_IO_CYLINDER_LOW,  // R/W
    ATA_IO_CYLINDER_HIGH = 5,                    // R/W
    ATA_IO_LBA_HIGH      = ATA_IO_CYLINDER_HIGH, // R/W
    ATA_IO_DRIVE_HEAD    = 6,                    // R/W
    ATA_IO_STATUS        = 7,                    // R
    ATA_IO_COMMAND       = 7,                    // W
};

enum ATA_CTL {
    ATA_CTL_ALT_STATUS = 0, // R
    ATA_CTL_CONTROL    = 0, // W
    ATA_CTL_ADDRESS    = 1, // R
};

enum ATA_ERROR_FLAG {
    ATA_ERROR_FLAG_AMNF  = 0x1,  // Address mark not found
    ATA_ERROR_FLAG_TKZNK = 0x2,  // Track zero not found
    ATA_ERROR_FLAG_ABRT  = 0x4,  // Aborted command
    ATA_ERROR_FLAG_MCR   = 0x8,  // Media change request
    ATA_ERROR_FLAG_IDNF  = 0x10, // ID not found
    ATA_ERROR_FLAG_MC    = 0x20, // Media changed
    ATA_ERROR_FLAG_UNC   = 0x40, // Uncorrectable data error
    ATA_ERROR_FLAG_BBK   = 0x80, // Bad block detected
};

enum ATA_STATUS_FLAG {
    ATA_STATUS_FLAG_ERR = 0x1, // Error occurred
    // ATA_STATUS_FLAG_IDX  = 0x2,  // Index, always zero
    // ATA_STATUS_FLAG_CORR = 0x4,  // Corrected data, always zero
    ATA_STATUS_FLAG_DRQ = 0x8,  // Drive has PIO data to transfer / ready to accept PIO data
    ATA_STATUS_FLAG_SRV = 0x10, // Overlapped mode service request
    ATA_STATUS_FLAG_DF  = 0x20, // Drive fault (does not set ERR)
    ATA_STATUS_FLAG_RDY = 0x40, // Drive is ready (spun up + no errors)
    ATA_STATUS_FLAG_BSY = 0x80, // Drive is preparing to send/receive data
};

enum ATA_CONTROL_FLAG {
    ATA_CONTROL_FLAG_NIEN = 0x2,  // Stop interrupts from the current device
    ATA_CONTROL_FLAG_SRST = 0x4,  // Software reset, set then clear after 5 us
    ATA_CONTROL_FLAG_HOB  = 0x80, // Red high order byte of last LBA48 sent to io
                                  // port
};

enum ATA_ADDRESS_FLAG {
    ATA_ADDRESS_FLAG_DS0 = 0x1,  // Select drive 0
    ATA_ADDRESS_FLAG_DS1 = 0x2,  // Select drive 1
    ATA_ADDRESS_FLAG_HS  = 0x3C, // 1's complement selected head
    ATA_ADDRESS_FLAG_WTG = 0x40, // Low when drive write is in progress
};

typedef struct _drv_ata {
    uint16_t io_base;
    uint16_t ct_base;
    uint32_t sect_count;
    char     buff[ATA_BUFFER_SIZE];
} drv_ata_t;

static bool ata_identify(drv_ata_t * device);
static void software_reset(drv_ata_t * device);
static bool ata_status(drv_ata_t * device);
// Read one sector into device buffer
static size_t ata_sect_read(drv_ata_t * device, size_t sect_count, uint32_t lba);
static size_t ata_sect_write(drv_ata_t * device, size_t sect_count, uint32_t lba);

static driver_register_t drv_register;

int drv_ata_init() {
    drv_register.type          = DRIVER_DEVICE_TYPE_DISK;
    drv_register.disk.fn_open  = drv_ata_open;
    drv_register.disk.fn_close = drv_ata_close;
    drv_register.disk.fn_stat  = drv_ata_stat;
    drv_register.disk.fn_read  = drv_ata_read;
    drv_register.disk.fn_write = drv_ata_write;

    // TODO register interrupts
    // register_interrupt_handler(IRQ14, drv_ata_irq_callback);

    int res = register_driver(&drv_register);
    return res;
}

driver_disk_t * drv_ata_open(int id) {
    if (id != 0) {
        return 0;
    }

    drv_ata_t * device = kmalloc(sizeof(drv_ata_t));
    if (!device) {
        return 0;
    }

    device->io_base = ATA_BUS_0_IO_BASE;
    device->ct_base = ATA_BUS_0_CTL_BASE;

    // will set sector count
    if (!ata_identify(device)) {
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

    if (ata_sect_read(device, sect_to_read, lba) != sect_to_read) {
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
        if (!ata_sect_read(device, 1, lba))
            return 0;
        sect_to_write++;
    }

    kmemcpy(device->buff, buff, count);

    if (ata_sect_write(device, sect_to_write, lba) != sect_to_write) {
        return 0;
    }

    return count;
}

static bool ata_identify(drv_ata_t * device) {
    if (!device)
        return false;

    port_byte_out(device->io_base + ATA_IO_DRIVE_HEAD, 0xA0);

    port_byte_out(device->io_base + ATA_IO_LBA_LOW, 0x0);
    port_byte_out(device->io_base + ATA_IO_LBA_MID, 0x0);
    port_byte_out(device->io_base + ATA_IO_LBA_HIGH, 0x0);

    port_byte_out(device->io_base + ATA_IO_COMMAND, 0xEC); // IDENTIFY command
    uint16_t status = port_word_in(device->io_base + ATA_IO_STATUS);

    if (status == 0) {
        // puts("Drive does not exist\n");
        return false;
    }

    size_t retry = 0;
    while (status & ATA_STATUS_FLAG_BSY) {
        status = port_byte_in(device->io_base + ATA_IO_STATUS);
        if (retry++ > MAX_RETRY) {
            // puts("[ERROR] max retries for ata_identity wait for first status\n");
            return 0;
        }
    }

    if (port_byte_in(device->io_base + ATA_IO_LBA_MID) || port_byte_in(device->io_base + ATA_IO_LBA_HIGH)) {
        // puts("Disk does not support ATA\n");
        return false;
    }

    retry = 0;
    while (!(status & (ATA_STATUS_FLAG_DRQ | ATA_STATUS_FLAG_ERR))) {
        status = port_byte_in(device->io_base + ATA_IO_STATUS);
        if (retry++ > MAX_RETRY) {
            // puts("[ERROR] max retries for ata_identity wait for second status\n");
            return 0;
        }
    }

    if (status & ATA_STATUS_FLAG_ERR) {
        // puts("Disk initialized with errors\n");
        return false;
    }


    uint16_t data[ATA_SECTOR_WORDS];
    for (size_t i = 0; i < ATA_SECTOR_WORDS; i++) {
        data[i] = port_word_in(device->io_base + ATA_IO_DATA);
    }

    bool has_lba = (data[83] & (1 << 10));

    uint32_t size28 = data[61];
    size28          = size28 << 16;
    size28 |= data[60];

    uint64_t size48 = data[100];
    size48          = (size48 << 16) | data[101];
    size48          = (size48 << 16) | data[102];
    size48          = (size48 << 16) | data[103];

    device->sect_count = size28;
    return true;
}

static void software_reset(drv_ata_t * device) {
    if (!device)
        return;

    port_byte_out(device->ct_base + ATA_CTL_CONTROL, ATA_CONTROL_FLAG_SRST);
    port_byte_out(device->ct_base + ATA_CTL_CONTROL, 0);

    // delay 400 ns
    port_byte_in(device->ct_base + ATA_CTL_ALT_STATUS);
    port_byte_in(device->ct_base + ATA_CTL_ALT_STATUS);
    port_byte_in(device->ct_base + ATA_CTL_ALT_STATUS);
    port_byte_in(device->ct_base + ATA_CTL_ALT_STATUS);

    uint8_t status = port_byte_in(device->ct_base + ATA_CTL_ALT_STATUS);
    // while ((status & (ATA_STATUS_FLAG_RDY | ATA_STATUS_FLAG_BSY)) !=
    // ATA_STATUS_FLAG_RDY) {
    size_t retry = 0;
    while ((status & 0xc0) != 0x40) {
        status = port_byte_in(device->ct_base + ATA_CTL_ALT_STATUS);
        if (retry++ > MAX_RETRY) {
            // puts("[ERROR] max retries for software_reset wait for drive\n");
            return;
        }
    }
}

static bool ata_status(drv_ata_t * device) {
    if (!device)
        return false;

    uint8_t status = port_byte_in(device->ct_base + ATA_CTL_ALT_STATUS);
    // if (debug) {
    //     printf("Status is %02X\n", status);
    //     if (status & ATA_STATUS_FLAG_ERR)
    //         puts("ERR ");
    //     if (status & ATA_STATUS_FLAG_DRQ)
    //         puts("DRQ ");
    //     if (status & ATA_STATUS_FLAG_SRV)
    //         puts("SRV ");
    //     if (status & ATA_STATUS_FLAG_DF)
    //         puts("DF ");
    //     if (status & ATA_STATUS_FLAG_RDY)
    //         puts("RDY ");
    //     if (status & ATA_STATUS_FLAG_BSY)
    //         puts("BSY ");
    //     putc('\n');
    // }

    if (status & ATA_STATUS_FLAG_ERR) {
        // if (debug) {
        //     uint8_t error = port_byte_in(device->io_base + ATA_IO_ERROR);
        //     if (error & ATA_ERROR_FLAG_AMNF)
        //         puts("ERROR: AMNF - Address mark not found\n");
        //     if (error & ATA_ERROR_FLAG_TKZNK)
        //         puts("ERROR: TKZNK - Track zero not found\n");
        //     if (error & ATA_ERROR_FLAG_ABRT)
        //         puts("ERROR: ABRT - Aborted command\n");
        //     if (error & ATA_ERROR_FLAG_MCR)
        //         puts("ERROR: MCR - Media change request\n");
        //     if (error & ATA_ERROR_FLAG_IDNF)
        //         puts("ERROR: IDNF - ID not found\n");
        //     if (error & ATA_ERROR_FLAG_MC)
        //         puts("ERROR: MC - Media changed\n");
        //     if (error & ATA_ERROR_FLAG_UNC)
        //         puts("ERROR: UNC - Uncorrectable data error\n");
        //     if (error & ATA_ERROR_FLAG_BBK)
        //         puts("ERROR: BBK - Bad block detected\n");
        // }
        return true;
    }

    return false;
}

static size_t ata_sect_read(drv_ata_t * device, size_t sect_count, uint32_t lba) {
    if (!device || !sect_count)
        return 0;

    // read max 256 sectors at a time
    if (sect_count > 256)
        sect_count = 256;

    if (lba > device->sect_count) {
        return 0;
    }
    else if (lba + sect_count > device->sect_count) {
        sect_count = device->sect_count - lba;
    }

    software_reset(device);
    size_t retry = 0;
    while (port_byte_in(device->ct_base + ATA_CTL_ALT_STATUS) & (ATA_STATUS_FLAG_DRQ | ATA_STATUS_FLAG_BSY)) {
        if (retry++ > MAX_RETRY) {
            // puts("[ERROR] max retries for ata_sect_read wait for first status\n");
            return 0;
        }
    }

    port_byte_out(device->io_base + ATA_IO_DRIVE_HEAD, (0xE0 | ((lba >> 24) & 0xF)));
    port_byte_out(0x1F1, 0); // delay?
    port_byte_out(device->io_base + ATA_IO_SECTOR_COUNT, (sect_count >= 256 ? 0 : sect_count));
    port_byte_out(device->io_base + ATA_IO_LBA_LOW, lba & 0xFF);
    port_byte_out(device->io_base + ATA_IO_LBA_MID, (lba >> 8) & 0xFF);
    port_byte_out(device->io_base + ATA_IO_LBA_HIGH, (lba >> 16) & 0xFF);
    port_byte_out(device->io_base + ATA_IO_COMMAND, 0x20); // read sectors

    uint8_t * buff = device->buff;

    for (size_t s = 0; s < sect_count; s++) {
        // Read entire sector
        for (size_t i = 0; i < ATA_SECTOR_WORDS; i++) {
            // Wait for drive to be ready
            retry = 0;
            while (!(port_byte_in(device->ct_base + ATA_CTL_ALT_STATUS) & ATA_STATUS_FLAG_DRQ)) {
                if (retry++ > MAX_RETRY) {
                    // puts("[ERROR] max retries for ata_sect_read wait to read next sect\n");
                    return 0;
                }
            }

            // read drive data
            uint16_t word = port_word_in(device->io_base + ATA_IO_DATA);

            buff[i * 2]     = word & 0xFF;
            buff[i * 2 + 1] = (word >> 8) & 0xFF;
        }
        buff += ATA_SECTOR_BYTES;
    }

    return sect_count;
}

static size_t ata_sect_write(drv_ata_t * device, size_t sect_count, uint32_t lba) {
    if (!device || !sect_count)
        return 0;

    // write max 256 sectors at a time
    if (sect_count > 256)
        sect_count = 256;

    if (lba > device->sect_count) {
        return 0;
    }
    else if (lba + sect_count > device->sect_count) {
        sect_count = device->sect_count - lba;
    }

    software_reset(device);
    size_t retry = 0;
    while (port_byte_in(device->ct_base + ATA_CTL_ALT_STATUS) & (ATA_STATUS_FLAG_DRQ | ATA_STATUS_FLAG_BSY)) {
        if (retry++ > MAX_RETRY) {
            // puts("[ERROR] max retries for ata_sect_write wait for first status\n");
            return 0;
        }
    }

    port_byte_out(device->io_base + ATA_IO_DRIVE_HEAD, (0xE0 | ((lba >> 24) & 0xF)));
    port_byte_out(0x1F1, 0); // delay?
    port_byte_out(device->io_base + ATA_IO_SECTOR_COUNT, (sect_count >= 256 ? 0 : sect_count));
    port_byte_out(device->io_base + ATA_IO_LBA_LOW, lba & 0xFF);
    port_byte_out(device->io_base + ATA_IO_LBA_MID, (lba >> 8) & 0xFF);
    port_byte_out(device->io_base + ATA_IO_LBA_HIGH, (lba >> 16) & 0xFF);
    port_byte_out(device->io_base + ATA_IO_COMMAND, 0x30); // write sectors

    uint8_t * buff = device->buff;

    size_t o_len = 0;
    for (size_t s = 0; s < sect_count; s++) {
        // Read entire sector
        for (size_t i = 0; i < ATA_SECTOR_WORDS; i++) {
            // Wait for drive to be ready
            retry = 0;
            while (!(port_byte_in(device->ct_base + ATA_CTL_ALT_STATUS) & ATA_STATUS_FLAG_DRQ)) {
                if (retry++ > MAX_RETRY) {
                    // puts("[ERROR] max retries for ata_sect_write wait to write next sect\n");
                    return 0;
                }
            }

            // write drive data
            uint16_t word = buff[i * 2] | (buff[i * 2 + 1] << 8);
            port_word_out(device->io_base + ATA_IO_DATA, word);
        }
        buff += ATA_SECTOR_BYTES;
    }

    port_byte_out(device->io_base + ATA_IO_COMMAND, 0xE7); // cache flush
    return sect_count;
}
