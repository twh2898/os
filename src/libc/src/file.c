#include "libc/file.h"

file_t file_open(const char * path, const char * mode) {
    return _sys_io_file_open(path, mode);
}

void file_close(file_t fp) {
    _sys_io_file_close(fp);
}

int file_read(file_t fp, size_t size, size_t count, char * buff) {
    return _sys_io_file_read(fp, size, count, buff);
}

int file_write(file_t fp, size_t size, size_t count, const char * buff) {
    return _sys_io_file_write(fp, size, count, buff);
}

int file_seek(file_t fp, int offset, int origin) {
    return _sys_io_file_seek(fp, offset, origin);
}

int file_tell(file_t fp) {
    return _sys_io_file_tell(fp);
}
