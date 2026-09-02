#include <gtest/gtest.h>

#include "util.h"

TEST(ParseOctal, NormalValue) {
    char field[12] = "00000000013";
    EXPECT_EQ(Util::parse_octal(field).value(), 11u);
}

TEST(ParseOctal, MaximumUstarSize) {
    char field[12] = "77777777777";
    EXPECT_EQ(Util::parse_octal(field).value(), 8589934591u);
}

TEST(ParseOctal, SpacePadded) {
    char field[12] = "        013";
    EXPECT_EQ(Util::parse_octal(field).value(), 11u);
}

TEST(ParseOctal, FullFieldWithoutTerminator) {
    char field[12] = {'7', '7', '7', '7', '7', '7', '7', '7', '7', '7', '7', '7'};
    EXPECT_EQ(Util::parse_octal(field).value(), 68719476735u);
}

TEST(ParseOctal, AllNulIsZero) {
    char field[12] = {};
    EXPECT_EQ(Util::parse_octal(field).value(), 0u);
}

TEST(ParseOctal, AllSpacesIsZero) {
    char field[12] = {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '};
    EXPECT_EQ(Util::parse_octal(field).value(), 0u);
}

TEST(ParseOctal, RejectsNonOctalDigits) {
    char field[12] = "0000abc0013";
    EXPECT_FALSE(Util::parse_octal(field).has_value());
}

TEST(ParseOctal, RejectsEightAsDigit) {
    char field[12] = "00000000018";
    EXPECT_FALSE(Util::parse_octal(field).has_value());
}

TEST(ParseString, StopsAtNul) {
    char field[16] = "hello.txt";
    EXPECT_EQ(Util::parse_string(field), "hello.txt");
}

TEST(ParseString, StopsAtFieldWidthWhenNotTerminated) {
    char field[8] = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h'};
    EXPECT_EQ(Util::parse_string(field), "abcdefgh");
}

TEST(ParseString, EmptyField) {
    char field[8] = {};
    EXPECT_EQ(Util::parse_string(field), "");
}
