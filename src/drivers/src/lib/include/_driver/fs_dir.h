#ifndef DRIVER_FS_DIR_H
#define DRIVER_FS_DIR_H

#include <stddef.h>

#include "_driver/fs.h"
#include "_driver/fs_file.h"

typedef void * DIR;

typedef DIR * (*driver_fs_fn_dir_open)(driver_fs_t *, const char * path);
typedef int (*driver_fs_fn_dir_close)(DIR * dir);
typedef const char * (*driver_fs_fn_dir_read)(DIR * dir);
typedef int (*driver_fs_fn_dir_seek)(DIR * dir, size_t pos, enum DRV_FS_FILE_SEEK_ORIGIN);
typedef int (*driver_fs_fn_dir_tell)(DIR * dir);

#endif // DRIVER_FS_DIR_H
