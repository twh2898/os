ENTRY(__start)
OUTPUT_FORMAT(elf32-i386)

SECTIONS {
	. = 1M;
	.text : ALIGN(4K) { *(.multiboot) }
	.text : ALIGN(4K) { *(.text) }
	.unused_section : ALIGN(4K) { kernel_end = . ; }
}