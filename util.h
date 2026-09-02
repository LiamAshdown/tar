#pragma once

#include <charconv>
#include <string_view>
#include <expected>
#include <string>
#include <cstdint>

using namespace std::string_view_literals;

class Util {
public:

    template<std::size_t N>
    static std::string parse_string(const char (&field)[N]) {
        const std::string_view view(field, N);
        const auto end = view.find('\0');

        return std::string(end == std::string_view::npos ? view : view.substr(0, end));
    }

    // Taken from github.com/golang/go/blob/master/src/archive/tar/strconv.go#L158
    template <std::size_t N>
    static std::expected<std::uint64_t, std::string> parse_octal(const char (&field)[N]) {
        constexpr auto padding = " \0"sv;
        std::string_view view(field, N);

        const auto first = view.find_first_not_of(padding);
        if (first == std::string_view::npos) {
            return 0;
        }

        const auto last = view.find_last_not_of(padding);
        view = view.substr(first, last - first + 1);

        std::uint64_t value = 0;
        const auto [ptr, ec] = std::from_chars(view.data(), view.data() + view.size(), value, 8);

        if (ec != std::errc{} || ptr != view.data() + view.size()) {
            return std::unexpected("Invalid octal in tar header field");
        }

        return value;
    }
};
