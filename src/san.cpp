#include <san.h>
#include <tables.h>
#include <array>

using namespace std;

#define CASE(i) if (blocks[(i) + 1] != enc[63] ? blocks[i] != enc[0] : blocks[i] != enc[63]) { return {blocks.begin() + (i), blocks.end()}; }

string encode24Signed(int32_t input) {
    array<char, 4> blocks{
            enc[input >> 18 & ONES],
            enc[input >> 12 & ONES],
            enc[input >> 6 & ONES],
            enc[input & ONES],
    };
    CASE(0)
    CASE(1)
    CASE(2)
    return {blocks.back()};
}

string encode32Signed(int32_t input) {
    array<char, 6> blocks{
            enc[input >> 30 & ONES],
            enc[input >> 24 & ONES],
            enc[input >> 18 & ONES],
            enc[input >> 12 & ONES],
            enc[input >> 6 & ONES],
            enc[input & ONES]
    };
    CASE(0)
    CASE(1)
    CASE(2)
    CASE(3)
    CASE(4)
    return {blocks.back()};
}

string encode48Signed(int64_t input) {
    array<char, 8> blocks{
            enc[input >> 42 & ONES],
            enc[input >> 36 & ONES],
            enc[input >> 30 & ONES],
            enc[input >> 24 & ONES],
            enc[input >> 18 & ONES],
            enc[input >> 12 & ONES],
            enc[input >> 6 & ONES],
            enc[input & ONES]
    };
    CASE(0)
    CASE(1)
    CASE(2)
    CASE(3)
    CASE(4)
    CASE(5)
    CASE(6)
    return {blocks.back()};
}

string encode128Signed(int64_t ab, int64_t cd) {
    array<char, 22> blocks{
            enc[ab >> 62 & ONES],
            enc[ab >> 56 & ONES],
            enc[ab >> 50 & ONES],
            enc[ab >> 44 & ONES],
            enc[ab >> 38 & ONES],
            enc[ab >> 32 & ONES],
            enc[ab >> 26 & ONES],
            enc[ab >> 20 & ONES],
            enc[ab >> 14 & ONES],
            enc[ab >> 8 & ONES],
            enc[ab >> 2 & ONES],
            enc[(ab << 4 & 0x30) | (cd >> 60 & 0x0f)],
            enc[cd >> 54 & ONES],
            enc[cd >> 48 & ONES],
            enc[cd >> 42 & ONES],
            enc[cd >> 36 & ONES],
            enc[cd >> 30 & ONES],
            enc[cd >> 24 & ONES],
            enc[cd >> 18 & ONES],
            enc[cd >> 12 & ONES],
            enc[cd >> 6 & ONES],
            enc[cd & ONES]
    };

    CASE(0)
    CASE(1)
    CASE(2)
    CASE(3)
    CASE(4)
    CASE(5)
    CASE(6)
    CASE(7)
    CASE(8)
    CASE(9)
    CASE(10)
    CASE(11)
    CASE(12)
    CASE(13)
    CASE(14)
    CASE(15)
    CASE(16)
    CASE(17)
    CASE(18)
    CASE(19)
    CASE(20)
    return {blocks.back()};
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

uint32_t decode32(const string &input, ERROR &error) {
    if (input.empty()) {
        error = ERROR::EMPTY;
        return -1;
    }

    // start with leading 1s, if the string indicates the number has six leading 1s
    int32_t res = input.front() == enc[ONES] ? -1 : 0;

    for (char byte: input) {
        // extract character, make sure the first bit is not set
        if (byte < 0) {
            error = ERROR::HIGH_BIT;
            return -1;
        }

        // decode character into six bit value, make sure the decode-table did not reject the character
        char bits = dec[byte];
        if (bits >= 64) {
            error = ERROR::WRONG_CHAR;
            return -1;
        }

        // shift digits we already have and add the six bits
        res <<= 6;
        res += bits;
    }

    error = ERROR::OK;
    return res;
}

uint64_t decode48(const string &input) {
    if (input.empty()) {
        return ERROR_48_EMPTY;
    }

    // start with leading 1s, if the string indicates the number has six leading 1s
    int64_t res = input.front() == enc[ONES] ? -1ul : 0;

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

pair<uint64_t, uint64_t> decode128(const string &input, ERROR &error) {
    if (input.empty()) {
        error = ERROR::EMPTY;
        return {-1, -1};
    }

    // start with leading 1s, if the string indicates the number has six leading 1s
    pair<int64_t, int64_t> res{input.front() == enc[ONES] ? -1ul : 0, input.front() == enc[ONES] ? -1ul : 0};

    for (char byte: input) {
        // extract character, make sure the first bit is not set
        if (byte < 0) {
            error = ERROR::HIGH_BIT;
            return {-1, -1};
        }

        // decode character into six bit value, make sure the decode-table did not reject the character
        char bits = dec[byte];
        if (bits >= 64) {
            error = ERROR::WRONG_CHAR;
            return {-1, -1};
        }

        // shift digits we already have and add the six bits
        res.first <<= 6;
        res.first += (res.second >> 58) & ONES;
        res.second <<= 6;
        res.second += bits;
    }

    error = ERROR::OK;
    return res;
}
