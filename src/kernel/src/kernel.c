#include "kernel.h"

#include "commands.h"
#include "cpu/gdt.h"
#include "cpu/isr.h"
#include "cpu/mmu.h"
#include "cpu/ports.h"
#include "cpu/tss.h"
#include "defs.h"
#include "drivers/ata.h"
#include "drivers/io_driver.h"
#include "drivers/keyboard.h"
#include "drivers/ramdisk.h"
#include "drivers/rtc.h"
#include "drivers/timer.h"
#include "drivers/vga.h"
#include "io/file.h"
#include "kernel/boot_params.h"
#include "kernel/system_call.h"
#include "kernel/system_call_io.h"
#include "kernel/system_call_mem.h"
#include "kernel/system_call_proc.h"
#include "kernel/system_call_stdio.h"
#include "libc/memory.h"
#include "libc/proc.h"
#include "libc/stdio.h"
#include "libc/string.h"
#include "libk/defs.h"
#include "libk/sys_call.h"
#include "process.h"
#include "ram.h"
#include "term.h"

static kernel_t __kernel;

extern _Noreturn void halt(void);

static void id_map_range(mmu_table_t * table, size_t start, size_t end);
static void id_map_page(mmu_table_t * table, size_t page);
static void cursor();
static void irq_install();
static int  kill(size_t argc, char ** argv);
static int  try_switch(size_t argc, char ** argv);
static void map_first_table(mmu_table_t * table);

extern void jump_kernel_mode(void * fn);

void kernel_main() {
    init_vga(UINT2PTR(PADDR_VGA));
    vga_clear();

    kmemset(&__kernel, 0, sizeof(kernel_t));

    boot_params_t * bparams = get_boot_params();

    // Init RAM
    __kernel.ram_table_addr = PADDR_RAM_TABLE;
    ram_init((void *)__kernel.ram_table_addr, (void *)VADDR_RAM_BITMASKS);

    for (size_t i = 0; i < bparams->mem_entries_count; i++) {
        upper_ram_t * entry = &bparams->mem_entries[i];

        // End of second stage kernel
        if (entry->base_addr <= 0x9fbff) {
            continue;
        }

        if (entry->type == RAM_TYPE_USABLE || entry->type == RAM_TYPE_ACPI_RECLAIMABLE) {
            ram_region_add_memory(entry->base_addr, entry->length);
        }
    }

    // Init Page Dir
    __kernel.cr3     = PADDR_KERNEL_DIR;
    mmu_dir_t * pdir = (mmu_dir_t *)__kernel.cr3;
    mmu_dir_clear(pdir);

    // Init first table
    uint32_t first_table_addr = ram_page_palloc();
    mmu_dir_set(pdir, 0, first_table_addr, MMU_DIR_RW);

    // Map first table
    mmu_table_t * first_table = UINT2PTR(first_table_addr);
    mmu_table_clear(first_table);
    map_first_table(first_table);

    // Map last table to dir for access to tables
    mmu_dir_set(pdir, MMU_DIR_SIZE - 1, __kernel.cr3, MMU_DIR_RW);

    // Enter Paging
    mmu_enable_paging(__kernel.cr3);

    // GDT & TSS
    init_gdt();
    init_tss();

    // Kernel process used for memory allocation
    __kernel.proc.next_heap_page = ADDR2PAGE(VADDR_RAM_BITMASKS) + ram_region_table_count();
    __kernel.proc.cr3            = PADDR_KERNEL_DIR;

    // Add isr stack to kernel's TSS
    set_kernel_stack(VADDR_ISR_STACK);

    // Setup kernel process as idle process
    __kernel.pm.idle_task            = &__kernel.proc;
    __kernel.pm.idle_task->next_proc = __kernel.pm.idle_task;
    __kernel.pm.curr_task            = __kernel.pm.idle_task;
    __kernel.pm.task_begin           = __kernel.pm.idle_task;

    arr_create(&__kernel.proc.io_handles, 1, sizeof(handle_t));

    isr_install();
    irq_install();

    init_system_call(IRQ16);
    system_call_register(SYS_INT_FAMILY_IO, sys_call_io_cb);
    system_call_register(SYS_INT_FAMILY_MEM, sys_call_mem_cb);
    system_call_register(SYS_INT_FAMILY_PROC, sys_call_proc_cb);
    system_call_register(SYS_INT_FAMILY_STDIO, sys_call_tmp_stdio_cb);

    // Init kernel memory after system calls are registered
    memory_init(&__kernel.kernel_memory, _sys_page_alloc);
    init_malloc(&__kernel.kernel_memory);

    vga_puts("Welcome to kernel v0.1.1\n");

    init_io_drivers();
    ata_register_driver();

    ata_t * disk = ata_open(0);
    register_io_device("disk/0", disk, "ata");

    printf("ATA disk registered\n");

    io_device_t * disk_dev = get_io_device("disk/0");

    printf("Got device %p\n", disk_dev);

    printf("Reading 10\n");
    char buff[1024];
    disk_dev->driver->read_fn(disk_dev->driver_data, buff, 10, 0);

    for (size_t i = 0; i < 10; i++) {
        printf("%x ", buff[i]);
    }
    putc('\n');

    term_init();
    commands_init();

    term_command_add("exit", kill);

    ramdisk_create(4096);

    // set_first_task(proc);

    // switch_to_task(proc);

    // jump_usermode(term_run);
    jump_kernel_mode(term_run);

    PANIC("You shouldn't be here!");
}

mmu_dir_t * get_kernel_dir() {
    return (mmu_dir_t *)__kernel.cr3;
}

mmu_table_t * get_kernel_table() {
    return (mmu_table_t *)VADDR_KERNEL_TABLE;
}

process_t * get_current_process() {
    return __kernel.pm.curr_task;
}

void tmp_register_signals_cb(signals_master_cb_t cb) {
    __kernel.pm.curr_task->signals_callback = cb;
    printf("Attached master signal callback at %p\n", __kernel.pm.curr_task->signals_callback);
}

static void cursor() {
    vga_cursor(3, 3);

    vga_cursor_hide();
    vga_cursor_show();
    vga_cursor(vga_cursor_row(), vga_cursor_col());
}

static void irq_install() {
    enable_interrupts();
    /* IRQ0: timer */
    init_timer(1000); // milliseconds
    /* IRQ1: keyboard */
    init_keyboard();
    /* IRQ14: ata disk */
    init_ata();
    /* IRQ8: real time clock */
    init_rtc(RTC_RATE_1024_HZ);
}

static int kill(size_t argc, char ** argv) {
    printf("Leaving process now\n");
    kernel_exit();
    PANIC("Never return!");
    return 0;
}

static void map_first_table(mmu_table_t * table) {
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

    // Kernel Table
    mmu_table_set(table, ADDR2PAGE(VADDR_KERNEL_TABLE), (uint32_t)table, MMU_TABLE_RW);

    // RAM region bitmasks
    ram_table_t * ram_table = (ram_table_t *)(__kernel.ram_table_addr);

    for (size_t i = 0; i < ram_region_table_count(); i++) {
        uint32_t bitmask_addr = ram_table->entries[i].addr_flags & MASK_ADDR;
        mmu_table_set(table, ADDR2PAGE(VADDR_RAM_BITMASKS) + i, bitmask_addr, MMU_TABLE_RW);
    }
}

static void id_map_range(mmu_table_t * table, size_t start, size_t end) {
    if (end > 1023) {
        PANIC("End is past table limits");
        end = 1023;
    }

    while (start <= end) {
        id_map_page(table, start);
        start++;
    }
}

static void id_map_page(mmu_table_t * table, size_t page) {
    mmu_table_set(table, page, page << 12, MMU_TABLE_RW);
}
