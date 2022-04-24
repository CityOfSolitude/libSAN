#include <gtest/gtest.h>
#include <san.h>
#include <set>
#include <vector>

TEST(testEncode48, encodeSmallNaturals) {
    std::string encoded;
    constexpr int64_t bound = 63;

    // we can encode 63 small numbers in one character, the 64th is reserved for -1
    for (auto i = 0l; i < bound; ++i) {
        encoded = encode48(i);
        ASSERT_EQ(encoded.length(), 1);
        encoded = encode48Signed(i);
        ASSERT_EQ(encoded.length(), 1);
    }

    // verify the next number is encoded larger
    encoded = encode48(bound);
    ASSERT_EQ(encoded.length(), 2);
    encoded = encode48Signed(bound);
    ASSERT_EQ(encoded.length(), 2);
}

TEST(testEncode48, encodeLargerNaturals) {
    std::string encoded;
    constexpr int64_t bound = 64 * 63;

    // we can encode 64 * 63 number with 2 (or 1) digit,
    // since all starting with an all 1s block have already size 3
    for (auto i = 63l; i < bound; ++i) {
        encoded = encode48(i);
        ASSERT_EQ(encoded.length(), 2);
        encoded = encode48Signed(i);
        ASSERT_EQ(encoded.length(), 2);
    }

    // again check that the next number is encoded larger
    encoded = encode48(bound);
    ASSERT_EQ(encoded.length(), 3);
    encoded = encode48Signed(bound);
    ASSERT_EQ(encoded.length(), 3);
}

TEST(testEncode48, encodeSmallNegatives) {
    std::string encoded;
    constexpr int64_t bound = -65;

    // the special -1, which just needs one character
    encoded = encode48(-1);
    ASSERT_EQ(encoded.length(), 1);
    encoded = encode48Signed(-1);
    ASSERT_EQ(encoded.length(), 1);

    // we can encode the next 63 small negative numbers with two chars
    for (auto i = -2l; i > bound; --i) {
        encoded = encode48(i);
        ASSERT_EQ(encoded.length(), 2);
        encoded = encode48Signed(i);
        ASSERT_EQ(encoded.length(), 2);
    }

    // verify that the next number encodes larger
    encoded = encode48(bound);
    ASSERT_EQ(encoded.length(), 3);
    encoded = encode48Signed(bound);
    ASSERT_EQ(encoded.length(), 3);
}

TEST(testEncode48, encodeLargerNegatives) {
    std::string encoded;
    constexpr int64_t bound = -64 * 64 - 1;

    // we can encode 64 * 63 number with 2 (or 1) digit,
    // since all starting with an all 1s block have already size 3
    for (auto i = -65l; i > bound; --i) {
        encoded = encode48(i);
        ASSERT_EQ(encoded.length(), 3);
        encoded = encode48Signed(i);
        ASSERT_EQ(encoded.length(), 3);
    }

    // again check that the next number is encoded larger
    encoded = encode48(bound);
    ASSERT_EQ(encoded.length(), 4);
    encoded = encode48Signed(bound);
    ASSERT_EQ(encoded.length(), 4);
}

TEST(testEncode48, encodeProperties) {
    // we count the encoding length of all possible 24 bit values,
    // which are either 1, 2, 3 or 4 characters
    std::vector<size_t> counter(5, 0);
    for (int64_t input = 0; input < (1u << 24); ++input) {
        std::string encoded = encode48(input);
        ASSERT_EQ(ERROR::OK, valid(encoded, 48));
        auto length = encoded.length();
        // empty is not allowed and neither is larger than 4
        ASSERT_GT(length, 0);
        ASSERT_LE(length, 5);
        ++counter[length - 1];
    }
    // these are the amount of numbers we can encode in each length segment
    // in contrast to the 24 bit encoding, some numbers will slip into the 5 byte range
    // sine we do not test the full range, we miss out on the short encodings of small negative
    // numbers)
    ASSERT_EQ(counter[0], 64 - 1); // 0 to 62, 63 has a two byte encoding, -1 is not tested
    ASSERT_EQ(counter[1], (1 << 12) - (1 << 7) + 1);          // 2^12 - 2^6 - 63 small negatives
    ASSERT_EQ(counter[2], (1 << 18) - (1 << 13) + (1 << 6));  // 2^18 - 2^12 - 63*64 small negatives
    ASSERT_EQ(counter[3], (1 << 24) - (1 << 19) + (1 << 12)); // 2^24 - 2^18 - 63*64*64 negatives
    ASSERT_EQ(counter[4], (1 << 18)); // positive numbers with leading 1s, that overflew into 5 byte
}

const std::vector<uint64_t> basePatterns = { // NOLINT(cert-err58-cpp)
    0x000000000000, 0x111111111111, 0x222222222222, 0x333333333333, 0x444444444444, 0x555555555555,
    0x666666666666, 0x777777777777, 0x888888888888, 0x999999999999, 0xaaaaaaaaaaaa, 0xbbbbbbbbbbbb,
    0xcccccccccccc, 0xdddddddddddd, 0xeeeeeeeeeeee, 0xffffffffffff, 0xdeadbeafdead, 0xbeafdeadbeaf};

std::set<uint64_t> createPatterns48() {
    std::set<uint64_t> seen;
    seen.insert(0);
    size_t duplicates = 0;

    // produce numbers with the most significant bit is 0 to 47, i.e., numbers of all sizes
    for (int8_t msb = 0; msb <= 48; ++msb) {
        // mask to set the MSB, and to wipe everything else by subtracting 1
        uint64_t msbMask = 1ul << msb;
        // now use all the different patterns we defined
        for (auto pattern : basePatterns) {
            // use all possible 6-bit blocks to also "move over" the pattern
            for (uint64_t block = 0; block <= 0x3f; ++block) {
                // move the block from the LSBs over the pattern, until it vanished beyond the MSB
                for (int8_t pos = 0; pos <= msb; ++pos) {
                    // create the input, by combining the different elements / ideas
                    uint64_t input = pattern;
                    // remove block bits
                    input &= ~(0x3ful << pos);
                    // remove add block
                    input |= (block << pos);
                    // cut to length
                    input &= (msbMask - 1);
                    // add MSB, if not outside of 48 bit range
                    if (msb < 48) {
                        input |= msbMask;
                    }
                    // see if we already produced this input
                    if (seen.find(input) != seen.end()) {
                        ++duplicates;
                    } else {
                        seen.insert(input);
                    }
                }
            }
        }
    }

    if (seen.size() + duplicates != basePatterns.size() * 64 * (25 * 49) + 1) {
        throw std::runtime_error(
            "inconsistent test state, unexpected number of visited 48-bit patterns");
    }
    if (seen.size() * 2 < duplicates) {
        throw std::runtime_error(
            "inconsistent test state, unexpected low percentage of unique 48-bit patterns");
    }
    return seen;
}

const std::set<uint64_t> patterns = createPatterns48(); // NOLINT(cert-err58-cpp)

TEST(testEncode48, patternProperties) {
    std::vector<size_t> bucket(49, 0);
    for (auto input : patterns) {
        ASSERT_LT(input, 1ul << 48);
        uint8_t digits = 0;
        while (input) {
            ++digits;
            input >>= 1;
        }
        ++bucket[digits];
    }
    ASSERT_EQ(bucket[0], 1);  // only 0
    ASSERT_EQ(bucket[1], 1);  // only 1
    ASSERT_EQ(bucket[2], 2);  // 10 and 11
    ASSERT_EQ(bucket[3], 4);  // 1xx
    ASSERT_EQ(bucket[4], 8);  // 1xxx
    ASSERT_EQ(bucket[5], 16); // 1xxxx
    ASSERT_EQ(bucket[6], 32); // 1xxxxx
    ASSERT_EQ(bucket[7], 64); // 1xxxxxx (leading 0 and full 6-bit block explored)
    ASSERT_EQ(bucket[8], 128);
    ASSERT_EQ(bucket[9], 256);
    ASSERT_EQ(bucket[10], 512);
    ASSERT_EQ(bucket[11], 1024); // up to here, all buckets are complete
    for (size_t i = 12u, p1 = bucket[11], p2 = bucket[10]; i < bucket.size(); ++i) {
        auto p0 = bucket[i];
        // we have more than previous or the one before (some spikes in the number of examples)
        ASSERT_TRUE(p0 > std::min(p1, p2));
        p2 = p1;
        p1 = bucket[i];
    }
}

TEST(testEncode48, patternImportant) {
    ASSERT_TRUE(patterns.find(0) != patterns.end());
    ASSERT_TRUE(patterns.find(0xffffffffffff) != patterns.end());
    for (auto i = 0u; i <= 48; ++i) {
        ASSERT_TRUE(i == 48 || patterns.find(1ul << i) != patterns.end());
        ASSERT_TRUE(patterns.find((1ul << i) - 1) != patterns.end());
    }
}

TEST(testEncode48, encodeDecodeManyUnsigned) {
    for (auto input : patterns) {
        std::string encoded = encode48(input);
        ASSERT_EQ(input, decode48(encoded)) << encoded;
    }
}

TEST(testEncode48, encodeDecodeAllSigned) {
    for (auto uns : patterns) {
        int64_t input = uns & 0x00800000000000
                            ? static_cast<int64_t>(uns) | static_cast<int64_t>(0xffff000000000000)
                            : static_cast<int64_t>(uns);
        std::string encoded = encode48Signed(input);
        ASSERT_EQ(input, decode48Signed(encoded)) << encoded;
    }
}
