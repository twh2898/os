#ifndef LIBK_DEFS_H
#define LIBK_DEFS_H

#define SYS_INT_FAMILY_IO    0x01
#define SYS_INT_FAMILY_MEM   0x02
#define SYS_INT_FAMILY_PROC  0x03
#define SYS_INT_FAMILY_STDIO 0x10

#define SYS_INT_IO_OPEN  0x0100
#define SYS_INT_IO_CLOSE 0x0101
#define SYS_INT_IO_READ  0x0102
#define SYS_INT_IO_WRITE 0x0103

#define SYS_INT_MEM_PAGE_ALLOC 0x0203

#define SYS_INT_PROC_EXIT    0x0300
#define SYS_INT_PROC_ABORT   0x0301
#define SYS_INT_PROC_PANIC   0x0302
#define SYS_INT_PROC_REG_SIG 0x0303

#define SYS_INT_STDIO_PUTC 0x1000
#define SYS_INT_STDIO_PUTS 0x1001

#endif // LIBK_DEFS_H
