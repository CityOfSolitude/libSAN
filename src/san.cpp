#include <san.h>
#include <tables.h>

using namespace std;

string encode24Signed(int32_t input) {
    uint8_t b4 = input & ONES;
    uint8_t b3 = input >> 6 & ONES;
    uint8_t b2 = input >> 12 & ONES;
    uint8_t b1 = input >> 18 & ONES;

    if (b2 != ONES ? b1 : b1 != ONES) {
        return {encodeTable[b1], encodeTable[b2], encodeTable[b3], encodeTable[b4]};
    }

    if (b3 != ONES ? b2 : b2 != ONES) {
        return {encodeTable[b2], encodeTable[b3], encodeTable[b4]};
    }

    if (b4 != ONES ? b3 : b3 != ONES) {
        return {encodeTable[b3], encodeTable[b4]};
    }

    return {encodeTable[b4]};
}

uint32_t decode24(const string &input) {
    if (input.empty()) {
        return ERROR_24_EMPTY;
    }

    // start with leading 1s, if the string indicates the number has six leading 1s
    int32_t res = input.front() == encodeTable[ONES] ? -1 : 0;

    for (char byte: input) {
        // extract character, make sure the first bit is not set
        if (byte < 0) {
            return ERROR_24_HIGH_BIT;
        }

        // decode character into six bit value, make sure the decode-table did not reject the character
        char bits = decodeTable[byte];
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
