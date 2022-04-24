#include <gtest/gtest.h>
#include <tables.h>
#include <unordered_set>

TEST(testTables, encodeTableSize) { ASSERT_EQ(strlen(enc), 64); }

TEST(testTables, encodeTablePrintable) {
    for (auto i = 0u; i < 64; ++i) {
        ASSERT_GE(enc[i], 32);
    }
}

TEST(testTables, encodeTable7Bit) {
    for (auto i = 0u; i < 64; ++i) {
        ASSERT_LT(enc[i], 128);
    }
}

TEST(testTables, encodeTableCharset) {
    for (auto i = 0u; i < 64; ++i) {
        auto &c = enc[i];
        auto isDigit = c >= '0' && c <= '9';
        auto isUpperCaseLetter = c >= 'A' && c <= 'Z';
        auto isLowerCaseLetter = c >= 'a' && c <= 'z';
        auto isPlusMinus = c == '-' || c == '+';
        ASSERT_TRUE(isDigit || isUpperCaseLetter || isLowerCaseLetter || isPlusMinus);
    }
}

TEST(testTables, decodeTableSize) {
    ASSERT_EQ(strlen(dec), 43); // '0'
    ASSERT_EQ(strlen(dec + 44), 128 - 44);
}

TEST(testTables, decodeTableBase64OrSpace) {
    for (auto i = 0u; i < 128; ++i) {
        auto &c = dec[i];
        if (c == '@') {
            continue;
        }
        ASSERT_GE(c, 0);
        ASSERT_LT(c, 64);
    }
}

TEST(testTables, decodeTableUnique) {
    std::unordered_set<char> seen;
    auto dead = 0u;
    for (auto i = 0u; i < 128; ++i) {
        auto &c = dec[i];
        if (c == '@') {
            ++dead;
        } else {
            ASSERT_EQ(seen.find(c), seen.end());
            seen.insert(c);
        }
    }
    ASSERT_EQ(seen.size(), 64);
    ASSERT_EQ(dead, 64);
}

TEST(testTables, bijectionEncodeDecode) {
    for (auto i = 0u; i < 64; ++i) {
        ASSERT_EQ(i, dec[enc[i]]);
    }
}

TEST(testTables, bijectionDecodeEncode) {
    auto dead = 0u;
    for (auto i = 0u; i < 128; ++i) {
        char c = dec[i];
        if (c == '@') {
            ++dead;
        } else {
            ASSERT_EQ(i, enc[c]);
        }
    }
    ASSERT_EQ(dead, 64);
}
