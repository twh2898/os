#include "libc/dir.h"

#include "libc/mem.h"

struct _dir {
    size_t pos;
};

dir_t * dir_open(const char * dirname, const char * mode) {
    dir_t * dir = malloc(sizeof(dir_t));
    if (dir) {

    }
    return dir;
}
void dir_close(dir_t * dir) {
    free(dir);
}

dir_entry_t * dir_read(dir_t * dir) {
    return 0;
}
