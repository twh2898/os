#include "libc/dir.h"

#include "libc/mem.h"

struct _dir {
    size_t pos;
};

dir_t * dir_open(const char * dirname, const char * mode) {
    if (!dirname || !mode)
        return 0;
    // TODO check if dirname exists

    dir_t * dir = malloc(sizeof(dir_t));
    if (dir) {
    }
    return dir;
}
void dir_close(dir_t * dir) {
    free(dir);
}

dir_entry_t * dir_read(dir_t * dir) {
    if (!dir)
        return 0;

    dir_entry_t * entry = malloc(sizeof(dir_entry_t));
    if (entry) {
    }
    return entry;
}
