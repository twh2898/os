ENTRY(__start)
OUTPUT_FORMAT(elf32-i386)

SECTIONS {
	. = 1M;
	.rodata BLOCK(4K) : ALIGN(4K) { *(.multiboot) }
	.text BLOCK(4K) : ALIGN(4K) { *(.text) }
	.rodata BLOCK(4K) : ALIGN(4K) { *(.rodata) }
	.data BLOCK(4K) : ALIGN(4K) { *(.data) }
}