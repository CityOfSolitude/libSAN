#include <gtest/gtest.h>
#include <san.h>
#include <vector>

using namespace san;

TEST(testEncode24, encodeSmallNaturals) {
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

TEST(testEncode24, encodeLargerNaturals) {
    std::string encoded;
    constexpr int32_t bound = 64 * 63;

    // we can encode 64 * 63 number with 2 (or 1) digit,
    // since all starting with an all 1s block have already size 3
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

TEST(testEncode24, encodeSmallNegatives) {
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

TEST(testEncode24, encodeLargerNegatives) {
    std::string encoded;
    constexpr int32_t bound = -64 * 64 - 1;

    // we can encode 64 * 63 number with 2 (or 1) digit,
    // since all starting with an all 1s block have already size 3
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

TEST(testEncode24, encodeProperties) {
    // we count the encoding length of all possible 24 bit values,
    // which are either 1, 2, 3 or 4 characters
    std::vector<size_t> counter(4, 0);
    for (int32_t input = 0; input < (1u << 24); ++input) {
        std::string encoded = encode24(input);
        ASSERT_EQ(ERROR::OK, valid(encoded, 24));
        auto length = encoded.length();
        // empty is not allowed and neither is larger than 4
        ASSERT_GT(length, 0);
        ASSERT_LE(length, 4);
        ++counter[length - 1];
    }
    // these are the amount of numbers we can encode in each length segment
    ASSERT_EQ(counter[0], 64); // 0 to 62 and -1, 63 has a two byte encoding (leading 0s block)
    ASSERT_EQ(counter[1], (1 << 12) - (1 << 6));  // 2^12 - 2^6
    ASSERT_EQ(counter[2], (1 << 18) - (1 << 12)); // 2^18 - 2^12
    ASSERT_EQ(counter[3], (1 << 24) - (1 << 18)); // 2^24 - 2^18
}

TEST(testEncode24, encodeDecodeAllUnsigned) {
    for (uint32_t input = 0; input < (1u << 24); ++input) {
        std::string encoded = encode24(input);
        ASSERT_EQ(input, decode24(encoded)) << encoded;
    }
}

TEST(testEncode24, encodeDecodeAllSigned) {
    for (int32_t input = -(1 << 23); input < (1 << 23); ++input) {
        std::string encoded = encode24(input);
        ASSERT_EQ(input, decode24Signed(encoded)) << encoded;
    }
}

TEST(testEncode24, allPatternsValid) {
    for (uint32_t input = 0; input < (1u << 24); ++input) {
        ASSERT_EQ(ERROR::OK, valid(encode24(input), 24));
    }
    for (int32_t input = -(1 << 23); input < (1 << 23); ++input) {
        ASSERT_EQ(ERROR::OK, valid(encode24Signed(input), 24));
    }
}
