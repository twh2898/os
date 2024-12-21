#include "commands.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "cpu/ports.h"
#include "cpu/ram.h"
#include "debug.h"
#include "drivers/disk.h"
#include "drivers/rtc.h"
#include "drivers/tar.h"
#include "drivers/timer.h"
#include "drivers/vga.h"
#include "exec.h"
#include "libc/memory.h"
#include "libc/stdio.h"
#include "libc/string.h"
#include "term.h"

bool debug = false;

static disk_t *   disk = 0;
static tar_fs_t * tar  = 0;

static int clear_cmd(size_t argc, char ** argv) {
    vga_clear();
    return 0;
}

static int echo_cmd(size_t argc, char ** argv) {
    bool next_line = true;
    if (argc > 1 && kmemcmp(argv[1], "-n", 2) == 0)
        next_line = false;

    size_t i = 1;
    if (!next_line)
        i++;
    for (; i < argc; i++) {
        puts(argv[i]);
        if (i < argc)
            putc(' ');
    }

    if (next_line)
        putc('\n');

    return 0;
}

static int debug_cmd(size_t argc, char ** argv) {
    debug = !debug;
    if (debug)
        puts("Enable debug\n");
    else
        puts("Disabling debug\n");
    return 0;
}

static int atoi_cmd(size_t argc, char ** argv) {
    if (argc != 2) {
        printf("Usage: %s <number>\n", argv[0]);
        return 1;
    }

    int i = katoi(argv[1]);
    printf("%d\n", i);
    return 0;
}

static uint8_t hex_digit(char c) {
    if (c >= '0' && c <= '9')
        return c - '0';
    else if (c >= 'a' && c <= 'f')
        return 10 + c - 'a';
    else if (c >= 'A' && c <= 'F')
        return 10 + c - 'A';
    else
        return 0;
}

static uint16_t parse_byte(const char * str) {
    size_t len = kstrlen(str);
    if (len < 1 || len > 4)
        return 1;

    uint16_t res = 0;
    while (*str) {
        res = (res << 4) | hex_digit(*str++);
    }
    return res;
}

static int port_out_cmd(size_t argc, char ** argv) {
    if (argc != 3) {
        printf("Usage: %s <port> <byte>\n", argv[0]);
        return 1;
    }

    if (kstrlen(argv[1]) > 4) {
        puts("port has max 4 hex digits\n");
        return 1;
    }
    uint16_t port = parse_byte(argv[1]);

    if (kstrlen(argv[2]) > 2) {
        puts("byte has max 2 hex digits\n");
        return 1;
    }
    uint8_t byte = parse_byte(argv[2]);

    port_byte_out(port, byte);
    return 0;
}

static int port_in_cmd(size_t argc, char ** argv) {
    if (argc != 2) {
        printf("Usage: %s <port>\n", argv[0]);
        return 1;
    }

    if (kstrlen(argv[1]) > 4) {
        puts("port has max 4 hex digits\n");
        return 1;
    }
    uint16_t port = parse_byte(argv[1]);

    uint8_t byte = port_byte_in(port);
    printf("0x%04X = 0x%02X\n", port, byte);

    return 0;
}

static int time_cmd(size_t argc, char ** argv) {
    uint32_t ms = get_ticks();
    printf("System ticks: %u ~= %u s\n", ms, ms / 1000);
    printf("RTC time: %u us = %u ms = %u s\n", time_us(), time_ms(), time_s());
    return 0;
}

static int ret_cmd(size_t argc, char ** argv) {
    printf("Last command exit code was %u\n", term_last_ret);
    return 0;
}

// static int format_cmd(size_t argc, char ** argv) {
//     if (fs) {
//         puts("Unmount disk before format\n");
//         return 1;
//     }

//     if (!disk) {
//         disk = disk_open(0, DISK_DRIVER_ATA);
//         if (!disk) {
//             puts("Failed to open disk\n");
//             return 1;
//         }
//     }

//     puts("Formatting disk, this may take some time\n");
//     // fs_format(disk);
//     puts("Done!\n");

//     return 0;
// }

static int mount_cmd(size_t argc, char ** argv) {
    if (tar) {
        puts("Filesystem already mounted\n");
        return 0;
    }

    if (!disk) {
        disk = disk_open(0, DISK_DRIVER_ATA);
        if (!disk) {
            puts("Failed to open disk\n");
            return 1;
        }
    }

    tar = tar_open(disk);
    if (!tar) {
        puts("Failed to mount filesystem\n");
        return 1;
    }

    return 0;
}

static int unmount_cmd(size_t argc, char ** argv) {
    if (tar) {
        tar_close(tar);
        tar = 0;
    }
    if (disk) {
        disk_close(disk);
        disk = 0;
    }
    return 0;
}

static void print_64(uint64_t v) {
    uint32_t u = v >> 32;
    uint32_t l = v & 0xffffffff;
    printf("0x%08X%08X", u, l);
}

static void print_upper(uint64_t start, uint64_t end, uint64_t size, enum RAM_TYPE type) {
    puts("| ");
    print_64(start);
    puts(" | ");
    print_64(end);
    puts(" | ");
    print_64(size);
    puts(" | ");
    switch (type) {
        case 1:
            puts("Usable RAM");
            break;
        case 2:
            puts("Reserved");
            break;
        case 3:
            puts("ACPI Reclaimable");
            break;
        case 4:
            puts("ACPI NVS");
            break;
        case 5:
            puts("BAD MEMORY");
            break;
    }
    putc('\n');
}

static int mem_cmd(size_t argc, char ** argv) {
    uint16_t low_mem = ram_lower_size();
    printf("Lower memory is %u k\n", low_mem);

    uint16_t count = ram_upper_count();

    printf("Total of %u blocks\n", count);

    puts("| Start              | End                | Size               | Type\n");
    for (size_t i = 0; i < count; i++) {
        print_upper(ram_upper_start(i), ram_upper_end(i), ram_upper_size(i), ram_upper_type(i));
    }
    return 0;
}

static void ls_print_file_size(size_t size) {
    size_t order = 0;
    while (size > 1024) {
        size /= 1024;
        order++;
    }
    char order_c = ' ';
    switch (order) {
        case 1:
            order_c = 'K';
            break;
        case 2:
            order_c = 'M';
            break;
        case 3:
            order_c = 'G';
            break;
        default:
            break;
    }
    printf("%4u%c", size, order_c);
}

static void ls_print_file(tar_stat_t * stat) {
    putc(stat->type == TAR_TYPE_DIR ? 'd' : '-');

    putc(stat->mode & TAR_PEM_USER_READ ? 'r' : '-');
    putc(stat->mode & TAR_PEM_USER_WRITE ? 'w' : '-');
    putc(stat->mode & TAR_PEM_USER_EXECUTE ? 'x' : '-');

    putc(stat->mode & TAR_PEM_GROUP_READ ? 'r' : '-');
    putc(stat->mode & TAR_PEM_GROUP_WRITE ? 'w' : '-');
    putc(stat->mode & TAR_PEM_GROUP_EXECUTE ? 'x' : '-');

    putc(stat->mode & TAR_PEM_OTHER_READ ? 'r' : '-');
    putc(stat->mode & TAR_PEM_OTHER_WRITE ? 'w' : '-');
    putc(stat->mode & TAR_PEM_OTHER_EXECUTE ? 'x' : '-');

    printf(" %4u %4u ", stat->uid, stat->gid);
    ls_print_file_size(stat->size);

    printf(" %4u ", stat->mtime);
    puts(stat->filename);
    putc('\n');
}

static int ls_cmd(size_t argc, char ** argv) {
    if (!tar) {
        puts("Filesystem not mounted\n");
        return 1;
    }

    size_t file_count = tar_file_count(tar);
    printf("Found %u files\n", file_count);

    tar_stat_t stat;
    for (size_t i = 0; i < file_count; i++) {
        if (!tar_stat_file_i(tar, i, &stat))
            return 1;
        ls_print_file(&stat);
    }

    putc('\n');
    return 0;
}

static int stat_cmd(size_t argc, char ** argv) {
    if (!tar) {
        puts("Filesystem not mounted\n");
        return 1;
    }

    tar_stat_t stat;
    if (!tar_stat_file_i(tar, 0, &stat)) {
        puts("Failed to stat file\n");
        return 1;
    }

    printf("File %s\n", stat.filename);
    printf("Size %u\n", stat.size);
    printf("%04x (%u : %u)\n", stat.mode, stat.uid, stat.gid);
    printf("mtime %04x\n", stat.mtime);
    printf("Type %02x\n", stat.type);

    return 0;
}

static int fs_read_cmd(size_t argc, char ** argv) {
    if (argc != 2) {
        printf("Usage: %s <filename>\n", argv[0]);
        return 1;
    }

    char * filename = argv[1];

    if (!tar) {
        puts("Filesystem not mounted\n");
        return 1;
    }

    tar_stat_t stat;
    if (!tar_stat_file(tar, filename, &stat)) {
        puts("Failed to find file\n");
        return 1;
    }

    ls_print_file(&stat);
    uint8_t * buff = kmalloc(stat.size);
    if (!buff)
        return 1;

    tar_fs_file_t * file = tar_file_open(tar, filename);
    if (!file) {
        kfree(buff);
        return 1;
    }

    if (!tar_file_read(file, buff, stat.size)) {
        tar_file_close(file);
        kfree(buff);
        return 1;
    }
    print_hexblock(buff, stat.size, 0);

    if (!disk || !buff)
        return 0;

    tar_file_close(file);
    kfree(buff);

    return 0;
}

static int fs_cat_cmd(size_t argc, char ** argv) {
    if (argc != 2) {
        printf("Usage: %s <filename>\n", argv[0]);
        return 1;
    }

    char * filename = argv[1];

    if (!tar) {
        puts("Filesystem not mounted\n");
        return 1;
    }

    tar_stat_t stat;
    if (!tar_stat_file(tar, filename, &stat)) {
        puts("Failed to find file\n");
        return 1;
    }

    uint8_t * buff = kmalloc(stat.size);
    if (!buff)
        return 1;

    tar_fs_file_t * file = tar_file_open(tar, filename);
    if (!file) {
        kfree(buff);
        return 1;
    }

    if (!tar_file_read(file, buff, stat.size)) {
        tar_file_close(file);
        kfree(buff);
        return 1;
    }
    for (size_t i = 0; i < stat.size; i++) {
        putc(buff[i]);
    }

    if (!disk || !buff)
        return 0;

    tar_file_close(file);
    kfree(buff);

    return 0;
}

// static int status_cmd(size_t argc, char ** argv) {
//     // drv_ata_status();
//     return 0;
// }

static int disk_read_cmd(size_t argc, char ** argv) {
    if (argc != 3) {
        printf("Usage: %s <pos> <count>\n", argv[0]);
        return 1;
    }

    size_t pos   = katoi(argv[1]);
    size_t count = katoi(argv[2]);

    if (!disk) {
        disk = disk_open(0, DISK_DRIVER_ATA);
        if (!disk) {
            puts("Failed to open disk\n");
            return 1;
        }
    }

    char   data[513];
    size_t steps = count / 512;
    if (count % 512)
        steps++;
    for (size_t i = 0; i < steps; i++) {
        size_t to_read = count;
        if (to_read > 512)
            to_read = 512;
        size_t read = disk_read(disk, data, to_read, pos);
        data[read]  = 0;
        print_hexblock(data, read, 512 * i);
        count -= to_read;
        pos += to_read;
    }
    putc('\n');
    return 0;
}

static int disk_write_cmd(size_t argc, char ** argv) {
    if (!disk) {
        disk = disk_open(0, DISK_DRIVER_ATA);
        if (!disk) {
            puts("Failed to open disk\n");
            return 1;
        }
    }

    char data[512] = {0};
    for (size_t i = 0; i < 512; i++) {
        if ((i >> 4) < 10)
            data[i * 2] = (i >> 4) + '0';
        else
            data[i * 2] = (i >> 4) + 'a' - 10;

        if ((i & 0xf) < 10)
            data[i * 2 + 1] = (i & 0xf) + '0';
        else
            data[i * 2 + 1] = (i & 0xf) + 'a' - 10;
    }
    disk_write(disk, data, 512, 0);
    return 0;
}

static int disk_size_cmd(size_t argc, char ** argv) {
    if (!disk) {
        disk = disk_open(0, DISK_DRIVER_ATA);
        if (!disk) {
            puts("Failed to open disk\n");
            return 1;
        }
    }
    size_t size = disk_size(disk);
    printf("Disk size %u\n", size);
    return 0;
}

static int command_lookup(size_t argc, char ** argv) {
    char * filename = argv[0];

    if (!tar) {
        puts("Filesystem not mounted\n");
        return 1;
    }

    tar_stat_t stat;
    if (!tar_stat_file(tar, filename, &stat)) {
        puts("Failed to find file\n");
        return 1;
    }

    uint8_t * buff = kmalloc(stat.size);
    if (!buff)
        return 1;

    tar_fs_file_t * file = tar_file_open(tar, filename);
    if (!file) {
        kfree(buff);
        return 1;
    }

    if (!tar_file_read(file, buff, stat.size)) {
        tar_file_close(file);
        kfree(buff);
        return 1;
    }

    int res = command_exec(buff, stat.size, argc, argv);

    if (!disk || !buff)
        return 0;

    tar_file_close(file);
    kfree(buff);

    return res;
}

void commands_init() {
    set_command_lookup(command_lookup);

    term_command_add("clear", clear_cmd);
    term_command_add("echo", echo_cmd);
    term_command_add("debug", debug_cmd);
    term_command_add("atoi", atoi_cmd);
    term_command_add("outb", port_out_cmd);
    term_command_add("inb", port_in_cmd);
    term_command_add("time", time_cmd);
    term_command_add("ret", ret_cmd);
    // term_command_add("format", format_cmd);
    term_command_add("mount", mount_cmd);
    term_command_add("unmount", unmount_cmd);
    term_command_add("mem", mem_cmd);
    term_command_add("ls", ls_cmd);
    term_command_add("stat", stat_cmd);
    // term_command_add("status", status_cmd);
    term_command_add("read", fs_read_cmd);
    term_command_add("cat", fs_cat_cmd);
    // term_command_add("read", disk_read_cmd);
    term_command_add("write", disk_write_cmd);
    term_command_add("size", disk_size_cmd);
}
