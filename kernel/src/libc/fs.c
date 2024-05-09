#include "libc/fs.h"

#include "drivers/ata.h"
#include "libc/mem.h"
#include "libc/stdio.h"

#define MAGIC 0x73664653 // "fsFS"
#define SECTOR_START 1 // skip bootsect
// #define INODE_BLOCKS 1

#define PAGE_SIZE 4096 // = 1 page = 8 sectors (512 each)
#define SECTORS_PER_PAGE (PAGE_SIZE / ATA_SECTOR_BYTES)

#define MAX_DIR_NAME_SIZE 55 // remove 1 for trailing 0
#define MAX_FILE_NAME_SIZE 47 // remove 1 for trailing 0

static uint8_t page_buff[PAGE_SIZE];

// DIR NODE
typedef struct _dnode {
    // TODO bool valid but bit aligned + adjust name size
    uint32_t id; // id = index?
    uint32_t parent;
    char name[MAX_DIR_NAME_SIZE + 1];
} __attribute__((__packed__)) dnode_t;

// FILE NODE
typedef struct _inode {
    // TODO bool valid but bit aligned + adjust name size
    uint32_t id; // id = index?
    uint32_t parent;
    uint32_t size;
    uint32_t lba;
    uint32_t direct[4];
    uint32_t indirect;
    char name[MAX_FILE_NAME_SIZE + 1];
} inode_t;

typedef struct _dnode_block {
    uint32_t next_lba; // dnode_block_t *
    uint8_t reserved[28]; // pad 32 bytes before name
    dnode_t nodes[(PAGE_SIZE - sizeof(uint32_t)) / sizeof(dnode_t)];
} dnode_block_t;

typedef struct _inode_block {
    uint32_t next_lba; // inode_block_t *
    uint8_t reserved[28]; // pad 32 bytes before name
    inode_t nodes[(PAGE_SIZE - sizeof(uint32_t)) / sizeof(inode_t)];
} inode_block_t;

/*
block of i nodes and d nodes has ref to next block of nodes
block of nodes is 1 page in size
*/

/*
i node for files includes id of parent d node
d node for directories with id of parent d node or 0 for root
*/

// TODO parse file path string

struct _filesystem {
    disk_t * disk;
    uint32_t sect_count;
    uint32_t start_lba;
    uint32_t dnode_start_lba; // dnode_block_t *
    uint32_t inode_start_lba; // inode_block_t *
};

struct _file {
    filesystem_t * fs;
};

/*

size in bytes

+------+------+-------------+
| ADDR | SIZE | NAME        |
+------+------+-------------+
| 0    | 4    | magic       |
+------+------+-------------+
| 4    | 4    | first data lba |
+------+------+-------------+
| 8    | 4    | first dnode block lba |
+------+------+-------------+
| 12   | 4    | first inode block lba |
+------+------+-------------+

*/

void fs_format(disk_t * disk) {
    page_buff[0] = MAGIC & 0xff;
    page_buff[1] = (MAGIC >> 4) & 0xff;
    page_buff[2] = (MAGIC >> 8) & 0xff;
    page_buff[3] = (MAGIC >> 12) & 0xff;

    uint32_t lba_count = disk_sector_count(disk);
    lba_count -= SECTOR_START + 2 * SECTORS_PER_PAGE; // + 2 for first block of
                                                      // inode and dnode
    *(uint32_t *)(page_buff + 4) = lba_count;
    *(uint32_t *)(page_buff + 8) = SECTOR_START + 1;
    *(uint32_t *)(page_buff + 12) = SECTOR_START + 2;

    // TODO initialize filesystem on disk, maybe write more sectors

    disk_sect_write(disk, page_buff, 1, SECTOR_START);

    // TODO write first dnode block
    dnode_block_t * dblock = (dnode_block_t *)page_buff;


    // TODO write first inode block
    inode_block_t * iblock = (inode_block_t *)page_buff;
}

#define ERROR(MSG)            \
    {                         \
        puts("[ERROR] " MSG); \
        free(fs);             \
        return 0;             \
    }

filesystem_t * fs_new(disk_t * disk) {
    filesystem_t * fs = malloc(sizeof(filesystem_t));
    if (fs) {
        fs->disk = disk;
        fs->sect_count = disk_sector_count(disk);
        fs->start_lba = SECTOR_START;

        if (disk_sect_read(fs->disk, page_buff, 1, fs->start_lba) != 1)
            ERROR("could not read filesystem from disk\n")

        uint32_t magic = page_buff[0];
        magic |= (page_buff[1] << 4);
        magic |= (page_buff[2] << 8);
        magic |= (page_buff[3] << 24);

        if (magic != MAGIC)
            ERROR("failed to match magic\n")

        fs->sect_count = page_buff[4];
        fs->sect_count |= (page_buff[5] << 4);
        fs->sect_count |= (page_buff[6] << 8);
        fs->sect_count |= (page_buff[7] << 24);
    }
    return fs;
}

#undef ERROR

void fs_free(filesystem_t * fs) {
    free(fs);
}

disk_t * fs_get_disk(filesystem_t * fs) {
    return fs->disk;
}

size_t fs_get_size(filesystem_t * fs) {
    return fs->sect_count * ATA_SECTOR_BYTES;
}

size_t fs_get_used(filesystem_t * fs) {
    return fs->sect_count * ATA_SECTOR_BYTES;
}

size_t fs_get_free(filesystem_t * fs) {
    return 0;
}
