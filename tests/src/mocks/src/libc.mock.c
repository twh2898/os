#include <memory.h>
#include <stdlib.h>
#include <string.h>

#include "libc/memory.mock.h"
#include "libc/proc.mock.h"
#include "libc/string.mock.h"

// libc/memory.h

DEFINE_FAKE_VALUE_FUNC(void *, pmalloc, size_t);
DEFINE_FAKE_VALUE_FUNC(void *, prealloc, void *, size_t);
DEFINE_FAKE_VOID_FUNC(pfree, void *);

void reset_libc_memory_mock(void) {
    RESET_FAKE(pmalloc);
    RESET_FAKE(prealloc);
    RESET_FAKE(pfree);

    pmalloc_fake.custom_fake  = malloc;
    prealloc_fake.custom_fake = realloc;
    pfree_fake.custom_fake    = free;
}

// libc/proc.h

DEFINE_FAKE_VOID_FUNC(proc_exit, uint8_t);
DEFINE_FAKE_VOID_FUNC(proc_abort, uint8_t, const char *);
DEFINE_FAKE_VOID_FUNC(proc_panic, const char *, const char *, unsigned int);
DEFINE_FAKE_VOID_FUNC(set_next_pid, uint32_t);
DEFINE_FAKE_VOID_FUNC(queue_event, ebus_event_t *);
DEFINE_FAKE_VALUE_FUNC(int, pull_event, int, ebus_event_t *);
DEFINE_FAKE_VOID_FUNC(yield);

void reset_libc_proc_mock(void) {
    RESET_FAKE(proc_exit);
    RESET_FAKE(proc_abort);
    RESET_FAKE(proc_panic);
    RESET_FAKE(set_next_pid);
    RESET_FAKE(queue_event);
    RESET_FAKE(pull_event);
    RESET_FAKE(yield);
}

// libc/string.h

DEFINE_FAKE_VALUE_FUNC(int, kmemcmp, const void *, const void *, size_t);
DEFINE_FAKE_VALUE_FUNC(void *, kmemcpy, void *, const void *, size_t);
DEFINE_FAKE_VALUE_FUNC(void *, kmemmove, void *, const void *, size_t);
DEFINE_FAKE_VALUE_FUNC(void *, kmemset, void *, int, size_t);
DEFINE_FAKE_VALUE_FUNC(size_t, kstrlen, const char *);
DEFINE_FAKE_VALUE_FUNC(size_t, knstrlen, const char *, int);
DEFINE_FAKE_VALUE_FUNC(int, kstrcmp, const char *, const char *);
DEFINE_FAKE_VALUE_FUNC(char *, kstrfind, const char *, int);
DEFINE_FAKE_VALUE_FUNC(char *, kstrtok, char *, const char *);
DEFINE_FAKE_VALUE_FUNC(int, katoi, const char *);

size_t knstrlen_custom(const char * str, int max) {
    if (!str || max < 0) {
        return 0;
    }
    size_t count = 0;
    while (*str++ && count < max) {
        count++;
    }
    return count;
}

void reset_libc_string_mock() {
    RESET_FAKE(kmemcmp);
    RESET_FAKE(kmemcpy);
    RESET_FAKE(kmemmove);
    RESET_FAKE(kmemset);
    RESET_FAKE(kstrlen);
    RESET_FAKE(knstrlen);
    RESET_FAKE(kstrcmp);
    RESET_FAKE(kstrfind);
    RESET_FAKE(kstrtok);
    RESET_FAKE(katoi);

    kmemcmp_fake.custom_fake  = memcmp;
    kmemcpy_fake.custom_fake  = memcpy;
    kmemmove_fake.custom_fake = memmove;
    kmemset_fake.custom_fake  = memset;
    kstrlen_fake.custom_fake  = strlen;
    knstrlen_fake.custom_fake = knstrlen_custom;
    kstrcmp_fake.custom_fake  = strcmp;
    kstrfind_fake.custom_fake = strchr;
    kstrtok_fake.custom_fake  = strtok;
    katoi_fake.custom_fake    = atoi;
}
