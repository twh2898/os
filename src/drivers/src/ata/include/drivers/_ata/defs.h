#ifndef DRIVER_ATA_DEFS_H
#define DRIVER_ATA_DEFS_H

#include <stdint.h>

#define MAX_RETRY 5000

#define ATA_SECTOR_WORDS 256
#define ATA_SECTOR_BYTES (ATA_SECTOR_WORDS * 2)

#define ATA_MAX_SECTOR_READ_WRITE 256

#define ATA_BUFFER_SIZE (ATA_SECTOR_BYTES * ATA_MAX_SECTOR_READ_WRITE)

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

#endif // DRIVER_ATA_DEFS_H
