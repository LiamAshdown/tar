#include <gtest/gtest.h>

#include <string>

#include "runner.h"
#include "test_helpers.h"

using namespace test_helpers;

TEST(Runner, ReadsSingleEntry) {
    std::string archive = make_entry("hello.txt", "hello world");
    archive += end_of_archive();

    const Runner runner(std::move(archive));
    const auto result = runner.run();

    ASSERT_TRUE(result.has_value()) << result.error();
    ASSERT_EQ(result->files.size(), 1u);
    EXPECT_EQ(result->files[0].file_name, "hello.txt");
    EXPECT_EQ(result->files[0].file_size, 11u);
    EXPECT_EQ(result->files[0].file_content, "hello world");
}

TEST(Runner, ReadsMultipleEntries) {
    std::string archive = make_entry("a.txt", "aaa");
    archive += make_entry("b.txt", "bbbbb");
    archive += make_entry("c.txt", "");
    archive += end_of_archive();

    const Runner runner(std::move(archive));
    const auto result = runner.run();

    ASSERT_TRUE(result.has_value()) << result.error();
    ASSERT_EQ(result->files.size(), 3u);
    EXPECT_EQ(result->files[0].file_name, "a.txt");
    EXPECT_EQ(result->files[1].file_content, "bbbbb");
    EXPECT_EQ(result->files[2].file_size, 0u);
}

TEST(Runner, ContentSpanningMultipleBlocks) {
    const std::string content(1500, 'z');
    std::string archive = make_entry("big.bin", content);
    archive += make_entry("after.txt", "still here");
    archive += end_of_archive();

    const Runner runner(std::move(archive));
    const auto result = runner.run();

    ASSERT_TRUE(result.has_value()) << result.error();
    ASSERT_EQ(result->files.size(), 2u);
    EXPECT_EQ(result->files[0].file_content, content);
    EXPECT_EQ(result->files[1].file_name, "after.txt");
}

TEST(Runner, DetectsCorruptChecksum) {
    std::string archive = make_entry("hello.txt", "hello world");
    archive[148] = '9';
    archive += end_of_archive();

    const Runner runner(std::move(archive));
    const auto result = runner.run();

    EXPECT_FALSE(result.has_value());
}

TEST(Runner, RejectsNonOctalSizeField) {
    std::string archive = make_entry("hello.txt", "hello world");
    archive[124] = 'Z';

    const Runner runner(std::move(archive));
    const auto result = runner.run();

    EXPECT_FALSE(result.has_value());
}

TEST(Runner, NameFillingFieldDoesNotOverrun) {
    const std::string long_name(100, 'a');
    std::string archive = make_entry(long_name, "x");
    archive += end_of_archive();

    const Runner runner(std::move(archive));
    const auto result = runner.run();

    ASSERT_TRUE(result.has_value()) << result.error();
    ASSERT_EQ(result->files.size(), 1u);
    EXPECT_EQ(result->files[0].file_name.size(), 100u);
    EXPECT_EQ(result->files[0].file_name, long_name);
}

TEST(Runner, SkipsPaxExtendedHeaders) {
    std::string archive = make_entry("./@PaxHeader", "30 mtime=1700000000.0\n", 'x');
    archive += make_entry("real.txt", "content");
    archive += end_of_archive();

    const Runner runner(std::move(archive));
    const auto result = runner.run();

    ASSERT_TRUE(result.has_value()) << result.error();
    ASSERT_EQ(result->files.size(), 1u);
    EXPECT_EQ(result->files[0].file_name, "real.txt");
}

TEST(Runner, EmptyArchiveYieldsNoEntries) {
    std::string archive = end_of_archive();

    const Runner runner(std::move(archive));
    const auto result = runner.run();

    ASSERT_TRUE(result.has_value()) << result.error();
    EXPECT_TRUE(result->files.empty());
}

TEST(Runner, EmptyInputYieldsNoEntries) {
    const Runner runner(std::string{});
    const auto result = runner.run();

    ASSERT_TRUE(result.has_value()) << result.error();
    EXPECT_TRUE(result->files.empty());
}
