#pragma once

#include <cstdio>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <string>

#include <zlib.h>

namespace test_helpers {
    inline void write_field(std::string& block, std::size_t offset, std::size_t width, const std::string& value) {
        for (std::size_t i = 0; i < width; ++i) {
            block[offset + i] = i < value.size() ? value[i] : '\0';
        }
    }

    inline std::string make_header(const std::string& name,
                                   std::uint64_t size,
                                   char type_flag = '0') {
        std::string block(512, '\0');

        char size_field[16];
        std::snprintf(size_field, sizeof(size_field), "%011llo", static_cast<unsigned long long>(size));

        write_field(block, 0,   100, name);
        write_field(block, 100,   8, "0000644");
        write_field(block, 108,   8, "0000000");
        write_field(block, 116,   8, "0000000");
        write_field(block, 124,  12, size_field);
        write_field(block, 136,  12, "00000000000");
        block[156] = type_flag;
        write_field(block, 257,   6, "ustar");
        block[263] = '0';
        block[264] = '0';

        for (int i = 0; i < 8; ++i) {
            block[148 + i] = ' ';
        }

        unsigned sum = 0;
        for (const unsigned char c : block) {
            sum += c;
        }

        char checksum_field[16];
        std::snprintf(checksum_field, sizeof(checksum_field), "%06o", sum);
        for (int i = 0; i < 6; ++i) {
            block[148 + i] = checksum_field[i];
        }
        block[154] = '\0';
        block[155] = ' ';

        return block;
    }

    inline std::string make_entry(const std::string& name,
                                  const std::string& content,
                                  char type_flag = '0') {
        std::string entry = make_header(name, content.size(), type_flag);
        entry += content;

        if (const std::size_t remainder = entry.size() % 512; remainder != 0) {
            entry.append(512 - remainder, '\0');
        }
        return entry;
    }

    inline std::string end_of_archive() {
        return std::string(1024, '\0');
    }

    inline std::string gzip_compress(const std::string& data) {
        z_stream stream{};
        deflateInit2(&stream, Z_DEFAULT_COMPRESSION, Z_DEFLATED, 16 + MAX_WBITS, 8, Z_DEFAULT_STRATEGY);

        stream.avail_in = static_cast<uInt>(data.size());
        stream.next_in  = reinterpret_cast<Bytef*>(const_cast<char*>(data.data()));

        std::string out;
        char buffer[16384];
        int ret;

        do {
            stream.avail_out = sizeof(buffer);
            stream.next_out  = reinterpret_cast<Bytef*>(buffer);
            ret = deflate(&stream, Z_FINISH);
            out.append(buffer, sizeof(buffer) - stream.avail_out);
        } while (ret != Z_STREAM_END);

        deflateEnd(&stream);
        return out;
    }

    class TempFile {
    public:
        explicit TempFile(const std::string& contents) {
            static int counter = 0;
            _path = std::filesystem::temp_directory_path() /
                    ("tar_test_" + std::to_string(++counter) + ".bin");

            std::ofstream out(_path, std::ios::binary);
            out.write(contents.data(), static_cast<std::streamsize>(contents.size()));
        }

        ~TempFile() {
            std::error_code ec;
            std::filesystem::remove(_path, ec);
        }

        TempFile(const TempFile&) = delete;
        TempFile& operator=(const TempFile&) = delete;

        [[nodiscard]] std::string path() const { return _path.string(); }

    private:
        std::filesystem::path _path;
    };

}
