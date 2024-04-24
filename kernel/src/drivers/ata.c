#include "drivers/ata.h"

#include "cpu/isr.h"
#include "cpu/ports.h"
#include "debug.h"
#include "libc/stdio.h"

// https://wiki.osdev.org/ATA_PIO_Mode

#define ATA_BUS_0_IO_BASE 0x1F0
#define ATA_BUS_0_CTL_BASE 0x3F6

#define ATA_BUS_0_IO_DATA (ATA_BUS_0_IO_BASE + 0) // R/W
#define ATA_BUS_0_IO_ERROR (ATA_BUS_0_IO_BASE + 1) // R
#define ATA_BUS_0_IO_FEATURE (ATA_BUS_0_IO_BASE + 1) // W
#define ATA_BUS_0_IO_SECTOR_COUNT (ATA_BUS_0_IO_BASE + 2) // R/W
#define ATA_BUS_0_IO_SECTOR_NUMBER (ATA_BUS_0_IO_BASE + 3) // R/W
#define ATA_BUS_0_IO_LBA_LOW ATA_BUS_0_IO_SECTOR_NUMBER // R/W
#define ATA_BUS_0_IO_CYLINDER_LOW (ATA_BUS_0_IO_BASE + 4) // R/W
#define ATA_BUS_0_IO_LBA_MID ATA_BUS_0_IO_CYLINDER_LOW // R/W
#define ATA_BUS_0_IO_CYLINDER_HIGH (ATA_BUS_0_IO_BASE + 5) // R/W
#define ATA_BUS_0_IO_LBA_HIGH ATA_BUS_0_IO_CYLINDER_HIGH // R/W
#define ATA_BUS_0_IO_DRIVE_HEAD (ATA_BUS_0_IO_BASE + 6) // R/W
#define ATA_BUS_0_IO_STATUS (ATA_BUS_0_IO_BASE + 7) // R
#define ATA_BUS_0_IO_COMMAND (ATA_BUS_0_IO_BASE + 7) // W

#define ATA_BUS_0_CTL_ALT_STATUS (ATA_BUS_0_CTL_BASE + 0) // R
#define ATA_BUS_0_CTL_CONTROL (ATA_BUS_0_CTL_BASE + 0) // W
#define ATA_BUS_0_CTL_ADDRESS (ATA_BUS_0_CTL_BASE + 1) // R

#define ATA_ERROR_FLAG_AMNF 0x1 // Address mark not found
#define ATA_ERROR_FLAG_TKZNK 0x2 // Track zero not found
#define ATA_ERROR_FLAG_ABRT 0x4 // Aborted command
#define ATA_ERROR_FLAG_MCR 0x8 // Media change request
#define ATA_ERROR_FLAG_IDNF 0x10 // ID not found
#define ATA_ERROR_FLAG_MC 0x20 // Media changed
#define ATA_ERROR_FLAG_UNC 0x40 // Uncorrectable data error
#define ATA_ERROR_FLAG_BBK 0x80 // Bad block detected

#define ATA_STATUS_FLAG_ERR 0x1 // Error occurred
// #define ATA_STATUS_FLAG_IDX 0x2 // Index, always zero
// #define ATA_STATUS_FLAG_CORR 0x4 // Corrected data, always zero
#define ATA_STATUS_FLAG_DRQ \
    0x8 // Drive has PIO data to transfer / ready to accept PIO data
#define ATA_STATUS_FLAG_SRV 0x10 // Overlapped mode service request
#define ATA_STATUS_FLAG_DF 0x20 // Drive fault (does not set ERR)
#define ATA_STATUS_FLAG_RDY 0x40 // Drive is ready (spun up + no errors)
#define ATA_STATUS_FLAG_BSY 0x80 // Drive is preparing to send/receive data

#define ATA_CONTROL_FLAG_NIEN 0x2 // Stop interrupts from the current device
#define ATA_CONTROL_FLAG_SRST 0x4 // Software reset, set then clear after 5 us
#define ATA_CONTROL_FLAG_HOB \
    0x80 // Red high order byte of last LBA48 sent to io port

#define ATA_ADDRESS_FLAG_DS0 0x1 // Select drive 0
#define ATA_ADDRESS_FLAG_DS1 0x2 // Select drive 1
#define ATA_ADDRESS_FLAG_HS 0x3C // 1's complement selected head
#define ATA_ADDRESS_FLAG_WTG 0x40 // Low when drive write is in progress

static void disk_callback(registers_t regs) {
    // printf("disk callback\n");
}

void init_disk() {
    /* Primary Drive */
    register_interrupt_handler(IRQ14, disk_callback);

    // /* Get the PIT value: hardware clock at 1193180 Hz */
    // uint32_t divisor = 1193180 / freq;
    // uint8_t low = (uint8_t)(divisor & 0xFF);
    // uint8_t high = (uint8_t)((divisor >> 8) & 0xFF);
    // /* Send the command */
    // port_byte_out(0x43, 0x36); /* Command port */
    // port_byte_out(0x40, low);
    // port_byte_out(0x40, high);
}

void disk_identify() {
    port_byte_out(ATA_BUS_0_IO_DRIVE_HEAD, 0xA0);

    port_byte_out(ATA_BUS_0_IO_LBA_LOW, 0x0);
    port_byte_out(ATA_BUS_0_IO_LBA_MID, 0x0);
    port_byte_out(ATA_BUS_0_IO_LBA_HIGH, 0x0);

    port_byte_out(ATA_BUS_0_IO_COMMAND, 0xEC); // IDENTIFY command
    uint16_t status = port_word_in(ATA_BUS_0_IO_STATUS);

    if (status == 0) {
        puts("Drive does not exist\n");
        return;
    }

    puts("Polling");
    while (status & ATA_STATUS_FLAG_BSY) {
        putc('.');
        status = port_byte_in(ATA_BUS_0_IO_STATUS);
    }
    putc('\n');

    if (port_byte_in(ATA_BUS_0_IO_LBA_MID) || port_byte_in(ATA_BUS_0_IO_LBA_HIGH)) {
        puts("Disk does not support ATA\n");
        return;
    }
    puts("Drive is ATA\n");

    puts("Polling");
    while (!(status & (ATA_STATUS_FLAG_DRQ | ATA_STATUS_FLAG_ERR))) {
        putc('.');
        status = port_byte_in(ATA_BUS_0_IO_STATUS);
    }
    putc('\n');

    if (status & ATA_STATUS_FLAG_ERR) {
        puts("Disk initialized with errors\n");
        return;
    }

    if (status & ATA_STATUS_FLAG_DRQ) {
        puts("Disk is ready\n");
    }

    uint16_t data[256];
    for (size_t i = 0; i < 256; i++) {
        data[i] = port_word_in(ATA_BUS_0_IO_DATA);
    }

    if (debug) {
        printf("Data is:\n");
        size_t step = 8;
        for (size_t i = 0; i < (256 / step); i++) {
            printf("%4u", i * step);
            for (size_t s = 0; s < step; s++) {
                printf(" %04X", data[(i * step) + s]);
            }
            putc('\n');
        }
    }

    bool has_lba = (data[83] & (1 << 10));
    if (has_lba) {
        printf("Drive has LBA48 Mode\n");
    }

    uint32_t size28 = data[61];
    size28 = size28 << 16;
    size28 |= data[60];

    printf("LDA28 has %u sectors\n", size28);

    uint64_t size48 = data[100];
    size48 = (size48 << 16) | data[101];
    size48 = (size48 << 16) | data[102];
    size48 = (size48 << 16) | data[103];

    printf("LDA48 has %u sectors\n", size48);

    uint32_t size = (size28 >> 10) * 7;
    printf("Disk size is %u kb\n", size);
}
