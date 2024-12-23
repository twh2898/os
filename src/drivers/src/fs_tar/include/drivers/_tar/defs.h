#ifndef DRIVER_FS_TAR_DEFS_H
#define DRIVER_FS_TAR_DEFS_H

#include <stddef.h>

#include "driver.h"

typedef struct _drv_fs_tar_raw_header {
    char filename[100];
    char mode[8];
    char uid[8];
    char gid[8];
    // NOTE all field are ascii numbers, size is base 8
    char size[12];
    char mtime[12];
    char checksum[8];
    char type[1];
    // There are 355 empty bytes to make header 512 long
    // uint8_t reserved[355];
} __attribute__((packed)) drv_fs_tar_raw_header_t;

typedef struct _drv_fs_tar_file {
    size_t index;
    size_t disk_pos;
    size_t cursor;

    struct _driver_fs_file_stat   stat;
    struct _drv_fs_tar_raw_header header;
} drv_fs_tar_file_t;

typedef struct _drv_fs_tar {
    size_t                 file_count;
    struct _driver_fs_stat stat;
    drv_fs_tar_file_t *    files;
    driver_disk_t *        disk;
} drv_fs_tar_t;

#endif // DRIVER_FS_TAR_DEFS_H
