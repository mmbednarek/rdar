#pragma once
#include <string>
#include <vector>
#include <fstream>

namespace rdar {

class file_sink {
    std::string m_base_path;
public:
    explicit file_sink(std::string base_path);

    [[nodiscard]] std::ofstream new_stream(std::string path);
};

}
