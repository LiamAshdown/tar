#include <gtest/gtest.h>

#include <cstddef>

#include "tar_header.h"

TEST(TarHeader, IsExactlyOneBlock) {
    EXPECT_EQ(sizeof(TarHeader), 512u);
}

TEST(TarHeader, FieldOffsetsMatchUstar) {
    EXPECT_EQ(offsetof(TarHeader, file_name),        0u);
    EXPECT_EQ(offsetof(TarHeader, file_mode),      100u);
    EXPECT_EQ(offsetof(TarHeader, owner_id),       108u);
    EXPECT_EQ(offsetof(TarHeader, group_id),       116u);
    EXPECT_EQ(offsetof(TarHeader, file_size),      124u);
    EXPECT_EQ(offsetof(TarHeader, last_mod_time),  136u);
    EXPECT_EQ(offsetof(TarHeader, checksum),       148u);
    EXPECT_EQ(offsetof(TarHeader, type_flag),      156u);
    EXPECT_EQ(offsetof(TarHeader, linked_file_name), 157u);
}

TEST(TarHeader, FieldWidthsMatchUstar) {
    EXPECT_EQ(sizeof(TarHeader::file_name),      100u);
    EXPECT_EQ(sizeof(TarHeader::file_mode),        8u);
    EXPECT_EQ(sizeof(TarHeader::file_size),       12u);
    EXPECT_EQ(sizeof(TarHeader::last_mod_time),   12u);
    EXPECT_EQ(sizeof(TarHeader::checksum),         8u);
}
