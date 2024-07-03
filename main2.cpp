#include <algorithm>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <thread>
#include <unordered_map>
#include <utility>
#include <vector>

#define BUFFER_SIZE 64
#define LOCATION_SEEK 10000
#define MIN_TEMP -1000
#define MAX_TEMP 1000

#pragma GCC optimize("Ofast,unroll-loops")
#pragma GCC target("avx,avx2,fma")

union Key {
    u_int64_t key;
    char str[8];
};

struct Info {
    int min;
    int max;
    int sum;
    int count;
};

typedef std::unordered_map<int64_t, Info> InfoMap;

int64_t getKey(const char* str, int len) {
    Key key;
    for (int i = 0; i < 8; i++) key.str[i] = str[i % len];
    key.key ^= len;
    return key.key;
}

int parseFloat(const char* str) {
    bool isNegative = *str == '-';
    if (isNegative) str++;
    int val = *str - '0';
    str++;
    if (*str != '.') val = (val * 10) + (*(str++) - '0');
    str++;
    val = (val * 10) + (*str - '0');
    return (isNegative) ? -val : val;
}

int main(int argc, char const* argv[]) {
    std::ios::sync_with_stdio(0);
    std::cin.tie(0);

    const char* filename = (argc > 1) ? argv[1] : "./measurements.txt";

    InfoMap data;
    std::set<std::string> locations;

    std::ifstream thread_file(filename);

    char str[BUFFER_SIZE];
    long long count = 0;
    while (thread_file.getline(str, BUFFER_SIZE)) {
        char* pos = strchr(str, ';');
        *pos = '\0';
        int val = parseFloat(pos + 1);
        int len = pos - str;
        int64_t key = getKey(str, len);
        if (count < LOCATION_SEEK) {
            locations.insert(str);
            count++;
        }
        auto entry = data.find(key);
        if (entry == data.end()) {
            data.insert(std::pair(key, Info{val, val, val, 1}));
        } else {
            Info& info = entry->second;
            info.min = std::min(info.min, val);
            info.max = std::max(info.max, val);
            info.sum += val;
            info.count++;
        }
    }

    std::cout << std::fixed << std::setprecision(1);
    std::cout << "{";
    bool isFirst = true;

    for (auto const& str : locations) {
        int64_t key = getKey(str.c_str(), str.size());
        if (!isFirst)
            std::cout << ", ";
        else
            isFirst = false;
        Info info = data.find(key)->second;
        std::cout << str << "=" << info.min / 10.0f << "/"
                  << info.sum / 10.0f / info.count << "/" << info.max / 10.0f;
    }
    std::cout << "}\n";

    return 0;
}
