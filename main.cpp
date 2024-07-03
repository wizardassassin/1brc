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

long long chunk_size;
long long file_size;
long long threadCount;
const char* filename;

int main(int argc, char const* argv[]) {
    std::ios::sync_with_stdio(0);
    std::cin.tie(0);

    filename = (argc > 1) ? argv[1] : "./measurements.txt";

    threadCount = std::thread::hardware_concurrency();

    std::vector<std::thread> thread_arr(threadCount);
    std::vector<InfoMap> data_arr(threadCount);
    std::set<std::string> locations;

    std::ifstream main_file(filename);
    main_file.seekg(0, main_file.end);
    file_size = main_file.tellg();
    main_file.seekg(0, main_file.beg);

    chunk_size = file_size / threadCount;

    for (int i = 0; i < threadCount; i++) {
        thread_arr[i] = std::thread([i, &data_arr, &locations]() {
            std::ifstream thread_file(filename);
            long long start_loc = (i == 0) ? 0 : chunk_size * i - 1;
            long long end_loc =
                (i == threadCount - 1) ? file_size : chunk_size * (i + 1);
            thread_file.seekg(start_loc, thread_file.beg);

            auto& data = data_arr[i];

            char str[BUFFER_SIZE];
            if (start_loc != 0) {
                char c;
                thread_file >> c;
                // delimiter (newline) check
                if ((long long)thread_file.tellg() - start_loc == 1) {
                    thread_file.getline(str, BUFFER_SIZE);
                }
            }
            long long count = 0;
            while (thread_file.tellg() < end_loc &&
                   thread_file.getline(str, BUFFER_SIZE)) {
                char* pos = strchr(str, ';');
                *pos = '\0';
                int val = parseFloat(pos + 1);
                int len = pos - str;
                int64_t key = getKey(str, len);
                if (i == 0 && count < LOCATION_SEEK) {
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
        });
    }
    for (int i = 0; i < threadCount; i++) thread_arr[i].join();

    std::cout << std::fixed << std::setprecision(1);
    std::cout << "{";
    bool isFirst = true;

    for (auto const& str : locations) {
        int64_t key = getKey(str.c_str(), str.size());
        if (!isFirst)
            std::cout << ", ";
        else
            isFirst = false;
        Info info{MAX_TEMP, MIN_TEMP, 0, 0};
        for (int i = 0; i < threadCount; i++) {
            auto the_pair = data_arr[i].find(key);
            if (the_pair == data_arr[i].end()) continue;
            auto& info2 = the_pair->second;
            info.min = std::min(info.min, info2.min);
            info.max = std::max(info.max, info2.max);
            info.sum += info2.sum;
            info.count += info2.count;
        }
        std::cout << str << "=" << info.min / 10.0f << "/"
                  << info.sum / 10.0f / info.count << "/" << info.max / 10.0f;
    }
    std::cout << "}\n";

    return 0;
}
