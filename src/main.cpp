#include "archive.h"
#include "util.h"
#include <chrono>
#include <cstring>
#include <ctime>
#include <fmt/core.h>
#include <fstream>

std::string human_readable_size(std::uint64_t size);

int main(int argc, char **argv) {
    if (argc < 3) {
        fmt::print(stderr, "not enough arguments");
        return 1;
    }

    const char *hashes_file_env = std::getenv("HASHES_FILE");
    std::string hashes_file = "hashes.csv";
    if (hashes_file_env != nullptr) {
        hashes_file = hashes_file_env;
    }

    const char *codebooks_file_env = std::getenv("CODEBOOKS_FILE");
    std::string codebooks_file = "packed_codebooks.bin";
    if (codebooks_file_env != nullptr) {
        codebooks_file = codebooks_file_env;
    }

    std::ifstream hashes_file_stream(hashes_file);
    if (!hashes_file_stream.is_open()) {
        fmt::print(stderr, "could not open hashes file");
        return 1;
    }
    auto hashes = rdar::read_hashes(hashes_file_stream);

    std::ifstream stream(argv[2]);
    if (!stream.is_open()) {
        fmt::print(stderr, "could not open file");
        return 1;
    }

    rdar::archive archive(stream, hashes, codebooks_file);

    if (std::strcmp(argv[1], "list") == 0) {
        auto files = archive.list_files();
        for (auto &f : files) {
            std::time_t unix_time = f.time;
            auto local = *std::localtime(&unix_time);
            fmt::print("{}-{}-{} {}:{}  {: <10} {:<32} {}\n", local.tm_year + 1900, local.tm_mon, local.tm_mday, local.tm_hour, local.tm_min, human_readable_size(f.size), f.hash, f.name);
        }
    } else if (std::strcmp(argv[1], "single") == 0) {
        if (argc < 4) {
            fmt::print(stderr, "not enough arguments");
            return 1;
        }

        auto hash = std::strtoull(argv[3], nullptr, 10);
        archive.extract_file(std::cout, hash);
    } else if (std::strcmp(argv[1], "extract") == 0) {
        if (argc < 4) {
            fmt::print(stderr, "not enough arguments");
            return 1;
        }

        rdar::file_sink sink(argv[3]);
        archive.extract_all(sink);
    } else if (std::strcmp(argv[1], "extract-wem") == 0) {
        if (argc < 4) {
            fmt::print(stderr, "not enough arguments");
            return 1;
        }

        rdar::file_sink sink(argv[3]);
        archive.extract_all_convert_wem(sink);
    }

    return 0;
}

std::string human_readable_size(std::uint64_t size) {
    if (size < 1024) {
        return std::to_string(size) + "B";
    }
    if (size < 1024 * 1024) {
        return std::to_string(size / 1024) + "K";
    }
    if (size < 1024 * 1024 * 1024) {
        return std::to_string(size / 1024 / 1024) + "M";
    }
    if (size < 1024l * 1024 * 1024 * 1024) {
        return std::to_string(size / 1024 / 1024 / 1024) + "G";
    }

    return std::to_string(size / 1024 / 1024 / 1024 / 1024) + "T";
}
