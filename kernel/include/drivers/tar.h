#ifndef TAR_H
#define TAR_H

#include <stddef.h>

#include "disk.h"

typedef struct tar_fs tar_fs_t;

tar_fs_t * tar_open(disk_t * disk);
// REMEMBER THIS DOES NOT CLOSE THE DISK
void tar_close(tar_fs_t * tar);

size_t tar_file_count(tar_fs_t * tar);
const char * tar_file_name(tar_fs_t * tar, size_t i);
size_t tar_file_size(tar_fs_t * tar, size_t i);

void tar_stat_file(tar_fs_t * tar, size_t i);

#endif // TAR_H
