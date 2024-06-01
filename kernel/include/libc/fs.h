#ifndef FS_H
#define FS_H

#include <stdbool.h>

#include "drivers/ata.h"

enum FS_NODE_TYPE {
    FS_NODE_TYPE_INVALID = 0,
    FS_NODE_TYPE_DNODE = 1,
    FS_NODE_TYPE_INODE = 2,
    FS_NODE_TYPE_DATA = 3,
};

typedef struct {
    enum FS_NODE_TYPE type;
} fs_node;

typedef struct _filesystem filesystem_t;

bool fs_format(ata_t * disk);

filesystem_t * fs_new(ata_t * disk);
// REMEMBER THIS DOES NOt CLOSE THE DISK!!!
void fs_free(filesystem_t * fs);

ata_t * fs_get_ata(filesystem_t * fs);
size_t fs_get_size(filesystem_t * fs);
size_t fs_get_used(filesystem_t * fs);
size_t fs_get_free(filesystem_t * fs);

void fs_create(filesystem_t * fs, const char * name);
void fs_remove(filesystem_t * fs, const char * name);

void fs_exists(filesystem_t * fs, const char * path);
void fs_is_dir(filesystem_t * fs, const char * path);
void fs_is_file(filesystem_t * fs, const char * path);

/*
Interaction with nodes

- Get node by path
- Get node type
- Get node data for type
- Read content of inode
- Write content to inode
*/

// probably replace with open DIR* like open FILE* and each read is a node in that dir
// void fs_list_dir(filesystem_t * fs, const char * path);

// TODO functions to interact with fs nodes and to read / write sectors

#endif // FS_H
