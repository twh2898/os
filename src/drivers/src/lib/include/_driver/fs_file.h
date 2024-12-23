#ifndef DRIVER_FS_FILE_H
#define DRIVER_FS_FILE_H

#include <stddef.h>

#include "_driver/fs.h"

enum DRV_FS_FILE_SEEK_ORIGIN {
    DRV_FS_FILE_SEEK_ORIGIN_START,
    DRV_FS_FILE_SEEK_ORIGIN_CURRENT,
    DRV_FS_FILE_SEEK_ORIGIN_END,
};

enum DRV_FS_FILE_TYPE {
    DRV_FS_FILE_TYPE_HARD_LINK  = 0,
    DRV_FS_FILE_TYPE_SYM_LINK   = 1,
    DRV_FS_FILE_TYPE_CHAR_DEV   = 2,
    DRV_FS_FILE_TYPE_BLOCK_DEV  = 3,
    DRV_FS_FILE_TYPE_DIRECTORY  = 4,
    DRV_FS_FILE_TYPE_NAMED_PIPE = 5, // FIFO
};

typedef struct _driver_fs_file_stat {
    const char *          path;
    size_t                size;
    int                   mode;
    int                   uid;
    int                   gid;
    int                   ctime;
    int                   mtime;
    int                   atime;
    enum DRV_FS_FILE_TYPE type;
} driver_fs_file_stat_t;

typedef struct _driver_fs_file {
    struct _driver_fs *         fs;
    struct _driver_fs_file_stat stat;
    void *                      drv_data;
} driver_fs_file_t;

typedef driver_fs_file_t * (*driver_fs_fn_file_open)(driver_fs_t *, const char * path, const char * mode);
typedef int (*driver_fs_fn_file_close)(driver_fs_file_t * file);
typedef int (*driver_fs_fn_file_read)(driver_fs_file_t * file, char * buff, size_t count);
typedef int (*driver_fs_fn_file_write)(driver_fs_file_t * file, const char * buff, size_t count);
typedef int (*driver_fs_fn_file_seek)(driver_fs_file_t * file, size_t pos, enum DRV_FS_FILE_SEEK_ORIGIN);
typedef int (*driver_fs_fn_file_tell)(driver_fs_file_t * file);
typedef int (*driver_fs_fn_file_stat)(driver_fs_t *, const char * path, driver_fs_file_stat_t *);

#endif // DRIVER_FS_FILE_H
