#include "drivers/ramdisk.h"

#include "kernel.h"
#include "libc/mem.h"
#include "libc/string.h"

struct _ramdisk {
    uint32_t id;
    size_t size;
    void * data;
};

static ramdisk_t devices[RAMDISK_MAX];
static uint32_t device_count;

uint32_t ramdisk_create(size_t size) {
    if (device_count == RAMDISK_MAX)
        KERNEL_PANIC("TOO MANY RAM DISK DEVICES");

    void * data = malloc(size);
    if (!data)
        KERNEL_PANIC("RAMDISK OUT OF MEMORY");

    devices[device_count].id = device_count;
    devices[device_count].size = size;
    devices[device_count].data = data;

    return device_count++;
}

ramdisk_t * ramdisk_open(uint32_t id) {
    if (id >= device_count)
        return 0;
    return &devices[id];
}

void ramdisk_close(ramdisk_t * rdisk) {
    // Nothing to do here
}

size_t ramdisk_size(ramdisk_t * rdisk) {
    return rdisk->size;
}

size_t ramdisk_read(ramdisk_t * rdisk, uint8_t * buff, size_t count, size_t pos) {
    if (rdisk->size - pos < count)
        count = rdisk->size - pos;

    memcpy(rdisk->data + pos, buff, count);
    return count;
}

size_t ramdisk_write(ramdisk_t * rdisk, uint8_t * buff, size_t count, size_t pos) {
    if (rdisk->size - pos < count)
        count = rdisk->size - pos;

    memcpy(buff, rdisk->data + pos, count);
    return count;
}
