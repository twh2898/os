#include "drivers/_tar/support.h"

#include "libc/memory.h"
#include "libc/string.h"

static int parse_octal(const char * str) {
    if (!str) {
        return -1;
    }

    int value = 0;
    int count = 0;

    while (str[count]) {
        value *= 8;
        char c = str[count];
        if (c < '0' || c > '8') {
            return -1;
        }
        value += str[count] - '0';
        count++;
    }

    return count;
}

int drv_fs_tar_count_files(driver_disk_t * disk, drv_fs_tar_t * fs) {
    if (!disk || !fs) {
        return -1;
    }

    drv_fs_tar_raw_header_t header;

    size_t disk_pos = 0;
    int    count    = 0;

    for (;;) {
        size_t n_read = driver_disk_read(disk, (char *)&header, sizeof(drv_fs_tar_raw_header_t), disk_pos);

        if (!n_read) {
            break;
        }

        if (n_read != sizeof(drv_fs_tar_raw_header_t)) {
            return -1;
        }

        if (!header.filename[0]) {
            break;
        }

        size_t file_size = parse_octal(header.size);
        count++;

        disk_pos += 512 + file_size;
        if (disk_pos % 512) {
            disk_pos += (512 - (disk_pos % 512));
        }
    }

    return count;
}

int drv_fs_tar_load_files(driver_disk_t * disk, drv_fs_tar_t * fs) {
    if (!disk || !fs) {
        return -1;
    }

    size_t disk_pos = 0;

    fs->files = kmalloc(sizeof(drv_fs_tar_file_t) * fs->file_count);

    for (size_t i = 0; i < fs->file_count; i++) {
        drv_fs_tar_file_t * file = &fs->files[i];

        size_t n_read = driver_disk_read(disk, (char *)&file->header, sizeof(drv_fs_tar_raw_header_t), disk_pos);

        if (n_read != sizeof(drv_fs_tar_raw_header_t)) {
            return -1;
        }

        size_t file_size = parse_octal(file->header.size);
        file->index      = i;
        file->disk_pos   = disk_pos * 512;
        // TODO fill stat

        disk_pos += 512 + file_size;
        if (disk_pos % 512) {
            disk_pos += (512 - (disk_pos % 512));
        }
    }

    return 0;
}

drv_fs_tar_file_t * drv_fs_tar_find_filename(drv_fs_tar_t * fs, const char * filename) {
    if (!fs || !filename || !kstrlen(filename)) {
        return 0;
    }

    if (filename[0] == '/') {
        filename++;
    }

    for (size_t i = 0; i < fs->file_count; i++) {
        size_t size          = knstrlen(filename, 100);
        size_t filename_size = knstrlen(fs->files[i].header.filename, 100);

        if (filename_size != size) {
            continue;
        }

        if (kmemcmp(filename, fs->files[i].header.filename, size) == 0) {
            return &fs->files[i];
        }
    }

    return 0;
}
