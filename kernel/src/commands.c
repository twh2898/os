#include "commands.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "cpu/ports.h"
#include "cpu/timer.h"
#include "debug.h"
#include "drivers/ata.h"
#include "drivers/ram.h"
#include "drivers/rtc.h"
#include "drivers/vga.h"
#include "libc/fs.h"
#include "libc/stdio.h"
#include "libc/string.h"
#include "term.h"

bool debug = false;
static disk_t * disk = 0;
static filesystem_t * fs = 0;

static int clear_cmd(size_t argc, char ** argv) {
    vga_clear();
    return 0;
}

static int echo_cmd(size_t argc, char ** argv) {
    bool next_line = true;
    if (argc > 1 && memcmp(argv[1], "-n", 2) == 0)
        next_line = false;

    size_t i = 1;
    if (!next_line)
        i++;
    for (; i < argc; i++) {
        kputs(argv[i]);
        if (i < argc)
            kputc(' ');
    }

    if (next_line)
        kputc('\n');

    return 0;
}

static int debug_cmd(size_t argc, char ** argv) {
    debug = !debug;
    if (debug)
        kputs("Enable debug\n");
    else
        kputs("Disabling debug\n");
    return 0;
}

static int atoi_cmd(size_t argc, char ** argv) {
    if (argc != 2) {
        kprintf("Usage: %s <number>\n", argv[0]);
        return 1;
    }

    int i = atoi(argv[1]);
    kprintf("%d\n", i);
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
    size_t len = strlen(str);
    if (len < 1 || len > 4)
        return -1;

    uint16_t res = 0;
    while (*str) {
        res = (res << 4) | hex_digit(*str++);
    }
    return res;
}

static int port_out_cmd(size_t argc, char ** argv) {
    if (argc != 3) {
        kprintf("Usage: %s <port> <byte>\n", argv[0]);
        return 1;
    }

    if (strlen(argv[1]) > 4) {
        kputs("port has max 4 hex digits\n");
        return 1;
    }
    uint16_t port = parse_byte(argv[1]);

    if (strlen(argv[2]) > 2) {
        kputs("byte has max 2 hex digits\n");
        return 1;
    }
    uint8_t byte = parse_byte(argv[2]);

    port_byte_out(port, byte);
    return 0;
}

static int port_in_cmd(size_t argc, char ** argv) {
    if (argc != 2) {
        kprintf("Usage: %s <port>\n", argv[0]);
        return 1;
    }

    if (strlen(argv[1]) > 4) {
        kputs("port has max 4 hex digits\n");
        return 1;
    }
    uint16_t port = parse_byte(argv[1]);

    uint8_t byte = port_byte_in(port);
    kprintf("0x%04X = 0x%02X\n", port, byte);

    return 0;
}

static int time_cmd(size_t argc, char ** argv) {
    uint32_t ms = get_ticks();
    kprintf("System ticks: %u ~= %u s\n", ms, ms / 1000);
    kprintf("RTC time: %u us = %u ms = %u s\n", time_us(), time_ms(), time_s());
    return 0;
}

static int ret_cmd(size_t argc, char ** argv) {
    kprintf("Last command exit code was %u\n", term_last_ret);
    return 0;
}

static int format_cmd(size_t argc, char ** argv) {
    if (fs) {
        kputs("Unmount disk before format\n");
        return 1;
    }

    if (!disk) {
        disk = disk_open(0);
        if (!disk) {
            kputs("Failed to open disk\n");
            return 1;
        }
    }

    kputs("Formatting disk, this may take some time\n");
    fs_format(disk);
    kputs("Done!\n");

    return 0;
}

static int mount_cmd(size_t argc, char ** argv) {
    if (fs) {
        kputs("Filesystem already mounted\n");
        return 0;
    }

    if (!disk) {
        disk = disk_open(0);
        if (!disk) {
            kputs("Failed to open disk\n");
            return 1;
        }
    }

    fs = fs_new(disk);
    if (!fs) {
        kputs("Failed to mount filesystem\n");
        return 1;
    }

    return 0;
}

static int unmount_cmd(size_t argc, char ** argv) {
    if (fs) {
        fs_free(fs);
        fs = 0;
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
    kprintf("0x%08X%08X", u, l);
}

static void print_upper(uint64_t start, uint64_t end, uint64_t size, enum RAM_TYPE type) {
    kputs("| ");
    print_64(start);
    kputs(" | ");
    print_64(end);
    kputs(" | ");
    print_64(size);
    kputs(" | ");
    switch (type) {
        case 1:
            kputs("Usable RAM");
            break;
        case 2:
            kputs("Reserved");
            break;
        case 3:
            kputs("ACPI Reclaimable");
            break;
        case 4:
            kputs("ACPI NVS");
            break;
        case 5:
            kputs("BAD MEMORY");
            break;
    }
    kputc('\n');
}

static int mem_cmd(size_t argc, char ** argv) {
    uint16_t low_mem = ram_lower_size();
    kprintf("Lower memory is %u\n", low_mem);

    uint16_t count = ram_upper_count();

    kprintf("Total of %u blocks\n", count);

    kputs("| Start              | End                | Size               | Type\n");
    for (size_t i = 0; i < count; i++) {
        print_upper(ram_upper_start(i),
                    ram_upper_end(i),
                    ram_upper_size(i),
                    ram_upper_type(i));
    }
    return 0;
}

static int ls_cmd(size_t argc, char ** argv) {
    if (!fs) {
        kputs("Filesystem not mounted\n");
        return 1;
    }

    // TODO implement

    return 0;
}

// static int status_cmd(size_t argc, char ** argv) {
//     // disk_status();
//     return 0;
// }

// static int read_cmd(size_t argc, char ** argv) {
//     char data[ATA_SECTOR_BYTES];
//     // size_t read = disk_sect_read(data, 1, 0);
//     data[9] = 0;
//     kprintf("read data %s\n", data);
//     return 0;
// }

// static int write_cmd(size_t argc, char ** argv) {
//     char data[ATA_SECTOR_BYTES] = {0};
//     for (size_t i = 0; i < ATA_SECTOR_WORDS; i++) {
//         if ((i >> 4) < 10)
//             data[i * 2] = (i >> 4) + '0';
//         else
//             data[i * 2] = (i >> 4) + 'a' - 10;

//         if ((i & 0xf) < 10)
//             data[i * 2 + 1] = (i & 0xf) + '0';
//         else
//             data[i * 2 + 1] = (i & 0xf) + 'a' - 10;
//     }
//     // disk_sect_write(data, 1, 0);
//     return 0;
// }

void commands_init() {
    term_command_add("clear", clear_cmd);
    term_command_add("echo", echo_cmd);
    term_command_add("debug", debug_cmd);
    term_command_add("atoi", atoi_cmd);
    term_command_add("outb", port_out_cmd);
    term_command_add("inb", port_in_cmd);
    term_command_add("time", time_cmd);
    term_command_add("ret", ret_cmd);
    term_command_add("format", format_cmd);
    term_command_add("mount", mount_cmd);
    term_command_add("unmount", unmount_cmd);
    term_command_add("mem", mem_cmd);
    term_command_add("ls", ls_cmd);
    // term_command_add("status", status_cmd);
    // term_command_add("read", read_cmd);
    // term_command_add("write", write_cmd);
}
