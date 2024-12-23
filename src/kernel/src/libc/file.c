#include "libc/file.h"

#include "memory.h"
#include "libc/string.h"

struct _file {
    size_t pos;
    size_t size;
};

file_t * file_open(const char * filename, const char * mode) {
    if (!filename)
        return 0;
    // TODO check if filename exists

    file_t * file = impl_kmalloc(sizeof(file_t));
    if (file) {
    }
    return file;
}

void file_close(file_t * file) {
    impl_kfree(file);
}

bool file_seek(file_t * file, int offset, enum FILE_SEEK_ORIGIN origin) {
    if (!file)
        return false;

    return false;
}

int file_tell(file_t * file) {
    if (!file)
        return -1;

    return 0;
}

size_t file_read(file_t * file, const char * buff, size_t count) {
    if (!file || !buff)
        return 0;

    return 0;
}

size_t file_write(file_t * file, const char * buff, size_t count) {
    if (!file || !buff)
        return 0;

    return 0;
}
