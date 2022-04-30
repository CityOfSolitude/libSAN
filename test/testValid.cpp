#include <gtest/gtest.h>
#include <san.h>
#include <tables.h>

using namespace san;

TEST(testValid, empty) {
    ASSERT_EQ(ERROR::EMPTY, valid(""));
    ASSERT_EQ(ERROR::EMPTY, valid("", 1));
    ASSERT_EQ(ERROR::EMPTY, valid("", 10));
    ASSERT_EQ(ERROR::EMPTY, valid("", 100));
}

TEST(testValid, highBitOrWrongChar) {
    size_t error_high = 0;
    size_t error_wrong = 0;
    size_t success = 0;
    for (int i = -128; i <= 127; ++i) {
        for (int j = -128; j <= 127; ++j) {
            for (int k = -128; k <= 127; ++k) {
                switch (valid({static_cast<char>(i), static_cast<char>(j), static_cast<char>(k)})) {
                case ERROR::HIGH_BIT:
                    ++error_high;
                    break;
                case ERROR::WRONG_CHAR:
                    ++error_wrong;
                    break;
                case ERROR::EMPTY:
                    FAIL();
                default:
                    ++success;
                    break;
                }
            }
        }
    }
    EXPECT_EQ(success, 64 * 64 * 64);
    EXPECT_EQ(error_high, 128 * 256 * 256 + 64 * 128 * 256 + 64 * 64 * 128);
    EXPECT_EQ(error_wrong, 64 * 256 * 256 + 64 * 64 * 256 + 64 * 64 * 64);
}

TEST(testValid, correctWithoutSize) {
    for (auto c = 0u; c < 64; ++c) {
        for (size_t s = 1u; s < 128; ++s) {
            std::string encoded = std::string(s, enc[c]);
            ASSERT_EQ(ERROR::OK, valid(encoded)) << encoded;
        }
    }
}

TEST(testValid, tooLongFirstByte6BitOk) {
    for (uint8_t pattern = 0; pattern < 64; ++pattern) {
        ASSERT_EQ(ERROR::OK, valid({enc[pattern]}, 6));
        ASSERT_EQ(ERROR::OK, valid({enc[pattern], enc[ONES]}, 12));
    }
}

TEST(testValid, tooLongFirstByte5Bit) {
    for (uint8_t pattern = 0; pattern < 64; ++pattern) {
        bool sign = pattern & 0x10;
        bool extra = pattern & 0x20;
        if (sign == extra) {
            ASSERT_EQ(ERROR::OK, valid({enc[pattern]}, 5));
            ASSERT_EQ(ERROR::OK, valid({enc[pattern], enc[ONES]}, 11));
            ASSERT_EQ(ERROR::OK, valid({enc[pattern], enc[pattern]}, 11));
        } else {
            ASSERT_EQ(ERROR::TOO_LONG, valid({enc[pattern]}, 5));
            ASSERT_EQ(ERROR::TOO_LONG, valid({enc[pattern], enc[ONES]}, 11));
            ASSERT_EQ(ERROR::TOO_LONG, valid({enc[pattern], enc[pattern]}, 11));
        }
    }
}

TEST(testValid, tooLongFirstByte4Bit) {
    for (uint8_t pattern = 0; pattern < 64; ++pattern) {
        bool sign = pattern & 0x08;
        bool extra1 = pattern & 0x20;
        bool extra2 = pattern & 0x10;
        if (sign == extra1 && sign == extra2) {
            ASSERT_EQ(ERROR::OK, valid({enc[pattern]}, 4));
            ASSERT_EQ(ERROR::OK, valid({enc[pattern], enc[ONES]}, 10));
            ASSERT_EQ(ERROR::OK, valid({enc[pattern], enc[pattern]}, 10));
        } else {
            ASSERT_EQ(ERROR::TOO_LONG, valid({enc[pattern]}, 4));
            ASSERT_EQ(ERROR::TOO_LONG, valid({enc[pattern], enc[ONES]}, 10));
            ASSERT_EQ(ERROR::TOO_LONG, valid({enc[pattern], enc[pattern]}, 10));
        }
    }
}

TEST(testValid, tooLongFirstByte3Bit) {
    for (uint8_t pattern = 0; pattern < 64; ++pattern) {
        bool sign = pattern & 0x04;
        bool extra1 = pattern & 0x20;
        bool extra2 = pattern & 0x10;
        bool extra3 = pattern & 0x08;
        if (sign == extra1 && sign == extra2 && sign == extra3) {
            ASSERT_EQ(ERROR::OK, valid({enc[pattern]}, 3));
            ASSERT_EQ(ERROR::OK, valid({enc[pattern], enc[ONES]}, 9));
            ASSERT_EQ(ERROR::OK, valid({enc[pattern], enc[pattern]}, 9));
        } else {
            ASSERT_EQ(ERROR::TOO_LONG, valid({enc[pattern]}, 3));
            ASSERT_EQ(ERROR::TOO_LONG, valid({enc[pattern], enc[ONES]}, 9));
            ASSERT_EQ(ERROR::TOO_LONG, valid({enc[pattern], enc[pattern]}, 9));
        }
    }
}

TEST(testValid, tooLongFirstByte2Bit) {
    for (uint8_t pattern = 0; pattern < 64; ++pattern) {
        bool sign = pattern & 0x02;
        bool extra1 = pattern & 0x20;
        bool extra2 = pattern & 0x10;
        bool extra3 = pattern & 0x08;
        bool extra4 = pattern & 0x04;
        if (sign == extra1 && sign == extra2 && sign == extra3 && sign == extra4) {
            ASSERT_EQ(ERROR::OK, valid({enc[pattern]}, 2));
            ASSERT_EQ(ERROR::OK, valid({enc[pattern], enc[ONES]}, 8));
            ASSERT_EQ(ERROR::OK, valid({enc[pattern], enc[pattern]}, 8));
        } else {
            ASSERT_EQ(ERROR::TOO_LONG, valid({enc[pattern]}, 2));
            ASSERT_EQ(ERROR::TOO_LONG, valid({enc[pattern], enc[ONES]}, 8));
            ASSERT_EQ(ERROR::TOO_LONG, valid({enc[pattern], enc[pattern]}, 8));
        }
    }
}

TEST(testValid, tooLongFirstByte1Bit) {
    for (uint8_t pattern = 0; pattern < 64; ++pattern) {
        bool sign = pattern & 0x01;
        bool extra1 = pattern & 0x20;
        bool extra2 = pattern & 0x10;
        bool extra3 = pattern & 0x08;
        bool extra4 = pattern & 0x04;
        bool extra5 = pattern & 0x02;
        if (sign == extra1 && sign == extra2 && sign == extra3 && sign == extra4 &&
            sign == extra5) {
            ASSERT_EQ(ERROR::OK, valid({enc[pattern]}, 1));
            ASSERT_EQ(ERROR::OK, valid({enc[pattern], enc[ONES]}, 7));
            ASSERT_EQ(ERROR::OK, valid({enc[pattern], enc[pattern]}, 7));
        } else {
            ASSERT_EQ(ERROR::TOO_LONG, valid({enc[pattern]}, 1));
            ASSERT_EQ(ERROR::TOO_LONG, valid({enc[pattern], enc[ONES]}, 7));
            ASSERT_EQ(ERROR::TOO_LONG, valid({enc[pattern], enc[pattern]}, 7));
        }
    }
}
