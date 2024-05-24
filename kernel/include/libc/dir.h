#ifndef DIR_H
#define DIR_H

#include "libc/fs.h"

typedef struct _fs_dir fs_dir_t;

enum DIR_ENTRY_TYPE {
    DIR_ENTRY_TYPE_DIR = 0,
    DIR_ENTRY_TYPE_FILE = 1,
};

typedef struct {
    const char * name;
    enum DIR_ENTRY_TYPE type;
} dir_entry_t;

fs_dir_t dir_open(filesystem_t * fs, const char * name);
void dir_close(fs_dir_t * dir);

dir_entry_t * dir_read(fs_dir_t * dir);

#endif // DIR_H
