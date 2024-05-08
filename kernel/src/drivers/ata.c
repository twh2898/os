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

static uint32_t disk_sector_count;

static void print_block(uint16_t * block);
static size_t disk_identify();
static void software_reset();

static void disk_callback(registers_t regs) {
    if (debug)
        puts("disk callback\n");
}

void init_disk() {
    /* Primary Drive */
    register_interrupt_handler(IRQ14, disk_callback);
    disk_sector_count = disk_identify();
}

size_t disk_size() {
    return disk_sector_count * ATA_SECTOR_BYTES;
}

bool disk_status() {
    uint8_t status = port_byte_in(ATA_BUS_0_CTL_ALT_STATUS);
    if (debug) {
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

    if (status & ATA_STATUS_FLAG_ERR) {
        if (debug) {
            uint8_t error = port_byte_in(ATA_BUS_0_IO_ERROR);
            if (error & ATA_ERROR_FLAG_AMNF)
                puts("ERROR: AMNF - Address mark not found\n");
            if (error & ATA_ERROR_FLAG_TKZNK)
                puts("ERROR: TKZNK - Track zero not found\n");
            if (error & ATA_ERROR_FLAG_ABRT)
                puts("ERROR: ABRT - Aborted command\n");
            if (error & ATA_ERROR_FLAG_MCR)
                puts("ERROR: MCR - Media change request\n");
            if (error & ATA_ERROR_FLAG_IDNF)
                puts("ERROR: IDNF - ID not found\n");
            if (error & ATA_ERROR_FLAG_MC)
                puts("ERROR: MC - Media changed\n");
            if (error & ATA_ERROR_FLAG_UNC)
                puts("ERROR: UNC - Uncorrectable data error\n");
            if (error & ATA_ERROR_FLAG_BBK)
                puts("ERROR: BBK - Bad block detected\n");
        }
        return true;
    }

    return false;
}

size_t disk_sect_read(uint8_t * buff, size_t sect_count, uint32_t lba) {
    if (sect_count == 0)
        return 0;

    // read max 256 sectors at a time
    if (sect_count > 256)
        sect_count = 256;

    if (lba > disk_sector_count) {
        return 0;
    }
    else if (lba + sect_count > disk_sector_count) {
        sect_count = disk_sector_count - lba;
    }

    software_reset();
    START_TIMEOUT
    while (port_byte_in(ATA_BUS_0_CTL_ALT_STATUS)
           & (ATA_STATUS_FLAG_DRQ | ATA_STATUS_FLAG_BSY)) {
        TEST_TIMEOUT
    }

    port_byte_out(ATA_BUS_0_IO_DRIVE_HEAD, (0xE0 | ((lba >> 24) & 0xF)));
    port_byte_out(0x1F1, 0); // delay?
    port_byte_out(ATA_BUS_0_IO_SECTOR_COUNT, (sect_count >= 256 ? 0 : sect_count));
    port_byte_out(ATA_BUS_0_IO_LBA_LOW, lba & 0xFF);
    port_byte_out(ATA_BUS_0_IO_LBA_MID, (lba >> 8) & 0xFF);
    port_byte_out(ATA_BUS_0_IO_LBA_HIGH, (lba >> 16) & 0xFF);
    port_byte_out(ATA_BUS_0_IO_COMMAND, 0x20); // read sectors

    for (size_t s = 0; s < sect_count; s++) {
        // Read entire sector
        for (size_t i = 0; i < ATA_SECTOR_WORDS; i++) {
            // Wait for drive to be ready
            while (!(port_byte_in(ATA_BUS_0_CTL_ALT_STATUS) & ATA_STATUS_FLAG_DRQ)) {
                TEST_TIMEOUT
            }

            // read drive data
            uint16_t word = port_word_in(ATA_BUS_0_IO_DATA);

            buff[i * 2] = word & 0xFF;
            buff[i * 2 + 1] = (word >> 8) & 0xFF;
        }
        buff += ATA_SECTOR_BYTES;
    }

    return sect_count;
}

size_t disk_sect_write(uint8_t * buff, size_t sect_count, uint32_t lba) {
    if (sect_count == 0)
        return 0;

    // write max 256 sectors at a time
    if (sect_count > 256)
        sect_count = 256;

    if (lba > disk_sector_count) {
        return 0;
    }
    else if (lba + sect_count > disk_sector_count) {
        sect_count = disk_sector_count - lba;
    }

    software_reset();
    START_TIMEOUT
    uint32_t start = time_ms();
    while (port_byte_in(ATA_BUS_0_CTL_ALT_STATUS)
           & (ATA_STATUS_FLAG_DRQ | ATA_STATUS_FLAG_BSY)) {
        TEST_TIMEOUT
    }

    port_byte_out(ATA_BUS_0_IO_DRIVE_HEAD, (0xE0 | ((lba >> 24) & 0xF)));
    port_byte_out(0x1F1, 0); // delay?
    port_byte_out(ATA_BUS_0_IO_SECTOR_COUNT, (sect_count >= 256 ? 0 : sect_count));
    port_byte_out(ATA_BUS_0_IO_LBA_LOW, lba & 0xFF);
    port_byte_out(ATA_BUS_0_IO_LBA_MID, (lba >> 8) & 0xFF);
    port_byte_out(ATA_BUS_0_IO_LBA_HIGH, (lba >> 16) & 0xFF);
    port_byte_out(ATA_BUS_0_IO_COMMAND, 0x30); // write sectors

    size_t o_len = 0;
    for (size_t s = 0; s < sect_count; s++) {
        // Read entire sector
        for (size_t i = 0; i < ATA_SECTOR_WORDS; i++) {
            // Wait for drive to be ready
            while (!(port_byte_in(ATA_BUS_0_CTL_ALT_STATUS) & ATA_STATUS_FLAG_DRQ)) {
                TEST_TIMEOUT
            }

            // write drive data
            uint16_t word = buff[i * 2] | (buff[i * 2 + 1] << 8);
            port_word_out(ATA_BUS_0_IO_DATA, word);
        }
        buff += ATA_SECTOR_BYTES;
    }

    port_byte_out(ATA_BUS_0_IO_COMMAND, 0xE7); // cache flush
    return sect_count;
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

static size_t disk_identify() {
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

    if (debug)
        puts("Polling");
    while (status & ATA_STATUS_FLAG_BSY) {
        if (debug)
            putc('.');
        status = port_byte_in(ATA_BUS_0_IO_STATUS);
        TEST_TIMEOUT
    }
    if (debug)
        putc('\n');

    if (port_byte_in(ATA_BUS_0_IO_LBA_MID) || port_byte_in(ATA_BUS_0_IO_LBA_HIGH)) {
        puts("Disk does not support ATA\n");
        return 0;
    }
    if (debug)
        puts("Drive is ATA\n");

    if (debug)
        puts("Polling");
    while (!(status & (ATA_STATUS_FLAG_DRQ | ATA_STATUS_FLAG_ERR))) {
        if (debug)
            putc('.');
        status = port_byte_in(ATA_BUS_0_IO_STATUS);
        TEST_TIMEOUT
    }
    if (debug)
        putc('\n');

    if (status & ATA_STATUS_FLAG_ERR) {
        puts("Disk initialized with errors\n");
        return 0;
    }

    if (status & ATA_STATUS_FLAG_DRQ) {
        if (debug)
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
        if (debug)
            printf("Drive has LBA48 Mode\n");
    }

    uint32_t size28 = data[61];
    size28 = size28 << 16;
    size28 |= data[60];

    if (debug)
        printf("LDA28 has %u sectors\n", size28);

    uint64_t size48 = data[100];
    size48 = (size48 << 16) | data[101];
    size48 = (size48 << 16) | data[102];
    size48 = (size48 << 16) | data[103];

    if (debug)
        printf("LDA48 has %u sectors\n", size48);

    uint32_t size = (size28 >> 10) * 7;
    if (debug)
        printf("Disk size is %u kb\n", size);
    return size;
}

static void software_reset() {
    START_TIMEOUT
    port_byte_out(ATA_BUS_0_CTL_CONTROL, ATA_CONTROL_FLAG_SRST);
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
        status = port_byte_in(ATA_BUS_0_CTL_ALT_STATUS);
        TEST_TIMEOUT_VOID
    }
}
