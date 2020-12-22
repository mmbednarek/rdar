#pragma once
#include <unordered_map>
#include <string>
#include <istream>

namespace rdar {

std::unordered_map<std::uint64_t, std::string> read_hashes(std::istream &stream);

std::uint64_t win_filetime_to_unix_ts(std::uint64_t filetime);

}