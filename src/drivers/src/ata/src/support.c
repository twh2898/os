#include "drivers/_ata/support.h"

#include "cpu/ports.h"
#include "drivers/ata.h"

bool drv_ata_identify(drv_ata_t * device) {
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

void drv_ata_software_reset(drv_ata_t * device) {
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
            // puts("[ERROR] max retries for drv_ata_software_reset wait for drive\n");
            return;
        }
    }
}

bool drv_ata_status(drv_ata_t * device) {
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

int drv_ata_sect_read(drv_ata_t * device, size_t sect_count, uint32_t lba) {
    if (!device) {
        return -1;
    }

    if (sect_count == 0) {
        return 0;
    }

    // read max 256 sectors at a time
    if (sect_count > 256)
        sect_count = 256;

    if (lba > device->sect_count) {
        return -1;
    }
    else if (lba + sect_count > device->sect_count) {
        sect_count = device->sect_count - lba;
    }

    drv_ata_software_reset(device);
    size_t retry = 0;
    while (port_byte_in(device->ct_base + ATA_CTL_ALT_STATUS) & (ATA_STATUS_FLAG_DRQ | ATA_STATUS_FLAG_BSY)) {
        if (retry++ > MAX_RETRY) {
            // puts("[ERROR] max retries for drv_ata_sect_read wait for first status\n");
            return -1;
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
                    // puts("[ERROR] max retries for drv_ata_sect_read wait to read next sect\n");
                    return -1;
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

int drv_ata_sect_write(drv_ata_t * device, size_t sect_count, uint32_t lba) {
    if (!device) {
        return -1;
    }

    if (sect_count == 0) {
        return 0;
    }

    // write max 256 sectors at a time
    if (sect_count > 256)
        sect_count = 256;

    if (lba > device->sect_count) {
        return -1;
    }
    else if (lba + sect_count > device->sect_count) {
        sect_count = device->sect_count - lba;
    }

    drv_ata_software_reset(device);
    size_t retry = 0;
    while (port_byte_in(device->ct_base + ATA_CTL_ALT_STATUS) & (ATA_STATUS_FLAG_DRQ | ATA_STATUS_FLAG_BSY)) {
        if (retry++ > MAX_RETRY) {
            // puts("[ERROR] max retries for drv_ata_sect_write wait for first status\n");
            return -1;
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
                    // puts("[ERROR] max retries for drv_ata_sect_write wait to write next sect\n");
                    return -1;
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
