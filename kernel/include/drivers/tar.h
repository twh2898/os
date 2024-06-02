#ifndef TAR_H
#define TAR_H

#include <stddef.h>

#include "disk.h"

typedef struct tar_fs tar_fs_t;

enum TAR_MODE {
    TAR_MODE_EXECUTE = 1,
    TAR_MODE_WRITE = 2,
    TAR_MODE_READ = 4,
};

enum TAR_PEM {
    TAR_PEM_OTHER_EXECUTE = TAR_MODE_EXECUTE,
    TAR_PEM_OTHER_WRITE = TAR_MODE_WRITE,
    TAR_PEM_OTHER_READ = TAR_MODE_READ,
    
    TAR_PEM_GROUP_EXECUTE = TAR_MODE_EXECUTE << 3,
    TAR_PEM_GROUP_WRITE = TAR_MODE_WRITE << 3,
    TAR_PEM_GROUP_READ = TAR_MODE_READ << 3,

    TAR_PEM_USER_EXECUTE = TAR_MODE_EXECUTE << 6,
    TAR_PEM_USER_WRITE = TAR_MODE_WRITE << 6,
    TAR_PEM_USER_READ = TAR_MODE_READ << 6,

    TAR_PEM_SET_GID = 0x400,
    TAR_PEM_SET_UID = 0x800,
};

enum TAR_TYPE {
    TAR_TYPE_FILE = 0,
    TAR_TYPE_HARD_LINK,
    TAR_TYPE_SYM_LINK,
    TAR_TYPE_CHAR_DEV,
    TAR_TYPE_BLOCK_DEV,
    TAR_TYPE_DIR,
    TAR_TYPE_FIFO,
};

typedef struct {
    char filename[101];
    uint16_t mode;
    uint16_t uid;
    uint16_t gid;
    size_t size;
    uint32_t mtime;
    uint8_t type;
} tar_stat_t;

tar_fs_t * tar_open(disk_t * disk);
// REMEMBER THIS DOES NOT CLOSE THE DISK
void tar_close(tar_fs_t * tar);

size_t tar_file_count(tar_fs_t * tar);
const char * tar_file_name(tar_fs_t * tar, size_t i);
size_t tar_file_size(tar_fs_t * tar, size_t i);

tar_stat_t * tar_stat_file(tar_fs_t * tar, size_t i, tar_stat_t * stat);

// TODO list directories

// TODO file io

#endif // TAR_H
