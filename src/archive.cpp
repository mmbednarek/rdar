#include "archive.h"
#include "libww/wwriff.h"
#include "util.h"
#include <fmt/core.h>
#include <sstream>
#include <utility>

namespace rdar {

constexpr auto g_expected_magic = std::array<char, 4>{'R', 'D', 'A', 'R'};
constexpr auto g_compression_magic = std::array<char, 4>{'K', 'A', 'R', 'K'};
constexpr auto g_wem_magic = std::array<char, 4>{'R', 'I', 'F', 'F'};
constexpr std::uint32_t g_expected_version = 12;

void header::deserialize(rdar::reader &r) {
    std::array<char, 4> magic{};
    r.read_n(magic);
    if (magic != g_expected_magic) {
        throw std::runtime_error("invalid file signature");
    }

    auto version = r.read<std::uint32_t>();
    if (version != g_expected_version) {
        throw std::runtime_error("invalid archive version");
    }

    m_table_offset = r.read<std::uint64_t>();
    m_table_size = r.read<std::uint64_t>();
    m_unk = r.read<std::uint64_t>();
    m_filesize = r.read<std::uint64_t>();
}

constexpr std::uint64_t header::table_offset() const {
    return m_table_offset;
}

void file_meta::deserialize(reader &r) {
    m_hash = r.read<std::uint64_t>();
    m_time = r.read<std::uint64_t>();
    m_flags = r.read<std::uint32_t>();
    m_first_sector = r.read<std::uint32_t>();
    m_last_sector = r.read<std::uint32_t>();
    m_first_unk = r.read<std::uint32_t>();
    m_last_unk = r.read<std::uint32_t>();
    r.read_n(m_sha1);
}
void offset::deserialize(reader &r) {
    m_offset = r.read<std::uint64_t>();
    m_physical_size = r.read<std::uint32_t>();
    m_virtual_size = r.read<std::uint32_t>();
}
void table::deserialize(reader &r) {
    m_number = r.read<std::uint32_t>();
    m_size = r.read<std::uint32_t>();
    m_checksum = r.read<std::uint64_t>();
    m_num_files = r.read<std::uint32_t>();
    m_num_offsets = r.read<std::uint32_t>();
    m_num_hashes = r.read<std::uint32_t>();

    for (std::size_t i = 0; i < m_num_files; ++i) {
        file_meta entry;
        entry.m_id = i;
        entry.deserialize(r);
        m_file_entries[entry.m_hash] = entry;
    }

    m_offsets.resize(m_num_offsets);
    std::generate_n(m_offsets.begin(), m_num_offsets, [&r]() {
        offset entry;
        entry.deserialize(r);
        return entry;
    });

    m_hashes.resize(m_num_hashes);
    std::generate_n(m_hashes.begin(), m_num_hashes, [&r]() {
        return r.read<std::uint64_t>();
    });
}

const std::unordered_map<std::uint64_t, file_meta> &table::file_entries() const {
    return m_file_entries;
}

const std::vector<offset> &table::file_offsets() const {
    return m_offsets;
}

const offset &table::offset_at(std::uint32_t id) const {
    return m_offsets[id];
}

const file_meta &table::meta_of(std::uint64_t hash) const {
    return m_file_entries.at(hash);
}

archive::archive(std::istream &fs, std::unordered_map<std::uint64_t, std::string> hashes, std::string codebooks_file) : m_reader(fs), m_hashes(std::move(hashes)), m_codebooks_file(std::move(codebooks_file)) {
    m_header.deserialize(m_reader);
    m_reader.seek(m_header.table_offset());
    m_table.deserialize(m_reader);
}

std::string archive::make_filename(std::uint64_t hash) const {
    auto at = m_hashes.find(hash);
    if (at != m_hashes.end()) {
        return at->second;
    }
    return std::to_string(hash) + ".bin";
}

std::size_t archive::size_by_meta(const file_meta &meta) {
    std::uint64_t size = 0;
    for (std::size_t i = meta.m_first_sector; i < meta.m_last_sector; ++i) {
        auto &off = m_table.offset_at(i);
        size += off.m_physical_size;
    }
    return size;
}

std::vector<file_parsed_info> archive::list_files() {
    auto &entries = m_table.file_entries();
    auto &offsets = m_table.file_offsets();

    std::vector<file_parsed_info> result(entries.size());
    std::transform(entries.begin(), entries.end(), result.begin(), [this, offsets](std::pair<std::uint64_t, file_meta> pair) {
        auto m = pair.second;
        return file_parsed_info{.name = make_filename(m.m_hash), .time = win_filetime_to_unix_ts(m.m_time), .size = size_by_meta(m), .hash = m.m_hash};
    });

    return result;
}

void archive::extract_file(std::ostream &out, std::uint64_t hash) {
    auto &meta = m_table.meta_of(hash);
    extract_file_by_meta(out, meta);
}

bool archive::is_wem_file(const file_meta &meta) {
    auto &off = m_table.offset_at(meta.m_first_sector);
    if (off.m_physical_size != off.m_virtual_size)
        return false;

    m_reader.seek(off.m_offset);
    std::array<char, 4> magic{};
    m_reader.read_n(magic);

    return magic == g_wem_magic;
}

void archive::extract_file_by_meta(std::ostream &out, const file_meta &meta) {
    for (std::size_t i = meta.m_first_sector; i < meta.m_last_sector; ++i) {
        auto &off = m_table.offset_at(i);
        m_reader.seek(off.m_offset);

        if (off.m_physical_size != off.m_virtual_size) {
            throw std::runtime_error("compression not supported");
        }

        m_reader.write_to(out, off.m_physical_size);
    }
}

void archive::extract_all(file_sink &sink) {
    auto &entries = m_table.file_entries();
    std::for_each(entries.begin(), entries.end(), [this, &sink](std::pair<std::uint64_t, file_meta> pair) {
        auto m = pair.second;
        auto name = make_filename(m.m_hash);

        auto out_stream = sink.new_stream(name);
        extract_file_by_meta(out_stream, m);
        out_stream.close();
    });
}

void archive::extract_all_convert_wem(file_sink &sink) {
    auto &entries = m_table.file_entries();
    std::for_each(entries.begin(), entries.end(), [this, &sink](std::pair<std::uint64_t, file_meta> pair) {
        auto m = pair.second;
        auto name = make_filename(m.m_hash);

        if (!is_wem_file(m)) {
            return;
        }

        auto dot_at = name.find_last_of('.');
        auto out_stream = sink.new_stream(name.substr(0, dot_at) + ".ogg");

        try {
            extract_single_convert_wem(out_stream, m);
        } catch (parse_error_str &e) {
            fmt::print("could not extract file: {}\n", name);
        }
    });
}

void archive::extract_single_convert_wem(std::ostream &s, const file_meta &meta) {
    std::stringstream ss;
    extract_file_by_meta(ss, meta);

    libww::converter conv(ss, size_by_meta(meta), m_codebooks_file, false, false, libww::force_packet_format::kNoForcePacketFormat);
    conv.generate_ogg(s);
}

}// namespace rdar
