#include "cpu/mmu.h"

#include "kernel.h"

#define PAGE_SIZE 0x1000 // 4k (>> 12)
#define PAGE_ALIGNED_DOWN(PTR) ((PTR) & 0xfffff000)
#define PAGE_ALIGNED_UP(PTR) ((PAGE_ALIGNED_DOWN(PTR)) + PAGE_SIZE)
#define PAGE_ALIGNED(PTR) (((PTR) & 0xfff) ? PAGE_ALIGNED_UP(PTR) : (PTR))

void * mmu_phys_addr(void * virt_addr) {
    unsigned long pdindex = (unsigned long)virt_addr >> 22;
    unsigned long ptindex = (unsigned long)virt_addr >> 12 & 0x03FF;

    // TODO THIS NEEDS TO BE WHERE THE PAGE DIR GETS MAPPED TO IN PAGING
    unsigned long * pd = (unsigned long *)0xFFFFF000;
    // TODO Here you need to check whether the PD entry is present.

    // TODO THIS NEEDS TO BE WHERE THE PAGE TABLE GETS MAPPED TO IN PAGING
    unsigned long * pt = ((unsigned long *)0xFFC00000) + (0x400 * pdindex);
    // TODO Here you need to check whether the PT entry is present.

    return (void *)((pt[ptindex] & ~0xFFF) + ((unsigned long)virt_addr & 0xFFF));
}

void mmu_map_page(mmu_page_dir_t * dir, void * phys_addr, void * virtual_addr, unsigned int flags) {
    if (mmu_paging_enabled())
        KERNEL_PANIC("Tried to map page to virtual address with paging enabled");

    // Make sure that both addresses are page-aligned.

    unsigned long pdindex = (unsigned long)virtual_addr >> 22;
    unsigned long ptindex = (unsigned long)virtual_addr >> 12 & 0x03FF;

    // TODO THIS NEEDS TO BE WHERE THE PAGE DIR GETS MAPPED TO IN PAGING
    unsigned long * pd = (unsigned long *)dir;
    // Here you need to check whether the PD entry is present.
    // When it is not present, you need to create a new empty PT and
    // adjust the PDE accordingly.

    // TODO THIS NEEDS TO BE WHERE THE PAGE TABLE GETS MAPPED TO IN PAGING
    unsigned long * pt = ((unsigned long *)0xFFC00000) + (0x400 * pdindex);
    // Here you need to check whether the PT entry is present.
    // When it is, then there is already a mapping present. What do you do now?

    pt[ptindex] = ((unsigned long)phys_addr) | (flags & 0xFFF) | 0x01; // Present

    // Now you need to flush the entry in the TLB
    // or you might not notice the change.
}
