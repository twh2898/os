#include "drivers/_tar/defs.h"
#include "drivers/_tar/support.h"
#include "drivers/tar.h"

void * drv_fs_tar_file_open(driver_fs_t * fs, const char * path, const char * mode) {
}

int drv_fs_tar_file_close(void * file) {
}

int drv_fs_tar_file_read(void * file, char * buff, size_t count, size_t addr) {
}

int drv_fs_tar_file_write(void * file, const char * buff, size_t count, size_t addr) {
}

int drv_fs_tar_file_seek(void * file, size_t pos, enum DRV_FS_FILE_SEEK_ORIGIN origin) {
}

int drv_fs_tar_file_tell(void * file) {
}

int drv_fs_tar_file_stat(const char * path, driver_fs_file_stat_t * stat) {
}
