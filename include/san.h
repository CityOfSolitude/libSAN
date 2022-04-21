#ifndef LIBSAN_SAN_H
#define LIBSAN_SAN_H

#include <string>

constexpr uint32_t ERROR_24_EMPTY = 0x01000000;
constexpr uint32_t ERROR_24_HIGH_BIT = 0x02000000;
constexpr uint32_t ERROR_24_WRONG_CHAR = 0x04000000;

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
  * Decodes a previously encoded 3-byte value from its string representation.
 *
 * @param input a 1-4 byte string, which was the output of a previous encoding call
 * @return the decoded 24 but value, interpreted as unsigned value
 */
uint32_t decode24(const std::string &input);

/**
 * Convenience method for signed values; the decoding does not differ from
 * the signed one, with the exception of the first 8 bits, which will repeat
 * the sign of the 24 bit number.
 *
 * @param input a 1-4 byte string, which was the output of a previous encoding call
 * @return the decoded 24 but value, with the highest 8 bit replicating the sign
 */
inline int32_t decode24Signed(const std::string &input) {
    auto res = static_cast<int32_t>(decode24(input));
    return res & 0x00800000 ? res | static_cast<int32_t>(0xff000000) : res;
}

#endif //LIBSAN_SAN_H
