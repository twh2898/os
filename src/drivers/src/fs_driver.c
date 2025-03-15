#include "drivers/fs_driver.h"

#include "libc/datastruct/array.h"
#include "libc/memory.h"
#include "libc/string.h"

arr_t drivers_array;
arr_t device_array;

static char * copy_string(const char * str);

void init_fs_drivers() {
    // TODO this is going to break as soon as device or driver 17 is added
    // all pointers will become invalid, if a device or driver is added while
    // a pointer is being used, the pointer will become invalid mid-use.
    arr_create(&drivers_array, 16, sizeof(fs_driver_t));
    arr_create(&device_array, 16, sizeof(fs_device_t));
}

int register_fs_driver(const char * id, fs_driver_t * driver) {
    // TODO remove this once pointer lifetimes are fixed
    if (arr_size(&drivers_array) >= 15) {
        return -1;
    }

    driver->name = copy_string(id);

    arr_insert(&drivers_array, arr_size(&drivers_array), driver); // makes a copy
}

int register_fs_device(const char * path, void * device_data, const char * driver_id) {
    // TODO remove this once pointer lifetimes are fixed
    if (arr_size(&device_array) >= 15) {
        return -1;
    }

    fs_driver_t * driver = get_fs_driver(driver_id);

    char * copy_path = copy_string(path);

    fs_device_t new_dev = {
        .path        = copy_path,
        .driver      = driver,
        .driver_data = device_data,
    };

    arr_insert(&device_array, arr_size(&device_array), &new_dev); // makes a copy
}

fs_driver_t * get_fs_driver(const char * name) {
    size_t driver_count = arr_size(&drivers_array);

    for (size_t i = 0; i < driver_count; i++) {
        fs_driver_t * driver = arr_at(&drivers_array, i);

        if (kstrcmp(driver->name, name) == 0) {
            return driver;
        }
    }

    return 0;
}

fs_device_t * get_fs_device(const char * path) {
    size_t device_count = arr_size(&device_array);

    for (size_t i = 0; i < device_count; i++) {
        fs_device_t * device = arr_at(&device_array, i);

        if (kstrcmp(device->path, path) == 0) {
            return device;
        }
    }

    return 0;
}

size_t fs_file_read(fs_file_t * file, char * buff, size_t count, size_t pos) {
    if (!file) {
        return 0;
    }

    return file->device->driver->read_fn(file->device->driver_data, buff, count, pos);
}

size_t fs_file_write(fs_file_t * device, const char * buff, size_t count, size_t pos) {
    if (!device || !device->driver->write_fn) {
        return 0;
    }

    return device->driver->write_fn(device->driver_data, buff, count, pos);
}

static char * copy_string(const char * str) {
    char * copy_str = kmalloc(kstrlen(str) + 1);
    if (copy_str) {
        kmemcpy(copy_str, str, kstrlen(str) + 1);
    }
    return copy_str;
}
