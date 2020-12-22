#include "reader.h"

namespace rdar {

reader::reader(std::istream &stream) : m_stream(stream) {
}

void reader::seek(std::uint64_t offset) {
    m_stream.seekg(static_cast<std::size_t>(offset), std::ios::beg);
}

void reader::write_to(std::ostream &out, std::size_t size) {
    auto buff = new char[size];
    m_stream.read(buff, size);
    out.write(buff, size);
    delete [] buff;
}

}
