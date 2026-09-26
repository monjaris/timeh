#include <cstring>
#include <print>
#include <chrono>
#include <cstdlib>
#include <unistd.h>
#include <array>
#include <climits>
#include <fstream>
#include "defs.hpp"


namespace config
{
    constexpr const char* timeh_cpp = "/tmp/timeh_main.cpp";
    constexpr const char* timeh_out = "/tmp/timeh.out";

    constexpr const char* compiler = "g++";
    constexpr const char* cpp_version = "c++23";
    constexpr const char* flags = "-Wattributes";
}
namespace cfg = config;

namespace option
{
    bool include_tree = false;
    bool compiler_stderr = false;
}
namespace opt = option;


int main(int argc, char** argv)
{
    if (argc == 2) {
        opt::include_tree = false;
    }
    else if (argc == 3 && !strcmp(argv[1], "-t")) {
        opt::include_tree = true;
    }
    else if (argc == 3 && !strcmp(argv[1], "-v")) {
        opt::compiler_stderr = true;
    }
    else if (argc == 3 && (!strcmp(argv[1], "-tv") || !strcmp(argv[1], "-vt"))) {
        opt::include_tree = true;
        opt::compiler_stderr = true;
    }
    else {
        std::print(
            "<header> :  print duration of the header to get compiled in an empty TU\n"
            "-t <header> :  print the tree of includes recursively\n"
        );
        ::exit(0);
    }


    auto fout = std::fstream{cfg::timeh_cpp, std::ios::out};
    if (!fout) {
        std::println("[ERROR]: Could not create file '{}'", cfg::timeh_cpp);
    }


    constexpr const char* silencer = "2> /dev/null";
    std::string command;
    std::string header;  // fed by user provided command line arg

    if (opt::include_tree)
    {
        header = argv[2];

        command = std::format(
            "{} -include{} -std={} {} -I. -I./include -o {} -H -c {}",
            cfg::compiler, header, cfg::cpp_version, cfg::flags, cfg::timeh_out, cfg::timeh_cpp
        );
    }
    else
    {
        header = argv[1];

        command = std::format(
            "{} -include{} -std={} {} -I. -I./include -o {} -c {} 2> /dev/null",
            cfg::compiler, header, cfg::cpp_version, cfg::flags, cfg::timeh_out, cfg::timeh_cpp,
            opt::compiler_stderr ? "" : silencer
        );
    }

    bool is_header_a_path = false;
    if (header.starts_with('/') || header.starts_with('~')) {
        is_header_a_path = true;
    }


    auto start = std::chrono::steady_clock::now();
    int command_failed = ::system(command.c_str());
    auto finish = std::chrono::steady_clock::now();

    std::chrono::duration<fp64, std::milli> elapsed = finish - start;

    if (!command_failed) {
        std::println("duration: \033[1m{}\033[3;34m ms\033[0m", elapsed.count());
    } else {
        std::println("Header \033[31m'{}'\033[0m doesnt exist!", header);
    }
}
