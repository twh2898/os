#include <array>
#include <cstdlib>

#include "test_common.h"

#define PAGE_SIZE 4096

#define MAGIC_FREE 0x46524545
#define MAGIC_USED 0x55534544

#define PAGE_COUNT_MAX 5

std::array<char, PAGE_SIZE * PAGE_COUNT_MAX> pages;
size_t                                       page_count;

extern "C" {
#include "memory_alloc.h"

FAKE_VALUE_FUNC(void *, alloc_page, size_t);
}

class MemoryAlloc : public ::testing::Test {
protected:
    memory_entry_t * entry_1;
    memory_entry_t * entry_2;
    memory_entry_t * entry_3;

    memory_t mem;

    void SetUp() override {
        init_mocks();

        RESET_FAKE(alloc_page);

        pages.fill(0);

        entry_1 = (memory_entry_t *)(pages.data());
        entry_2 = (memory_entry_t *)(pages.data() + PAGE_SIZE);
        entry_3 = (memory_entry_t *)(pages.data() + PAGE_SIZE * 2);

        entry_1->magic = MAGIC_FREE;
        entry_1->size  = PAGE_SIZE - sizeof(memory_entry_t);
        entry_1->prev  = 0;
        entry_1->next  = entry_2;

        entry_2->magic = MAGIC_FREE;
        entry_2->size  = PAGE_SIZE - sizeof(memory_entry_t);
        entry_2->prev  = entry_1;
        entry_2->next  = entry_3;

        entry_3->magic = MAGIC_FREE;
        entry_3->size  = PAGE_SIZE - sizeof(memory_entry_t);
        entry_3->prev  = entry_2;
        entry_3->next  = 0;

        mem.first          = entry_1;
        mem.last           = entry_3;
        mem.alloc_pages_fn = alloc_page;

        ASSERT_EQ(pages.data(), (void *)entry_1);
        SCOPED_TRACE("SetUp");
        expect_memory_joined();
    }

    void expect_memory_joined() {
        memory_entry_t * entry = mem.first;

        size_t i = 0;

        while (entry->next) {
            memory_entry_t * expect_next = (memory_entry_t *)((uint32_t)entry + entry->size + sizeof(memory_entry_t));
            EXPECT_EQ(expect_next, entry->next) << "Entry " << i;

            i++;
            entry = entry->next;
        }

        EXPECT_EQ(mem.last, entry);
    }
};

TEST_F(MemoryAlloc, memory_init) {
    EXPECT_NE(0, memory_init(&mem, 0));
    EXPECT_NE(0, memory_init(0, alloc_page));

    alloc_page_fake.return_val = 0;

    EXPECT_NE(0, memory_init(&mem, alloc_page));
    EXPECT_EQ(1, alloc_page_fake.call_count);
    EXPECT_EQ(1, alloc_page_fake.arg0_val);

    alloc_page_fake.return_val = pages.data();
    pages.fill(0);

    EXPECT_EQ(0, memory_init(&mem, alloc_page));
    EXPECT_EQ(entry_1, mem.first);
    EXPECT_EQ(entry_1, mem.last);

    EXPECT_EQ(2, alloc_page_fake.call_count);
    EXPECT_EQ(1, alloc_page_fake.arg0_val);
    EXPECT_EQ(MAGIC_FREE, entry_1->magic);
    EXPECT_EQ(PAGE_SIZE - sizeof(memory_entry_t), entry_1->size);
    EXPECT_EQ(nullptr, entry_1->next);
    EXPECT_EQ(nullptr, entry_1->prev);
}

TEST_F(MemoryAlloc, memory_alloc_Success) {
    EXPECT_EQ(nullptr, memory_alloc(&mem, 0));
    EXPECT_EQ(nullptr, memory_alloc(0, 1));

    // No size change
    EXPECT_EQ(ENTRY_PTR(entry_1), memory_alloc(&mem, PAGE_SIZE - sizeof(memory_entry_t)));
    EXPECT_EQ(MAGIC_USED, entry_1->magic);
    EXPECT_EQ(PAGE_SIZE - sizeof(memory_entry_t), entry_1->size);

    EXPECT_EQ(ENTRY_PTR(entry_2), memory_alloc(&mem, PAGE_SIZE - sizeof(memory_entry_t)));
    EXPECT_EQ(MAGIC_USED, entry_2->magic);
    EXPECT_EQ(PAGE_SIZE - sizeof(memory_entry_t), entry_2->size);
}

TEST_F(MemoryAlloc, memory_alloc_SmallerThanFreeEntry) {
    EXPECT_EQ(ENTRY_PTR(entry_1), memory_alloc(&mem, 1));
    EXPECT_EQ(MAGIC_USED, entry_1->magic);
    EXPECT_EQ(4, entry_1->size);

    EXPECT_NE(nullptr, entry_1->next);
    EXPECT_NE(entry_2, entry_1->next);

    memory_entry_t * entry_1_5 = entry_1->next;
    EXPECT_EQ(MAGIC_FREE, entry_1_5->magic);
    EXPECT_EQ(entry_2, entry_1_5->next);
    EXPECT_EQ(entry_1, entry_1_5->prev);
    EXPECT_EQ(PAGE_SIZE - sizeof(memory_entry_t) * 2 - 4, entry_1_5->size);
    expect_memory_joined();
}

TEST_F(MemoryAlloc, memory_alloc_LargerThanFreeEntry_Combine) {
    EXPECT_EQ(ENTRY_PTR(entry_1), memory_alloc(&mem, PAGE_SIZE));
    EXPECT_EQ(MAGIC_USED, entry_1->magic);
    EXPECT_EQ(PAGE_SIZE, entry_1->size);

    memory_entry_t * entry_1_5 = entry_1->next;
    EXPECT_EQ(MAGIC_FREE, entry_1_5->magic);
    EXPECT_EQ(entry_3, entry_1_5->next);
    EXPECT_EQ(entry_1, entry_1_5->prev);
    EXPECT_EQ(entry_1_5, entry_3->prev);
    EXPECT_EQ(entry_1_5, entry_1->next);
    EXPECT_EQ(PAGE_SIZE - sizeof(memory_entry_t) * 2, entry_1_5->size);
    expect_memory_joined();
}

TEST_F(MemoryAlloc, memory_alloc_NeedMore_LastFree) {
    entry_1->magic = MAGIC_USED;
    entry_2->magic = MAGIC_USED;

    alloc_page_fake.return_val = 0;

    // Needs more memory but fails (last entry free)
    EXPECT_EQ(nullptr, memory_alloc(&mem, PAGE_SIZE));

    alloc_page_fake.return_val = pages.data() + PAGE_SIZE * 3;

    // Needs more memory (last entry free)
    EXPECT_EQ(ENTRY_PTR(entry_3), memory_alloc(&mem, PAGE_SIZE));
    EXPECT_EQ(PAGE_SIZE, entry_3->size);
    EXPECT_EQ(2, alloc_page_fake.call_count);
    EXPECT_EQ(1, alloc_page_fake.arg0_val);

    memory_entry_t * entry_4 = entry_3->next;

    EXPECT_EQ(entry_4, mem.last);
    EXPECT_EQ(entry_3, entry_4->prev);
    EXPECT_EQ(entry_4, entry_3->next);
    EXPECT_EQ(PAGE_SIZE - sizeof(memory_entry_t) * 2, entry_4->size);
    expect_memory_joined();
}

TEST_F(MemoryAlloc, memory_alloc_NeedMore_LastUsed) {
    entry_1->magic = MAGIC_USED;
    entry_2->magic = MAGIC_USED;
    entry_3->magic = MAGIC_USED;

    alloc_page_fake.return_val = 0;

    // Needs more memory but fails (last entry used)
    EXPECT_EQ(nullptr, memory_alloc(&mem, PAGE_SIZE));

    alloc_page_fake.return_val = pages.data() + PAGE_SIZE * 3;

    memory_entry_t * entry_4 = (memory_entry_t *)(pages.data() + PAGE_SIZE * 3);

    // Needs more memory (last entry used)
    EXPECT_EQ(ENTRY_PTR(entry_4), memory_alloc(&mem, PAGE_SIZE));
    EXPECT_EQ(2, alloc_page_fake.call_count);
    EXPECT_EQ(2, alloc_page_fake.arg0_val);

    EXPECT_EQ(MAGIC_USED, entry_4->magic);
    EXPECT_EQ(PAGE_SIZE, entry_4->size);
    EXPECT_EQ(entry_3, entry_4->prev);
    EXPECT_EQ(entry_4, entry_3->next);

    memory_entry_t * entry_5 = entry_4->next;

    EXPECT_EQ(MAGIC_FREE, entry_5->magic);
    EXPECT_EQ(PAGE_SIZE - sizeof(memory_entry_t) * 2, entry_5->size);
    EXPECT_EQ(entry_4, entry_5->prev);
    EXPECT_EQ(nullptr, entry_5->next);
    EXPECT_EQ(entry_5, mem.last);
    expect_memory_joined();
}

TEST_F(MemoryAlloc, memory_realloc) {
    EXPECT_EQ(nullptr, memory_realloc(&mem, ENTRY_PTR(entry_1), 0));
    EXPECT_EQ(nullptr, memory_realloc(&mem, 0, 2));
    EXPECT_EQ(nullptr, memory_realloc(0, ENTRY_PTR(entry_1), 2));

    // Not free
    EXPECT_EQ(nullptr, memory_realloc(&mem, ENTRY_PTR(entry_1), 2));

    // Not found
    EXPECT_EQ(nullptr, memory_realloc(&mem, (void *)4, 2));

    // Not aligned
    EXPECT_EQ(nullptr, memory_realloc(&mem, (void *)1, 2));

    // TODO success

    // entry_1->magic = MAGIC_USED;

    // void * expect_ptr = ENTRY_PTR(entry_1);

    // // Size does not change
    // EXPECT_EQ(expect_ptr, memory_realloc(&mem, ENTRY_PTR(entry_1), entry_1->size));

    // // Size is less
    // EXPECT_EQ(expect_ptr, memory_realloc(&mem, ENTRY_PTR(entry_1), entry_1->size - 8));
    // EXPECT_EQ(PAGE_SIZE - sizeof(memory_entry_t) - 4, entry_1->size);
    // EXPECT_EQ(MAGIC_USED, entry_1->magic);

    // memory_entry_t * entry_1_5 = (memory_entry_t *)(pages.data() + PAGE_SIZE - sizeof(memory_entry_t) * 2 - 8);

    // EXPECT_EQ(MAGIC_FREE, entry_1_5->magic);
    // EXPECT_EQ(4, entry_1_5->size);
    // EXPECT_EQ(entry_1_5, entry_1->next);
    // EXPECT_EQ(entry_1, entry_1_5->next);
    // EXPECT_EQ(entry_2, entry_1_5->prev);
    // EXPECT_EQ(entry_1_5, entry_2->prev);

    // // Size is more (next is free)
    // EXPECT_EQ(expect_ptr, memory_realloc(&mem, ENTRY_PTR(entry_1), entry_1->size + 8));
    // EXPECT_EQ(PAGE_SIZE - sizeof(memory_entry_t) - 4, entry_1->size);
    // EXPECT_EQ(MAGIC_USED, entry_1->magic);
    // EXPECT_EQ(entry_2, entry_1->next);
    // EXPECT_EQ(entry_1, entry_2->prev);

    // // Next is free (too small)
    // EXPECT_EQ(ENTRY_PTR(entry_1), memory_realloc(&mem, ENTRY_PTR(entry_1), PAGE_SIZE * 2));
    // EXPECT_EQ(PAGE_SIZE * 2, entry_1->size);

    // entry_1_5 = (memory_entry_t *)(pages.data() + PAGE_SIZE * 2 + sizeof(memory_entry_t));
    // EXPECT_EQ(entry_1_5, entry_1->next);
    // EXPECT_EQ(entry_1, entry_1_5->prev);
    // EXPECT_EQ(nullptr, entry_1_5->prev);
    // EXPECT_EQ(entry_1_5, mem.last);

    /*
    - Next is free (too small)
    - Next is free (good size)
    - Next is free (too big)
    - Add memory (last is free)
    - Add memory(last is not free)
    */
}

TEST_F(MemoryAlloc, memory_free) {
    entry_2->magic = MAGIC_USED;

    EXPECT_NE(0, memory_free(&mem, 0));
    EXPECT_NE(0, memory_free(0, ENTRY_PTR(entry_2)));

    // Already free
    EXPECT_NE(0, memory_free(0, ENTRY_PTR(entry_1)));

    // Not Aligned
    EXPECT_NE(0, memory_free(&mem, (void *)3));

    EXPECT_EQ(0, memory_free(&mem, ENTRY_PTR(entry_2)));
    EXPECT_EQ(MAGIC_FREE, entry_2->magic);

    // Does not exist
    EXPECT_NE(0, memory_free(&mem, (void *)0x1000));
}
