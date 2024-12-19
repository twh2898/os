#ifndef PROC_H
#define PROC_H

#include "defs.h"
#include "cpu/tss.h"

enum PROCESS_FLAGS {
    PROCESS_FLAGS_ACTIVE = 0x1,
    PROCESS_FLAGS_ERROR  = 0x2,
    PROCESS_FLAGS_DEAD   = 0x4,
};

typedef struct {
    uint32_t    pid;
    uint16_t    flags;
    uint32_t    page_dir_paddr;
    tss_entry_t tss;
    char        command[257];
} process_t;

process_t * proc_new(uint32_t pid, const char * command, uint16_t flags);
void        proc_free(process_t * proc);

#endif // PROC_H
