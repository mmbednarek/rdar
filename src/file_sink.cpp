#include "file_sink.h"
#include <fmt/core.h>
#include <algorithm>
#include <filesystem>
#include <utility>

namespace rdar {

file_sink::file_sink(std::string base_path) : m_base_path(std::move(base_path)) {
    if (m_base_path.empty()) {
        throw std::runtime_error("base_path cannot be empty");
    }
}

std::ofstream file_sink::new_stream(std::string path) {
    std::replace(path.begin(), path.end(), '\\', '/');
    auto full_path = m_base_path + '/' + path;

    auto last_slash = full_path.find_last_of('/');
    std::filesystem::create_directories(full_path.substr(0, last_slash));

    fmt::print("extracting {}\n", full_path);

    return std::ofstream(full_path);
}

}
