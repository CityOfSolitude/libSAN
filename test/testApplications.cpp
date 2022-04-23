#include <gtest/gtest.h>
#include <san.h>
#include <arpa/inet.h>
#include <map>

std::map<std::string, std::string> ips{ // NOLINT(cert-err58-cpp)
        {"0.0.0.0",         "+"},
        {"255.255.255.255", "-"},
        {"192.168.0.0",     "az+"},
        {"192.168.41.0",    "aqz+"},
        {"192.168.41.1",    "1aqz+"},
        {"192.168.41.255",  "-aqz+"},
        {"192.168.255.0",   "+-Wz+"},
        {"10.0.0.0",        "a"},
        {"10.0.0.1",        "1+++a"},
        {"10.0.0.24",       "o+++a"},
        {"10.0.0.192",      "-++++a"},
        {"10.0.0.193",      "-1+++a"},
        {"172.16.0.0",      "12I"},
        {"172.80.99.0",     "oR2I"},
        {"1.2.3.4",         "4+M81"},
        {"80.40.20.10",     "a52xg"},
        {"43.86.129.172",   "0IwloH"},
        {"43.86.129.197",   "-5wloH"},
};

TEST(testApplications, commonIps) {
    for (const auto &it: ips) {
        const auto &ip = it.first;
        const auto &enc = it.second;
        in_addr addr{};
        inet_aton(ip.c_str(), &addr);
        ASSERT_EQ(enc, encode32(addr.s_addr)) << ip;
    }
}
