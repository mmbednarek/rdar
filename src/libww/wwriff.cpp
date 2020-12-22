#define __STDC_CONSTANT_MACROS
#include "wwriff.h"
#include "codebook.h"
#include "errors.h"
#include "oggstream.h"
#include <cstdint>
#include <cstring>
#include <iostream>

using namespace std;

namespace libww {

/* Modern 2 or 6 byte header */
class packet {
    long m_offset;
    uint16_t m_size = -1;
    uint32_t m_absolute_granule{};
    bool m_no_granule;

public:
    packet(istream &i, long o, bool little_endian, bool no_granule = false) : m_offset(o), m_no_granule(no_granule) {
        i.seekg(m_offset);

        if (little_endian) {
            m_size = read_16_le(i);
            if (!m_no_granule) {
                m_absolute_granule = read_32_le(i);
            }
        } else {
            m_size = read_16_be(i);
            if (!m_no_granule) {
                m_absolute_granule = read_32_be(i);
            }
        }
    }

    [[nodiscard]] constexpr long header_size() const { return m_no_granule ? 2 : 6; }
    [[nodiscard]] constexpr long offset() const { return m_offset + header_size(); }
    [[nodiscard]] constexpr uint16_t size() const { return m_size; }
    [[nodiscard]] constexpr uint32_t granule() const { return m_absolute_granule; }
    [[nodiscard]] constexpr long next_offset() const { return m_offset + header_size() + m_size; }
};

/* Old 8 byte header */
class packet_old {
    long m_offset;
    uint32_t m_size;
    uint32_t m_absolute_granule;

public:
    packet_old(istream &i, long o, bool little_endian) : m_offset(o), m_size(-1), m_absolute_granule(0) {
        i.seekg(m_offset);

        if (little_endian) {
            m_size = read_32_le(i);
            m_absolute_granule = read_32_le(i);
        } else {
            m_size = read_32_be(i);
            m_absolute_granule = read_32_be(i);
        }
    }

    [[nodiscard]] constexpr static long header_size() { return 8; }
    [[nodiscard]] constexpr long offset() const { return m_offset + header_size(); }
    [[nodiscard]] constexpr uint32_t size() const { return m_size; }
    [[nodiscard]] constexpr uint32_t granule() const { return m_absolute_granule; }
    [[nodiscard]] constexpr long next_offset() const { return m_offset + header_size() + m_size; }
};

class header {
    uint8_t m_type;

    static const char g_vorbis_str[6];

public:
    explicit header(uint8_t t) : m_type(t) {}

    friend oggstream &operator<<(oggstream &bstream, const header &vph) {
        Bit_uint<8> t(vph.m_type);
        bstream << t;

        for (char i : g_vorbis_str) {
            Bit_uint<8> c(i);
            bstream << c;
        }

        return bstream;
    }
};

const char header::g_vorbis_str[6] = {'v', 'o', 'r', 'b', 'i', 's'};

converter::converter(
        std::istream &stream,
        std::size_t buffsize,
        const string &codebooks_name,
        bool inline_codebooks,
        bool full_setup,
        force_packet_format force_packet_format)
    : m_codebooks_name(codebooks_name),
      m_stream(stream),
      m_buff_size(buffsize),
      m_inline_codebooks(inline_codebooks),
      m_full_setup(full_setup) {
    // check RIFF header
    {
        unsigned char riff_head[4], wave_head[4];
        m_stream.seekg(0, ios::beg);
        m_stream.read(reinterpret_cast<char *>(riff_head), 4);

        if (memcmp(&riff_head[0], "RIFX", 4)) {
            if (memcmp(&riff_head[0], "RIFF", 4)) {
                throw parse_error_str("missing RIFF");
            } else {
                m_little_endian = true;
            }
        } else {
            m_little_endian = false;
        }

        if (m_little_endian) {
            m_read_16 = read_16_le;
            m_read_32 = read_32_le;
        } else {
            m_read_16 = read_16_be;
            m_read_32 = read_32_be;
        }

        m_riff_size = m_read_32(m_stream) + 8;

        if (m_riff_size > m_buff_size) throw parse_error_str("RIFF truncated");

        m_stream.read(reinterpret_cast<char *>(wave_head), 4);
        if (memcmp(&wave_head[0], "WAVE", 4)) throw parse_error_str("missing WAVE");
    }

    // read chunks
    long chunk_offset = 12;
    while (chunk_offset < m_riff_size) {
        m_stream.seekg(chunk_offset, ios::beg);

        if (chunk_offset + 8 > m_riff_size) throw parse_error_str("chunk header truncated");

        char chunk_type[4];
        m_stream.read(chunk_type, 4);
        uint32_t chunk_size;

        chunk_size = m_read_32(m_stream);

        if (!memcmp(chunk_type, "fmt ", 4)) {
            m_fmt_offset = chunk_offset + 8;
            m_fmt_size = chunk_size;
        } else if (!memcmp(chunk_type, "cue ", 4)) {
            m_cue_offset = chunk_offset + 8;
            m_cue_size = chunk_size;
        } else if (!memcmp(chunk_type, "LIST", 4)) {
            m_list_offset = chunk_offset + 8;
            m_list_size = chunk_size;
        } else if (!memcmp(chunk_type, "smpl", 4)) {
            m_smpl_offset = chunk_offset + 8;
            m_smpl_size = chunk_size;
        } else if (!memcmp(chunk_type, "vorb", 4)) {
            m_vorb_offset = chunk_offset + 8;
            m_vorb_size = chunk_size;
        } else if (!memcmp(chunk_type, "data", 4)) {
            m_data_offset = chunk_offset + 8;
            m_data_size = chunk_size;
        }

        chunk_offset = chunk_offset + 8 + chunk_size;
    }

    if (chunk_offset > m_riff_size) throw parse_error_str("chunk truncated");

    // check that we have the chunks we're expecting
    if (-1 == m_fmt_offset && -1 == m_data_offset) throw parse_error_str("expected fmt, data chunks");

    // read fmt
    if (-1 == m_vorb_offset && 0x42 != m_fmt_size) throw parse_error_str("expected 0x42 fmt if vorb missing");

    if (-1 != m_vorb_offset && 0x28 != m_fmt_size && 0x18 != m_fmt_size && 0x12 != m_fmt_size) throw parse_error_str("bad fmt size");

    if (-1 == m_vorb_offset && 0x42 == m_fmt_size) {
        // fake it out
        m_vorb_offset = m_fmt_offset + 0x18;
    }

    m_stream.seekg(m_fmt_offset, ios::beg);
    if (UINT16_C(0xFFFF) != m_read_16(m_stream)) throw parse_error_str("bad codec id");
    m_channels = m_read_16(m_stream);
    m_sample_rate = m_read_32(m_stream);
    m_avg_bytes_per_second = m_read_32(m_stream);
    if (0U != m_read_16(m_stream)) throw parse_error_str("bad block align");
    if (0U != m_read_16(m_stream)) throw parse_error_str("expected 0 bps");
    if (m_fmt_size - 0x12 != m_read_16(m_stream)) throw parse_error_str("bad extra fmt length");

    if (m_fmt_size - 0x12 >= 2) {
        // read extra fmt
        m_ext_unk = m_read_16(m_stream);
        if (m_fmt_size - 0x12 >= 6) {
            m_subtype = m_read_32(m_stream);
        }
    }

    if (m_fmt_size == 0x28) {
        char whoknowsbuf[16];
        const unsigned char whoknowsbuf_check[16] = {1, 0, 0, 0, 0, 0, 0x10, 0, 0x80, 0, 0, 0xAA, 0, 0x38, 0x9b, 0x71};
        m_stream.read(whoknowsbuf, 16);
        if (memcmp(whoknowsbuf, whoknowsbuf_check, 16)) throw parse_error_str("expected signature in extra fmt?");
    }

    // read cue
    if (-1 != m_cue_offset) {
#if 0
        if (0x1c != _cue_size) throw Parse_error_str("bad cue size");
#endif
        m_stream.seekg(m_cue_offset);

        m_cue_count = m_read_32(m_stream);
    }

    // read LIST
    if (-1 != m_list_offset) {
#if 0
        if ( 4 != _LIST_size ) throw Parse_error_str("bad LIST size");
        char adtlbuf[4];
        const char adtlbuf_check[4] = {'a','d','t','l'};
        _infile.seekg(_LIST_offset);
        _infile.read(adtlbuf, 4);
        if (memcmp(adtlbuf, adtlbuf_check, 4)) throw Parse_error_str("expected only adtl in LIST");
#endif
    }

    // read smpl
    if (-1 != m_smpl_offset) {
        m_stream.seekg(m_smpl_offset + 0x1C);
        m_loop_count = m_read_32(m_stream);

        if (1 != m_loop_count) throw parse_error_str("expected one loop");

        m_stream.seekg(m_smpl_offset + 0x2c);
        m_loop_start = m_read_32(m_stream);
        m_loop_end = m_read_32(m_stream);
    }

    // read vorb
    switch (m_vorb_size) {
        case -1:
        case 0x28:
        case 0x2A:
        case 0x2C:
        case 0x32:
        case 0x34:
            m_stream.seekg(m_vorb_offset + 0x00, ios::beg);
            break;

        default:
            throw parse_error_str("bad vorb size");
            break;
    }

    m_sample_count = m_read_32(m_stream);

    switch (m_vorb_size) {
        case -1:
        case 0x2A: {
            m_no_granule = true;

            m_stream.seekg(m_vorb_offset + 0x4, ios::beg);
            uint32_t mod_signal = m_read_32(m_stream);

            // set
            // D9     11011001
            // CB     11001011
            // BC     10111100
            // B2     10110010
            // unset
            // 4A     01001010
            // 4B     01001011
            // 69     01101001
            // 70     01110000
            // A7     10100111 !!!

            // seems to be 0xD9 when _mod_packets should be set
            // also seen 0xCB, 0xBC, 0xB2
            if (0x4A != mod_signal && 0x4B != mod_signal && 0x69 != mod_signal && 0x70 != mod_signal) {
                m_mod_packets = true;
            }
            m_stream.seekg(m_vorb_offset + 0x10, ios::beg);
            break;
        }

        default:
            m_stream.seekg(m_vorb_offset + 0x18, ios::beg);
            break;
    }

    if (force_packet_format == force_packet_format::kForceNoModPackets) {
        m_mod_packets = false;
    } else if (force_packet_format == force_packet_format::kForceModPackets) {
        m_mod_packets = true;
    }

    m_setup_packet_offset = m_read_32(m_stream);
    m_first_audio_packet_offset = m_read_32(m_stream);

    switch (m_vorb_size) {
        case -1:
        case 0x2A:
            m_stream.seekg(m_vorb_offset + 0x24, ios::beg);
            break;

        case 0x32:
        case 0x34:
            m_stream.seekg(m_vorb_offset + 0x2C, ios::beg);
            break;
    }

    switch (m_vorb_size) {
        case 0x28:
        case 0x2C:
            // ok to leave _uid, _blocksize_0_pow and _blocksize_1_pow unset
            m_header_triad_present = true;
            m_old_packet_headers = true;
            break;

        case -1:
        case 0x2A:
        case 0x32:
        case 0x34:
            m_uid = m_read_32(m_stream);
            m_blocksize_0_pow = m_stream.get();
            m_blocksize_1_pow = m_stream.get();
            break;
    }

    // check/set loops now that we know total sample count
    if (0 != m_loop_count) {
        if (m_loop_end == 0) {
            m_loop_end = m_sample_count;
        } else {
            m_loop_end = m_loop_end + 1;
        }

        if (m_loop_start >= m_sample_count || m_loop_end > m_sample_count || m_loop_start > m_loop_end)
            throw parse_error_str("loops out of range");
    }

    // check subtype now that we know the vorb info
    // this is clearly just the channel layout
    switch (m_subtype) {
        case 4:    /* 1 channel, no seek table */
        case 3:    /* 2 channels */
        case 0x33: /* 4 channels */
        case 0x37: /* 5 channels, seek or not */
        case 0x3b: /* 5 channels, no seek table */
        case 0x3f: /* 6 channels, no seek table */
            break;
        default:
            //throw Parse_error_str("unknown subtype");
            break;
    }
}

void converter::print_info(void) {
    if (m_little_endian) {
        cout << "RIFF WAVE";
    } else {
        cout << "RIFX WAVE";
    }
    cout << " " << m_channels << " channel";
    if (m_channels != 1) cout << "s";
    cout << " " << m_sample_rate << " Hz " << m_avg_bytes_per_second * 8 << " bps" << endl;
    cout << m_sample_count << " samples" << endl;

    if (0 != m_loop_count) {
        cout << "loop from " << m_loop_start << " to " << m_loop_end << endl;
    }

    if (m_old_packet_headers) {
        cout << "- 8 byte (old) packet headers" << endl;
    } else if (m_no_granule) {
        cout << "- 2 byte packet headers, no granule" << endl;
    } else {
        cout << "- 6 byte packet headers" << endl;
    }

    if (m_header_triad_present) {
        cout << "- Vorbis header triad present" << endl;
    }

    if (m_full_setup || m_header_triad_present) {
        cout << "- full setup header" << endl;
    } else {
        cout << "- stripped setup header" << endl;
    }

    if (m_inline_codebooks || m_header_triad_present) {
        cout << "- inline codebooks" << endl;
    } else {
        cout << "- external codebooks (" << m_codebooks_name << ")" << endl;
    }

    if (m_mod_packets) {
        cout << "- modified Vorbis packets" << endl;
    } else {
        cout << "- standard Vorbis packets" << endl;
    }

#if 0
    if (0 != _cue_count)
    {
        cout << _cue_count << " cue point";
        if (_cue_count != 1) cout << "s";
        cout << endl;
    }
#endif
}

void converter::generate_ogg_header(oggstream &os, bool *&mode_blockflag, int &mode_bits) {
    // generate identification packet
    {
        header vhead(1);

        os << vhead;

        Bit_uint<32> version(0);
        os << version;

        Bit_uint<8> ch(m_channels);
        os << ch;

        Bit_uint<32> srate(m_sample_rate);
        os << srate;

        Bit_uint<32> bitrate_max(0);
        os << bitrate_max;

        Bit_uint<32> bitrate_nominal(m_avg_bytes_per_second * 8);
        os << bitrate_nominal;

        Bit_uint<32> bitrate_minimum(0);
        os << bitrate_minimum;

        Bit_uint<4> blocksize_0(m_blocksize_0_pow);
        os << blocksize_0;

        Bit_uint<4> blocksize_1(m_blocksize_1_pow);
        os << blocksize_1;

        Bit_uint<1> framing(1);
        os << framing;

        // identification packet on its own page
        os.flush_page();
    }

    // generate comment packet
    {
        header vhead(3);

        os << vhead;

        static const char vendor[] = "converted from Audiokinetic Wwise by ww2ogg " VERSION;
        Bit_uint<32> vendor_size(strlen(vendor));

        os << vendor_size;
        for (unsigned int i = 0; i < vendor_size; i++) {
            Bit_uint<8> c(vendor[i]);
            os << c;
        }

        if (0 == m_loop_count) {
            // no user comments
            Bit_uint<32> user_comment_count(0);
            os << user_comment_count;
        } else {
            // two comments, loop start and end
            Bit_uint<32> user_comment_count(2);
            os << user_comment_count;

            stringstream loop_start_str;
            stringstream loop_end_str;

            loop_start_str << "LoopStart=" << m_loop_start;
            loop_end_str << "LoopEnd=" << m_loop_end;

            Bit_uint<32> loop_start_comment_length;
            loop_start_comment_length = loop_start_str.str().length();
            os << loop_start_comment_length;
            for (unsigned int i = 0; i < loop_start_comment_length; i++) {
                Bit_uint<8> c(loop_start_str.str().c_str()[i]);
                os << c;
            }

            Bit_uint<32> loop_end_comment_length;
            loop_end_comment_length = loop_end_str.str().length();
            os << loop_end_comment_length;
            for (unsigned int i = 0; i < loop_end_comment_length; i++) {
                Bit_uint<8> c(loop_end_str.str().c_str()[i]);
                os << c;
            }
        }

        Bit_uint<1> framing(1);
        os << framing;

        //os.flush_bits();
        os.flush_page();
    }

    // generate setup packet
    {
        header vhead(5);

        os << vhead;

        packet setup_packet(m_stream, m_data_offset + m_setup_packet_offset, m_little_endian, m_no_granule);

        m_stream.seekg(setup_packet.offset());
        if (setup_packet.granule() != 0) throw parse_error_str("setup packet granule != 0");
        bit_oggstream ss(m_stream);

        // codebook count
        Bit_uint<8> codebook_count_less1;
        ss >> codebook_count_less1;
        unsigned int codebook_count = codebook_count_less1 + 1;
        os << codebook_count_less1;

        //cout << codebook_count << " codebooks" << endl;

        // rebuild codebooks
        if (m_inline_codebooks) {
            codebook_library cbl;

            for (unsigned int i = 0; i < codebook_count; i++) {
                if (m_full_setup) {
                    cbl.copy(ss, os);
                } else {
                    cbl.rebuild(ss, 0, os);
                }
            }
        } else {
            /* external codebooks */

            codebook_library cbl(m_codebooks_name);

            for (unsigned int i = 0; i < codebook_count; i++) {
                Bit_uint<10> codebook_id;
                ss >> codebook_id;
                //cout << "Codebook " << i << " = " << codebook_id << endl;
                try {
                    cbl.rebuild(codebook_id, os);
                } catch (invalid_id e) {
                    //         B         C         V
                    //    4    2    4    3    5    6
                    // 0100 0010 0100 0011 0101 0110
                    // \_______|____ ___|/
                    //              X
                    //            11 0100 0010

                    if (codebook_id == 0x342) {
                        Bit_uint<14> codebook_identifier;
                        ss >> codebook_identifier;

                        //         B         C         V
                        //    4    2    4    3    5    6
                        // 0100 0010 0100 0011 0101 0110
                        //           \_____|_ _|_______/
                        //                   X
                        //         01 0101 10 01 0000
                        if (codebook_identifier == 0x1590) {
                            // starts with BCV, probably --full-setup
                            throw parse_error_str(
                                    "invalid codebook id 0x342, try --full-setup");
                        }
                    }

                    // just an invalid codebook
                    throw e;
                }
            }
        }

        // Time Domain transforms (placeholder)
        Bit_uint<6> time_count_less1(0);
        os << time_count_less1;
        Bit_uint<16> dummy_time_value(0);
        os << dummy_time_value;

        if (m_full_setup) {

            while (ss.get_total_bits_read() < setup_packet.size() * 8u) {
                Bit_uint<1> bitly;
                ss >> bitly;
                os << bitly;
            }
        } else// _full_setup
        {
            // floor count
            Bit_uint<6> floor_count_less1;
            ss >> floor_count_less1;
            unsigned int floor_count = floor_count_less1 + 1;
            os << floor_count_less1;

            // rebuild floors
            for (unsigned int i = 0; i < floor_count; i++) {
                // Always floor type 1
                Bit_uint<16> floor_type(1);
                os << floor_type;

                Bit_uint<5> floor1_partitions;
                ss >> floor1_partitions;
                os << floor1_partitions;

                unsigned int *floor1_partition_class_list = new unsigned int[floor1_partitions];

                unsigned int maximum_class = 0;
                for (unsigned int j = 0; j < floor1_partitions; j++) {
                    Bit_uint<4> floor1_partition_class;
                    ss >> floor1_partition_class;
                    os << floor1_partition_class;

                    floor1_partition_class_list[j] = floor1_partition_class;

                    if (floor1_partition_class > maximum_class)
                        maximum_class = floor1_partition_class;
                }

                unsigned int *floor1_class_dimensions_list = new unsigned int[maximum_class + 1];

                for (unsigned int j = 0; j <= maximum_class; j++) {
                    Bit_uint<3> class_dimensions_less1;
                    ss >> class_dimensions_less1;
                    os << class_dimensions_less1;

                    floor1_class_dimensions_list[j] = class_dimensions_less1 + 1;

                    Bit_uint<2> class_subclasses;
                    ss >> class_subclasses;
                    os << class_subclasses;

                    if (0 != class_subclasses) {
                        Bit_uint<8> masterbook;
                        ss >> masterbook;
                        os << masterbook;

                        if (masterbook >= codebook_count)
                            throw parse_error_str("invalid floor1 masterbook");
                    }

                    for (unsigned int k = 0; k < (1U << class_subclasses); k++) {
                        Bit_uint<8> subclass_book_plus1;
                        ss >> subclass_book_plus1;
                        os << subclass_book_plus1;

                        int subclass_book = static_cast<int>(subclass_book_plus1) - 1;
                        if (subclass_book >= 0 && static_cast<unsigned int>(subclass_book) >= codebook_count)
                            throw parse_error_str("invalid floor1 subclass book");
                    }
                }

                Bit_uint<2> floor1_multiplier_less1;
                ss >> floor1_multiplier_less1;
                os << floor1_multiplier_less1;

                Bit_uint<4> rangebits;
                ss >> rangebits;
                os << rangebits;

                for (unsigned int j = 0; j < floor1_partitions; j++) {
                    unsigned int current_class_number = floor1_partition_class_list[j];
                    for (unsigned int k = 0; k < floor1_class_dimensions_list[current_class_number]; k++) {
                        Bit_uintv X(rangebits);
                        ss >> X;
                        os << X;
                    }
                }

                delete[] floor1_class_dimensions_list;
                delete[] floor1_partition_class_list;
            }

            // residue count
            Bit_uint<6> residue_count_less1;
            ss >> residue_count_less1;
            unsigned int residue_count = residue_count_less1 + 1;
            os << residue_count_less1;

            // rebuild residues
            for (unsigned int i = 0; i < residue_count; i++) {
                Bit_uint<2> residue_type;
                ss >> residue_type;
                os << Bit_uint<16>(residue_type);

                if (residue_type > 2) throw parse_error_str("invalid residue type");

                Bit_uint<24> residue_begin, residue_end, residue_partition_size_less1;
                Bit_uint<6> residue_classifications_less1;
                Bit_uint<8> residue_classbook;

                ss >> residue_begin >> residue_end >> residue_partition_size_less1 >> residue_classifications_less1 >> residue_classbook;
                unsigned int residue_classifications = residue_classifications_less1 + 1;
                os << residue_begin << residue_end << residue_partition_size_less1 << residue_classifications_less1 << residue_classbook;

                if (residue_classbook >= codebook_count) throw parse_error_str("invalid residue classbook");

                unsigned int *residue_cascade = new unsigned int[residue_classifications];

                for (unsigned int j = 0; j < residue_classifications; j++) {
                    Bit_uint<5> high_bits(0);
                    Bit_uint<3> low_bits;

                    ss >> low_bits;
                    os << low_bits;

                    Bit_uint<1> bitflag;
                    ss >> bitflag;
                    os << bitflag;
                    if (bitflag) {
                        ss >> high_bits;
                        os << high_bits;
                    }

                    residue_cascade[j] = high_bits * 8 + low_bits;
                }

                for (unsigned int j = 0; j < residue_classifications; j++) {
                    for (unsigned int k = 0; k < 8; k++) {
                        if (residue_cascade[j] & (1 << k)) {
                            Bit_uint<8> residue_book;
                            ss >> residue_book;
                            os << residue_book;

                            if (residue_book >= codebook_count) throw parse_error_str("invalid residue book");
                        }
                    }
                }

                delete[] residue_cascade;
            }

            // mapping count
            Bit_uint<6> mapping_count_less1;
            ss >> mapping_count_less1;
            unsigned int mapping_count = mapping_count_less1 + 1;
            os << mapping_count_less1;

            for (unsigned int i = 0; i < mapping_count; i++) {
                // always mapping type 0, the only one
                Bit_uint<16> mapping_type(0);

                os << mapping_type;

                Bit_uint<1> submaps_flag;
                ss >> submaps_flag;
                os << submaps_flag;

                unsigned int submaps = 1;
                if (submaps_flag) {
                    Bit_uint<4> submaps_less1;

                    ss >> submaps_less1;
                    submaps = submaps_less1 + 1;
                    os << submaps_less1;
                }

                Bit_uint<1> square_polar_flag;
                ss >> square_polar_flag;
                os << square_polar_flag;

                if (square_polar_flag) {
                    Bit_uint<8> coupling_steps_less1;
                    ss >> coupling_steps_less1;
                    unsigned int coupling_steps = coupling_steps_less1 + 1;
                    os << coupling_steps_less1;

                    for (unsigned int j = 0; j < coupling_steps; j++) {
                        Bit_uintv magnitude(ilog(m_channels - 1)), angle(ilog(m_channels - 1));

                        ss >> magnitude >> angle;
                        os << magnitude << angle;

                        if (angle == magnitude || magnitude >= m_channels || angle >= m_channels) throw parse_error_str("invalid coupling");
                    }
                }

                // a rare reserved field not removed by Ak!
                Bit_uint<2> mapping_reserved;
                ss >> mapping_reserved;
                os << mapping_reserved;
                if (0 != mapping_reserved) throw parse_error_str("mapping reserved field nonzero");

                if (submaps > 1) {
                    for (unsigned int j = 0; j < m_channels; j++) {
                        Bit_uint<4> mapping_mux;
                        ss >> mapping_mux;
                        os << mapping_mux;

                        if (mapping_mux >= submaps) throw parse_error_str("mapping_mux >= submaps");
                    }
                }

                for (unsigned int j = 0; j < submaps; j++) {
                    // Another! Unused time domain transform configuration placeholder!
                    Bit_uint<8> time_config;
                    ss >> time_config;
                    os << time_config;

                    Bit_uint<8> floor_number;
                    ss >> floor_number;
                    os << floor_number;
                    if (floor_number >= floor_count) throw parse_error_str("invalid floor mapping");

                    Bit_uint<8> residue_number;
                    ss >> residue_number;
                    os << residue_number;
                    if (residue_number >= residue_count) throw parse_error_str("invalid residue mapping");
                }
            }

            // mode count
            Bit_uint<6> mode_count_less1;
            ss >> mode_count_less1;
            unsigned int mode_count = mode_count_less1 + 1;
            os << mode_count_less1;

            mode_blockflag = new bool[mode_count];
            mode_bits = ilog(mode_count - 1);

            //cout << mode_count << " modes" << endl;

            for (unsigned int i = 0; i < mode_count; i++) {
                Bit_uint<1> block_flag;
                ss >> block_flag;
                os << block_flag;

                mode_blockflag[i] = (block_flag != 0);

                // only 0 valid for windowtype and transformtype
                Bit_uint<16> windowtype(0), transformtype(0);
                os << windowtype << transformtype;

                Bit_uint<8> mapping;
                ss >> mapping;
                os << mapping;
                if (mapping >= mapping_count) throw parse_error_str("invalid mode mapping");
            }

            Bit_uint<1> framing(1);
            os << framing;

        }// _full_setup

        os.flush_page();

        if ((ss.get_total_bits_read() + 7) / 8 != setup_packet.size()) throw parse_error_str("didn't read exactly setup packet");

        if (setup_packet.next_offset() != m_data_offset + static_cast<long>(m_first_audio_packet_offset)) throw parse_error_str("first audio packet doesn't follow setup packet");
    }
}

void converter::generate_ogg(std::ostream &of) {
    oggstream os(of);

    bool *mode_blockflag = NULL;
    int mode_bits = 0;
    bool prev_blockflag = false;

    if (m_header_triad_present) {
        generate_ogg_header_with_triad(os);
    } else {
        generate_ogg_header(os, mode_blockflag, mode_bits);
    }

    // Audio pages
    {
        long offset = m_data_offset + m_first_audio_packet_offset;

        while (offset < m_data_offset + m_data_size) {
            uint32_t size, granule;
            long packet_header_size, packet_payload_offset, next_offset;

            if (m_old_packet_headers) {
                packet_old audio_packet(m_stream, offset, m_little_endian);
                packet_header_size = audio_packet.header_size();
                size = audio_packet.size();
                packet_payload_offset = audio_packet.offset();
                granule = audio_packet.granule();
                next_offset = audio_packet.next_offset();
            } else {
                packet audio_packet(m_stream, offset, m_little_endian, m_no_granule);
                packet_header_size = audio_packet.header_size();
                size = audio_packet.size();
                packet_payload_offset = audio_packet.offset();
                granule = audio_packet.granule();
                next_offset = audio_packet.next_offset();
            }

            if (offset + packet_header_size > m_data_offset + m_data_size) {
                throw parse_error_str("page header truncated");
            }

            offset = packet_payload_offset;

            m_stream.seekg(offset);
            // HACK: don't know what to do here
            if (granule == UINT32_C(0xFFFFFFFF)) {
                os.set_granule(1);
            } else {
                os.set_granule(granule);
            }

            // first byte
            if (m_mod_packets) {
                // need to rebuild packet type and window info

                if (!mode_blockflag) {
                    throw parse_error_str("didn't load mode_blockflag");
                }

                // OUT: 1 bit packet type (0 == audio)
                Bit_uint<1> packet_type(0);
                os << packet_type;

                Bit_uintv *mode_number_p = 0;
                Bit_uintv *remainder_p = 0;

                {
                    // collect mode number from first byte

                    bit_oggstream ss(m_stream);

                    // IN/OUT: N bit mode number (max 6 bits)
                    mode_number_p = new Bit_uintv(mode_bits);
                    ss >> *mode_number_p;
                    os << *mode_number_p;

                    // IN: remaining bits of first (input) byte
                    remainder_p = new Bit_uintv(8 - mode_bits);
                    ss >> *remainder_p;
                }

                if (mode_blockflag[*mode_number_p]) {
                    // long window, peek at next frame

                    m_stream.seekg(next_offset);
                    bool next_blockflag = false;
                    if (next_offset + packet_header_size <= m_data_offset + m_data_size) {

                        // mod_packets always goes with 6-byte headers
                        packet audio_packet(m_stream, next_offset, m_little_endian, m_no_granule);
                        uint32_t next_packet_size = audio_packet.size();
                        if (next_packet_size > 0) {
                            m_stream.seekg(audio_packet.offset());

                            bit_oggstream ss(m_stream);
                            Bit_uintv next_mode_number(mode_bits);

                            ss >> next_mode_number;

                            next_blockflag = mode_blockflag[next_mode_number];
                        }
                    }

                    // OUT: previous window type bit
                    Bit_uint<1> prev_window_type(prev_blockflag);
                    os << prev_window_type;

                    // OUT: next window type bit
                    Bit_uint<1> next_window_type(next_blockflag);
                    os << next_window_type;

                    // fix seek for rest of stream
                    m_stream.seekg(offset + 1);
                }

                prev_blockflag = mode_blockflag[*mode_number_p];
                delete mode_number_p;

                // OUT: remaining bits of first (input) byte
                os << *remainder_p;
                delete remainder_p;
            } else {
                // nothing unusual for first byte
                int v = m_stream.get();
                if (v < 0) {
                    throw parse_error_str("file truncated");
                }
                Bit_uint<8> c(v);
                os << c;
            }

            // remainder of packet
            for (unsigned int i = 1; i < size; i++) {
                int v = m_stream.get();
                if (v < 0) {
                    throw parse_error_str("file truncated");
                }
                Bit_uint<8> c(v);
                os << c;
            }

            offset = next_offset;
            os.flush_page(false, (offset == m_data_offset + m_data_size));
        }
        if (offset > m_data_offset + m_data_size) throw parse_error_str("page truncated");
    }

    delete[] mode_blockflag;
}

void converter::generate_ogg_header_with_triad(oggstream &os) {
    // Header page triad
    {
        long offset = m_data_offset + m_setup_packet_offset;

        // copy information packet
        {
            packet_old information_packet(m_stream, offset, m_little_endian);
            uint32_t size = information_packet.size();

            if (information_packet.granule() != 0) {
                throw parse_error_str("information packet granule != 0");
            }

            m_stream.seekg(information_packet.offset());

            Bit_uint<8> c(m_stream.get());
            if (1 != c) {
                throw parse_error_str("wrong type for information packet");
            }

            os << c;

            for (unsigned int i = 1; i < size; i++) {
                c = m_stream.get();
                os << c;
            }

            // identification packet on its own page
            os.flush_page();

            offset = information_packet.next_offset();
        }

        // copy comment packet
        {
            packet_old comment_packet(m_stream, offset, m_little_endian);
            uint16_t size = comment_packet.size();

            if (comment_packet.granule() != 0) {
                throw parse_error_str("comment packet granule != 0");
            }

            m_stream.seekg(comment_packet.offset());

            Bit_uint<8> c(m_stream.get());
            if (3 != c) {
                throw parse_error_str("wrong type for comment packet");
            }

            os << c;

            for (unsigned int i = 1; i < size; i++) {
                c = m_stream.get();
                os << c;
            }

            // identification packet on its own page
            os.flush_page();

            offset = comment_packet.next_offset();
        }

        // copy setup packet
        {
            packet_old setup_packet(m_stream, offset, m_little_endian);

            m_stream.seekg(setup_packet.offset());
            if (setup_packet.granule() != 0) throw parse_error_str("setup packet granule != 0");
            bit_oggstream ss(m_stream);

            Bit_uint<8> c;
            ss >> c;

            // type
            if (5 != c) {
                throw parse_error_str("wrong type for setup packet");
            }
            os << c;

            // 'vorbis'
            for (unsigned int i = 0; i < 6; i++) {
                ss >> c;
                os << c;
            }

            // codebook count
            Bit_uint<8> codebook_count_less1;
            ss >> codebook_count_less1;
            unsigned int codebook_count = codebook_count_less1 + 1;
            os << codebook_count_less1;

            codebook_library cbl;

            // rebuild codebooks
            for (unsigned int i = 0; i < codebook_count; i++) {
                cbl.copy(ss, os);
            }

            while (ss.get_total_bits_read() < setup_packet.size() * 8u) {
                Bit_uint<1> bitly;
                ss >> bitly;
                os << bitly;
            }

            os.flush_page();

            offset = setup_packet.next_offset();
        }

        if (offset != m_data_offset + static_cast<long>(m_first_audio_packet_offset)) throw parse_error_str("first audio packet doesn't follow setup packet");
    }
}

}// namespace libww