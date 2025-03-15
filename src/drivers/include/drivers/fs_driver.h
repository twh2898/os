#ifndef FS_DRIVER_H
#define FS_DRIVER_H

#include <stddef.h>

enum FS_SEEK_ORIGIN {
    FS_SEEK_ORIGIN_START,
    FS_SEEK_ORIGIN_CURRENT,
    FS_SEEK_ORIGIN_END,
};

typedef size_t (*driver_fs_read_t)(void * ptr, char * buff, size_t count);
typedef size_t (*driver_fs_write_t)(void * ptr, const char * buff, size_t count);
typedef size_t (*driver_fs_seek_t)(void * ptr, size_t pos, int origin);
typedef size_t (*driver_fs_tell_t)(void * ptr);

typedef struct _fs_driver {
    const char * name;

    driver_fs_read_t  read_fn;
    driver_fs_write_t write_fn;
    driver_fs_seek_t  seek_fn;
    driver_fs_tell_t  tell_fn;
} fs_driver_t;

typedef struct _fs_device {
    const char * path;

    fs_driver_t * driver;
    void *        driver_data;
} fs_device_t;

typedef struct _fs_file {
    const char * path;

    fs_device_t * device;
    void *        file_data;
} fs_file_t;

void init_fs_drivers();

int register_fs_driver(const char * name, fs_driver_t * driver);
int register_fs_device(const char * path, void * driver_data, const char * driver_name);

fs_driver_t * get_fs_driver(const char * name);
fs_device_t * get_fs_device(const char * path);

size_t fs_file_read(fs_file_t * device, char * buff, size_t count);
size_t fs_file_write(fs_file_t * device, const char * buff, size_t count);
size_t fs_file_seek(fs_file_t * device, size_t pos, int origin);
size_t fs_file_tell(fs_file_t * device);

#endif // FS_DRIVER_H
