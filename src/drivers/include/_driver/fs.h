#ifndef DRIVER_FS_H
#define DRIVER_FS_H

#include <stddef.h>
#include <stdint.h>

#include "_driver/disk.h"
#include "_driver/fs_file.h"

typedef enum DRIVER_FS_STATE {
    DRIVER_FS_STATE_IDLE,
    DRIVER_FS_STATE_BUSY,
} driver_fs_state_t;

typedef struct _driver_fs_stat {
    uint32_t             size;
    enum DRIVER_FS_STATE state;
    const char *         format;
} driver_fs_stat_t;

typedef struct _driver_fs {
    struct _driver_disk *  disk;
    struct _driver_fs_stat stat;
} driver_fs_t;

typedef driver_fs_t * (*driver_fs_fn_open)(driver_disk_t *);
typedef int (*driver_fs_fn_close)(driver_fs_t *);
typedef int (*driver_fs_fn_stat)(driver_fs_t *, driver_fs_stat_t *);

struct _driver_device_fs {
    driver_fs_fn_open  fn_open;
    driver_fs_fn_close fn_close;
    driver_fs_fn_stat  fn_stat;

    driver_fs_fn_file_open  fn_file_open;
    driver_fs_fn_file_close fn_file_close;
    driver_fs_fn_file_stat  fn_file_stat;
    driver_fs_fn_file_read  fn_file_read;
    driver_fs_fn_file_write fn_file_write;
};

#endif // DRIVER_FS_H
