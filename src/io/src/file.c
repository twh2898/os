#include "io/file.h"

#include "libc/memory.h"
#include "libk/sys_call.h"

struct _io_file {
    int handle;
};

IO * open(const char * path, const char * mode) {
    int handle = _sys_io_open(path, mode);
    if (!handle) {
        return 0;
    }

    IO * io    = kmalloc(sizeof(IO));
    io->handle = handle;
    return io;
}

int close(IO * io) {
    if (!io) {
        return -1;
    }

    int res = _sys_io_close(io->handle);
    kfree(io);
    return res;
}

int read(IO * io, char * buff, size_t count) {
    if (!io) {
        return -1;
    }

    return _sys_io_read(io->handle, buff, count);
}

int write(IO * io, const char * buff, size_t count) {
    if (!io) {
        return -1;
    }

    return _sys_io_write(io->handle, buff, count);
}

int seek(IO * io, int pos, int seek) {
    if (!io) {
        return -1;
    }

    return _sys_io_seek(io->handle, pos, seek);
}

int tell(IO * io) {
    if (!io) {
        return -1;
    }

    return _sys_io_tell(io->handle);
}
