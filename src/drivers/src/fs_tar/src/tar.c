#include "drivers/tar.h"

#include <stdbool.h>

#include "drivers/_tar/defs.h"
#include "drivers/_tar/support.h"
#include "libc/memory.h"
#include "libc/string.h"

static driver_register_t drv_register;

int drv_fs_tar_init() {
    drv_register.type        = DRIVER_DEVICE_TYPE_FS;
    drv_register.fs.format   = "tar";
    drv_register.fs.fn_open  = drv_fs_tar_open;
    drv_register.fs.fn_close = drv_fs_tar_close;

    drv_register.fs.fn_file_open  = drv_fs_tar_file_open;
    drv_register.fs.fn_file_close = drv_fs_tar_file_close;
    drv_register.fs.fn_file_stat  = drv_fs_tar_file_stat;
    drv_register.fs.fn_file_read  = drv_fs_tar_file_read;
    drv_register.fs.fn_file_write = drv_fs_tar_file_write;

    drv_register.fs.fn_file_open  = drv_fs_tar_file_open;
    drv_register.fs.fn_file_close = drv_fs_tar_file_close;
    drv_register.fs.fn_file_read  = drv_fs_tar_file_read;
    drv_register.fs.fn_file_write = drv_fs_tar_file_write;
    drv_register.fs.fn_file_seek  = drv_fs_tar_file_seek;
    drv_register.fs.fn_file_tell  = drv_fs_tar_file_tell;
    drv_register.fs.fn_file_stat  = drv_fs_tar_file_stat;

    drv_register.fs.fn_dir_open  = drv_fs_tar_dir_open;
    drv_register.fs.fn_dir_close = drv_fs_tar_dir_close;
    drv_register.fs.fn_dir_read  = drv_fs_tar_dir_read;
    drv_register.fs.fn_dir_seek  = drv_fs_tar_dir_seek;
    drv_register.fs.fn_dir_tell  = drv_fs_tar_dir_tell;

    int res = register_driver(&drv_register);
    return res;
}

driver_fs_t * drv_fs_tar_open(driver_disk_t * disk) {
    if (!disk) {
        return 0;
    }

    drv_fs_tar_t * tar = kcalloc(sizeof(drv_fs_tar_t), 0);
    if (!tar) {
        return 0;
    }

    if (drv_fs_tar_load_files(disk, tar) < 0) {
        if (tar->files) {
            kfree(tar->files);
        }
        kfree(tar);
        return 0;
    }

    driver_fs_t * fs = kmalloc(sizeof(driver_fs_t));
    if (!fs) {
        kfree(tar->files);
        kfree(tar);
        return 0;
    }

    fs->disk     = disk;
    fs->drv_data = tar;

    return fs;
}

int drv_fs_tar_close(driver_fs_t * fs) {
    if (!fs) {
        return -1;
    }

    drv_fs_tar_t * tar = (drv_fs_tar_t *)fs->drv_data;

    kfree(tar->files);
    kfree(tar);
    return 0;
}
