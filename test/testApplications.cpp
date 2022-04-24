#include <arpa/inet.h>
#include <gtest/gtest.h>
#include <map>
#include <san.h>

using namespace san;

std::map<std::string, std::string> ips{
    // NOLINT(cert-err58-cpp)
    {"0.0.0.0", "+"},           {"255.255.255.255", "-"},    {"192.168.0.0", "az+"},
    {"192.168.41.0", "aqz+"},   {"192.168.41.1", "1aqz+"},   {"192.168.41.255", "-aqz+"},
    {"192.168.255.0", "+-Wz+"}, {"10.0.0.0", "a"},           {"10.0.0.1", "1+++a"},
    {"10.0.0.24", "o+++a"},     {"10.0.0.192", "-++++a"},    {"10.0.0.193", "-1+++a"},
    {"172.16.0.0", "12I"},      {"172.80.99.0", "oR2I"},     {"1.2.3.4", "4+M81"},
    {"80.40.20.10", "a52xg"},   {"43.86.129.172", "0IwloH"}, {"43.86.129.197", "-5wloH"},
};

TEST(testApplications, commonIps) {
    for (const auto &it : ips) {
        const auto &ip = it.first;
        const auto &enc = it.second;
        in_addr addr{};
        inet_aton(ip.c_str(), &addr);
        std::string encoded = encode32(addr.s_addr);
        EXPECT_EQ(enc, encoded) << ip;
        EXPECT_EQ(addr.s_addr, decode32(encoded)) << ip;
    }
}

std::map<uint64_t, std::string> macs{
    // NOLINT(cert-err58-cpp)
    {0x000000000000, "+"},        {0xffffffffffff, "-"},        {0xc4c4c4c4c4c4, "Ncj4Ncj4"},
    {0x123456789012, "4zhmu9+i"}, {0xa1b2c3d4e5f6, "Erb3RenS"}, {0x123456000000, "4zhm++++"},
    {0x000000123456, "4zhm"},     {0x001b44113ab7, "1J44jGT"},  {0x2c549188c9e3, "b5ihycDz"}};

TEST(testApplications, someMACs) {
    for (const auto &it : macs) {
        const auto &mac = it.first;
        EXPECT_LT(mac, 1ul << 48);
        const auto &enc = it.second;
        std::string encoded = encode48(mac);
        EXPECT_EQ(enc, encoded) << std::hex << mac;
        EXPECT_EQ(mac, decode48(encoded)) << std::hex << mac;
    }
}

std::map<std::pair<uint64_t, uint64_t>, std::string> uuids{
    // NOLINT(cert-err58-cpp)
    {{0, 0}, "+"},
    {{-1, -1}, "-"},
    {{-1, 0}, "-M++++++++++"},
    {{0, -1}, "f----------"},
    {{3, -1}, "+-----------"},
    {{0xC4C4C4C4C4C4C4C4, 0xC4C4C4C4C4C4C4C4}, "-4Ncj4Ncj4Ncj4Ncj4Ncj4"},
    {{0x0123456789abcdef, 0x0123456789abcdef}, "18QlDyqLdXM4zhmu9GYTL"},
    {{0xfedcba9876543210, 0xfedcba9876543210}, "-0TbGotBgO4fXsKFxSl38g"},
    {{0x1234567800000000, 0x1234567800000000}, "id5pU+++++18QlDw+++++"},
    {{0x0000000012345678, 0x0000000012345678}, "4zhmu++++++id5pU"},
    {{0xc09db6b20b1443ba, 0xa90a554634cc44fa}, "-+DrqO2Nh3KGAalkoQP4jW"},
    {{0xa8922d72d00e493c, 0x95475983f28e384a}, "0EAyROQ+V9f9l7mofOzzxa"},
    {{0x9ba14f0a9c6041f6, 0x9305ab2876ce492c}, "0rEkYaD611ZFc5GOxSPAAI"}};

TEST(testApplications, someUUIDs) {
    for (const auto &it : uuids) {
        const auto &uuid = it.first;
        const auto &enc = it.second;
        std::string encoded = encode128(uuid.first, uuid.second);
        EXPECT_EQ(enc, encoded) << std::hex << uuid.first << " " << uuid.second;
        EXPECT_EQ(uuid, decode128(encoded)) << std::hex << uuid.first << " " << uuid.second;
    }
}
