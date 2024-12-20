#ifndef DRIVER_DISK_H
#define DRIVER_DISK_H

#include <stddef.h>
#include <stdint.h>

typedef enum DRIVER_DISK_STATE {
    DRIVER_DISK_STATE_CLOSED,
    DRIVER_DISK_STATE_IDLE,
    DRIVER_DISK_STATE_WAITING,
} driver_disk_state_t;

typedef struct _driver_disk_stat {
    uint32_t               size;
    enum DRIVER_DISK_STATE state;
} disk_stat_t;

typedef struct _driver_disk {
    uint8_t                  id;
    struct _driver_disk_stat stat;
} driver_disk_t;

typedef driver_disk_t * (*driver_disk_fn_open)(int id);
typedef int (*driver_disk_fn_close)(driver_disk_t *);
typedef int (*driver_disk_fn_stat)(driver_disk_t *, disk_stat_t *);
typedef int (*driver_disk_fn_read)(driver_disk_t *, char * buff, size_t count, size_t addr);
typedef int (*driver_disk_fn_write)(driver_disk_t *, const char * buff, size_t count, size_t addr);

struct _driver_device_disk {
    driver_disk_fn_open  fn_open;
    driver_disk_fn_close fn_close;
    driver_disk_fn_stat  fn_stat;
    driver_disk_fn_read  fn_read;
    driver_disk_fn_write fn_write;
};

#endif // DRIVER_DISK_H
