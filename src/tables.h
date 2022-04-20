#ifndef SAN_TABLES_H
#define SAN_TABLES_H

constexpr const char encodeTable[65] = "+123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0-";
constexpr const char decodeTable[129] =
        "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@"
        "\x00" // +
        "@"
        "\x3f" // -
        "@@"
        "\x3e\x01\x02\x03\x04\x05\x06\x07\x08\x09" // 0-9
        "@@@@@@@"
        "\x24\x25\x26\x27\x28\x29\x2a\x2b\x2c\x2d\x2e\x2f\x30" // A-M
        "\x31\x32\x33\x34\x35\x36\x37\x38\x39\x3a\x3b\x3c\x3d" // N-Z
        "@@@@@@"
        "\x0a\x0b\x0c\x0d\x0e\x0f\x10\x11\x12\x13\x14\x15\x16" // a-m
        "\x17\x18\x19\x1a\x1b\x1c\x1d\x1e\x1f\x20\x21\x22\x23" // n-z
        "@@@@@";

#endif //SAN_TABLES_H
