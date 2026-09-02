#pragma once

#include "tar_header.h"
#include "util.h"

#include "file.h"
#include <string>
#include <cstddef>
#include <cstdlib>
#include <cctype>
#include <expected>
#include <utility>
#include <vector>

constexpr char END_OF_BLOCK = '\0';
constexpr int BLOCK_SIZE = 512;
constexpr char EXTRA_METADATA = 'X';

struct FileResult {
    std::string file_name;
    std::size_t file_size;
    std::string file_content;
};

struct RunnerResult {
    std::vector<FileResult> files;
};

class Runner {
public:
    explicit Runner(std::string&& data) : _data(std::move(data)) {}

    [[nodiscard]] std::expected<RunnerResult, std::string> run() const {
        std::size_t offset = 0;

        RunnerResult result{};

        while (offset + BLOCK_SIZE <= _data.size()) {
            const TarHeader& header = *(reinterpret_cast<const TarHeader*>(_data.data() + offset));

            if (header.file_name[0] == END_OF_BLOCK) {
                offset += BLOCK_SIZE;
                continue;
            }

            const auto checksum = Util::parse_octal(header.checksum);

            if (!checksum) {
                return std::unexpected(checksum.error());
            }

            if (const std::uint64_t calculated_checksum = calculate_checksum(header); calculated_checksum != checksum.value()) {
                return std::unexpected("Checksum do not match");
            }

            const auto file_size = Util::parse_octal(header.file_size);

            if (!file_size) {
                return std::unexpected(file_size.error());
            }

            if (header.type_flag == EXTRA_METADATA || std::toupper(header.type_flag) == EXTRA_METADATA) {
                increment_offset(offset, file_size.value());
                continue;
            }

            std::string file_content = _data.substr(offset + BLOCK_SIZE, file_size.value());

            increment_offset(offset, file_size.value());

            result.files.emplace_back(Util::parse_string(header.file_name), file_size.value(), std::move(file_content));
        }

        return result;
    }

private:
    static std::uint64_t calculate_checksum(const TarHeader& header) {
        TarHeader header_copy = header;

        for (char& i : header_copy.checksum) {
            i = ' ';
        }

        const auto* header_copy_data = reinterpret_cast<const uint8_t*>(&header_copy);

        std::uint64_t calculated_checksum = 0;

        for (std::size_t i = 0; i < sizeof(header_copy); ++i) {
            calculated_checksum += header_copy_data[i];
        }

        return calculated_checksum;
    }

    static void increment_offset(std::size_t& offset, const std::size_t& size) {
        const std::size_t block_size = calculate_block_size(size);
        offset += BLOCK_SIZE + (block_size * BLOCK_SIZE);
    }

    static std::size_t calculate_block_size(const std::size_t& size) {
        if (size == 0) {
            return 0;
        }
        const std::size_t blocks = (size + BLOCK_SIZE - 1) / BLOCK_SIZE;
        return blocks;
    }

    std::string _data;
};