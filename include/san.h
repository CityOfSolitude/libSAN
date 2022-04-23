#ifndef LIBSAN_SAN_H
#define LIBSAN_SAN_H

#include <string>

enum class ERROR {
    OK, EMPTY, HIGH_BIT, WRONG_CHAR
};

constexpr uint32_t ERROR_24_EMPTY = 1u << 24;
constexpr uint32_t ERROR_24_HIGH_BIT = 1u << 25;
constexpr uint32_t ERROR_24_WRONG_CHAR = 1u << 26;

constexpr uint64_t ERROR_48_EMPTY = 1ul << 48;
constexpr uint64_t ERROR_48_HIGH_BIT = 1ul << 49;
constexpr uint64_t ERROR_48_WRONG_CHAR = 1ul << 50;


/**
 * Encodes a 3-byte input value into an up-to 4-byte output string.
 * The first byte is irrelevant and will be ignored.
 * Signedness will just work, within the bound of a signed 24 bit value.
 *
 * The output will be as short as possible, by omitting leading 0 blocks.
 * Also we omit repeated, leading blocks of 1s, with the downside of having
 * to explicitly mark some positive numbers with a leading 1s block with an
 * additional 0s block.
 *
 * @param input a 24 bit value, encoded within a 32 bit value
 * @return a non-empty encoding of the input value.
 */
std::string encode24Signed(int32_t input);

/**
 * Convenience method for unsigned values; the encoding does not change and
 * will still encode very high unsigned values sparse.
 * @param input a 24 bit value, embedded within an unsigned 32 bit value
 * @return a non-empty encoding of the input value.
 */
inline std::string encode24(uint32_t input) {
    return encode24Signed(static_cast<int32_t>(input));
}

/**
 * Encodes a 4-byte input value into an up-to 6-byte output string.
 *
 * The output will be as short as possible, by omitting leading 0 blocks.
 * Also we omit repeated, leading blocks of 1s, with the downside of having
 * to explicitly mark some positive numbers with a leading 1s block with an
 * additional 0s block.
 *
 * For 32-bit values, we are left with a 2-bit high block, that will only ever
 * be encoded with the 000000, 000001, 111110 and 111111 block (since we honor
 * the sign!). For our encoding this means '+', '1', '0' or '-', respectively.
 *
 * @param input a 32 bit value
 * @return a non-empty encoding of the input value.
 */
std::string encode32Signed(int32_t input);

/**
 * Convenience method for unsigned values; the encoding does not change and
 * will still encode very high unsigned values sparse.
 * @param input a 32 bit value
 * @return a non-empty encoding of the input value.
 */
inline std::string encode32(uint32_t input) {
    return encode32Signed(static_cast<int32_t>(input));
}

/**
 * Encodes a 6-byte input value into an up-to 8-byte output string.
 * The first two bytes are irrelevant and will be ignored.
 * Signedness will just work, within the bound of a signed 48 bit value.
 *
 * The output will be as short as possible, by omitting leading 0 blocks.
 * Also we omit repeated, leading blocks of 1s, with the downside of having
 * to explicitly mark some positive numbers with a leading 1s block with an
 * additional 0s block.
 *
 * @param input a 24 bit value, encoded within a 64 bit value
 * @return a non-empty encoding of the input value.
 */
std::string encode48Signed(int64_t input);

/**
 * Convenience method for unsigned values; the encoding does not change and
 * will still encode very high unsigned values sparse.
 * @param input a 24 bit value, embedded within an unsigned 32 bit value
 * @return a non-empty encoding of the input value.
 */
inline std::string encode48(uint64_t input) {
    return encode48Signed(static_cast<int64_t>(input));
}

/**
 * Encodes a 16-byte input value into an up-to 22-byte output string.
 *
 * The output will be as short as possible, by omitting leading 0 blocks.
 * Also we omit repeated, leading blocks of 1s, with the downside of having
 * to explicitly mark some positive numbers with a leading 1s block with an
 * additional 0s block.
 *
 * @param input two 64 bit values
 * @return a non-empty encoding of the input value.
 */
std::string encode128Signed(int64_t ab, int64_t cd);

/**
 * Convenience method for unsigned values; the encoding does not change and
 * will still encode very high unsigned values sparse.
 * @param input two 64 bit values
 * @return a non-empty encoding of the input value.
 */
inline std::string encode128(uint64_t ab, uint64_t cd) {
    return encode128Signed(static_cast<int64_t>(ab), static_cast<int64_t>(cd));
}

/**
 * Decodes a previously encoded 3-byte value from its string representation.
 *
 * @param input a 1-4 byte string, which was the output of a previous encoding call
 * @return the decoded 24 bit value, interpreted as unsigned value
 */
uint32_t decode24(const std::string &input);

/**
 * Convenience method for signed values; the decoding does not differ from
 * the signed one, with the exception of the first 8 bits, which will repeat
 * the sign of the 24 bit number.
 *
 * @param input a 1-4 byte string, which was the output of a previous encoding call
 * @return the decoded 24 bit value, with the highest 8 bit replicating the sign
 */
inline int32_t decode24Signed(const std::string &input) {
    auto res = static_cast<int32_t>(decode24(input));
    return res & 0x00800000 ? res | static_cast<int32_t>(0xff000000) : res;
}

/**
 * Decodes a previously encoded 4-byte value from its string representation.
 *
 * @param input a 1-6 byte string, which was the output of a previous encoding call
 * @param error reference parameter to return whether the operation was successful or not
 * @return the decoded 32 bit value, interpreted as unsigned value
 */
uint32_t decode32(const std::string &input, ERROR &error);

/**
 * Convenience method for signed values; the decoding does not differ from
 * the signed one.
 *
 * @param input a 1-6 byte string, which was the output of a previous encoding call
 * @param error reference parameter to return whether the operation was successful or not
 * @return the decoded 32 bit value
 */
inline int32_t decode32Signed(const std::string &input, ERROR &error) {
    return static_cast<int32_t>(decode32(input, error));
}

/**
 * Decodes a previously encoded 6-byte value from its string representation.
 *
 * @param input a 1-8 byte string, which was the output of a previous encoding call
 * @return the decoded 48 bit value, interpreted as unsigned value
 */
uint64_t decode48(const std::string &input);

/**
 * Convenience method for signed values; the decoding does not differ from
 * the signed one, with the exception of the first 8 bits, which will repeat
 * the sign of the 48 bit number.
 *
 * @param input a 1-8 byte string, which was the output of a previous encoding call
 * @return the decoded 48 bit value, with the highest 8 bit replicating the sign
 */
inline int64_t decode48Signed(const std::string &input) {
    auto res = static_cast<int64_t>(decode48(input));
    return res & 0x00800000000000 ? res | static_cast<int64_t>(0xffff000000000000) : res;
}

/**
 * Decodes a previously encoded 16-byte value from its string representation.
 *
 * @param input a 1-22 byte string, which was the output of a previous encoding call
 * @param error reference parameter to return whether the operation was successful or not
 * @return the decoded 128 bit value, interpreted as unsigned value
 */
std::pair<uint64_t, uint64_t> decode128(const std::string &input, ERROR &error);

/**
 * Convenience method for signed values; the decoding does not differ from the signed one.
 *
 * @param input a 1-22 byte string, which was the output of a previous encoding call
 * @param error reference parameter to return whether the operation was successful or not
 * @return the decoded 128 bit value, with the highest 8 bit replicating the sign
 */
inline std::pair<int64_t, int64_t> decode128Signed(const std::string &input, ERROR &error) {
    return static_cast<std::pair<int64_t, int64_t>>(decode128(input, error));
}

#endif //LIBSAN_SAN_H
