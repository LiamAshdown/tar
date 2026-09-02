#include <gtest/gtest.h>

#include <random>
#include <string>

#include "file.h"
#include "test_helpers.h"

using namespace test_helpers;

TEST(File, MissingFileIsReported) {
    File file("this_file_does_not_exist_12345.tar.gz");
    const auto result = file.read();

    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), "File not found");
}

TEST(File, RejectsNonGzipInput) {
    const TempFile temp("this is plain text, not a gzip stream at all");
    File file(temp.path());
    const auto result = file.read();

    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), "Not a gzip file");
}

TEST(File, RejectsUncompressedTar) {
    const TempFile temp(make_entry("hello.txt", "hello world") + end_of_archive());
    File file(temp.path());
    const auto result = file.read();

    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error(), "Not a gzip file");
}

TEST(File, RejectsEmptyFile) {
    const TempFile temp("");
    File file(temp.path());
    const auto result = file.read();

    ASSERT_FALSE(result.has_value());
}

TEST(File, DecompressesValidGzip) {
    const std::string tar = make_entry("hello.txt", "hello world") + end_of_archive();
    const TempFile temp(gzip_compress(tar));

    File file(temp.path());
    const auto result = file.read();

    ASSERT_TRUE(result.has_value()) << result.error();
    EXPECT_EQ(*result, tar);
}

TEST(File, HandlesBinaryDataWithCrlfBytes) {
    std::string payload;
    for (int i = 0; i < 5000; ++i) {
        payload += static_cast<char>(0x0d);
        payload += static_cast<char>(0x0a);
        payload += static_cast<char>(0x1a);
    }

    const std::string tar = make_entry("binary.bin", payload) + end_of_archive();
    const TempFile temp(gzip_compress(tar));

    File file(temp.path());
    const auto result = file.read();

    ASSERT_TRUE(result.has_value()) << result.error();
    EXPECT_EQ(result->size(), tar.size());
    EXPECT_EQ(*result, tar);
}

TEST(File, TruncatedGzipFailsInsteadOfHanging) {
    const std::string tar = make_entry("big.bin", std::string(400000, 'q')) + end_of_archive();
    const std::string compressed = gzip_compress(tar);
    ASSERT_GT(compressed.size(), 100u);

    const TempFile temp(compressed.substr(0, compressed.size() / 2));

    File file(temp.path());
    const auto result = file.read();

    ASSERT_FALSE(result.has_value());
    EXPECT_NE(result.error().find("GZIP decompression failed"), std::string::npos);
}

TEST(File, CorruptGzipIsReported) {
    std::mt19937 rng(12345);
    std::string payload(50000, '\0');
    for (char& c : payload) {
        c = static_cast<char>(rng() & 0xff);
    }

    const std::string tar = make_entry("random.bin", payload) + end_of_archive();
    std::string compressed = gzip_compress(tar);
    ASSERT_GT(compressed.size(), 1000u);

    for (std::size_t i = compressed.size() / 2; i < compressed.size() / 2 + 32; ++i) {
        compressed[i] = static_cast<char>(compressed[i] ^ 0xff);
    }

    const TempFile temp(compressed);
    File file(temp.path());
    const auto result = file.read();

    ASSERT_FALSE(result.has_value());
    EXPECT_NE(result.error().find("GZIP decompression failed"), std::string::npos);
}
