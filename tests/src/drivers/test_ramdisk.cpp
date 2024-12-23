#include <cstdlib>
#include <cstring>

#include "test_header.h"

extern "C" {
#include "drivers/ramdisk.h"

FAKE_VALUE_FUNC(void *, kmemcpy, void *, const void *, size_t);
FAKE_VALUE_FUNC(void *, kmalloc, size_t);
FAKE_VOID_FUNC(kfree, void *);
FAKE_VALUE_FUNC(int, register_driver, driver_register_t *);
}

class Ramdisk : public testing::Test {
protected:
    void SetUp() override {
        RESET_FAKE(kmemcpy);
        RESET_FAKE(kmalloc);
        RESET_FAKE(kfree);

        kmemcpy_fake.custom_fake = memcpy;
        kmalloc_fake.custom_fake = malloc;
        kfree_fake.custom_fake   = free;

        drv_ramdisk_init();

        RESET_FAKE(register_driver);
    }
};

TEST_F(Ramdisk, drv_ramdisk_init) {
    register_driver_fake.return_val = 0;

    int res = drv_ramdisk_init();

    EXPECT_EQ(0, res);

    EXPECT_EQ(1, register_driver_fake.call_count);
    driver_register_t * reg = register_driver_fake.arg0_val;

    ASSERT_NE(nullptr, reg);
    EXPECT_EQ(DRIVER_DEVICE_TYPE_DISK, reg->type);

    EXPECT_NE(nullptr, reg->disk.fn_open);
    EXPECT_NE(nullptr, reg->disk.fn_close);
    EXPECT_NE(nullptr, reg->disk.fn_stat);
    EXPECT_NE(nullptr, reg->disk.fn_read);
    EXPECT_NE(nullptr, reg->disk.fn_write);
}

TEST_F(Ramdisk, drv_ramdisk_create_device) {
    EXPECT_EQ(0, drv_ramdisk_create_device(10));

    EXPECT_EQ(1, kmalloc_fake.call_count);
    EXPECT_EQ(10, kmalloc_fake.arg0_val);
}

TEST_F(Ramdisk, drv_ramdisk_create_device_no_mem) {
    kmalloc_fake.custom_fake = 0;
    kmalloc_fake.return_val  = 0;
    EXPECT_EQ(-1, drv_ramdisk_create_device(10));
    EXPECT_EQ(1, kmalloc_fake.call_count);
    EXPECT_EQ(10, kmalloc_fake.arg0_val);

    SetUp();

    for (int i = 0; i < DRV_RAMDISK_MAX_DEVICES; i++) {
        ASSERT_EQ(i, drv_ramdisk_create_device(10));
    }

    EXPECT_EQ(-1, drv_ramdisk_create_device(10));
    EXPECT_EQ(DRV_RAMDISK_MAX_DEVICES, kmalloc_fake.call_count);
}

TEST_F(Ramdisk, drv_ramdisk_open) {
    EXPECT_EQ(nullptr, drv_ramdisk_open(-1));
    EXPECT_EQ(nullptr, drv_ramdisk_open(0));
    EXPECT_EQ(nullptr, drv_ramdisk_open(DRV_RAMDISK_MAX_DEVICES));
    EXPECT_EQ(nullptr, drv_ramdisk_open(DRV_RAMDISK_MAX_DEVICES + 1));

    drv_ramdisk_create_device(10);

    driver_disk_t * disk = drv_ramdisk_open(0);
    ASSERT_NE(nullptr, disk);
    EXPECT_EQ(0, disk->id);
    EXPECT_EQ(10, disk->stat.size);
    EXPECT_EQ(DRIVER_DISK_STATE_IDLE, disk->stat.state);

    EXPECT_EQ(nullptr, drv_ramdisk_open(1));

    // disk already opened above
    EXPECT_EQ(nullptr, drv_ramdisk_open(0));
}

class RamdiskDevice : public Ramdisk {
protected:
    driver_disk_t * disk;

    void SetUp() override {
        Ramdisk::SetUp();

        drv_ramdisk_create_device(3);
        disk = drv_ramdisk_open(0);

        ASSERT_NE(nullptr, disk);

        RESET_FAKE(kmalloc);
        kmemcpy_fake.custom_fake = 0;
    }
};

TEST_F(RamdiskDevice, drv_ramdisk_close) {
    EXPECT_NE(0, drv_ramdisk_close(0));

    disk->id = -1;
    EXPECT_NE(0, drv_ramdisk_close(disk));
    EXPECT_EQ(1, kfree_fake.call_count);
    EXPECT_EQ(disk, kfree_fake.arg0_val);

    SetUp();

    disk->id = 1;
    EXPECT_NE(0, drv_ramdisk_close(disk));
    EXPECT_EQ(1, kfree_fake.call_count);
    EXPECT_EQ(disk, kfree_fake.arg0_val);

    SetUp();

    disk->id = DRV_RAMDISK_MAX_DEVICES;
    EXPECT_NE(0, drv_ramdisk_close(disk));
    EXPECT_EQ(1, kfree_fake.call_count);
    EXPECT_EQ(disk, kfree_fake.arg0_val);

    SetUp();

    EXPECT_EQ(0, drv_ramdisk_close(disk));
    EXPECT_EQ(1, kfree_fake.call_count);
    EXPECT_EQ(disk, kfree_fake.arg0_val);
}

TEST_F(RamdiskDevice, drv_ramdisk_stat) {
    driver_disk_stat_t stat;
    kmemcpy_fake.custom_fake = memcpy;

    EXPECT_NE(0, drv_ramdisk_stat(0, &stat));
    EXPECT_NE(0, drv_ramdisk_stat(disk, 0));

    EXPECT_EQ(0, drv_ramdisk_stat(disk, &stat));
    EXPECT_EQ(1, kmemcpy_fake.call_count);
    EXPECT_EQ(&stat, kmemcpy_fake.arg0_val);
    EXPECT_EQ(&disk->stat, kmemcpy_fake.arg1_val);
    EXPECT_EQ(sizeof(driver_disk_stat_t), kmemcpy_fake.arg2_val);

    kmemcpy_fake.custom_fake = 0;
    kmemcpy_fake.return_val  = 0;
    EXPECT_NE(0, drv_ramdisk_stat(disk, &stat));
    EXPECT_EQ(2, kmemcpy_fake.call_count);
}

TEST_F(RamdiskDevice, drv_ramdisk_read) {
    char buff[3];

    // Bad parameter
    EXPECT_EQ(-1, drv_ramdisk_read(0, 0, 0, 0));
    EXPECT_EQ(-1, drv_ramdisk_read(0, buff, 3, 0));
    EXPECT_EQ(-1, drv_ramdisk_read(disk, 0, 3, 0));

    // Bad id
    disk->id = -1;
    EXPECT_EQ(-1, drv_ramdisk_read(disk, buff, 3, 0));

    disk->id = 1;
    EXPECT_EQ(-1, drv_ramdisk_read(disk, buff, 3, 0));

    EXPECT_EQ(0, kmemcpy_fake.call_count);

    disk->id = 0;

    // Zero count
    EXPECT_EQ(0, drv_ramdisk_read(disk, buff, 0, 0));
    EXPECT_EQ(0, drv_ramdisk_read(disk, buff, 0, 1));
    EXPECT_EQ(0, drv_ramdisk_read(disk, buff, 0, 2));

    EXPECT_EQ(0, kmemcpy_fake.call_count);

    kmemcpy_fake.return_val = (void *)1;

    // Good
    EXPECT_EQ(3, drv_ramdisk_read(disk, buff, 3, 0));
    EXPECT_EQ(1, kmemcpy_fake.call_count);
    EXPECT_EQ(buff, kmemcpy_fake.arg0_val);
    EXPECT_NE(nullptr, kmemcpy_fake.arg1_val);
    EXPECT_EQ(3, kmemcpy_fake.arg2_val);

    SetUp();

    kmemcpy_fake.return_val = (void *)1;

    // Address
    EXPECT_EQ(2, drv_ramdisk_read(disk, buff, 3, 1));
    EXPECT_EQ(1, kmemcpy_fake.call_count);
    EXPECT_EQ(buff, kmemcpy_fake.arg0_val);
    EXPECT_NE(nullptr, kmemcpy_fake.arg1_val);
    EXPECT_EQ(2, kmemcpy_fake.arg2_val);

    EXPECT_EQ(1, drv_ramdisk_read(disk, buff, 3, 2));
    EXPECT_EQ(2, kmemcpy_fake.call_count);
    EXPECT_EQ(buff, kmemcpy_fake.arg0_val);
    EXPECT_NE(nullptr, kmemcpy_fake.arg1_val);
    EXPECT_EQ(1, kmemcpy_fake.arg2_val);

    EXPECT_EQ(0, drv_ramdisk_read(disk, buff, 3, 3));
    EXPECT_EQ(2, kmemcpy_fake.call_count);

    EXPECT_EQ(0, drv_ramdisk_read(disk, buff, 3, 4));
    EXPECT_EQ(2, kmemcpy_fake.call_count);

    SetUp();

    kmemcpy_fake.return_val = 0;

    // Fail kmemcpy
    EXPECT_EQ(-1, drv_ramdisk_read(disk, buff, 3, 0));
    EXPECT_EQ(1, kmemcpy_fake.call_count);
}

TEST_F(RamdiskDevice, drv_ramdisk_write) {
    char buff[3];

    // Bad args
    EXPECT_EQ(-1, drv_ramdisk_write(0, 0, 1, 0));
    EXPECT_EQ(-1, drv_ramdisk_write(0, buff, 1, 0));
    EXPECT_EQ(-1, drv_ramdisk_write(disk, 0, 1, 0));

    // Bad id
    disk->id = -1;
    EXPECT_EQ(-1, drv_ramdisk_write(disk, buff, 1, 0));

    disk->id = 1;
    EXPECT_EQ(-1, drv_ramdisk_write(disk, buff, 1, 0));

    EXPECT_EQ(0, kmemcpy_fake.call_count);

    disk->id = 0;

    // Zero count
    EXPECT_EQ(0, drv_ramdisk_write(disk, buff, 0, 0));
    EXPECT_EQ(0, drv_ramdisk_write(disk, buff, 0, 1));
    EXPECT_EQ(0, drv_ramdisk_write(disk, buff, 0, 2));

    EXPECT_EQ(0, kmemcpy_fake.call_count);

    kmemcpy_fake.return_val = (void *)1;

    // Good
    EXPECT_EQ(3, drv_ramdisk_write(disk, buff, 3, 0));
    EXPECT_EQ(1, kmemcpy_fake.call_count);
    EXPECT_NE(nullptr, kmemcpy_fake.arg0_val);
    EXPECT_EQ(buff, kmemcpy_fake.arg1_val);
    EXPECT_EQ(3, kmemcpy_fake.arg2_val);

    // Address
    EXPECT_EQ(2, drv_ramdisk_write(disk, buff, 3, 1));
    EXPECT_EQ(2, kmemcpy_fake.call_count);
    EXPECT_NE(nullptr, kmemcpy_fake.arg0_val);
    EXPECT_EQ(buff, kmemcpy_fake.arg1_val);
    EXPECT_EQ(2, kmemcpy_fake.arg2_val);

    EXPECT_EQ(1, drv_ramdisk_write(disk, buff, 3, 2));
    EXPECT_EQ(3, kmemcpy_fake.call_count);
    EXPECT_NE(nullptr, kmemcpy_fake.arg0_val);
    EXPECT_EQ(buff, kmemcpy_fake.arg1_val);
    EXPECT_EQ(1, kmemcpy_fake.arg2_val);

    EXPECT_EQ(0, drv_ramdisk_write(disk, buff, 3, 3));
    EXPECT_EQ(3, kmemcpy_fake.call_count);

    EXPECT_EQ(0, drv_ramdisk_write(disk, buff, 3, 4));
    EXPECT_EQ(3, kmemcpy_fake.call_count);

    SetUp();

    kmemcpy_fake.return_val = 0;

    // Fail kmemcpy
    EXPECT_EQ(-1, drv_ramdisk_write(disk, buff, 3, 0));
    EXPECT_EQ(1, kmemcpy_fake.call_count);
}
