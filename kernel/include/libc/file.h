#ifndef FILE_H
#define FILE_H

#include "libc/fs.h"

typedef struct _fs_file fs_file_t;

fs_file_t * file_open(filesystem_t * fs, const char * name);
void file_close(fs_file_t * file);

void file_seek(fs_file_t * file);
size_t file_tell(fs_file_t * file);
size_t file_read(fs_file_t * file, uint8_t * buff, size_t count);
size_t file_write(fs_file_t * file, uint8_t * buff, size_t count);

#endif // FILE_H
