#include "libc/fs.h"

#include "drivers/ata.h"
#include "libc/mem.h"
#include "libc/stdio.h"
#include "libc/string.h"

#define MAGIC 0x53467366 // "fsFS"

#define PAGE_SIZE 4096

#define BLOCK_SIZE_BYTES 64
#define BLOCKS_PER_GROUP 256 // 8 bit max

#define DNODE_PER_BLOCK (sizeof(dnode_t) / (BLOCK_SIZE_BYTES))

#define LOC_BLOCK(LOC) ((LOC) & 0xff)
#define LOC_GROUP(LOC) (((LOC) >> 8) & 0x3fff)
#define LOC_TYPE(LOC) ((LOC) >> 30)
#define LOC_ADDR(BLOCK, GROUP, TYPE) \
    ((((TYPE) & 0x3) << 30) | (((GROUP) & 0x3fff) << 8) | ((BLOCK) & 0xff))

typedef uint32_t loc_t;

typedef struct {
    uint8_t block : 8;
    uint32_t group : 22;
    uint8_t type : 2;
} __attribute__((packed)) block_addr_t;

enum NODE_TYPE {
    NODE_TYPE_INVALID = 0,
    NODE_TYPE_DNODE = 1,
    NODE_TYPE_INODE = 2,
    NODE_TYPE_DATA = 3,
};

typedef struct {
    uint32_t magic;
    uint32_t block_size;
    uint32_t group_block_count;
    uint32_t group_count;
    block_addr_t root_dnode;
} __attribute__((packed)) superblock_t;

typedef struct {
    uint32_t id;
    uint32_t parent;
    block_addr_t name_ptr;
    block_addr_t indirect;
    // remainder of block is list of children id's
    block_addr_t direct[4];
} __attribute__((packed)) dnode_t;

typedef struct {
    block_addr_t next;
    // remainder of block is list of children id's
    uint32_t direct[15];
} __attribute__((packed)) dnode_child_list_t;

typedef struct {
    uint32_t id;
    uint32_t parent_id;
    uint32_t size;
    block_addr_t name_ptr;
    block_addr_t indirect;
    uint32_t direct[3];
} __attribute__((packed)) inode_t;

typedef struct {
    block_addr_t next;
    // remainder of block is list of children id's
    uint32_t direct[15];
} __attribute__((packed)) inode_child_list_t;

typedef struct {
    uint8_t bitmask[32];
} __attribute__((packed)) group_t;

struct _filesystem {
    disk_t * disk;
    superblock_t super_block;
};

uint8_t page_buff[PAGE_SIZE];

static bool group_get_block(const group_t * group, size_t block);
static void group_set_block(group_t * group, size_t block, bool free);

static void fs_format_superblock(superblock_t * sb, uint32_t size);
static void fs_format_block_group(group_t * group);
static void fs_format_dnode(dnode_t * buff);

void fs_format(disk_t * disk) {
    // memset(page_buff, 0, PAGE_SIZE);

    size_t size = disk_size(disk);

    // TODO handle error
    fs_format_superblock((superblock_t *)page_buff, size);

    group_t * group = (group_t *)page_buff + BLOCK_SIZE_BYTES;
    memset(group + sizeof(group_t), 0, BLOCK_SIZE_BYTES - sizeof(group_t));

    // TODO handle error
    fs_format_block_group(group);

    // Set root dnode block to used
    group_set_block(group, 1, false);

    dnode_t * root_dnode = (dnode_t *)page_buff + BLOCK_SIZE_BYTES * 2;

    // TODO handle error
    fs_format_dnode(root_dnode);
    root_dnode->id = 1;

    // block has 2 dnodes
    fs_format_dnode(root_dnode + sizeof(dnode_t));

    // TODO handle error
    disk_sect_write(disk, page_buff, 1, 0);

    // TODO generate remaining groups bitmask
    // for (size_t i = 1; i < )
}

static bool group_get_block(const group_t * group, size_t block) {
    size_t byte = block / 8;
    size_t bit = block % 8;
    return (group->bitmask[byte] & (1 << bit)) >> bit;
}

static void group_set_block(group_t * group, size_t block, bool free) {
    size_t byte = block / 8;
    size_t bit = block % 8;
    uint8_t mask = 1 << bit;

    if (free)
        group->bitmask[byte] |= mask;
    else
        group->bitmask[byte] &= ~mask;
}

static void fs_format_superblock(superblock_t * sb, uint32_t size) {
    sb->magic = MAGIC;
    sb->block_size = BLOCK_SIZE_BYTES;
    sb->group_block_count = BLOCKS_PER_GROUP;
    sb->group_count =
        (size - BLOCK_SIZE_BYTES) / (BLOCK_SIZE_BYTES * BLOCKS_PER_GROUP);
    sb->root_dnode.block = 1;
    sb->root_dnode.group = 0;
    sb->root_dnode.type = NODE_TYPE_DNODE;
}

static void fs_format_block_group(group_t * group) {
    memset(group->bitmask, 1, sizeof(group->bitmask));
    group_set_block(group, 0, false);
}

static void fs_format_dnode(dnode_t * node) {
    block_addr_t empty = {
        .block = 0,
        .group = 0,
        .type = NODE_TYPE_INVALID,
    };

    node->id = 1;
    node->parent = 0;
    node->name_ptr = empty;
    node->indirect = empty;
    for (size_t i = 0; i < sizeof(node->direct); i++) {
        node->direct[i] = empty;
    }
}

filesystem_t * fs_new(disk_t * disk) {
    filesystem_t * fs = malloc(sizeof(filesystem_t));
    if (fs) {
        // TODO handle errors
        disk_sect_read(disk, page_buff, 1, 0);

        fs->super_block = *(superblock_t *)page_buff;
        if (fs->super_block.magic != MAGIC) {
            puts("[ERROR] disk is not formatted\n");
            free(fs);
            return 0;
        }

        // TODO read fields

        // TODO read root dnode
    }
    return fs;
}

void fs_free(filesystem_t * fs) {
    free(fs);
}
