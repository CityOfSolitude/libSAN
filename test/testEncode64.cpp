#include <gtest/gtest.h>
#include <san.h>
#include <set>
#include <vector>

using namespace san;

TEST(testEncode64, encodeSmallNaturals) {
    std::string encoded;
    constexpr int64_t bound = 63;

    // we can encode 63 small numbers in one character, the 64th is reserved for -1
    for (auto i = 0l; i < bound; ++i) {
        encoded = encode64(i);
        ASSERT_EQ(encoded.length(), 1);
        encoded = encode64Signed(i);
        ASSERT_EQ(encoded.length(), 1);
    }

    // verify the next number is encoded larger
    encoded = encode64(bound);
    ASSERT_EQ(encoded.length(), 2);
    encoded = encode64Signed(bound);
    ASSERT_EQ(encoded.length(), 2);
}

TEST(testEncode64, encodeLargerNaturals) {
    std::string encoded;
    constexpr int64_t bound = 64 * 63;

    // we can encode 64 * 63 number with 2 (or 1) digit,
    // since all starting with an all 1s block have already size 3
    for (auto i = 63l; i < bound; ++i) {
        encoded = encode64(i);
        ASSERT_EQ(encoded.length(), 2);
        encoded = encode64Signed(i);
        ASSERT_EQ(encoded.length(), 2);
    }

    // again check that the next number is encoded larger
    encoded = encode64(bound);
    ASSERT_EQ(encoded.length(), 3);
    encoded = encode64Signed(bound);
    ASSERT_EQ(encoded.length(), 3);
}

TEST(testEncode64, encodeSmallNegatives) {
    std::string encoded;
    constexpr int64_t bound = -65;

    // the special -1, which just needs one character
    encoded = encode64(-1);
    ASSERT_EQ(encoded.length(), 1);
    encoded = encode64Signed(-1);
    ASSERT_EQ(encoded.length(), 1);

    // we can encode the next 63 small negative numbers with two chars
    for (auto i = -2l; i > bound; --i) {
        encoded = encode64(i);
        ASSERT_EQ(encoded.length(), 2);
        encoded = encode64Signed(i);
        ASSERT_EQ(encoded.length(), 2);
    }

    // verify that the next number encodes larger
    encoded = encode64(bound);
    ASSERT_EQ(encoded.length(), 3);
    encoded = encode64Signed(bound);
    ASSERT_EQ(encoded.length(), 3);
}

TEST(testEncode64, encodeLargerNegatives) {
    std::string encoded;
    constexpr int64_t bound = -64 * 64 - 1;

    // we can encode 64 * 63 number with 2 (or 1) digit,
    // since all starting with an all 1s block have already size 3
    for (auto i = -65l; i > bound; --i) {
        encoded = encode64(i);
        ASSERT_EQ(encoded.length(), 3);
        encoded = encode64Signed(i);
        ASSERT_EQ(encoded.length(), 3);
    }

    // again check that the next number is encoded larger
    encoded = encode64(bound);
    ASSERT_EQ(encoded.length(), 4);
    encoded = encode64Signed(bound);
    ASSERT_EQ(encoded.length(), 4);
}

TEST(testEncode64, encodeProperties) {
    // we count the encoding length of all possible 24 bit values,
    // which are either 1, 2, 3 or 4 characters
    std::vector<size_t> counter(5, 0);
    for (int64_t input = 0; input < (1u << 24); ++input) {
        std::string encoded = encode64(input);
        ASSERT_EQ(ERROR::OK, valid(encoded, 64));
        auto length = encoded.length();
        // empty is not allowed and neither is larger than 4
        ASSERT_GT(length, 0);
        ASSERT_LE(length, 5);
        ++counter[length - 1];
    }
    // these are the amount of numbers we can encode in each length segment
    // in contrast to the 24 bit encoding, some numbers will slip into the 5 byte range,
    // since we do not test the full range, we miss out on the short encodings of
    // small negative numbers
    ASSERT_EQ(counter[0], 64 - 1); // 0 to 62, 63 has a two byte encoding, -1 is not tested
    ASSERT_EQ(counter[1], (1 << 12) - (1 << 7) + 1);          // 2^12 - 2^6 - 63 small negatives
    ASSERT_EQ(counter[2], (1 << 18) - (1 << 13) + (1 << 6));  // 2^18 - 2^12 - 63*64 small negatives
    ASSERT_EQ(counter[3], (1 << 24) - (1 << 19) + (1 << 12)); // 2^24 - 2^18 - 63*64*64 negatives
    ASSERT_EQ(counter[4], (1 << 18)); // positive numbers with leading 1s, that overflew into 5 byte
}

const std::vector<uint64_t> basePatterns = { // NOLINT(cert-err58-cpp)
    0x0000000000000000, 0x1111111111111111, 0x2222222222222222, 0x3333333333333333,
    0x4444444444444444, 0x5555555555555555, 0x6666666666666666, 0x7777777777777777,
    0x8888888888888888, 0x9999999999999999, 0xaaaaaaaaaaaaaaaa, 0xbbbbbbbbbbbbbbbb,
    0xcccccccccccccccc, 0xdddddddddddddddd, 0xeeeeeeeeeeeeeeee, 0xffffffffffffffff,
    0xdeadbeafdeadbeaf, 0xbeafdeadbeafdead};

std::set<uint64_t> createPatterns64() {
    std::set<uint64_t> seen;
    seen.insert(0);
    size_t duplicates = 0;

    // produce numbers with the most significant bit is 0 to 63, i.e., numbers of all sizes
    for (int8_t msb = 0; msb <= 64; ++msb) {
        // mask to set the MSB, and to wipe everything else by subtracting 1
        uint64_t msbMask = 1ul << msb;
        // now use all the different patterns we defined
        for (auto pattern : basePatterns) {
            // use all possible 6-bit blocks to also "move over" the pattern
            for (uint64_t block = 0; block <= 0xful; ++block) {
                // move the block from the LSBs over the pattern, until it vanished beyond the MSB
                for (int8_t pos = 0; pos <= msb; ++pos) {
                    // create the input, by combining the different elements / ideas
                    uint64_t input = pattern;
                    // remove block bits
                    input &= ~(0xful << pos);
                    // remove add block
                    input |= (block << pos);
                    // cut to length
                    input &= (msbMask - 1);
                    // add MSB, if not outside of 64 bit range
                    if (msb < 64) {
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

    if (seen.size() + duplicates != basePatterns.size() * 16 * (33 * 65) + 1) {
        throw std::runtime_error(
            "inconsistent test state, unexpected number of visited 64-bit patterns");
    }
    if (seen.size() * 10 < duplicates) {
        throw std::runtime_error(
            "inconsistent test state, unexpected low percentage of unique 64-bit patterns");
    }

    return seen;
}

const std::set<uint64_t> patterns = createPatterns64(); // NOLINT(cert-err58-cpp)

TEST(testEncode64, patternProperties) {
    std::vector<size_t> bucket(65, 0);
    for (auto input : patterns) {
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
    ASSERT_EQ(bucket[10], 512); // up to here, all buckets are complete
    for (size_t i = 11u, p1 = bucket[10], p2 = bucket[9]; i < bucket.size(); ++i) {
        auto p0 = bucket[i];
        // we have more than previous or the one before (some spikes in the number of examples)
        EXPECT_TRUE(p0 > std::min(p1, p2)) << i << ": " << p0 << " " << p1 << " " << p2;
        p2 = p1;
        p1 = bucket[i];
    }
}

TEST(testEncode64, patternImportant) {
    ASSERT_TRUE(patterns.find(0) != patterns.end());
    ASSERT_TRUE(patterns.find(-1ul) != patterns.end());
    for (auto i = 0u; i <= 64; ++i) {
        ASSERT_TRUE(patterns.find(1ul << i) != patterns.end());
        ASSERT_TRUE(patterns.find((1ul << i) - 1) != patterns.end());
    }
}

TEST(testEncode64, encodeDecodeManyUnsigned) {
    for (auto input : patterns) {
        std::string encoded = encode64(input);
        ASSERT_EQ(input, decode64(encoded)) << encoded;
    }
}

TEST(testEncode64, encodeDecodeAllSigned) {
    for (auto uns : patterns) {
        auto input = static_cast<int64_t>(uns);
        std::string encoded = encode64Signed(input);
        ASSERT_EQ(input, decode64Signed(encoded)) << encoded;
    }
}

TEST(testEncode64, allPatternsValid) {
    for (auto input : patterns) {
        ASSERT_EQ(ERROR::OK, valid(encode64(input), 64));
        ASSERT_EQ(ERROR::OK, valid(encode64Signed(input), 64));
    }
}
