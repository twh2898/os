#include <cstdlib>

#include "test_common.h"

extern "C" {
#include "libc/memory.h"
#include "libc/string.h"
}

class String : public testing::Test {
protected:
    void SetUp() override {
        init_mocks();
    }
};

TEST_F(String, kmemcmp) {
    EXPECT_EQ(0, kmemcmp(0, 0, 0));
    EXPECT_EQ(0, kmemcmp("", 0, 0));
    EXPECT_EQ(0, kmemcmp(0, "", 0));
    EXPECT_EQ(0, kmemcmp("", "", 0));

    char a[2] = {1, 2};
    char b[2] = {0, 0};

    EXPECT_EQ(0, kmemcmp(a, b, 0));
    EXPECT_EQ(0, kmemcmp(a, a, 1));
    EXPECT_EQ(0, kmemcmp(b, b, 1));

    EXPECT_LT(0, kmemcmp(a, b, 2));
    EXPECT_GT(0, kmemcmp(b, a, 2));
}

TEST_F(String, kmemcpy) {
    EXPECT_EQ(0, kmemcpy(0, 0, 0));

    char a[2] = {1, 2};
    char b[2] = {0, 0};

    EXPECT_EQ(0, kmemcpy(b, 0, 1));
    EXPECT_EQ(0, kmemcpy(0, a, 1));

    EXPECT_EQ(b, kmemcpy(b, a, 0));
    EXPECT_EQ(0, b[0]);
    EXPECT_EQ(0, b[1]);

    EXPECT_EQ(b, kmemcpy(b, a, 1));
    EXPECT_EQ(1, b[0]);
    EXPECT_EQ(0, b[1]);

    EXPECT_EQ(b, kmemcpy(b, a, 2));
    EXPECT_EQ(1, b[0]);
    EXPECT_EQ(2, b[1]);
}

TEST_F(String, kmemmove) {
    EXPECT_EQ(0, kmemmove(0, 0, 0));

    char a[2] = {1, 2};
    char b[2] = {0, 0};

    EXPECT_EQ(0, kmemmove(b, 0, 1));
    EXPECT_EQ(0, kmemmove(0, a, 1));

    EXPECT_EQ(b, kmemmove(b, a, 0));
    EXPECT_EQ(0, b[0]);
    EXPECT_EQ(0, b[1]);

    EXPECT_EQ(b, kmemmove(b, a, 1));
    EXPECT_EQ(1, b[0]);
    EXPECT_EQ(0, b[1]);

    b[0] = 0;

    EXPECT_EQ(b, kmemmove(b, a, 2));
    EXPECT_EQ(1, b[0]);
    EXPECT_EQ(2, b[1]);

    a[0] = a[1] = 0;

    EXPECT_EQ(a, kmemmove(a, b, 1));
    EXPECT_EQ(1, a[0]);
    EXPECT_EQ(0, a[1]);

    a[0] = 0;

    EXPECT_EQ(a, kmemmove(a, b, 2));
    EXPECT_EQ(1, a[0]);
    EXPECT_EQ(2, a[1]);

    char c[3] = {1, 2, 3};

    EXPECT_EQ(c + 1, kmemmove(c + 1, c, 2));
    EXPECT_EQ(1, c[0]);
    EXPECT_EQ(1, c[1]);
    EXPECT_EQ(2, c[2]);

    c[1] = 2;
    c[2] = 3;

    EXPECT_EQ(c, kmemmove(c, c + 1, 2));
    EXPECT_EQ(2, c[0]);
    EXPECT_EQ(3, c[1]);
    EXPECT_EQ(3, c[2]);
}

TEST_F(String, kmemset) {
    EXPECT_EQ(0, kmemset(0, 0, 0));
    EXPECT_EQ(0, kmemset(0, 0, 1));

    char a[3] = {1, 2, 3};

    EXPECT_EQ(a, kmemset(a, 7, 0));
    EXPECT_EQ(1, a[0]);
    EXPECT_EQ(2, a[1]);
    EXPECT_EQ(3, a[2]);

    EXPECT_EQ(a, kmemset(a, 8, 1));
    EXPECT_EQ(8, a[0]);
    EXPECT_EQ(2, a[1]);
    EXPECT_EQ(3, a[2]);

    EXPECT_EQ(a, kmemset(a, 4, 2));
    EXPECT_EQ(4, a[0]);
    EXPECT_EQ(4, a[1]);
    EXPECT_EQ(3, a[2]);

    EXPECT_EQ(a, kmemset(a, 5, 3));
    EXPECT_EQ(5, a[0]);
    EXPECT_EQ(5, a[1]);
    EXPECT_EQ(5, a[2]);
}

TEST_F(String, kstrlen) {
    EXPECT_EQ(0, kstrlen(0));
    EXPECT_EQ(0, kstrlen(""));
    EXPECT_EQ(1, kstrlen("1"));
    EXPECT_EQ(3, kstrlen(" a "));
}

TEST_F(String, knstrlen) {
    EXPECT_EQ(0, knstrlen(0, -1));
    EXPECT_EQ(0, knstrlen("", -1));
    EXPECT_EQ(0, knstrlen(0, 0));

    EXPECT_EQ(0, knstrlen("", 1));
    EXPECT_EQ(0, knstrlen("1", 0));
    EXPECT_EQ(1, knstrlen("1", 1));
    EXPECT_EQ(0, knstrlen(" a ", 0));
    EXPECT_EQ(1, knstrlen(" a ", 1));
    EXPECT_EQ(2, knstrlen(" a ", 2));
    EXPECT_EQ(3, knstrlen(" a ", 3));
}

TEST_F(String, kstrcmp) {
    EXPECT_EQ(0, kstrcmp(0, 0));

    char a[3] = {'a', 'b', 0};
    char b[3] = {'a', 'c', 0};
    char c[2] = {'a', 0};

    EXPECT_EQ(0, kstrcmp(a, a));
    EXPECT_EQ(0, kstrcmp(b, b));

    EXPECT_LT(0, kstrcmp(b, a));
    EXPECT_GT(0, kstrcmp(a, b));

    EXPECT_LT(0, kstrcmp(a, c));
}

TEST_F(String, kstrfind) {
    EXPECT_EQ(0, kstrfind(0, 0));

    const char * str = "abc";

    EXPECT_EQ(0, kstrfind(str, 'd'));

    EXPECT_EQ(str, kstrfind(str, 'a'));
    EXPECT_EQ(str + 1, kstrfind(str, 'b'));
    EXPECT_EQ(str + 2, kstrfind(str, 'c'));
}

// TODO kstrtok
// TEST_F(String, kstrtok) {
//     char a[3] = {'a', 'b', 0};

//     EXPECT_EQ(0, kstrtok(0, 0));
//     EXPECT_EQ(0, kstrtok(a, 0));
//     EXPECT_EQ(0, kstrtok(0, ""));

//     EXPECT_EQ(a, kstrtok(a, "a"));

//     char b[3] = {'a', 'b', 0};

//     EXPECT_EQ(b + 1, kstrtok(b, "b"));

//     char c[3] = {'a', 'b', 0};

//     EXPECT_EQ(0, kstrtok(c, "c"));
// }

TEST_F(String, katoi) {
    EXPECT_EQ(0, katoi(0));
    EXPECT_EQ(0, katoi("0"));
    EXPECT_EQ(0, katoi("n"));
    EXPECT_EQ(0, katoi("/")); // < 0
    EXPECT_EQ(0, katoi(""));

    EXPECT_EQ(1, katoi("1"));
    EXPECT_EQ(123, katoi("123"));
    EXPECT_EQ(123, katoi("0123"));
    EXPECT_EQ(-123, katoi("-123"));
    EXPECT_EQ(123, katoi("+123"));
}
