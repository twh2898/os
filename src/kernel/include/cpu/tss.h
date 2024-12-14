#ifndef TSS_H
#define TSS_H

#include "defs.h"

typedef struct {
    uint16_t prev_tss;
    uint16_t __res1;

    uint32_t esp0;
    
    uint16_t ss0;
    uint16_t __res2;
    
    uint32_t esp1;
    
    uint16_t ss1;
    uint16_t __res3;
    
    uint32_t esp3;
    
    uint16_t ss3;
    uint16_t __res4;
    
    uint32_t cr3;
    
    uint32_t eip;
    
    uint32_t eflags;
    
    uint32_t eax;
    
    uint32_t ecx;
    
    uint32_t edx;
    
    uint32_t ebx;
    
    uint32_t esp;
    
    uint32_t ebp;
    
    uint32_t esi;
    
    uint32_t edi;
    
    uint16_t es;
    uint16_t __res5;
    
    uint16_t cs;
    uint16_t __res6;
    
    uint16_t ss;
    uint16_t __res7;
    
    uint16_t ds;
    uint16_t __res8;
    
    uint16_t fs;
    uint16_t __res9;
    
    uint16_t gs;
    uint16_t __res10;
    
    uint16_t iopb;
    uint16_t __res12;
    
    uint32_t ssp;
} __attribute__((packed)) tss_entry_t;

#endif // TSS_H
