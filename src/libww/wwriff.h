#ifndef _WWRIFF_H
#define _WWRIFF_H

#ifndef __STDC_CONSTANT_MACROS
#define __STDC_CONSTANT_MACROS
#endif
#include "errors.h"
#include "oggstream.h"
#include <cstdint>
#include <fstream>
#include <iostream>
#include <string>

#define VERSION "0.24"

namespace libww {

enum class force_packet_format {
    kNoForcePacketFormat,
    kForceModPackets,
    kForceNoModPackets
};


class converter {
    std::string m_codebooks_name;
    std::istream &m_stream;
    long m_buff_size;

    bool m_little_endian = true;

    long m_riff_size = -1;
    long m_fmt_offset = -1, m_cue_offset = -1, m_list_offset = -1, m_smpl_offset = -1, m_vorb_offset = -1, m_data_offset = -1;
    long m_fmt_size = -1, m_cue_size = -1, m_list_size = -1, m_smpl_size = -1, m_vorb_size = -1, m_data_size = -1;

    // RIFF fmt
    uint16_t m_channels{};
    uint32_t m_sample_rate{};
    uint32_t m_avg_bytes_per_second{};

    // RIFF extended fmt
    uint16_t m_ext_unk{};
    uint32_t m_subtype{};

    // cue info
    uint32_t m_cue_count{};

    // smpl info
    uint32_t m_loop_count{}, m_loop_start{}, m_loop_end{};

    // vorbis info
    uint32_t m_sample_count{};
    uint32_t m_setup_packet_offset{};
    uint32_t m_first_audio_packet_offset{};
    uint32_t m_uid{};
    uint8_t m_blocksize_0_pow{};
    uint8_t m_blocksize_1_pow{};

    const bool m_inline_codebooks = false, m_full_setup = false;
    bool m_header_triad_present = false, m_old_packet_headers = false;
    bool m_no_granule = false, m_mod_packets = false;

    uint16_t (*m_read_16)(std::istream &is) = nullptr;
    uint32_t (*m_read_32)(std::istream &is) = nullptr;

public:
    converter(
            std::istream &stream,
            std::size_t buffsize,
            const std::string &_codebooks_name,
            bool inline_codebooks,
            bool full_setup,
            force_packet_format force_packet_format);

    void print_info();

    void generate_ogg(std::ostream &of);
    void generate_ogg_header(oggstream &os, bool *&mode_blockflag, int &mode_bits);
    void generate_ogg_header_with_triad(oggstream &os);
};

#endif

}