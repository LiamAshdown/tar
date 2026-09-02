#pragma once

#include <cstdint>
#include <fstream>
#include <expected>

#include <zlib.h>

class File {
public:
    explicit File(const std::string& path) : _file(path, std::ios::binary) {
    }

    std::expected<std::string, std::string> read() {
        if (!exists()) {
            return std::unexpected("File not found");
        }

        _file.seekg(0, std::ios::end);
        const std::streamsize file_size = _file.tellg();
        _file.seekg(0, std::ios::beg);

        if (file_size < 0) {
            return std::unexpected("Could not determine file size");
        }

        std::string potential_compressed_data(file_size, '\0');

        if (!_file.read(&potential_compressed_data[0], file_size)) {
            return std::unexpected("Could not read file");
        }

        // Only support gzip
        if (!is_gzip(potential_compressed_data)) {
            return std::unexpected("Not a gzip file");
        }

        return decompress(potential_compressed_data);
    }

private:
    bool exists() const {
        return _file.good();
    }

    static bool is_gzip(const std::string& data) {
        if (data.size() < 3) {
            return false;
        }

        // www.rfc-editor.org/info/rfc1952/#section-2.3
        const auto id1 = static_cast<uint8_t>(data[0]);
        const auto id2 = static_cast<uint8_t>(data[1]);
        const auto compression_method = static_cast<uint8_t>(data[2]);

        return id1 == 0x1f && id2 == 0x8b && compression_method == 8;
    }

    static std::expected<std::string, std::string> decompress(const std::string& compressed_data) {
        z_stream stream{};
        stream.zalloc = Z_NULL;
        stream.zfree = Z_NULL;
        stream.opaque = Z_NULL;
        stream.avail_in = compressed_data.size();
        stream.next_in = reinterpret_cast<Bytef*>(const_cast<char*>(compressed_data.data()));

        if (inflateInit2(&stream, 16 + MAX_WBITS) != Z_OK) {
            return std::unexpected("inflateInit2 failed");
        }

        constexpr std::size_t MAX_UNCOMPRESSED = 1ull * 1024 * 1024 * 1024; // 1GB

        std::string uncompressed_data;
        char buffer[32768];
        int ret;

        do {
            stream.avail_out = sizeof(buffer);
            stream.next_out = reinterpret_cast<Bytef*>(buffer);

            ret = inflate(&stream, Z_NO_FLUSH);

            if (ret != Z_OK && ret != Z_STREAM_END) {
                inflateEnd(&stream);
                return std::unexpected("GZIP decompression failed with error code: " + std::to_string(ret));
            }

            const std::size_t have = sizeof(buffer) - stream.avail_out;

            if (uncompressed_data.size() + have > MAX_UNCOMPRESSED) {
                inflateEnd(&stream);
                return std::unexpected("Decompressed data exceeds maximum allowed size");
            }

            uncompressed_data.append(buffer, have);

        } while (ret != Z_STREAM_END);

        inflateEnd(&stream);
        return uncompressed_data;
    }


    std::ifstream _file;

};
