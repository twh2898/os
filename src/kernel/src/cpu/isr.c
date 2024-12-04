#include "cpu/isr.h"

#include "cpu/idt.h"
#include "cpu/ports.h"
#include "cpu/timer.h"
#include "drivers/ata.h"
#include "drivers/keyboard.h"
#include "drivers/rtc.h"
#include "kernel.h"
#include "libc/stdio.h"

static void print_trace(registers_t *);

isr_t interrupt_handlers[256];

/* Can't do this with a loop because we need the address
 * of the function names */
void isr_install() {
    set_idt_gate(0, (uint32_t)isr0);
    set_idt_gate(1, (uint32_t)isr1);
    set_idt_gate(2, (uint32_t)isr2);
    set_idt_gate(3, (uint32_t)isr3);
    set_idt_gate(4, (uint32_t)isr4);
    set_idt_gate(5, (uint32_t)isr5);
    set_idt_gate(6, (uint32_t)isr6);
    set_idt_gate(7, (uint32_t)isr7);
    set_idt_gate(8, (uint32_t)isr8);
    set_idt_gate(9, (uint32_t)isr9);
    set_idt_gate(10, (uint32_t)isr10);
    set_idt_gate(11, (uint32_t)isr11);
    set_idt_gate(12, (uint32_t)isr12);
    set_idt_gate(13, (uint32_t)isr13);
    set_idt_gate(14, (uint32_t)isr14);
    set_idt_gate(15, (uint32_t)isr15);
    set_idt_gate(16, (uint32_t)isr16);
    set_idt_gate(17, (uint32_t)isr17);
    set_idt_gate(18, (uint32_t)isr18);
    set_idt_gate(19, (uint32_t)isr19);
    set_idt_gate(20, (uint32_t)isr20);
    set_idt_gate(21, (uint32_t)isr21);
    set_idt_gate(22, (uint32_t)isr22);
    set_idt_gate(23, (uint32_t)isr23);
    set_idt_gate(24, (uint32_t)isr24);
    set_idt_gate(25, (uint32_t)isr25);
    set_idt_gate(26, (uint32_t)isr26);
    set_idt_gate(27, (uint32_t)isr27);
    set_idt_gate(28, (uint32_t)isr28);
    set_idt_gate(29, (uint32_t)isr29);
    set_idt_gate(30, (uint32_t)isr30);
    set_idt_gate(31, (uint32_t)isr31);

    // Remap the PIC
    port_byte_out(0x20, 0x11);
    port_byte_out(0xA0, 0x11);
    port_byte_out(0x21, 0x20);
    port_byte_out(0xA1, 0x28);
    port_byte_out(0x21, 0x04);
    port_byte_out(0xA1, 0x02);
    port_byte_out(0x21, 0x01);
    port_byte_out(0xA1, 0x01);
    port_byte_out(0x21, 0x0);
    port_byte_out(0xA1, 0x0);

    // Install the IRQs
    set_idt_gate(32, (uint32_t)irq0);
    set_idt_gate(33, (uint32_t)irq1);
    set_idt_gate(34, (uint32_t)irq2);
    set_idt_gate(35, (uint32_t)irq3);
    set_idt_gate(36, (uint32_t)irq4);
    set_idt_gate(37, (uint32_t)irq5);
    set_idt_gate(38, (uint32_t)irq6);
    set_idt_gate(39, (uint32_t)irq7);
    set_idt_gate(40, (uint32_t)irq8);
    set_idt_gate(41, (uint32_t)irq9);
    set_idt_gate(42, (uint32_t)irq10);
    set_idt_gate(43, (uint32_t)irq11);
    set_idt_gate(44, (uint32_t)irq12);
    set_idt_gate(45, (uint32_t)irq13);
    set_idt_gate(46, (uint32_t)irq14);
    set_idt_gate(47, (uint32_t)irq15);
    set_idt_gate(48, (uint32_t)irq16);

    set_idt(); // Load with ASM
}

/* To print the message which defines every exception */
char * exception_messages[] = {
    //
    "Division By Zero",
    "Debug",
    "Non Maskable Interrupt",
    "Breakpoint",
    "Into Detected Overflow",
    "Out of Bounds",
    "Invalid Opcode",
    "No Coprocessor",

    "Double Fault",
    "Coprocessor Segment Overrun",
    "Bad TSS",
    "Segment Not Present",
    "Stack Fault",
    "General Protection Fault",
    "Page Fault",
    "Unknown Interrupt",

    "Coprocessor Fault",
    "Alignment Check",
    "Machine Check",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",

    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
    "Reserved",
};

void isr_handler(registers_t r) {
    kprintf("%s\n", exception_messages[r.int_no]);
    kprintf("ISR %u (err 0x%X)\n", r.int_no, r.err_code);
    print_trace(&r);
    switch (r.int_no) {
        case 14: {
            switch (r.err_code) {
                case 0:
                    kputs("Supervisory process tried to read a non-present page entry\n");
                    break;
                case 1:
                    kputs("Supervisory process tried to read a page and caused a protection fault\n");
                    break;
                case 2:
                    kputs("Supervisory process tried to write to a non-present page entry\n");
                    break;
                case 3:
                    kputs("Supervisory process tried to write a page and caused a protection fault\n");
                    break;
                case 4:
                    kputs("User process tried to read a non-present page entry\n");
                    break;
                case 5:
                    kputs("User process tried to read a page and caused a protection fault\n");
                    break;
                case 6:
                    kputs("User process tried to write to a non-present page entry\n");
                    break;
                case 7:
                    kputs("User process tried to write a page and caused a protection fault\n");
                    break;
                default:
                    break;
            }
        } break;
        default:
            break;
    }
    KERNEL_PANIC("STOP HERE");
    return;
}

/*
US RW  P - Description
0  0  0 - Supervisory process tried to read a non-present page entry
0  0  1 - Supervisory process tried to read a page and caused a protection fault
0  1  0 - Supervisory process tried to write to a non-present page entry
0  1  1 - Supervisory process tried to write a page and caused a protection fault
1  0  0 - User process tried to read a non-present page entry
1  0  1 - User process tried to read a page and caused a protection fault
1  1  0 - User process tried to write to a non-present page entry
1  1  1 - User process tried to write a page and caused a protection fault
*/

void register_interrupt_handler(uint8_t n, isr_t handler) {
    interrupt_handlers[n] = handler;
}

void irq_handler(registers_t r) {
    /* After every interrupt we need to send an EOI to the PICs
     * or they will not send another interrupt again */
    if (r.int_no >= 40)
        port_byte_out(0xA0, 0x20); /* slave */
    port_byte_out(0x20, 0x20); /* master */

    if (r.int_no >= 256) {
        kprintf("BAD INTERRUPT 0x%X\n", r.int_no);
        print_trace(&r);
        KERNEL_PANIC("BAD INTERRUPT");
        return;
    }

    /* Handle the interrupt in a more modular way */
    if (interrupt_handlers[r.int_no] != 0) {
        isr_t handler = interrupt_handlers[r.int_no];
        handler(r);
    }
}

void irq_install() {
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

extern uint32_t send_interrupt(uint16_t);

uint32_t system_call(uint8_t ah, uint8_t al) {
    uint32_t i = send_interrupt((ah << 4) | al);
    return i;
}

void disable_interrupts() {
    asm("cli");
}

void enable_interrupts() {
    asm("sti");
}

static void print_cr0(uint32_t cr0) {
    kputs("[ ");
    if (cr0 & (1 << 0))
        kputs("PE ");
    if (cr0 & (1 << 1))
        kputs("MP ");
    if (cr0 & (1 << 2))
        kputs("EM ");
    if (cr0 & (1 << 3))
        kputs("TS ");
    if (cr0 & (1 << 4))
        kputs("ET ");
    if (cr0 & (1 << 5))
        kputs("NE ");
    if (cr0 & (1 << 16))
        kputs("WP ");
    if (cr0 & (1 << 18))
        kputs("AM ");
    if (cr0 & (1 << 29))
        kputs("NW ");
    if (cr0 & (1 << 30))
        kputs("CD ");
    if (cr0 & (1 << 31))
        kputs("PG ");
    kputc(']');
}

static void print_trace(registers_t * r) {
    kprintf("EAX: 0x%08X EBX: 0x%08X ECX: 0x%08X EDX: 0x%08X\n", r->eax, r->ebx, r->ecx, r->edx);
    kprintf("ESI: 0x%08X EDI: 0x%08X EBP: 0x%08X ESP: 0x%08X\n", r->esi, r->edi, r->ebp, r->esp);
    kprintf("CR0: 0x%08X CR2: 0x%08X CR3: 0x%08X CR4: 0x%08X\n", r->cr0, r->cr2, r->cr3, r->cr4);
    print_cr0(r->cr0);
    kputc('\n');
}
/*
Bit	Label	Description
0	PE	Protected Mode Enable
1	MP	Monitor co-processor
2	EM	x87 FPU Emulation
3	TS	Task switched
4	ET	Extension type
5	NE	Numeric error
16	WP	Write protect
18	AM	Alignment mask
29	NW	Not-write through
30	CD	Cache disable
31	PG	Paging

  EAX: 0x00000000  EBX: 0x00000000  ECX: 0x00000000  EDX: 0x00000663  o d i t s
z a p c ESI: 0x00000000  EDI: 0x00000000  EBP: 0x00000000  ESP: 0x00000000  EIP:
0x0000FFF0 CS: F000  DS: 0000  ES: 0000  FS: 0000  GS: 0000  SS: 0000
*/