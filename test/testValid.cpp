#include <gtest/gtest.h>
#include <san.h>
#include <tables.h>

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

TEST(testValid, tooLongFirstByte) {
    ASSERT_EQ(ERROR::TOO_LONG, valid({enc[ONES]}, 1));
    ASSERT_EQ(ERROR::TOO_LONG, valid({enc[ONES]}, 2));
    ASSERT_EQ(ERROR::TOO_LONG, valid({enc[ONES]}, 3));
    ASSERT_EQ(ERROR::TOO_LONG, valid({enc[ONES]}, 4));
    ASSERT_EQ(ERROR::TOO_LONG, valid({enc[ONES]}, 5));
    ASSERT_EQ(ERROR::OK, valid({enc[ONES]}, 6));
    ASSERT_EQ(ERROR::OK, valid({enc[ONES]}, 7));

    ASSERT_EQ(ERROR::TOO_LONG, valid({enc[ONES], enc[0]}, 7));
    ASSERT_EQ(ERROR::TOO_LONG, valid({enc[ONES], enc[0]}, 8));
    ASSERT_EQ(ERROR::TOO_LONG, valid({enc[ONES], enc[0]}, 9));
    ASSERT_EQ(ERROR::TOO_LONG, valid({enc[ONES], enc[0]}, 10));
    ASSERT_EQ(ERROR::TOO_LONG, valid({enc[ONES], enc[0]}, 11));
    ASSERT_EQ(ERROR::OK, valid({enc[ONES], enc[0]}, 12));
    ASSERT_EQ(ERROR::OK, valid({enc[ONES], enc[0]}, 13));
}

TEST(testValid, firstByteCorrect) {
    for (auto bits = 1u; bits < 256; ++bits) {
        auto bytes = (bits - 1) / 6 + 1; // except first
        auto rest = bits % 6; // that many in first

        for (size_t first = 0; first < 64; ++first) {
            std::string tooLong(bytes + 1, enc[first]);
            ASSERT_EQ(ERROR::TOO_LONG, valid(tooLong, bits));
            std::string unclear(bytes, enc[first]);
            bool firstTooLarge = rest && first >= 1u << rest;
            ASSERT_EQ(firstTooLarge ? ERROR::TOO_LONG : ERROR::OK, valid(unclear, bits));
        }
    }
}
