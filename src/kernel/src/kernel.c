#include "kernel.h"

#include "commands.h"
#include "config.h"
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
#include "exec.h"
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
#include "process_manager.h"
#include "ram.h"

static kernel_t __kernel;

extern _Noreturn void halt(void);

static void id_map_range(mmu_table_t * table, size_t start, size_t end);
static void id_map_page(mmu_table_t * table, size_t page);
static void cursor();
static void irq_install();
static int  kill(size_t argc, char ** argv);
static void map_first_table(mmu_table_t * table);

extern void jump_kernel_mode(void * fn);

static void idle_loop();
static int  start_shell();

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
    __kernel.proc.esp0           = VADDR_ISR_STACK;
    __kernel.proc.state          = PROCESS_STATE_LOADED;
    set_active_task(&__kernel.proc);

    // Set initial ESP0 before first task switch
    tss_set_esp0(VADDR_ISR_STACK);

    isr_install();

    init_system_call(IRQ16);
    system_call_register(SYS_INT_FAMILY_IO, sys_call_io_cb);
    system_call_register(SYS_INT_FAMILY_MEM, sys_call_mem_cb);
    system_call_register(SYS_INT_FAMILY_PROC, sys_call_proc_cb);
    system_call_register(SYS_INT_FAMILY_STDIO, sys_call_tmp_stdio_cb);

    // Init kernel memory after system calls are registered
    memory_init(&__kernel.proc.memory, kernel_alloc_page);

    // Create ebus for kernel (target of queue_event)
    if (ebus_create(&__kernel.event_queue, 4096)) {
        KPANIC("Failed to init ebus\n");
    }

    // Create process manager
    pm_create(&__kernel.pm);

    // Setup kernel process and add it to pm
    __kernel.pm.idle_task   = &__kernel.proc;
    __kernel.proc.next_proc = __kernel.pm.idle_task;
    pm_add_proc(&__kernel.pm, &__kernel.proc);

    // Create ebus for kernel proc
    if (ebus_create(&__kernel.proc.event_queue, 4096)) {
        KPANIC("Failed to init ebus\n");
    }

    // Init drivers and hardware interrupts
    irq_install();

    vga_puts("Welcome to kernel v" PROJECT_VERSION "\n");

    ramdisk_create(4096);

    __kernel.disk = disk_open(0, DISK_DRIVER_ATA);
    if (!__kernel.disk) {
        KPANIC("Failed to open ATA disk");
    }

    __kernel.tar = tar_open(__kernel.disk);
    if (!__kernel.tar) {
        KPANIC("Failed to open tar");
    }

    start_shell();

    pm_resume_process(&__kernel.pm, __kernel.pm.idle_task->pid, 0);

    idle_loop();

    KPANIC("You shouldn't be here!");
}

static int ebus_cycle(ebus_t * bus) {
    if (!bus) {
        return -1;
    }

    while (cb_len(&bus->queue) > 0) {
        ebus_event_t event;
        if (cb_pop(&bus->queue, &event)) {
            return -1;
        }

        // if (handle_event(bus, &event)) {
        //     // Handler consumed event
        //     continue;
        // }

        if (pm_push_event(kernel_get_proc_man(), &event)) {
            return -1;
        }
    }

    return 0;
}

static void idle_loop() {
    for (;;) {
        ebus_cycle(get_kernel_ebus());
        asm("hlt");
        yield();
    }
}

static int start_shell() {
    char * filename = "shell";

    tar_stat_t stat;
    if (!tar_stat_file(kernel_get_tar(), filename, &stat)) {
        puts("Failed to find file\n");
        return -1;
    }

    uint8_t * buff = kmalloc(stat.size);
    if (!buff) {
        return -1;
    }

    tar_fs_file_t * file = tar_file_open(kernel_get_tar(), filename);
    if (!file) {
        kfree(buff);
        return -1;
    }

    if (!tar_file_read(file, buff, stat.size)) {
        tar_file_close(file);
        kfree(buff);
        return -1;
    }

    if (command_exec(buff, stat.size, 1, &filename)) {
        tar_file_close(file);
        kfree(buff);
        return -1;
    }

    tar_file_close(file);
    kfree(buff);

    return 0;
}

void * kernel_alloc_page(size_t count) {
    return process_add_pages(get_active_task(), count);
}

int kernel_switch_task(int next_pid) {
    return pm_resume_process(&__kernel.pm, next_pid, 0);
}

mmu_dir_t * get_kernel_dir() {
    return (mmu_dir_t *)__kernel.cr3;
}

mmu_table_t * get_kernel_table() {
    return (mmu_table_t *)VADDR_KERNEL_TABLE;
}

process_t * get_current_process() {
    return get_active_task();
}

ebus_t * get_kernel_ebus() {
    return &__kernel.event_queue;
}

disk_t * kernel_get_disk() {
    return __kernel.disk;
}

tar_fs_t * kernel_get_tar() {
    return __kernel.tar;
}

void tmp_register_signals_cb(signals_master_cb_t cb) {
    get_active_task()->signals_callback = cb;
    printf("Attached master signal callback at %p\n", get_active_task()->signals_callback);
}

int kernel_add_task(process_t * proc) {
    return pm_add_proc(&__kernel.pm, proc);
}

int kernel_next_task() {
    return pm_resume_process(&__kernel.pm, get_active_task()->pid, 0);
}

int kernel_close_process(process_t * proc) {
    if (!proc) {
        return -1;
    }

    proc->state = PROCESS_STATE_DEAD;

    process_t * next = pm_get_next(&__kernel.pm);
    if (!next) {
        next = __kernel.pm.idle_task;
    }

    return 0;
}

NO_RETURN void kernel_panic(const char * msg, const char * file, unsigned int line) {
    vga_color(VGA_FG_WHITE | VGA_BG_RED);
    vga_puts("[KERNEL PANIC]");
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
    halt();
}

kernel_t * get_kernel() {
    return &__kernel;
}

proc_man_t * kernel_get_proc_man() {
    return &__kernel.pm;
}

process_t * kernel_find_pid(int pid) {
    return pm_find_pid(&__kernel.pm, pid);
}

void * kmalloc(size_t size) {
    return memory_alloc(&__kernel.proc.memory, size);
}

void * krealloc(void * ptr, size_t size) {
    return memory_realloc(&__kernel.proc.memory, ptr, size);
}

void kfree(void * ptr) {
    memory_free(&__kernel.proc.memory, ptr);
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
    init_timer(TIMER_FREQ_MS); // milliseconds
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
    KPANIC("Never return!");
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
        KPANIC("End is past table limits");
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
