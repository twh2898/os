#include "drivers/_tar/defs.h"
#include "drivers/_tar/support.h"
#include "drivers/tar.h"
#include "libc/memory.h"
#include "libc/string.h"

driver_fs_file_t * drv_fs_tar_file_open(driver_fs_t * fs, const char * path, const char * mode) {
    if (!fs || !path || !mode) {
        return 0;
    }

    drv_fs_tar_t * tar = fs->drv_data;

    drv_fs_tar_file_t * file = drv_fs_tar_find_filename(tar, path);
    if (!file) {
        return 0;
    }

    driver_fs_file_t * fs_file = kmalloc(sizeof(driver_fs_file_t));
    if (!fs_file) {
        return 0;
    }

    fs_file->fs       = fs;
    fs_file->drv_data = file;
    file->cursor      = 0;

    return fs_file;
}

int drv_fs_tar_file_close(driver_fs_file_t * file) {
    return 0;
}

int drv_fs_tar_file_read(driver_fs_file_t * file, char * buff, size_t count) {
    if (!file) {
        return -1;
    }

    drv_fs_tar_file_t * f = (drv_fs_tar_file_t *)file;

    if (f->cursor + count > f->stat.size) {
        count = f->stat.size - f->cursor;
    }

    if (count == 0) {
        return 0;
    }

    int n_read = driver_disk_read(file->fs->disk, buff, count, f->disk_pos + f->cursor);
    if (n_read == count) {
        f->cursor += count;
        return count;
    }

    return count;
}

int drv_fs_tar_file_write(driver_fs_file_t * file, const char * buff, size_t count) {
    if (!file) {
        return -1;
    }

    drv_fs_tar_file_t * f = (drv_fs_tar_file_t *)file;

    if (f->cursor + count > f->stat.size) {
        count = f->stat.size - f->cursor;
    }

    if (count == 0) {
        return 0;
    }

    int n_read = driver_disk_write(file->fs->disk, buff, count, f->disk_pos + f->cursor);
    if (n_read == count) {
        f->cursor += count;
        return count;
    }

    return count;
}

int drv_fs_tar_file_seek(driver_fs_file_t * file, size_t pos, enum DRV_FS_FILE_SEEK_ORIGIN origin) {
    if (!file) {
        return -1;
    }

    drv_fs_tar_file_t * f = (drv_fs_tar_file_t *)file;

    switch (origin) {
        default:
            return -1;
        DRV_FS_FILE_SEEK_ORIGIN_START: {
            if (pos >= f->stat.size) {
                pos = f->stat.size - 1;
            }
            f->cursor = pos;
        } break;
        DRV_FS_FILE_SEEK_ORIGIN_CURRENT: {
            if (f->cursor + pos > f->stat.size) {
                f->cursor = f->stat.size - 1;
            }
            else {
                f->cursor += pos;
            }
        } break;
        DRV_FS_FILE_SEEK_ORIGIN_END: {
            if (pos >= f->stat.size) {
                pos = f->stat.size - 1;
            }
            f->cursor = f->stat.size - pos - 1;
        } break;
    };

    return 0;
}

int drv_fs_tar_file_tell(driver_fs_file_t * file) {
    if (!file) {
        return -1;
    }

    drv_fs_tar_file_t * f = (drv_fs_tar_file_t *)file;
    return f->cursor;
}

int drv_fs_tar_file_stat(driver_fs_t * fs, const char * path, driver_fs_file_stat_t * stat) {
    if (!fs || !path || !stat) {
        return -1;
    }

    drv_fs_tar_t * tar = fs->drv_data;

    drv_fs_tar_file_t * file = drv_fs_tar_find_filename(tar, path);
    if (!file) {
        return -1;
    }

    if (!kmemcpy(stat, &file->stat, sizeof(driver_fs_file_stat_t))) {
        return -1;
    }

    return 0;
}
