#include <gtest/gtest.h>
#include <san.h>
#include <vector>

TEST(testEncode, encodeSmallNaturals) {
    std::string encoded;
    constexpr int32_t bound = 63;

    // we can encode 63 small numbers in one character, the 64th is reserved for -1
    for (auto i = 0; i < bound; ++i) {
        encoded = encode24(i);
        ASSERT_EQ(encoded.length(), 1);
        encoded = encode24Signed(i);
        ASSERT_EQ(encoded.length(), 1);
    }

    // verify the next number is encoded larger
    encoded = encode24(bound);
    ASSERT_EQ(encoded.length(), 2);
    encoded = encode24Signed(bound);
    ASSERT_EQ(encoded.length(), 2);
}

TEST(testEncode, encodeLargerNaturals) {
    std::string encoded;
    constexpr int32_t bound = 64 * 63;

    // we can encode 64 * 63 number with 2 (or 1) digit, since all starting with an all 1s block have already size 3
    for (auto i = 63; i < bound; ++i) {
        encoded = encode24(i);
        ASSERT_EQ(encoded.length(), 2);
        encoded = encode24Signed(i);
        ASSERT_EQ(encoded.length(), 2);
    }

    // again check that the next number is encoded larger
    encoded = encode24(bound);
    ASSERT_EQ(encoded.length(), 3);
    encoded = encode24Signed(bound);
    ASSERT_EQ(encoded.length(), 3);
}

TEST(testEncode, encodeSmallNegatives) {
    std::string encoded;
    constexpr int32_t bound = -65;

    // the special -1, which just needs one character
    encoded = encode24(-1);
    ASSERT_EQ(encoded.length(), 1);
    encoded = encode24Signed(-1);
    ASSERT_EQ(encoded.length(), 1);

    // we can encode the next 63 small negative numbers with two chars
    for (auto i = -2; i > bound; --i) {
        encoded = encode24(i);
        ASSERT_EQ(encoded.length(), 2);
        encoded = encode24Signed(i);
        ASSERT_EQ(encoded.length(), 2);
    }

    // verify that the next number encodes larger
    encoded = encode24(bound);
    ASSERT_EQ(encoded.length(), 3);
    encoded = encode24Signed(bound);
    ASSERT_EQ(encoded.length(), 3);
}

TEST(testEncode, encodeLargerNegatives) {
    std::string encoded;
    constexpr int32_t bound = -64 * 64 - 1;

    // we can encode 64 * 63 number with 2 (or 1) digit, since all starting with an all 1s block have already size 3
    for (auto i = -65; i > bound; --i) {
        encoded = encode24(i);
        ASSERT_EQ(encoded.length(), 3);
        encoded = encode24Signed(i);
        ASSERT_EQ(encoded.length(), 3);
    }

    // again check that the next number is encoded larger
    encoded = encode24(bound);
    ASSERT_EQ(encoded.length(), 4);
    encoded = encode24Signed(bound);
    ASSERT_EQ(encoded.length(), 4);
}

TEST(testEncode, encodeProperties) {
    // we count the encoding length of all possible 24 bit values, which are either 1, 2, 3 or 4 characters
    std::vector<size_t> counter(4, 0);
    for (int32_t input = 0; input < (1u << 24); ++input) {
        std::string encoded = encode24(input);
        auto length = encoded.length();
        // empty is not allowed and neither is larger than 4
        ASSERT_GT(length, 0);
        ASSERT_LE(length, 4);
        ++counter[length - 1];
    }
    // these are the amount of numbers we can encode in each length segment
    ASSERT_EQ(counter[0], 64); // 0 to 63 and -1, 64 has a two byte encoding
    ASSERT_EQ(counter[1], (1 << 12) - (1 << 6)); // 2^12 - 2^6
    ASSERT_EQ(counter[2], (1 << 18) - (1 << 12)); // 2^18 - 2^12
    ASSERT_EQ(counter[3], (1 << 24) - (1 << 18)); // 2^24 - 2^18
}

TEST(testEncode, encodeDecodeAllUnsigned) {
    for (uint32_t input = 0; input < (1u << 24); ++input) {
        std::string encoded = encode24(input);
        ASSERT_EQ(input, decode24(encoded)) << encoded;
    }
}

TEST(testEncode, encodeDecodeAllSigned) {
    for (int32_t input = -(1 << 23); input < (1 << 23); ++input) {
        std::string encoded = encode24(input);
        ASSERT_EQ(input, decode24Signed(encoded)) << encoded;
    }
}

TEST(testEncode, invalidEmpty) {
    ASSERT_EQ(ERROR_24_EMPTY, decode24(""));
}

TEST(testEncode, invalidHighBit) {
    std::string encoded;
    for (auto i = -1; i < 63; ++i) {
        encoded = encode24(i);
        encoded.front() |= static_cast<char>(0x80);
        ASSERT_EQ(ERROR_24_HIGH_BIT, decode24(encoded));

        encoded = encode24Signed(i);
        encoded.front() |= static_cast<char>(0x80);
        ASSERT_EQ(ERROR_24_HIGH_BIT, decode24(encoded));
    }
}

TEST(testEncode, invalidWrongChar) {
    size_t error_high = 0;
    size_t error_wrong = 0;
    size_t success = 0;
    for (int i = -128; i <= 127; ++i) {
        uint32_t decoded = decode24({static_cast<char>(i)});
        switch (decoded) {
            case ERROR_24_HIGH_BIT:
                ++error_high;
                break;
            case ERROR_24_WRONG_CHAR:
                ++error_wrong;
                break;
            case ERROR_24_EMPTY:
                FAIL();
            default:
                ++success;
                if (decoded != 0x00ffffff) {
                    ASSERT_LT(decoded, 63);
                }
                break;
        }
    }
    ASSERT_EQ(success, 64);
    ASSERT_EQ(error_high, 128);
    ASSERT_EQ(error_wrong, 64);
}
