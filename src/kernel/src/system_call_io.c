#include "kernel/system_call_io.h"

#include "io/file.h"
#include "libk/defs.h"

uint32_t sys_call_io_cb(uint16_t int_no, void * args_data, registers_t * regs) {
    uint32_t res = 0;

    switch (int_no) {
        case SYS_INT_IO_OPEN: {
            struct _args {
                const char * path;
                const char * mode;
            } args = *(struct _args *)args_data;

            res = 0;
        } break;

        case SYS_INT_IO_CLOSE: {
            struct _args {
                int handle;
            } args = *(struct _args *)args_data;
        } break;

        case SYS_INT_IO_READ: {
            struct _args {
                int    handle;
                char * buff;
                size_t count;
            } args = *(struct _args *)args_data;
        } break;

        case SYS_INT_IO_WRITE: {
            struct _args {
                int          handle;
                const char * buff;
                size_t       count;
            } args = *(struct _args *)args_data;
        } break;

        case SYS_INT_IO_SEEK: {
            struct _args {
                int handle;
                int pos;
                int seek;
            } args = *(struct _args *)args_data;
        } break;

        case SYS_INT_IO_TELL: {
            struct _args {
                int handle;
            } args = *(struct _args *)args_data;
        } break;
    }

    return res;
}
