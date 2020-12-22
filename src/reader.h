#pragma once
#include <array>
#include <iostream>
#include <algorithm>

namespace rdar {

class reader {
    std::istream &m_stream;
public:
    explicit reader(std::istream &stream);

    void seek(std::uint64_t offset);
    void write_to(std::ostream &out, std::size_t size);

    template <typename T> [[nodiscard]] T read();
    template <typename T, std::size_t S> void read_n(std::array<T, S> &arr);
};

template <typename T> T reader::read() {
    T result;
    m_stream.read(reinterpret_cast<char *>(&result), sizeof(T));
    return result;
}

template <typename T, std::size_t S> void reader::read_n(std::array<T, S> &arr) {
    std::generate_n(arr.begin(), S, [this](){
      T result;
      m_stream.read(reinterpret_cast<char *>(&result), sizeof(T));
      return result;
    });
}

}
