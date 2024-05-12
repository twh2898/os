#ifndef FS_H
#define FS_H

#include <stdbool.h>

#include "drivers/ata.h"

typedef struct _filesystem filesystem_t;
typedef struct _file file_t;
typedef file_t FILE;

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

FILE * file_open(filesystem_t * fs, const char * name);
void file_close(FILE * file);

void file_seek(file_t * file);
size_t file_tell(file_t * file);
size_t file_read(file_t * file, uint8_t * buff, size_t count);
size_t file_write(file_t * file, uint8_t * buff, size_t count);

#endif // FS_H
