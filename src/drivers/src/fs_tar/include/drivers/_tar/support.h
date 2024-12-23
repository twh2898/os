#ifndef DRIVER_FS_TAR_SUPPORT_H
#define DRIVER_FS_TAR_SUPPORT_H

#include <stdbool.h>
#include <stddef.h>

#include "driver.h"
#include "drivers/_tar/defs.h"

int                 drv_fs_tar_count_files(driver_disk_t * disk, drv_fs_tar_t * fs);
int                 drv_fs_tar_load_files(driver_disk_t * disk, drv_fs_tar_t * fs);
drv_fs_tar_file_t * drv_fs_tar_find_filename(drv_fs_tar_t * fs, const char * filename);

#endif // DRIVER_FS_TAR_SUPPORT_H
