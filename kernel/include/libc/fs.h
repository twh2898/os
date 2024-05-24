#ifndef FS_H
#define FS_H

#include <stdbool.h>

#include "drivers/ata.h"

typedef struct _filesystem filesystem_t;

bool fs_format(disk_t * disk);

filesystem_t * fs_new(disk_t * disk);
// REMEMBER THIS DOES NOt CLOSE THE DISK!!!
void fs_free(filesystem_t * fs);

disk_t * fs_get_disk(filesystem_t * fs);
size_t fs_get_size(filesystem_t * fs);
size_t fs_get_used(filesystem_t * fs);
size_t fs_get_free(filesystem_t * fs);

void fs_create(filesystem_t * fs, const char * name);
void fs_remove(filesystem_t * fs, const char * name);

void fs_exists(filesystem_t * fs, const char * path);
void fs_is_dir(filesystem_t * fs, const char * path);
void fs_is_file(filesystem_t * fs, const char * path);

// probably replace with open DIR* like open FILE* and each read is a node in that dir
// void fs_list_dir(filesystem_t * fs, const char * path);

// TODO functions to interact with fs nodes and to read / write sectors

#endif // FS_H
