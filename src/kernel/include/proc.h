#ifndef PROC_H
#define PROC_H

#include <stdint.h>

enum PROCESS_FLAGS {
    PROCESS_FLAGS_ACTIVE = 0x1,
    PROCESS_FLAGS_ERROR  = 0x2,
    PROCESS_FLAGS_DEAD   = 0x4,
};

typedef struct {
    uint32_t eip;
    uint32_t esp;
    uint32_t cr3;
    uint32_t ss0;
    uint16_t pid;
    void *   next_proc;
} process_t;

process_t * proc_new(uint32_t     eip,
                     uint32_t     esp,
                     uint32_t     cr3,
                     uint32_t     ss0,
                     uint32_t     pid,
                     const char * command,
                     uint16_t     flags);
void        proc_free(process_t * proc);

extern void switch_to_task(process_t * next_proc);

#endif // PROC_H
