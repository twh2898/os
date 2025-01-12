#include <cstdlib>
#include <vector>

#include "test_common.h"

extern "C" {
#include "libc/circbuff.h"
// #include "libc/memory.h"
}

extern "C" {
FAKE_VALUE_FUNC(void *, kmalloc, size_t);
FAKE_VALUE_FUNC(void *, kcalloc, size_t, uint8_t);
FAKE_VALUE_FUNC(void *, krealloc, void *, size_t);
FAKE_VOID_FUNC(kfree, void *);

void * custom_kmalloc(size_t size) {
    return malloc(size);
}

void * custom_kcalloc(size_t size, uint8_t value) {
    return calloc(size, value);
}

void * custom_krealloc(void * ptr, size_t size) {
    return realloc(ptr, size);
}

void custom_kfree(void * ptr) {
    free(ptr);
}
}

static void setup_fakes() {
    RESET_FAKE(kmalloc);
    RESET_FAKE(kcalloc);
    RESET_FAKE(krealloc);
    RESET_FAKE(kfree);

    kmalloc_fake.custom_fake  = custom_kmalloc;
    kcalloc_fake.custom_fake  = custom_kcalloc;
    krealloc_fake.custom_fake = custom_krealloc;
    kfree_fake.custom_fake    = custom_kfree;
}

TEST(CircBuffStatic, test_circbuff_new_empty) {
    setup_fakes();

    circbuff_t * cbuff = circbuff_new(0);

    ASSERT_EQ(nullptr, cbuff);
    EXPECT_EQ(0, kmalloc_fake.call_count);
}

TEST(CircBuffStatic, test_circbuff_new) {
    setup_fakes();

    circbuff_t * cbuff = circbuff_new(4);

    ASSERT_NE(nullptr, cbuff);
    EXPECT_EQ(0, circbuff_len(cbuff));
    EXPECT_EQ(4, circbuff_buff_size(cbuff));

    ASSERT_EQ(2, kmalloc_fake.call_count);
    EXPECT_EQ(16, kmalloc_fake.arg0_history[0]);
    EXPECT_EQ(4, kmalloc_fake.arg0_history[1]);

    circbuff_free(cbuff);
}

TEST(CircBuffStatic, test_circbuff_free) {

    circbuff_t * cbuff = circbuff_new(4);

    setup_fakes();

    circbuff_free(cbuff);

    ASSERT_EQ(2, kfree_fake.call_count);
    EXPECT_EQ(cbuff, kfree_fake.arg0_history[1]);
}

class CircBuff : public testing::Test {
protected:
    circbuff_t * cbuff;
    size_t       cbuff_n;

    void SetUp() override {
        setup_fakes();

        cbuff_n = 4;
        cbuff   = circbuff_new(cbuff_n);

        ASSERT_NE(nullptr, cbuff);
        ASSERT_EQ(0, circbuff_len(cbuff));
        ASSERT_EQ(cbuff_n, circbuff_buff_size(cbuff));
    }

    void TearDown() override {
        circbuff_free(cbuff);
    }

    void expect_contents(std::vector<char> expect) {
        size_t actual_len = circbuff_len(cbuff);
        EXPECT_EQ(expect.size(), actual_len);
        for (size_t i = 0; i < cbuff_n; i++) {
            if (i < actual_len) {
                EXPECT_EQ(expect[i], circbuff_at(cbuff, i)) << "index " << i;
            }
            else {
                EXPECT_EQ(0, circbuff_at(cbuff, i)) << "index " << i;
            }
        }
    }

    void assert_contents(std::vector<char> expect) {
        size_t actual_len = circbuff_len(cbuff);
        ASSERT_EQ(expect.size(), actual_len);
        for (size_t i = 0; i < cbuff_n; i++) {
            if (i < actual_len) {
                ASSERT_EQ(expect[i], circbuff_at(cbuff, i)) << "index " << i;
            }
            else {
                ASSERT_EQ(0, circbuff_at(cbuff, i)) << "index " << i;
            }
        }
    }
};

TEST_F(CircBuff, test_circbuff_at) {
    expect_contents({});

    EXPECT_EQ(1, circbuff_push(cbuff, 1)); // [1, x, x]
    expect_contents({1});

    EXPECT_EQ(1, circbuff_push(cbuff, 2)); // [1, 2, x]
    expect_contents({1, 2});

    EXPECT_EQ(1, circbuff_pop(cbuff)); // [x, 2, x]
    expect_contents({2});

    EXPECT_EQ(1, circbuff_push(cbuff, 3)); // [x, 2, 3]
    expect_contents({2, 3});

    // at will wrap index around
    EXPECT_EQ(1, circbuff_push(cbuff, 4)); // [4, 2, 3]
    expect_contents({2, 3, 4});
}

TEST_F(CircBuff, test_circbuff_push_pop) {
    expect_contents({});

    EXPECT_EQ(1, circbuff_push(cbuff, 1)); // [1, x, x, x]
    expect_contents({1});

    EXPECT_EQ(1, circbuff_push(cbuff, 2)); // [1, 2, x, x]
    expect_contents({1, 2});

    EXPECT_EQ(1, circbuff_push(cbuff, 3)); // [1, 2, 3, x]
    expect_contents({1, 2, 3});

    EXPECT_EQ(1, circbuff_push(cbuff, 4)); // [1, 2, 3, 4]
    expect_contents({1, 2, 3, 4});

    EXPECT_EQ(0, circbuff_push(cbuff, 5)); // [1, 2, 3, 4]
    expect_contents({1, 2, 3, 4});

    EXPECT_EQ(1, circbuff_pop(cbuff)); // [x, 2, 3, 4]
    expect_contents({2, 3, 4});

    EXPECT_EQ(1, circbuff_push(cbuff, 3)); // [3, 2, 3, 4]
    expect_contents({2, 3, 4, 3});

    EXPECT_EQ(0, circbuff_push(cbuff, 4)); // [3, 2, 3, 4]
    expect_contents({2, 3, 4, 3});

    EXPECT_EQ(2, circbuff_pop(cbuff)); // [3, x, 3, 4]
    expect_contents({3, 4, 3});

    EXPECT_EQ(3, circbuff_pop(cbuff)); // [3, x, x, 4]
    expect_contents({4, 3});

    EXPECT_EQ(4, circbuff_pop(cbuff)); // [3, x, x, x]
    expect_contents({3});

    EXPECT_EQ(3, circbuff_pop(cbuff)); // [x, x, x, x]
    expect_contents({});

    EXPECT_EQ(0, circbuff_pop(cbuff)); // [x, x, x, x]
    expect_contents({});

    // TODO test circbuff_rpop
}

TEST_F(CircBuff, test_circbuff_insert) {
    uint8_t tmp[4] = {1, 2, 3, 4};

    // insert 0
    ASSERT_EQ(0, circbuff_insert(cbuff, tmp, 0));
    ASSERT_NO_FATAL_FAILURE(assert_contents({})) << "insert 0";

    // insert 1
    ASSERT_EQ(1, circbuff_insert(cbuff, tmp, 1));
    ASSERT_NO_FATAL_FAILURE(assert_contents({1})) << "insert 1";

    // insert 1+
    ASSERT_EQ(3, circbuff_insert(cbuff, tmp, 3));
    ASSERT_NO_FATAL_FAILURE(assert_contents({1, 1, 2, 3})) << "insert 1+";

    // insert 0 full
    ASSERT_EQ(0, circbuff_insert(cbuff, tmp, 0));
    ASSERT_NO_FATAL_FAILURE(assert_contents({1, 1, 2, 3})) << "insert 0 full";

    // insert 1 full
    ASSERT_EQ(0, circbuff_insert(cbuff, tmp, 1));
    ASSERT_NO_FATAL_FAILURE(assert_contents({1, 1, 2, 3})) << "insert 1 full";

    // insert 1+ full
    ASSERT_EQ(0, circbuff_insert(cbuff, tmp, 2));
    ASSERT_NO_FATAL_FAILURE(assert_contents({1, 1, 2, 3})) << "insert 1+ full";
}

TEST_F(CircBuff, test_circbuff_read) {
    uint8_t tmp[4] = {1, 2, 3, 4};
    uint8_t out[4] = {0};

    // read 0 empty
    EXPECT_EQ(0, circbuff_read(cbuff, out, 0));

    // read 1 empty
    EXPECT_EQ(0, circbuff_read(cbuff, out, 1));

    // read 1+ empty
    EXPECT_EQ(0, circbuff_read(cbuff, out, 3));

    // read too much empty
    EXPECT_EQ(0, circbuff_read(cbuff, out, 8));

    // add some data
    EXPECT_EQ(4, circbuff_insert(cbuff, tmp, 4));
    ASSERT_NO_FATAL_FAILURE(assert_contents({1, 2, 3, 4})) << "add some data";

    // read 0
    EXPECT_EQ(0, circbuff_read(cbuff, out, 0));

    // read 1
    ASSERT_EQ(1, circbuff_read(cbuff, out, 1));
    EXPECT_EQ(tmp[0], out[0]);

    // read 1+
    ASSERT_EQ(3, circbuff_read(cbuff, out, 3));
    EXPECT_EQ(tmp[0], out[0]);
    EXPECT_EQ(tmp[1], out[1]);
    EXPECT_EQ(tmp[2], out[2]);

    // read too much
    ASSERT_EQ(4, circbuff_read(cbuff, out, 7));
    EXPECT_EQ(tmp[0], out[0]);
    EXPECT_EQ(tmp[1], out[1]);
    EXPECT_EQ(tmp[2], out[2]);
    EXPECT_EQ(tmp[3], out[3]);

    // edge case read after remove where start has changed
    EXPECT_EQ(1, circbuff_pop(cbuff));
    expect_contents({2, 3, 4});

    // read 1+
    ASSERT_EQ(3, circbuff_read(cbuff, out, 3));
    EXPECT_EQ(tmp[1], out[0]);
    EXPECT_EQ(tmp[2], out[1]);
    EXPECT_EQ(tmp[3], out[2]);
}

TEST_F(CircBuff, test_circbuff_remove) {
    uint8_t tmp[4] = {1, 2, 3, 4};
    uint8_t out[4] = {0};

    // remove 0 empty
    EXPECT_EQ(0, circbuff_remove(cbuff, 0));
    EXPECT_EQ(0, circbuff_len(cbuff));

    // remove 1 empty
    EXPECT_EQ(0, circbuff_remove(cbuff, 1));
    EXPECT_EQ(0, circbuff_len(cbuff));

    // remove 1+ empty
    EXPECT_EQ(0, circbuff_remove(cbuff, 3));
    EXPECT_EQ(0, circbuff_len(cbuff));

    // remove too much empty
    EXPECT_EQ(0, circbuff_remove(cbuff, 5));
    EXPECT_EQ(0, circbuff_len(cbuff));

    // remove 0
    ASSERT_EQ(4, circbuff_insert(cbuff, tmp, 4));
    EXPECT_EQ(0, circbuff_remove(cbuff, 0));
    ASSERT_EQ(4, circbuff_len(cbuff));
    EXPECT_EQ(tmp[0], circbuff_at(cbuff, 0));
    EXPECT_EQ(tmp[1], circbuff_at(cbuff, 1));
    EXPECT_EQ(tmp[2], circbuff_at(cbuff, 2));
    EXPECT_EQ(tmp[3], circbuff_at(cbuff, 3));

    // remove 1
    EXPECT_EQ(1, circbuff_remove(cbuff, 1));
    EXPECT_EQ(3, circbuff_len(cbuff));
    EXPECT_EQ(tmp[1], circbuff_at(cbuff, 0));
    EXPECT_EQ(tmp[2], circbuff_at(cbuff, 1));
    EXPECT_EQ(tmp[3], circbuff_at(cbuff, 2));

    // remove 1+
    EXPECT_EQ(3, circbuff_remove(cbuff, 3));
    EXPECT_EQ(0, circbuff_len(cbuff));

    // remove all
    ASSERT_EQ(4, circbuff_insert(cbuff, tmp, 4));
    EXPECT_EQ(4, circbuff_remove(cbuff, 4));
    EXPECT_EQ(0, circbuff_len(cbuff));

    // remove too much
    ASSERT_EQ(4, circbuff_insert(cbuff, tmp, 4));
    EXPECT_EQ(4, circbuff_remove(cbuff, 7));
    EXPECT_EQ(0, circbuff_len(cbuff));
}
