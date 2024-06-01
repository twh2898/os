#include "drivers/tar.h"

#include <stdint.h>

#include "kernel.h"
#include "libc/mem.h"
#include "libc/stdio.h"
#include "libc/string.h"

typedef struct {
    char filename[100];
    char mode[8];
    char uid[8];
    char gid[8];
    // NOTE all field are ascii numbers, size is base 8
    char size[12];
    char mtime[12];
    char chksum[8];
    char typeflag[1];
    // There are 355 empty bytes to make header 512 long
    // uint8_t reserved[355];
} __attribute__((packed)) raw_header_t;

typedef struct {
    char filename[101];
    size_t size;
    size_t index;
    size_t disk_pos;

    raw_header_t header;
} tar_file_t;

struct tar_fs {
    disk_t * disk;
    size_t file_count;
    tar_file_t * files;
} __attribute__((packed));

static size_t parse_size(raw_header_t * header);
static size_t count_files(tar_fs_t * tar);
static void load_headers(tar_fs_t * tar);

tar_fs_t * tar_open(disk_t * disk) {
    tar_fs_t * tar = malloc(sizeof(tar_fs_t));
    if (tar) {
        tar->disk = disk;
        tar->file_count = count_files(tar);
        tar->files = malloc(sizeof(tar_file_t) * tar->file_count);
        load_headers(tar);
    }
    return tar;
}

void tar_close(tar_fs_t * tar) {
    free(tar);
}

size_t tar_file_count(tar_fs_t * tar) {
    return tar->file_count;
}

const char * tar_file_name(tar_fs_t * tar, size_t i) {
    if (i > tar->file_count)
        return 0;

    return tar->files[i].filename;
}

size_t tar_file_size(tar_fs_t * tar, size_t i) {
    if (i > tar->file_count)
        return 0;

    return tar->files[i].size;
}

static size_t parse_size(raw_header_t * header) {
    size_t size = 0;
    size_t count = 1;

    for (size_t j = 11; j > 0; j--, count *= 8) {
        size += ((header->size[j - 1] - '0') * count);
    }

    return size;
}

static size_t count_files(tar_fs_t * tar) {
    raw_header_t header;

    size_t disk_pos = 0;
    size_t count = 0;
    for (;;) {
        size_t n_read = disk_read(
            tar->disk, (uint8_t *)&header, sizeof(raw_header_t), disk_pos);

        if (!n_read)
            break;

        if (n_read != sizeof(raw_header_t)) {
            if (debug)
                kprintf("Read only got %u bytes\n", n_read);
            KERNEL_PANIC("Failed to read full tar header from disk");
        }

        if (header.filename[0] == 0)
            break;

        size_t file_size = parse_size(&header);
        count++;

        // add 512 for header size
        disk_pos += 512 + file_size;
        if (disk_pos % 512)
            disk_pos += (512 - (disk_pos % 512));
    }

    return count;
}

static void load_headers(tar_fs_t * tar) {
    size_t disk_pos = 0;

    for (size_t i = 0; i < tar->file_count; i++) {
        tar_file_t * file = &tar->files[i];

        size_t n_read = disk_read(
            tar->disk, (uint8_t *)&file->header, sizeof(raw_header_t), disk_pos);

        if (!n_read)
            break;

        if (n_read != sizeof(raw_header_t)) {
            if (debug)
                kprintf("Read only got %u bytes\n", n_read);
            KERNEL_PANIC("Failed to read full tar header from disk");
        }

        size_t file_size = file->size = parse_size(&file->header);

        size_t name_len = nstrlen(file->header.filename, 100);
        memcpy(file->filename, file->header.filename, name_len);
        file->filename[name_len] = 0;

        file->index = i;
        file->disk_pos = disk_pos + 512;

        // add 512 for header size
        disk_pos += 512 + file_size;
        if (disk_pos % 512)
            disk_pos += (512 - (disk_pos % 512));
    }
}
