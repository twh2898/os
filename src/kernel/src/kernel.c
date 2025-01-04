#include "kernel.h"

#include "commands.h"
#include "cpu/gdt.h"
#include "cpu/isr.h"
#include "cpu/mmu.h"
#include "cpu/ports.h"
#include "cpu/tss.h"
#include "defs.h"
#include "drivers/ata.h"
#include "drivers/keyboard.h"
#include "drivers/ramdisk.h"
#include "drivers/rtc.h"
#include "drivers/timer.h"
#include "drivers/vga.h"
#include "interrupts.h"
#include "libc/process.h"
#include "libc/stdio.h"
#include "libc/string.h"
#include "libk/defs.h"
#include "memory.h"
#include "proc.h"
#include "ram.h"
#include "term.h"

static kernel_t __kernel;

extern _Noreturn void halt(void);

static process_t * get_current_process();
static void        map_first_table(mmu_page_table_t * table);
static void        id_map_range(mmu_page_table_t * table, size_t start, size_t end);
static void        id_map_page(mmu_page_table_t * table, size_t page);
static void        cursor();
static void        irq_install();
static uint32_t    int_mem_cb(uint16_t int_no, registers_t * regs);
static uint32_t    int_proc_cb(uint16_t int_no, registers_t * regs);
static uint32_t    int_tmp_stdio_cb(uint16_t int_no, registers_t * regs);
static int         kill(size_t argc, char ** argv);
static int         try_switch(size_t argc, char ** argv);
static void        map_first_table(mmu_page_table_t * table);

extern void jump_kernel_mode(void * fn);

void kernel_main() {
    init_vga(UINT2PTR(PADDR_VGA));
    vga_clear();

    if (kernel_init(&__kernel) != 0) {
        vga_color(VGA_RED_ON_WHITE);
        vga_puts("KERNEL INIT FAILED");
        halt();
    }

    // Init RAM
    __kernel.ram_table_addr = PADDR_RAM_TABLE;
    ram_init((void *)__kernel.ram_table_addr, &__kernel.ram_table_count);

    // Init Page Dir
    __kernel.cr3          = PADDR_PAGE_DIR;
    mmu_page_dir_t * pdir = mmu_dir_create((void *)__kernel.cr3);

    // Init first table
    uint32_t first_table_addr = ram_page_palloc();
    mmu_dir_set(pdir, 0, first_table_addr, MMU_DIR_RW);

    // Map first table
    mmu_page_table_t * first_table = mmu_table_create(UINT2PTR(first_table_addr));
    map_first_table(first_table);

    // Map last table to dir for access to tables
    mmu_dir_set(pdir, PAGE_DIR_SIZE - 1, __kernel.cr3, MMU_DIR_RW);

    // Enter Paging
    mmu_enable_paging(pdir);

    // GDT & TSS
    init_gdt();
    init_tss();

    // TODO isr stack

    isr_install();
    irq_install();

    init_system_interrupts(IRQ16);
    system_interrupt_register(SYS_INT_FAMILY_MEM, int_mem_cb);
    system_interrupt_register(SYS_INT_FAMILY_PROC, int_proc_cb);
    system_interrupt_register(SYS_INT_FAMILY_STDIO, int_tmp_stdio_cb);

    // TODO kernel heap with new memory_alloc (needs process or page allocator)

    init_malloc(pdir, VADDR_FREE_MEM_KERNEL >> 12);

    // check_malloc();

    vga_puts("Welcome to kernel v..\n");

    term_init();
    commands_init();

    term_command_add("exit", kill);

    ramdisk_create(4096);

    // process_t * idle_task = proc_new(term_run, 0x10);

    // proc_man_t * pm = proc_man_new();
    // proc_man_set_idle(pm, idle_task);

    // proc = proc_new(try_switch, 0x10);

    // set_first_task(proc);

    // switch_to_task(proc);

    // jump_usermode(term_run);
    jump_kernel_mode(term_run);

    PANIC("You shouldn't be here!");
}

int kernel_init(kernel_t * kernel) {
    if (!kernel) {
        return -1;
    }

    kmemset(kernel, 0, sizeof(kernel_t));

    kernel->pm.curr_task  = &kernel->proc;
    kernel->pm.idle_task  = &kernel->proc;
    kernel->pm.task_begin = &kernel->proc;

    kernel->proc.next_proc = kernel->pm.idle_task;

    return 0;
}

static process_t * get_current_process() {
    return __kernel.pm.curr_task;
}

static void cursor() {
    vga_cursor(3, 3);

    vga_cursor_hide();
    vga_cursor_show();
    vga_cursor(vga_cursor_row(), vga_cursor_col());
}

static void irq_install() {
    /* Enable interruptions */
    asm volatile("sti");
    /* IRQ0: timer */
    init_timer(1000); // milliseconds
    /* IRQ1: keyboard */
    init_keyboard();
    /* IRQ14: ata disk */
    init_ata();
    /* IRQ8: real time clock */
    init_rtc(RTC_RATE_1024_HZ);
}

static uint32_t int_mem_cb(uint16_t int_no, registers_t * regs) {
    uint32_t res = 0;

    switch (int_no) {
            // case SYS_INT_MEM_MALLOC: {
            //     size_t size = regs->ebx;
            //     res         = PTR2UINT(impl_kmalloc(size));
            // } break;

            // case SYS_INT_MEM_REALLOC: {
            //     void * ptr  = UINT2PTR(regs->ebx);
            //     size_t size = regs->ecx;
            //     // res = PTR2UINT(krealloc(ptr, size));
            // } break;

            // case SYS_INT_MEM_FREE: {
            //     void * ptr = UINT2PTR(regs->ebx);
            //     impl_kfree(ptr);
            // } break;

        case SYS_INT_MEM_PAGE_ALLOC: {
            size_t count = regs->ebx;

            process_t * curr_proc = get_current_process();

            // TODO alloc more pages
        } break;
    }

    return res;
}

static uint32_t int_proc_cb(uint16_t int_no, registers_t * regs) {
    uint32_t res = 0;

    switch (int_no) {
        case SYS_INT_PROC_EXIT: {
            uint8_t code = regs->ebx;
            printf("Proc exit with code %u\n", code);
            regs->eip = PTR2UINT(term_run);
            // kernel_exit();
        } break;

        case SYS_INT_PROC_ABORT: {
            uint8_t      code = regs->ebx;
            const char * msg  = UINT2PTR(regs->ecx);
            printf("Proc exit with code %u\n", code);
            puts(msg);
            regs->eip = PTR2UINT(term_run);
            // kernel_exit();
        } break;

        case SYS_INT_PROC_PANIC: {
            const char * msg  = UINT2PTR(regs->ebx);
            const char * file = UINT2PTR(regs->ecx);
            unsigned int line = regs->edx;
            vga_color(VGA_FG_WHITE | VGA_BG_RED);
            vga_puts("[PANIC]");
            if (file) {
                vga_putc('[');
                vga_puts(file);
                vga_puts("]:");
                vga_putu(line);
            }
            if (msg) {
                vga_putc(' ');
                vga_puts(msg);
            }
            vga_cursor_hide();
            asm("cli");
            for (;;) {
                asm("hlt");
            }
        } break;

        case SYS_INT_PROC_REG_SIG: {
            __kernel.pm.curr_task->sys_call_callback = (signals_master_callback)regs->ebx;
            printf("Attached master signal callback at %p\n", __kernel.pm.curr_task->sys_call_callback);
        } break;
    }

    return 0;
}

static uint32_t int_tmp_stdio_cb(uint16_t int_no, registers_t * regs) {
    uint32_t res = 0;

    switch (int_no) {
        case SYS_INT_STDIO_PUTC: {
            char c = regs->ebx;
            res    = vga_putc(c);
        } break;

        case SYS_INT_STDIO_PUTS: {
            char * str = UINT2PTR(regs->ebx);
            res        = vga_puts(str);
        } break;
    }

    return 0;
}

static int kill(size_t argc, char ** argv) {
    printf("Leaving process now\n");
    kernel_exit();
    PANIC("Never return!");
    return 0;
}

static void map_first_table(mmu_page_table_t * table) {
    // null page 0
    mmu_table_set(table, 0, 0, 0);

    // Page Directory
    mmu_table_set(table, 1, __kernel.cr3, MMU_DIR_RW);

    // Create first table
    mmu_table_set(table, 2, __kernel.ram_table_addr, MMU_DIR_RW);

    // Stack
    id_map_range(table, 3, 6);

    // Kernel
    id_map_range(table, 7, 0x9e);

    // VGA
    id_map_page(table, 0xb8);

    // RAM region bitmasks
    for (size_t i = 0; i < __kernel.ram_table_count; i++) {
        mmu_table_set(table, ADDR2PAGE(VADDR_RAM_BITMASKS) + i, ram_bitmask_paddr(i), MMU_TABLE_RW);
    }
}

static void id_map_range(mmu_page_table_t * table, size_t start, size_t end) {
    if (end > 1023) {
        PANIC("End is past table limits");
        end = 1023;
    }

    while (start <= end) {
        id_map_page(table, start);
        start++;
    }
}

static void id_map_page(mmu_page_table_t * table, size_t page) {
    mmu_table_set(table, page, page << 12, MMU_TABLE_RW);
}
