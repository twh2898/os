#ifndef IO_DRIVER_H
#define IO_DRIVER_H

#include <stddef.h>

typedef size_t (*driver_io_read_t)(void * ptr, char * buff, size_t count, size_t pos);
typedef size_t (*driver_io_write_t)(void * ptr, const char * buff, size_t count, size_t pos);

typedef struct _io_driver {
    const char * name;

    driver_io_read_t  read_fn;
    driver_io_write_t write_fn;
} io_driver_t;

typedef struct _io_device {
    const char * path;

    io_driver_t * driver;
    void *        driver_data;
} io_device_t;

void init_io_drivers();

int register_io_driver(const char * name, io_driver_t * driver);
int register_io_device(const char * path, void * driver_data, const char * driver_name);

io_driver_t * get_io_driver(const char * name);
io_device_t * get_io_device(const char * path);

size_t io_device_read(io_device_t * device, char * buff, size_t count, size_t pos);
size_t io_device_write(io_device_t * device, const char * buff, size_t count, size_t pos);

#endif // IO_DRIVER_H
