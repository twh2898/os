#ifndef LIBC_FILE_H
#define LIBC_FILE_H

#include <stddef.h>

typedef int file_t;

file_t file_open(const char * path, const char * mode);
void   file_close(file_t fp);
int    file_read(file_t fp, size_t size, size_t count, char * buff);
int    file_write(file_t fp, size_t size, size_t count, const char * buff);
int    file_seek(file_t fp, int offset, int origin);
int    file_tell(file_t fp);

#endif // LIBC_FILE_H
