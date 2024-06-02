#ifndef FILE_H
#define FILE_H

#include <stdbool.h>
#include <stddef.h>

typedef struct _file file_t;

enum FILE_SEEK_ORIGIN {
    FILE_SEEK_ORIGIN_START,
    FILE_SEEK_ORIGIN_CURRENT,
    FILE_SEEK_ORIGIN_END,
};

file_t * file_open(const char * filename, const char * mode);
void file_close(file_t * file);

bool file_seek(file_t * file, int offset, enum FILE_SEEK_ORIGIN origin);
int file_tell(file_t * file);

size_t file_read(file_t * file, const char * buff, size_t count);
size_t file_write(file_t * file, const char * buff, size_t count);

#endif // FILE_H
