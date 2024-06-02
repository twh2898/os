#include "libc/file.h"

#include "libc/mem.h"
#include "libc/string.h"

struct _file {
    size_t pos;
    size_t size;
};

file_t * file_open(const char * filename, const char * mode) {
    file_t * file = malloc(sizeof(file_t));
    if (file) {

    }
    return file;
}

void file_close(file_t * file) {
    free(file);
}

size_t file_tell(file_t * file) {
    return 0;
}

size_t file_seek(file_t * file, int offset, enum FILE_SEEK_ORIGIN origin) {
    return 0;
}

size_t file_read(file_t * file, const char * buff, size_t count) {
    return 0;
}

size_t file_write(file_t * file, const char * buff, size_t count) {
    return 0;
}
