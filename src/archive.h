#pragma once
#include "file_sink.h"
#include "reader.h"
#include <cstddef>
#include <cstdint>
#include <cstring>

namespace rdar {

class header {
    std::uint64_t m_table_offset{};
    std::uint64_t m_table_size{};
    std::uint64_t m_unk{};
    std::uint64_t m_filesize{};

public:
    void deserialize(reader &r);

    [[nodiscard]] constexpr std::uint64_t table_offset() const;
    ;
};

class file_meta {
public:
    std::uint32_t m_id{};
    std::uint64_t m_hash{};
    std::uint64_t m_time{};
    std::uint32_t m_flags{};
    std::uint32_t m_first_sector{};
    std::uint32_t m_last_sector{};
    std::uint32_t m_first_unk{};
    std::uint32_t m_last_unk{};
    std::array<std::uint8_t, 20> m_sha1{};

    void deserialize(reader &r);
};

class offset {
public:
    std::uint64_t m_offset{};
    std::uint32_t m_physical_size{};
    std::uint32_t m_virtual_size{};

    void deserialize(reader &r);
};

class table {
    std::uint32_t m_number{};
    std::uint32_t m_size{};
    std::uint64_t m_checksum{};
    std::uint32_t m_num_files{};
    std::uint32_t m_num_offsets{};
    std::uint32_t m_num_hashes{};

    std::unordered_map<std::uint64_t, file_meta> m_file_entries{};
    std::vector<offset> m_offsets{};
    std::vector<std::uint64_t> m_hashes{};

public:
    void deserialize(reader &r);

    [[nodiscard]] const std::unordered_map<std::uint64_t, file_meta> &file_entries() const;
    [[nodiscard]] const std::vector<offset> &file_offsets() const;
    [[nodiscard]] const file_meta &meta_of(std::uint64_t hash) const;
    [[nodiscard]] const offset &offset_at(std::uint32_t id) const;
};

struct file_parsed_info {
    std::string name;
    std::uint64_t time;
    std::uint64_t size;
    std::uint64_t hash;
};

class archive {
    reader m_reader;
    std::unordered_map<std::uint64_t, std::string> m_hashes;
    header m_header{};
    table m_table{};
    std::string m_codebooks_file;

public:
    archive(std::istream &fs, std::unordered_map<std::uint64_t, std::string> hashes, std::string codebooks_file);

    std::string make_filename(std::uint64_t hash) const;
    std::vector<file_parsed_info> list_files();
    void extract_file(std::ostream &s, std::uint64_t hash);
    void extract_file_by_meta(std::ostream &s, const file_meta &meta);
    void extract_all(file_sink &sink);
    void extract_all_convert_wem(file_sink &sink);
    [[nodiscard]] std::size_t size_by_meta(const file_meta &meta);

private:
    [[nodiscard]] bool is_wem_file(const file_meta &meta);
    void extract_single_convert_wem(std::ostream &s, const file_meta &meta);
};

}// namespace rdar
