#include <gtest/gtest.h>
#include <san.h>
#include <set>
#include <vector>

TEST(testEncode32, encodeSmallNaturals) {
    std::string encoded;
    constexpr int32_t bound = 63;

    // we can encode 63 small numbers in one character, the 64th is reserved for -1
    for (auto i = 0; i < bound; ++i) {
        encoded = encode32(i);
        ASSERT_EQ(encoded.length(), 1);
        encoded = encode32Signed(i);
        ASSERT_EQ(encoded.length(), 1);
    }

    // verify the next number is encoded larger
    encoded = encode32(bound);
    ASSERT_EQ(encoded.length(), 2);
    encoded = encode32Signed(bound);
    ASSERT_EQ(encoded.length(), 2);
}

TEST(testEncode32, encodeLargerNaturals) {
    std::string encoded;
    constexpr int32_t bound = 64 * 63;

    // we can encode 64 * 63 number with 2 (or 1) digit,
    // since all starting with an all 1s block have already size 3
    for (auto i = 63; i < bound; ++i) {
        encoded = encode32(i);
        ASSERT_EQ(encoded.length(), 2);
        encoded = encode32Signed(i);
        ASSERT_EQ(encoded.length(), 2);
    }

    // again check that the next number is encoded larger
    encoded = encode32(bound);
    ASSERT_EQ(encoded.length(), 3);
    encoded = encode32Signed(bound);
    ASSERT_EQ(encoded.length(), 3);
}

TEST(testEncode32, encodeHighestTwoBits) {
    std::set<uint32_t> seen;
    size_t counter = 0;
    for (uint32_t i = 0; i < 1u << 14; ++i) {
        seen.insert(i);
        const std::string &enc = encode32(i << 16);
        if (enc.length() < 6) {
            continue;
        }
        ASSERT_EQ('+', enc[0]);
        ++counter;
    }
    ASSERT_EQ(1u << 14, seen.size());
    // only if the highest 6 bit are all ones, we need to add the leading '+'
    // since we generate all patterns with 14 bits, this leaves 8 bits, i.e., 256 different patterns
    ASSERT_EQ(256, counter);
    for (uint32_t i = 1u << 14; i < 1u << 15; ++i) {
        seen.insert(i);
        ASSERT_EQ('1', encode32(i << 16)[0]);
        ++counter;
    }
    ASSERT_EQ((1u << 14) * 2, seen.size());
    ASSERT_EQ((1u << 14) + 256, counter); // the remaining cases do not have this "problem"
    for (uint32_t i = 1u << 15; i < (1u << 15) + (1u << 14); ++i) {
        seen.insert(i);
        ASSERT_EQ('0', encode32(i << 16)[0]);
        ++counter;
    }
    ASSERT_EQ((1u << 14) * 3, seen.size());
    ASSERT_EQ((1u << 14) * 2 + 256, counter);
    for (uint32_t i = (1u << 15) + (1u << 14); i < 1u << 16; ++i) {
        seen.insert(i);
        ASSERT_EQ('-', encode32(i << 16)[0]);
        ++counter;
    }
    ASSERT_EQ((1u << 14) * 4, seen.size());
    ASSERT_EQ((1u << 14) * 3 + 256, counter);
}

TEST(testEncode32, encodeSmallNegatives) {
    std::string encoded;
    constexpr int32_t bound = -65;

    // the special -1, which just needs one character
    encoded = encode32(-1);
    ASSERT_EQ(encoded.length(), 1);
    encoded = encode32Signed(-1);
    ASSERT_EQ(encoded.length(), 1);

    // we can encode the next 63 small negative numbers with two chars
    for (auto i = -2; i > bound; --i) {
        encoded = encode32(i);
        ASSERT_EQ(encoded.length(), 2);
        encoded = encode32Signed(i);
        ASSERT_EQ(encoded.length(), 2);
    }

    // verify that the next number encodes larger
    encoded = encode32(bound);
    ASSERT_EQ(encoded.length(), 3);
    encoded = encode32Signed(bound);
    ASSERT_EQ(encoded.length(), 3);
}

TEST(testEncode32, encodeLargerNegatives) {
    std::string encoded;
    constexpr int32_t bound = -64 * 64 - 1;

    // we can encode 64 * 63 number with 2 (or 1) digit,
    // since all starting with an all 1s block have already size 3
    for (auto i = -65; i > bound; --i) {
        encoded = encode32(i);
        ASSERT_EQ(encoded.length(), 3);
        encoded = encode32Signed(i);
        ASSERT_EQ(encoded.length(), 3);
    }

    // again check that the next number is encoded larger
    encoded = encode32(bound);
    ASSERT_EQ(encoded.length(), 4);
    encoded = encode32Signed(bound);
    ASSERT_EQ(encoded.length(), 4);
}

TEST(testEncode32, encodeProperties) {
    // we count the encoding length of all possible 24 bit values,
    // which are either 1, 2, 3 or 4 characters
    std::vector<size_t> counter(5, 0);
    for (int32_t input = 0; input < (1u << 24); ++input) {
        std::string encoded = encode32(input);
        ASSERT_EQ(ERROR::OK, valid(encoded, 32));
        auto length = encoded.length();
        // empty is not allowed and neither is larger than 4
        ASSERT_GT(length, 0);
        ASSERT_LE(length, 5);
        ++counter[length - 1];
    }
    // these are the amount of numbers we can encode in each length segment
    // in contrast to the 24 bit encoding, some numbers are in the 5 byte range,
    // since we do not test the full range, we miss out on the short encodings
    // of small negative numbers
    ASSERT_EQ(counter[0], 64 - 1); // 0 to 62, 63 has a two byte encoding, -1 is not tested
    ASSERT_EQ(counter[1], (1 << 12) - (1 << 7) + 1);          // 2^12 - 2^6 - 63 small negatives
    ASSERT_EQ(counter[2], (1 << 18) - (1 << 13) + (1 << 6));  // 2^18 - 2^12 - 63*64 small negatives
    ASSERT_EQ(counter[3], (1 << 24) - (1 << 19) + (1 << 12)); // 2^24 - 2^18 - 63*64*64 negatives
    ASSERT_EQ(counter[4], (1 << 18)); // positive numbers with leading 1s, that overflew into 5 byte
}

const std::vector<uint32_t> basePatterns = { // NOLINT(cert-err58-cpp)
    0x00000000, 0x11111111, 0x22222222, 0x33333333, 0x44444444, 0x55555555,
    0x66666666, 0x77777777, 0x88888888, 0x99999999, 0xaaaaaaaa, 0xbbbbbbbb,
    0xcccccccc, 0xdddddddd, 0xeeeeeeee, 0xffffffff, 0xdeadbeaf, 0xbeafdead};

std::set<uint32_t> createPatterns32() {
    std::set<uint32_t> seen;
    seen.insert(0);
    size_t duplicates = 0;

    // produce numbers with the most significant bit is 0 to 31, i.e., numbers of all sizes
    for (int8_t msb = 0; msb <= 32; ++msb) {
        // mask to set the MSB, and to wipe everything else by subtracting 1
        uint32_t msbMask = 1 << msb;
        // now use all the different patterns we defined
        for (auto pattern : basePatterns) {
            // use all possible 6-bit blocks to also "move over" the pattern
            for (uint32_t block = 0; block <= 0x3f; ++block) {
                // move the block from the LSBs over the pattern, until it vanished beyond the MSB
                for (int8_t pos = 0; pos <= msb; ++pos) {
                    // create the input, by combining the different elements / ideas
                    uint32_t input = pattern;
                    // remove block bits
                    input &= ~(0x3f << pos);
                    // remove add block
                    input |= (block << pos);
                    // cut to length
                    input &= (msbMask - 1);
                    // add MSB, if not outside of 32 bit range
                    if (msb < 32) {
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

    if (seen.size() + duplicates != basePatterns.size() * 64 * (17 * 33) + 1) {
        throw std::runtime_error(
            "inconsistent test state, unexpected number of visited 32-bit patterns");
    }
    if (seen.size() * 3 < duplicates) {
        throw std::runtime_error(
            "inconsistent test state, unexpected low percentage of unique 32-bit patterns");
    }
    return seen;
}

const std::set<uint32_t> patterns = createPatterns32(); // NOLINT(cert-err58-cpp)

TEST(testEncode32, patternProperties) {
    std::vector<size_t> bucket(33, 0);
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

TEST(testEncode32, patternImportant) {
    ASSERT_TRUE(patterns.find(0) != patterns.end());
    ASSERT_TRUE(patterns.find(0xffffffff) != patterns.end());
    for (auto i = 0u; i <= 32; ++i) {
        ASSERT_TRUE(i == 32 || patterns.find(1u << i) != patterns.end());
        ASSERT_TRUE(patterns.find((1u << i) - 1) != patterns.end());
    }
}

TEST(testEncode32, encodeDecodeManyUnsigned) {
    for (auto input : patterns) {
        std::string encoded = encode32(input);
        ASSERT_EQ(input, decode32(encoded)) << encoded;
    }
}

TEST(testEncode32, encodeDecodeAllSigned) {
    for (auto uns : patterns) {
        auto input = static_cast<int32_t>(uns);
        std::string encoded = encode32Signed(input);
        ASSERT_EQ(input, decode32Signed(encoded)) << encoded;
    }
}
