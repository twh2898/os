#ifndef DRIVER_FS_TAR_H
#define DRIVER_FS_TAR_H

#include <stddef.h>

#include "driver.h"

/**
 * @brief Register tar filesystem driver.
 *
 * @return int 0 for success
 */
int drv_fs_tar_init();

/**
 * @brief Open ramdisk device by id.
 *
 * @param disk disk driver object
 * @return driver_fs_t* filesystem driver object
 */
driver_fs_t * drv_fs_tar_open(driver_disk_t * disk);

/**
 * @brief Close filesystem.
 *
 * This does not close the underlying disk driver.
 *
 * @return int 0 for success
 */
int drv_fs_tar_close(driver_fs_t * fs);

/**
 * @brief Open a file for reading or writing.
 *
 * @param fs filesystem driver object
 * @param path path to file
 * @param mode file operation mode
 * @return driver_fs_file_t* file object or null
 */
driver_fs_file_t * drv_fs_tar_file_open(driver_fs_t * fs, const char * path, const char * mode);

/**
 * @brief Close a file object.
 *
 * @param file file object
 * @return int 0 = success
 */
int drv_fs_tar_file_close(driver_fs_file_t * file);
int drv_fs_tar_file_read(driver_fs_file_t * file, char * buff, size_t count);
int drv_fs_tar_file_write(driver_fs_file_t * file, const char * buff, size_t count);
int drv_fs_tar_file_seek(driver_fs_file_t * file, size_t pos, enum DRV_FS_FILE_SEEK_ORIGIN origin);
int drv_fs_tar_file_tell(driver_fs_file_t * file);
int drv_fs_tar_file_stat(driver_fs_t * fs, const char * path, driver_fs_file_stat_t * stat);

void *       drv_fs_tar_dir_open(driver_fs_t * fs, const char * path);
int          drv_fs_tar_dir_close(void * dir);
const char * drv_fs_tar_dir_read(void * dir);
int          drv_fs_tar_dir_seek(void * dir, size_t pos, enum DRV_FS_FILE_SEEK_ORIGIN origin);
int          drv_fs_tar_dir_tell(void * dir);

#endif // DRIVER_FS_TAR_H
