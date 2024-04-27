#include "drivers/ata.h"

#include "cpu/isr.h"
#include "cpu/ports.h"
#include "debug.h"
#include "drivers/rtc.h"
#include "libc/stdio.h"

// https://wiki.osdev.org/ATA_PIO_Mode

#define TIMEOUT_MS 1000
#define START_TIMEOUT uint32_t __timeout = time_ms() + TIMEOUT_MS;
#define TEST_TIMEOUT                                          \
    if (time_ms() > __timeout) {                              \
        puts("TIMEOUT\n");                                    \
        return 0;                                             \
    }                                                         \
    else if (debug) {                                         \
        printf("no timeout %u < %u\n", time_ms(), __timeout); \
    }
#define TEST_TIMEOUT_VOID                                     \
    if (time_ms() > __timeout) {                              \
        puts("TIMEOUT\n");                                    \
        return;                                               \
    }                                                         \
    else if (debug) {                                         \
        printf("no timeout %u < %u\n", time_ms(), __timeout); \
    }

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

#define ATA_SECTOR_WORDS 256

static void disk_callback(registers_t regs) {
    puts("disk callback\n");
}

static void print_block(uint16_t * block) {
    size_t step = 16;
    for (size_t i = 0; i < (ATA_SECTOR_WORDS / step); i++) {
        // printf("%4u", i * step);
        for (size_t s = 0; s < step; s++) {
            if (s)
                putc(' ');
            printf("%04X", block[(i * step) + s]);
        }
        putc('\n');
    }
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

size_t disk_identify() {
    START_TIMEOUT
    port_byte_out(ATA_BUS_0_IO_DRIVE_HEAD, 0xA0);

    port_byte_out(ATA_BUS_0_IO_LBA_LOW, 0x0);
    port_byte_out(ATA_BUS_0_IO_LBA_MID, 0x0);
    port_byte_out(ATA_BUS_0_IO_LBA_HIGH, 0x0);

    port_byte_out(ATA_BUS_0_IO_COMMAND, 0xEC); // IDENTIFY command
    uint16_t status = port_word_in(ATA_BUS_0_IO_STATUS);

    if (status == 0) {
        puts("Drive does not exist\n");
        return 0;
    }

    puts("Polling");
    while (status & ATA_STATUS_FLAG_BSY) {
        putc('.');
        status = port_byte_in(ATA_BUS_0_IO_STATUS);
        TEST_TIMEOUT
    }
    putc('\n');

    if (port_byte_in(ATA_BUS_0_IO_LBA_MID) || port_byte_in(ATA_BUS_0_IO_LBA_HIGH)) {
        puts("Disk does not support ATA\n");
        return 0;
    }
    puts("Drive is ATA\n");

    puts("Polling");
    while (!(status & (ATA_STATUS_FLAG_DRQ | ATA_STATUS_FLAG_ERR))) {
        putc('.');
        status = port_byte_in(ATA_BUS_0_IO_STATUS);
        TEST_TIMEOUT
    }
    putc('\n');

    if (status & ATA_STATUS_FLAG_ERR) {
        puts("Disk initialized with errors\n");
        return 0;
    }

    if (status & ATA_STATUS_FLAG_DRQ) {
        puts("Disk is ready\n");
    }

    uint16_t data[ATA_SECTOR_WORDS];
    for (size_t i = 0; i < ATA_SECTOR_WORDS; i++) {
        data[i] = port_word_in(ATA_BUS_0_IO_DATA);
    }

    if (debug) {
        printf("Data is:\n");
        size_t step = 8;
        for (size_t i = 0; i < (ATA_SECTOR_WORDS / step); i++) {
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
    return size;
}

void software_reset() {
    START_TIMEOUT
    port_byte_out(ATA_BUS_0_CTL_CONTROL, ATA_CONTROL_FLAG_SRST);
    disk_status();
    port_byte_out(ATA_BUS_0_CTL_CONTROL, 0);

    // delay 400 ns
    port_byte_in(ATA_BUS_0_CTL_ALT_STATUS);
    port_byte_in(ATA_BUS_0_CTL_ALT_STATUS);
    port_byte_in(ATA_BUS_0_CTL_ALT_STATUS);
    port_byte_in(ATA_BUS_0_CTL_ALT_STATUS);

    uint8_t status = port_byte_in(ATA_BUS_0_CTL_ALT_STATUS);
    // while ((status & (ATA_STATUS_FLAG_RDY | ATA_STATUS_FLAG_BSY)) !=
    // ATA_STATUS_FLAG_RDY) {
    while ((status & 0xc0) != 0x40) {
        if (debug)
            disk_status();
        status = port_byte_in(ATA_BUS_0_CTL_ALT_STATUS);
        TEST_TIMEOUT_VOID
    }
}

void disk_status() {
    uint8_t status = port_byte_in(ATA_BUS_0_CTL_ALT_STATUS);
    printf("Status is %02X\n", status);
    if (status & ATA_STATUS_FLAG_ERR)
        puts("ERR ");
    if (status & ATA_STATUS_FLAG_DRQ)
        puts("DRQ ");
    if (status & ATA_STATUS_FLAG_SRV)
        puts("SRV ");
    if (status & ATA_STATUS_FLAG_DF)
        puts("DF ");
    if (status & ATA_STATUS_FLAG_RDY)
        puts("RDY ");
    if (status & ATA_STATUS_FLAG_BSY)
        puts("BSY ");
    putc('\n');
}

// size_t disk_read(uint8_t * data, size_t count, uint32_t lba) {
// if (count > 0x3fffff) {
//     puts("[ERROR] Size is more than 2GB\n");
//     return 0;
// }

size_t disk_read(uint32_t lba) {
    software_reset();
    START_TIMEOUT
    puts("waiting for drive ready");
    while (port_byte_in(ATA_BUS_0_CTL_ALT_STATUS)
           & (ATA_STATUS_FLAG_DRQ | ATA_STATUS_FLAG_BSY)) {
        if (debug)
            putc('.');
        TEST_TIMEOUT
    }
    putc('\n');

    port_byte_out(ATA_BUS_0_IO_DRIVE_HEAD, (0xE0 | ((lba >> 24) & 0xF)));
    port_byte_out(0x1F1, 0); // delay?
    port_byte_out(ATA_BUS_0_IO_SECTOR_COUNT, 1);
    port_byte_out(ATA_BUS_0_IO_LBA_LOW, lba & 0xFF);
    port_byte_out(ATA_BUS_0_IO_LBA_MID, (lba >> 8) & 0xFF);
    port_byte_out(ATA_BUS_0_IO_LBA_HIGH, (lba >> 16) & 0xFF);
    port_byte_out(ATA_BUS_0_IO_COMMAND, 0x20); // read sectors

    // TODO poll or wait for IRQ then read next (count) sectors
    puts("waiting for data");
    // while (!(port_byte_in(ATA_BUS_0_IO_STATUS) & ATA_STATUS_FLAG_DRQ)) {
    //     if (debug)
    //         putc('.');
    // }
    putc('\n');

    uint16_t data[ATA_SECTOR_WORDS];
    for (size_t i = 0; i < ATA_SECTOR_WORDS; i++) {
        while (!(port_byte_in(ATA_BUS_0_CTL_ALT_STATUS) & ATA_STATUS_FLAG_DRQ)) {
            disk_status();
            TEST_TIMEOUT
        }
        data[i] = port_word_in(ATA_BUS_0_IO_DATA);
    }
    print_block(data);
    return 0;
}

size_t disk_write(uint32_t lba) {
    software_reset();
    START_TIMEOUT
    uint32_t start = time_ms();
    puts("waiting for drive ready");
    while (port_byte_in(ATA_BUS_0_CTL_ALT_STATUS)
           & (ATA_STATUS_FLAG_DRQ | ATA_STATUS_FLAG_BSY)) {
        if (debug)
            putc('.');
        if (time_ms() > start + TIMEOUT_MS)
            return 0;
        TEST_TIMEOUT
    }
    putc('\n');

    port_byte_out(ATA_BUS_0_IO_DRIVE_HEAD, (0xE0 | ((lba >> 24) & 0xF)));
    port_byte_out(0x1F1, 0); // delay?
    port_byte_out(ATA_BUS_0_IO_SECTOR_COUNT, 1);
    port_byte_out(ATA_BUS_0_IO_LBA_LOW, lba & 0xFF);
    port_byte_out(ATA_BUS_0_IO_LBA_MID, (lba >> 8) & 0xFF);
    port_byte_out(ATA_BUS_0_IO_LBA_HIGH, (lba >> 16) & 0xFF);
    port_byte_out(ATA_BUS_0_IO_COMMAND, 0x30); // write sectors

    // TODO poll or wait for IRQ then read next (count) sectors
    puts("waiting for data");
    // while (!(port_byte_in(ATA_BUS_0_IO_STATUS) & ATA_STATUS_FLAG_DRQ)) {
    //     if (debug)
    //         putc('.');
    // }
    putc('\n');

    uint16_t data[ATA_SECTOR_WORDS];
    for (uint16_t i = 0; i < ATA_SECTOR_WORDS; i++) {
        data[i] = i;
    }
    for (size_t i = 0; i < ATA_SECTOR_WORDS; i++) {
        port_word_out(ATA_BUS_0_IO_DATA, data[i]);
    }
    return 0;
}
