#include <iostream>
#include <algorithm>
#include <iterator>
#include <format>

#include <cxxopts.hpp>

#include "runner.h"
#include "file.h"

int main(int argc, char* argv[]) {
    cxxopts::Options options("TAR Reader", "This unpacks your `.tar` file and prints out the file names and file contents");

    options.add_options()
     ("p,path", "Path to your `.tar.gz` file", cxxopts::value<std::string>())
     ("h, help", "Print Usage");

    cxxopts::ParseResult options_result;

    try {
        options_result = options.parse(argc, argv);
    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    if (options_result.count("help")) {
        std::cout << options.help();
        return 0;
    }

    if (!options_result.count("path")) {
        std::cerr << "tar: no archive specified\n\n" << options.help();
        return 2;
    }

    std::cout << "Found path: " + options_result["path"].as<std::string>() << std::endl;

    File file(options_result["path"].as<std::string>());

    auto result = file.read();

    if (!result) {
        std::cerr << "tar: " << result.error() << '\n';
        return 1;
    }

    const Runner runner(std::move(result).value());

    const auto run_result = runner.run();

    if (!run_result) {
        std::cerr << "tar: " << run_result.error() << '\n';
        return 1;
    }

    std::cout << "Found " + std::to_string(run_result->files.size()) + " files." << std::endl;

    for (auto& file_result : run_result->files) {
        std::cout << std::format("{:>10} {}\n", file_result.file_size, file_result.file_name);
    }

    return 0;
}