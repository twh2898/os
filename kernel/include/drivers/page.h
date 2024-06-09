#ifndef PAGE_H
#define PAGE_H

void init_pages();

/*
 Bits 12 - 31 of memory address (ie. page aligned pointer)
 Same as found in Page Table entry.
 0 is error
 */
void * page_alloc();

/*
 Bits 12 - 31 of memory address (ie. page aligned pointer)
 Same as found in Page Table entry.
 0 is error
 */
void page_free(void * addr);

#endif // PAGE_H
