#include "util.h"

namespace rdar {

std::unordered_map<std::uint64_t, std::string> read_hashes(std::istream &stream) {
    std::unordered_map<std::uint64_t, std::string> result;
    std::string line;

    while(std::getline(stream, line)) {
        auto at = line.find(',');
        if (at >= line.size()) {
            continue;
        }
        auto name_part = line.substr(0, at);
        auto hash_part = line.substr(at+1);
        auto hash = std::strtoull(hash_part.c_str(), nullptr, 10);

        result[hash] = name_part;
    }

    return result;
}

constexpr std::uint64_t g_win_tick = 10000000;
constexpr std::uint64_t g_epoch_diff = 11644473600LL;

std::uint64_t win_filetime_to_unix_ts(std::uint64_t filetime) {
    return filetime / g_win_tick - g_epoch_diff;
}

}