#include "libc/intern/page.h"

static void set_assigned(page_t * page);

void page_init(page_t * page, uint32_t index) {
    page->data = index & 0xfffff;
    page->size = 0;
}

uint32_t page_address(const page_t * page) {
    return page->data << 12;
}

uint32_t page_index(const page_t * page) {
    return page->data & 0xfffff;
}

void page_set_index(page_t * page, uint32_t index) {
    page->data = (page->data & 0xfff00000) | (index & 0xfffff);
    set_assigned(page);
}

void page_set_address(page_t * page, uint32_t address) {
    page->data = (page->data & 0xfff00000) | ((address >> 12) & 0xfffff);
    set_assigned(page);
}

bool page_used(const page_t * page) {
    return (page->data >> 20) & 1;
}

void page_set_used(page_t * page) {
    page->data |= PAGE_FLAG_USED;
}

void page_set_free(page_t * page) {
    page->data &= ~PAGE_FLAG_USED;
}

static void set_assigned(page_t * page) {
    page->data |= PAGE_FLAG_ASSIGNED;
}

bool page_assigned(const page_t * page) {
    return (page->data >> 21) & 1;
}
