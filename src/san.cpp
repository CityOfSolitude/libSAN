#include <san.h>
#include <tables.h>

using namespace std;

string encode24Signed(int32_t input) {
    uint8_t b4 = input & ONES;
    uint8_t b3 = input >> 6 & ONES;
    uint8_t b2 = input >> 12 & ONES;
    uint8_t b1 = input >> 18 & ONES;

    if (b2 != ONES ? b1 : b1 != ONES) {
        return {enc[b1], enc[b2], enc[b3], enc[b4]};
    }

    if (b3 != ONES ? b2 : b2 != ONES) {
        return {enc[b2], enc[b3], enc[b4]};
    }

    if (b4 != ONES ? b3 : b3 != ONES) {
        return {enc[b3], enc[b4]};
    }

    return {enc[b4]};
}

string encode48Signed(int64_t input) {
    uint8_t b8 = input & ONES;
    uint8_t b7 = input >> 6 & ONES;
    uint8_t b6 = input >> 12 & ONES;
    uint8_t b5 = input >> 18 & ONES;
    uint8_t b4 = input >> 24 & ONES;
    uint8_t b3 = input >> 30 & ONES;
    uint8_t b2 = input >> 36 & ONES;
    uint8_t b1 = input >> 42 & ONES;

    if (b2 != ONES ? b1 : b1 != ONES) {
        return {enc[b1], enc[b2], enc[b3], enc[b4], enc[b5], enc[b6], enc[b7], enc[b8]};
    }

    if (b3 != ONES ? b2 : b2 != ONES) {
        return {enc[b2], enc[b3], enc[b4], enc[b5], enc[b6], enc[b7], enc[b8]};
    }

    if (b4 != ONES ? b3 : b3 != ONES) {
        return {enc[b3], enc[b4], enc[b5], enc[b6], enc[b7], enc[b8]};
    }

    if (b5 != ONES ? b4 : b4 != ONES) {
        return {enc[b4], enc[b5], enc[b6], enc[b7], enc[b8]};
    }

    if (b6 != ONES ? b5 : b5 != ONES) {
        return {enc[b5], enc[b6], enc[b7], enc[b8]};
    }

    if (b7 != ONES ? b6 : b6 != ONES) {
        return {enc[b6], enc[b7], enc[b8]};
    }

    if (b8 != ONES ? b7 : b7 != ONES) {
        return {enc[b7], enc[b8]};
    }

    return {enc[b8]};
}

uint32_t decode24(const string &input) {
    if (input.empty()) {
        return ERROR_24_EMPTY;
    }

    // start with leading 1s, if the string indicates the number has six leading 1s
    int32_t res = input.front() == enc[ONES] ? -1 : 0;

    for (char byte: input) {
        // extract character, make sure the first bit is not set
        if (byte < 0) {
            return ERROR_24_HIGH_BIT;
        }

        // decode character into six bit value, make sure the decode-table did not reject the character
        char bits = dec[byte];
        if (bits >= 64) {
            return ERROR_24_WRONG_CHAR;
        }

        // shift digits we already have and add the six bits
        res <<= 6;
        res += bits;
    }

    // undo leading ones, which were needed for filling up sparse encodings
    return res & 0x00ffffff;
}

uint64_t decode48(const string &input) {
    if (input.empty()) {
        return ERROR_48_EMPTY;
    }

    // start with leading 1s, if the string indicates the number has six leading 1s
    int64_t res = input.front() == enc[ONES] ? -1 : 0;

    for (char byte: input) {
        // extract character, make sure the first bit is not set
        if (byte < 0) {
            return ERROR_48_HIGH_BIT;
        }

        // decode character into six bit value, make sure the decode-table did not reject the character
        char bits = dec[byte];
        if (bits >= 64) {
            return ERROR_48_WRONG_CHAR;
        }

        // shift digits we already have and add the six bits
        res <<= 6;
        res += bits;
    }

    // undo leading ones, which were needed for filling up sparse encodings
    return res & 0x0000ffffffffffff;
}
