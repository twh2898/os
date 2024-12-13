#ifndef DIR_H
#define DIR_H

#include <stddef.h>

typedef struct _dir dir_t;

enum DIR_ENTRY_TYPE {
    DIR_ENTRY_TYPE_DIR  = 0,
    DIR_ENTRY_TYPE_FILE = 1,
};

typedef struct {
    const char *        name;
    enum DIR_ENTRY_TYPE type;
} dir_entry_t;

dir_t * dir_open(const char * dirname, const char * mode);
void    dir_close(dir_t * dir);

dir_entry_t * dir_read(dir_t * dir);

#endif // DIR_H
