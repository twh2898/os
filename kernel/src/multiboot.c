#include "multiboot.h"

#include <stdint.h>

#include "libc/stdio.h"

enum MB_FLAG {
    MB_FLAG_MEM = 1 << 0,
    MB_FLAG_BOOT_DEV = 1 << 1,
    MB_FLAG_CMDLINE = 1 << 2,
    MB_FLAG_MODS = 1 << 3,
    MB_FLAG_SYM_TABLE = 1 << 4,
    MB_FLAG_HEDER_TABLE = 1 << 5,
    MB_FLAG_MMAP = 1 << 6,
    MB_FLAG_DRIVES = 1 << 7,
    MB_FLAG_CONFIG = 1 << 8,
    MB_FLAG_BOOT_LOADER_NAME = 1 << 9,
    MB_FLAG_APM_TABLE = 1 << 10,
    MB_FLAG_VBE_TABLE = 1 << 11,
    MB_FLAG_FRAMEBUFF_TABLE = 1 << 12,
};

typedef struct {
    uint32_t mod_start;
    uint32_t mod_end;
    // uint32_t string;
    const char * string;
    uint32_t _reserved;
} mods_t;

enum MB_MMAP_TYPE {
    MB_MMAP_TYPE_AVAILABLE = 1,
    MB_MMAP_TYPE_RESERVED,
    MB_MMAP_TYPE_ACPI,
    MB_MMAP_TYPE_NVS,
    MB_MMAP_TYPE_BADRAM,
};

typedef struct {
    uint32_t size;
    uint64_t base_addr;
    uint64_t length;
    uint32_t type;
} mmap_t;

typedef struct {
    uint16_t version;
    uint16_t code_seg_32;
    uint32_t offset;
    uint16_t code_seg_16;
    uint16_t data_seg_16;
    uint16_t flags;
    uint16_t code_seg_len_32;
    uint16_t code_seg_len_16;
    uint16_t data_seg_len_16;
} __attribute__((packed)) apm_table_t;

typedef struct {
    uint32_t flags;

    // flags & 1
    uint32_t mem_lower;
    uint32_t mem_upper;

    // flags & 2
    struct {
        uint8_t part3;
        uint8_t part2;
        uint8_t part1;
        uint8_t drive;
    } boot;

    // flags & 4
    // uint32_t cmdline;
    const char * cmdline;

    // flags & 8
    uint32_t mods_count;
    // uint32_t mods_addr;
    const mods_t * mods_addr;

    union {
        // flags & 0x10
        struct {
            uint32_t tabsize;
            uint32_t strsize;
            uint32_t addr;
            uint32_t _reserved;
        } sym_table;

        // flags & 0x20
        struct {
            uint32_t num;
            uint32_t size;
            uint32_t addr;
            uint32_t shndx;
        } header_table;
    };

    // flags & 0x40
    uint32_t mmap_length;
    // uint32_t mmap_addr;
    const mmap_t * mmap_addr;

    // flags & 0x80
    uint32_t drives_length;
    uint32_t drives_addr;

    // flags & 0x100
    uint32_t config_table;

    // flags & 0x200
    // uint32_t boot_loader_name;
    const char * boot_loader_name;

    // flags & 0x400
    // uint32_t apm_table;
    const apm_table_t * apm_table;

    // flag & 0x800
    uint32_t vbe_control_info;
    uint32_t veb_mode_info;
    uint32_t veb_mode;
    uint32_t veb_interface_seg;
    uint32_t veb_interface_off;
    uint32_t veb_interface_len;

    // flags & 0x1000
    uint32_t framebuffer_addr;
    uint32_t framebuffer_pitch;
    uint32_t framebuffer_width;
    uint32_t framebuffer_height;
    uint32_t framebuffer_bpp;
    uint32_t framebuffer_type;
    uint32_t color_info[2];
} __attribute__((packed)) multiboot_t;

extern multiboot_t * MULTIBOOT_SECTOR;

void multiboot_init() {
    multiboot_t * mb = MULTIBOOT_SECTOR;
    puts("Dump multiboot struct:\n");

    if (mb->flags & MB_FLAG_MEM)
        printf("Mem lower 0x%X kb  upper 0x%X kb\n", mb->mem_lower, mb->mem_upper);

    if (mb->flags & MB_FLAG_BOOT_DEV)
        printf("Boot drive 0x%02X partition %02x.%02x.%02x\n",
               mb->boot.drive,
               mb->boot.part1,
               mb->boot.part2,
               mb->boot.part3);

    if (mb->flags & MB_FLAG_CMDLINE)
        printf("Command Line %s\n", mb->cmdline);

    if (mb->flags & MB_FLAG_MODS) {
        if (!mb->mods_count)
            puts("No mods loaded\n");
        else
            printf("%u mods loaded\n", mb->mods_count);

        for (uint32_t i = 0; i < mb->mods_count; i++) {
            printf("Mod %u = %s\n    start %p  end %p\n",
                   mb->mods_addr[i].string,
                   mb->mods_addr[i].mod_start,
                   mb->mods_addr[i].mod_end);
        }
    }

    if ((mb->flags & MB_FLAG_SYM_TABLE) && (mb->flags & MB_FLAG_HEDER_TABLE)) {
        puts("[ERROR] both sym table and header table flags are set\n");
        return;
    }

    // TODO isn't set yet
    if (mb->flags & MB_FLAG_SYM_TABLE)
        printf("Symbols Table at %p\n", mb->sym_table.addr);

    // TODO isn't set yet
    if (mb->flags & MB_FLAG_HEDER_TABLE)
        printf("Header Table at %p with %u entries\n",
               mb->header_table.addr,
               mb->header_table.num);

    if (mb->flags & MB_FLAG_MMAP) {
        printf("Memory Map of size %u at %p\n", mb->mmap_length, mb->mmap_addr);
        const mmap_t * map = mb->mmap_addr;
        const mmap_t * end = mb->mmap_addr + mb->mmap_length;
        // FIXME This isn't working
        // for (; map < end; map = map + map->size + sizeof(map->size)) {
        //     // printf("size %u base_addr 0x%x%08x length 0x%x%08x type %u\n",
        //     printf("(%u)@ 0x%x%08x (0x%x%08x) Tp%u ",
        //            map->size,
        //            map->base_addr >> 32,
        //            map->base_addr & 0xFFFFFFFF,
        //            map->length >> 32,
        //            map->length & 0xFFFFFFFF,
        //            map->type);
        // }
        // putc('\n');
    }

    if (mb->flags & MB_FLAG_DRIVES) {
        printf("Drives at %p length %u\n", mb->drives_addr, mb->drives_length);
        // TODO isn't set yet
    }
    else
        puts("No drives\n");

    if (mb->flags & MB_FLAG_CONFIG)
        printf("Config table at %p\n", mb->config_table);

    if (mb->flags & MB_FLAG_BOOT_LOADER_NAME)
        printf("Boot loader name %s\n", mb->boot_loader_name);

    if (mb->flags & MB_FLAG_APM_TABLE) {
        printf("APM Table at %p\n", mb->apm_table);
        printf("version = %u code_seg_32 = %u offset = %u code_seg_16 = %u data_seg_16 = %u flags = %u code_seg_len_32 = %u code_seg_len_16 = %u data_seg_len_16 = %u\n",
               mb->apm_table->version,
               mb->apm_table->code_seg_32,
               mb->apm_table->offset,
               mb->apm_table->code_seg_16,
               mb->apm_table->data_seg_16,
               mb->apm_table->flags,
               mb->apm_table->code_seg_len_32,
               mb->apm_table->code_seg_len_16,
               mb->apm_table->data_seg_len_16);
    }

    if (mb->flags & MB_FLAG_VBE_TABLE) {
        printf("VBE controller info at %p mode info at %p\n",
               mb->vbe_control_info,
               mb->veb_mode_info);
        printf("    mode 0x%X\n", mb->veb_mode);
        printf("    seg 0x%X  off 0x%X  len %u\n",
               mb->veb_interface_seg,
               mb->veb_interface_off,
               mb->veb_interface_len);
    }

    if (mb->flags & MB_FLAG_FRAMEBUFF_TABLE) {
        printf("Framebuffer palette at %p\n", mb->framebuffer_addr);
        printf("    pitch = 0x%X  width = 0x%X  height = 0x%X  bpp = 0x%X  type = 0x%X\n",
               mb->framebuffer_pitch,
               mb->framebuffer_width,
               mb->framebuffer_height,
               mb->framebuffer_bpp,
               mb->framebuffer_type);
    }

    putc('\n');
}
