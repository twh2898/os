#ifndef DRIVER_FS_FILE_H
#define DRIVER_FS_FILE_H

#include <stddef.h>

#include "_driver/fs.h"

typedef struct _driver_fs_file_stat {
    const char * path;
    size_t       size;
} driver_fs_file_stat_t;

typedef struct _driver_fs_file {
    struct _driver_fs *         fs;
    struct _driver_fs_file_stat stat;
} driver_fs_file_t;

typedef driver_fs_file_t * (*driver_fs_fn_file_open)(struct _driver_fs *, const char * path);
typedef int (*driver_fs_fn_file_close)(driver_fs_file_t *, driver_fs_file_t *);
typedef int (*driver_fs_fn_file_stat)(driver_fs_file_t *, driver_fs_file_stat_t *);
typedef int (*driver_fs_fn_file_read)(driver_fs_file_t *, char * buff, size_t count, size_t addr);
typedef int (*driver_fs_fn_file_write)(driver_fs_file_t *, const char * buff, size_t count, size_t addr);

#endif // DRIVER_FS_FILE_H
