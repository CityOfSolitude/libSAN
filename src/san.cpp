#include <san.h>
#include <tables.h>

using namespace std;

string encode24Signed(int32_t input) {
    // catch the sign
    input <<= 8;
    input >>= 8;

    // add first character, always
    string res;
    do {
        res += encodeTable[input & 0x3f];
        input >>= 6;
        // stop if not all zeros or all ones and we did not add a block of all ones yet
    } while (input && (input != -1 || res.length() < 4 && res[res.length() - 1] != encodeTable[0x3f]));

    // if we stop with a positive number, but the highest six bits are all ones, add a leading zero
    if (input == 0 && res[res.length() - 1] == encodeTable[0x3f]) {
        res += encodeTable[0];
    }

    return res;
}

uint32_t decode24(const string &input) {
    // empty string is not allowed
    if (input.empty()) {
        return -1;
    }

    // start with leading 1s, if the string indicates the number has six leading 1s
    int32_t res = input.back() == encodeTable[0x3f] ? -1 : 0;

    // read string in reverse
    for (auto i = input.length() - 1; i + 1; --i) {
        // extract character, make sure the first bit is not set
        char byte = input[i];
        if (byte < 0) {
            return -1;
        }

        // decode character into six bit value, make sure the decode-table did not reject the character
        char bits = decodeTable[byte];
        if (bits >= 64) {
            return -1;
        }

        // shift digits we already have and add the six bits
        res <<= 6;
        res += bits;
    }

    // undo leading ones, which were needed for filling up sparse encodings
    return res & 0x00ffffff;
}
