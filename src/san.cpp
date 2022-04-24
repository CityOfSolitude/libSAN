#include <array>
#include <san.h>
#include <tables.h>

using namespace std;

namespace san {

ERROR valid(const string &input, size_t bitSize) {
    if (input.empty()) {
        return ERROR::EMPTY;
    }

    for (char byte : input) {
        if (byte < 0) {
            return ERROR::HIGH_BIT;
        }
        if (dec[byte] >= 64) {
            return ERROR::WRONG_CHAR;
        }
    }

    if (bitSize) {
        int unneededBytes = static_cast<int>(bitSize + 5) / 6 - input.size();
        if (unneededBytes < 0) {
            return ERROR::TOO_LONG;
        } else if (unneededBytes == 0) {
            auto rest = bitSize % 6;
            if (rest && dec[input.front()] & ~((1u << rest) - 1)) {
                return ERROR::TOO_LONG;
            }
        }
    }

    return ERROR::OK;
}

#define CASE(i)                                                                                    \
    if (blocks[(i) + 1] != enc[63] ? blocks[i] != enc[0] : blocks[i] != enc[63]) {                 \
        return {blocks.begin() + (i), blocks.end()};                                               \
    }

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
    array<char, 6> blocks{enc[input >> 30 & ONES], enc[input >> 24 & ONES], enc[input >> 18 & ONES],
                          enc[input >> 12 & ONES], enc[input >> 6 & ONES],  enc[input & ONES]};
    CASE(0)
    CASE(1)
    CASE(2)
    CASE(3)
    CASE(4)
    return {blocks.back()};
}

string encode48Signed(int64_t input) {
    array<char, 8> blocks{enc[input >> 42 & ONES], enc[input >> 36 & ONES], enc[input >> 30 & ONES],
                          enc[input >> 24 & ONES], enc[input >> 18 & ONES], enc[input >> 12 & ONES],
                          enc[input >> 6 & ONES],  enc[input & ONES]};
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
        enc[ab >> 62 & ONES], enc[ab >> 56 & ONES], enc[ab >> 50 & ONES],
        enc[ab >> 44 & ONES], enc[ab >> 38 & ONES], enc[ab >> 32 & ONES],
        enc[ab >> 26 & ONES], enc[ab >> 20 & ONES], enc[ab >> 14 & ONES],
        enc[ab >> 8 & ONES],  enc[ab >> 2 & ONES],  enc[(ab << 4 & 0x30) | (cd >> 60 & 0x0f)],
        enc[cd >> 54 & ONES], enc[cd >> 48 & ONES], enc[cd >> 42 & ONES],
        enc[cd >> 36 & ONES], enc[cd >> 30 & ONES], enc[cd >> 24 & ONES],
        enc[cd >> 18 & ONES], enc[cd >> 12 & ONES], enc[cd >> 6 & ONES],
        enc[cd & ONES]};

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
    int32_t res = input.front() == enc[ONES] ? -1u : 0;
    for (char byte : input) {
        res = (res << 6) + dec[byte];
    }
    return res & (1u << 24) - 1;
}

uint32_t decode32(const string &input) {
    auto res = input.front() == enc[ONES] ? -1u : 0;
    for (char byte : input) {
        res = (res << 6) + dec[byte];
    }
    return res;
}

uint64_t decode48(const string &input) {
    auto res = input.front() == enc[ONES] ? -1ul : 0;
    for (char byte : input) {
        res = (res << 6) + dec[byte];
    }
    return res & (1ul << 48) - 1;
}

pair<uint64_t, uint64_t> decode128(const string &input) {
    auto init = input.front() == enc[ONES] ? -1ul : 0;
    pair<uint64_t, uint64_t> res{init, init};
    for (char byte : input) {
        res.first = (res.first << 6) + (res.second >> 58 & ONES);
        res.second = (res.second << 6) + dec[byte];
    }
    return res;
}

} // namespace san
