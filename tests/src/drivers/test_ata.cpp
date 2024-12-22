#include <cstdlib>
#include <cstring>

#include "test_header.h"

extern "C" {
#include "drivers/_ata/defs.h"
#include "drivers/ata.h"

FAKE_VALUE_FUNC(bool, drv_ata_identify, drv_ata_t *);
FAKE_VOID_FUNC(drv_ata_software_reset, drv_ata_t *);
FAKE_VALUE_FUNC(bool, drv_ata_status, drv_ata_t *);

FAKE_VALUE_FUNC(int, drv_ata_sect_read, drv_ata_t *, size_t, uint32_t);
FAKE_VALUE_FUNC(int, drv_ata_sect_write, drv_ata_t *, size_t, uint32_t);

FAKE_VALUE_FUNC(void *, kmemcpy, void *, const void *, size_t);
FAKE_VALUE_FUNC(void *, kmalloc, size_t);
FAKE_VOID_FUNC(kfree, void *);
FAKE_VALUE_FUNC(int, register_driver, driver_register_t *);

bool drv_ata_identify_custom(drv_ata_t * device) {
    device->sect_count = 3;
    return true;
}
}

class ATA : public testing::Test {
protected:
    void SetUp() override {
        RESET_FAKE(drv_ata_identify);
        RESET_FAKE(drv_ata_software_reset);
        RESET_FAKE(drv_ata_status);

        RESET_FAKE(kmemcpy);
        RESET_FAKE(kmalloc);
        RESET_FAKE(kfree);

        kmemcpy_fake.custom_fake          = memcpy;
        kmalloc_fake.custom_fake          = malloc;
        kfree_fake.custom_fake            = free;
        drv_ata_identify_fake.custom_fake = drv_ata_identify_custom;

        drv_ata_init();

        RESET_FAKE(register_driver);
    }
};

TEST_F(ATA, drv_ata_init) {
    register_driver_fake.return_val = 0;

    int res = drv_ata_init();

    EXPECT_GE(0, res);

    EXPECT_EQ(1, register_driver_fake.call_count);
    driver_register_t * reg = register_driver_fake.arg0_val;

    ASSERT_NE(nullptr, reg);
    EXPECT_EQ(DRIVER_DEVICE_TYPE_DISK, reg->type);

    EXPECT_NE(nullptr, reg->disk.fn_open);
    EXPECT_NE(nullptr, reg->disk.fn_close);
    EXPECT_NE(nullptr, reg->disk.fn_stat);
    EXPECT_NE(nullptr, reg->disk.fn_read);
    EXPECT_NE(nullptr, reg->disk.fn_write);

    // TODO test register_interrupt_handler called?
}

TEST_F(ATA, drv_ata_open) {
    // Bad ID
    EXPECT_EQ(nullptr, drv_ata_open(-1));

    kmalloc_fake.custom_fake = 0;
    kfree_fake.custom_fake   = 0;
    kmalloc_fake.return_val  = 0;

    // First malloc fail
    EXPECT_EQ(0, drv_ata_open(0));
    EXPECT_EQ(1, kmalloc_fake.call_count);
    EXPECT_EQ(sizeof(drv_ata_t), kmalloc_fake.arg0_val);

    SetUp();

    drv_ata_identify_fake.custom_fake = 0;
    drv_ata_identify_fake.return_val  = false;

    // ATA Identity Fails
    EXPECT_EQ(0, drv_ata_open(0));
    EXPECT_EQ(1, kmalloc_fake.call_count);
    EXPECT_EQ(1, drv_ata_identify_fake.call_count);
    // TODO verify this is done

    SetUp();

    void * dev = malloc(sizeof(drv_ata_t));

    void * kmalloc_returns[2] = {dev, 0};

    kmalloc_fake.custom_fake = 0;
    SET_RETURN_SEQ(kmalloc, kmalloc_returns, 2);

    // Second malloc fails
    EXPECT_EQ(0, drv_ata_open(0));
    EXPECT_EQ(2, kmalloc_fake.call_count);
    EXPECT_EQ(sizeof(driver_disk_t), kmalloc_fake.arg0_val);
    EXPECT_EQ(1, kfree_fake.call_count);
    EXPECT_EQ(dev, kfree_fake.arg0_val);

    SetUp();

    // Good
    driver_disk_t * disk = drv_ata_open(0);
    EXPECT_EQ(2, kmalloc_fake.call_count);
    EXPECT_EQ(1, drv_ata_identify_fake.call_count);

    EXPECT_NE(nullptr, drv_ata_identify_fake.arg0_val);

    ASSERT_NE(nullptr, disk);

    EXPECT_EQ(0, disk->id);
    EXPECT_EQ(3 * ATA_SECTOR_BYTES, disk->stat.size);
    EXPECT_EQ(DRIVER_DISK_STATE_IDLE, disk->stat.state);
    ASSERT_NE(nullptr, disk->drv_data);

    drv_ata_t * device = (drv_ata_t *)disk->drv_data;
    EXPECT_EQ(ATA_BUS_0_IO_BASE, device->io_base);
    EXPECT_EQ(ATA_BUS_0_CTL_BASE, device->ct_base);
    EXPECT_EQ(3, device->sect_count);

    // // disk already opened above
    // EXPECT_EQ(nullptr, drv_ata_open(0));
}

class ATADevice : public ATA {
protected:
    driver_disk_t * disk;

    void SetUp() override {
        ATA::SetUp();

        disk = drv_ata_open(0);

        ASSERT_NE(nullptr, disk);

        RESET_FAKE(kmalloc);
        kmalloc_fake.custom_fake = malloc;
    }
};

TEST_F(ATADevice, drv_ata_close) {
    // Null disk
    EXPECT_EQ(-1, drv_ata_close(0));

    disk->drv_data = 0;

    // No device to free
    EXPECT_EQ(0, drv_ata_close(disk));
    EXPECT_EQ(1, kfree_fake.call_count);
    EXPECT_EQ(disk, kfree_fake.arg0_val);

    SetUp();

    // Good
    EXPECT_EQ(0, drv_ata_close(disk));
    EXPECT_EQ(2, kfree_fake.call_count);
    EXPECT_EQ(disk->drv_data, kfree_fake.arg0_history[0]);
    EXPECT_EQ(disk, kfree_fake.arg0_history[1]);
}

TEST_F(ATADevice, drv_ata_stat) {
    disk_stat_t stat = {.size = 0, .state = DRIVER_DISK_STATE_CLOSED};

    // Null disk or Null stat
    EXPECT_EQ(-1, drv_ata_stat(0, 0));
    EXPECT_EQ(-1, drv_ata_stat(disk, 0));
    EXPECT_EQ(-1, drv_ata_stat(0, &stat));

    // kmemcpy fail
    kmemcpy_fake.custom_fake = 0;
    kmemcpy_fake.return_val  = 0;

    EXPECT_EQ(-1, drv_ata_stat(disk, &stat));

    SetUp();

    // Good
    EXPECT_GE(0, drv_ata_stat(disk, &stat));
    EXPECT_EQ(1, kmemcpy_fake.call_count);
    EXPECT_EQ(kmemcpy_fake.arg0_val, &stat);
    EXPECT_EQ(kmemcpy_fake.arg1_val, &disk->stat);
    EXPECT_EQ(kmemcpy_fake.arg2_val, sizeof(disk_stat_t));

    EXPECT_EQ(disk->stat.size, stat.size);
    EXPECT_EQ(disk->stat.state, stat.state);
}

TEST_F(ATADevice, drv_ata_read) {
    char buff[3];

    // Bad parameter
    EXPECT_EQ(-1, drv_ata_read(0, 0, 0, 0));
    EXPECT_EQ(-1, drv_ata_read(0, buff, 3, 0));
    EXPECT_EQ(-1, drv_ata_read(disk, 0, 3, 0));

    // Bad id
    disk->id = -1;
    EXPECT_EQ(-1, drv_ata_read(disk, buff, 3, 0));

    disk->id = 1;
    EXPECT_EQ(-1, drv_ata_read(disk, buff, 3, 0));

    EXPECT_EQ(0, kmemcpy_fake.call_count);

    disk->id = 0;

    // Zero count
    EXPECT_EQ(0, drv_ata_read(disk, buff, 0, 0));
    EXPECT_EQ(0, drv_ata_read(disk, buff, 0, 1));
    EXPECT_EQ(0, drv_ata_read(disk, buff, 0, 2));

    EXPECT_EQ(0, kmemcpy_fake.call_count);

    kmemcpy_fake.return_val = (void *)1;

    // Good
    EXPECT_EQ(3, drv_ata_read(disk, buff, 3, 0));
    EXPECT_EQ(1, kmemcpy_fake.call_count);
    EXPECT_EQ(buff, kmemcpy_fake.arg0_val);
    EXPECT_NE(nullptr, kmemcpy_fake.arg1_val);
    EXPECT_EQ(3, kmemcpy_fake.arg2_val);

    SetUp();

    kmemcpy_fake.return_val = (void *)1;

    // Address
    EXPECT_EQ(2, drv_ata_read(disk, buff, 3, 1));
    EXPECT_EQ(1, kmemcpy_fake.call_count);
    EXPECT_EQ(buff, kmemcpy_fake.arg0_val);
    EXPECT_NE(nullptr, kmemcpy_fake.arg1_val);
    EXPECT_EQ(2, kmemcpy_fake.arg2_val);

    EXPECT_EQ(1, drv_ata_read(disk, buff, 3, 2));
    EXPECT_EQ(2, kmemcpy_fake.call_count);
    EXPECT_EQ(buff, kmemcpy_fake.arg0_val);
    EXPECT_NE(nullptr, kmemcpy_fake.arg1_val);
    EXPECT_EQ(1, kmemcpy_fake.arg2_val);

    EXPECT_EQ(0, drv_ata_read(disk, buff, 3, 3));
    EXPECT_EQ(2, kmemcpy_fake.call_count);

    EXPECT_EQ(0, drv_ata_read(disk, buff, 3, 4));
    EXPECT_EQ(2, kmemcpy_fake.call_count);

    SetUp();

    kmemcpy_fake.return_val = 0;

    // Fail kmemcpy
    EXPECT_EQ(-1, drv_ata_read(disk, buff, 3, 0));
    EXPECT_EQ(1, kmemcpy_fake.call_count);
}

TEST_F(ATADevice, drv_ata_write) {
    char buff[3];

    // Bad args
    EXPECT_EQ(-1, drv_ata_write(0, 0, 1, 0));
    EXPECT_EQ(-1, drv_ata_write(0, buff, 1, 0));
    EXPECT_EQ(-1, drv_ata_write(disk, 0, 1, 0));

    // Bad id
    disk->id = -1;
    EXPECT_EQ(-1, drv_ata_write(disk, buff, 1, 0));

    disk->id = 1;
    EXPECT_EQ(-1, drv_ata_write(disk, buff, 1, 0));

    EXPECT_EQ(0, kmemcpy_fake.call_count);

    disk->id = 0;

    // Zero count
    EXPECT_EQ(0, drv_ata_write(disk, buff, 0, 0));
    EXPECT_EQ(0, drv_ata_write(disk, buff, 0, 1));
    EXPECT_EQ(0, drv_ata_write(disk, buff, 0, 2));

    EXPECT_EQ(0, kmemcpy_fake.call_count);

    kmemcpy_fake.return_val = (void *)1;

    // Good
    EXPECT_EQ(3, drv_ata_write(disk, buff, 3, 0));
    EXPECT_EQ(1, kmemcpy_fake.call_count);
    EXPECT_NE(nullptr, kmemcpy_fake.arg0_val);
    EXPECT_EQ(buff, kmemcpy_fake.arg1_val);
    EXPECT_EQ(3, kmemcpy_fake.arg2_val);

    // Address
    EXPECT_EQ(2, drv_ata_write(disk, buff, 3, 1));
    EXPECT_EQ(2, kmemcpy_fake.call_count);
    EXPECT_NE(nullptr, kmemcpy_fake.arg0_val);
    EXPECT_EQ(buff, kmemcpy_fake.arg1_val);
    EXPECT_EQ(2, kmemcpy_fake.arg2_val);

    EXPECT_EQ(1, drv_ata_write(disk, buff, 3, 2));
    EXPECT_EQ(3, kmemcpy_fake.call_count);
    EXPECT_NE(nullptr, kmemcpy_fake.arg0_val);
    EXPECT_EQ(buff, kmemcpy_fake.arg1_val);
    EXPECT_EQ(1, kmemcpy_fake.arg2_val);

    EXPECT_EQ(0, drv_ata_write(disk, buff, 3, 3));
    EXPECT_EQ(3, kmemcpy_fake.call_count);

    EXPECT_EQ(0, drv_ata_write(disk, buff, 3, 4));
    EXPECT_EQ(3, kmemcpy_fake.call_count);

    SetUp();

    kmemcpy_fake.return_val = 0;

    // Fail kmemcpy
    EXPECT_EQ(-1, drv_ata_write(disk, buff, 3, 0));
    EXPECT_EQ(1, kmemcpy_fake.call_count);
}
