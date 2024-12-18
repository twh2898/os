#include <array>
#include <cstdlib>
#include <cstring>
#include <vector>

#include "test_header.h"

extern "C" {
#include "drivers/vga.h"
}

extern "C" {
FAKE_VOID_FUNC(port_byte_out, uint16_t, uint8_t);
FAKE_VOID_FUNC(port_word_out, uint16_t, uint16_t);
FAKE_VALUE_FUNC(uint8_t, port_byte_in, uint16_t);
FAKE_VALUE_FUNC(uint16_t, port_word_in, uint16_t);

void * kmemmove(void * dest, void * src, size_t n) {
    return memmove(dest, src, n);
}
}

class VGA : public testing::Test {
protected:
    std::array<char, 80 * 25 * 2> buff;

    void SetUp() override {
        RESET_FAKE(port_byte_out);
        RESET_FAKE(port_byte_in);
        RESET_FAKE(port_word_out);
        RESET_FAKE(port_word_in);

        buff.fill(0);
        init_vga(buff.data());
    }
};

TEST_F(VGA, vga_clear) {
    buff.fill(0x82);
    vga_clear();
    for (size_t i = 0; i < buff.size(); i++) {
        if (i % 2 == 0) {
            EXPECT_EQ(' ', buff[i]);
        }
        else {
            EXPECT_EQ(7, buff[i]);
        }
    }
}

TEST_F(VGA, vga_put) {
    vga_put(3, 'Q', 0x21);

    EXPECT_EQ('Q', buff[6]);
    EXPECT_EQ(0x21, buff[7]);
}

TEST_F(VGA, vga_row) {
    EXPECT_EQ(0, vga_row(0));
    EXPECT_EQ(0, vga_row(79));
    EXPECT_EQ(1, vga_row(80));
    EXPECT_EQ(1, vga_row(159));
    EXPECT_EQ(2, vga_row(160));
    EXPECT_EQ(25, vga_row(80*25));
}

TEST_F(VGA, vga_col) {
    EXPECT_EQ(0, vga_col(0));
    EXPECT_EQ(79, vga_col(79));
    EXPECT_EQ(0, vga_col(80));
    EXPECT_EQ(79, vga_col(159));
    EXPECT_EQ(0, vga_col(160));
}

TEST_F(VGA, vga_cursor) {
    
}