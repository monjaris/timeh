#include <cstring>
#include <print>
#include <chrono>
#include <cstdlib>
#include <unistd.h>
#include <array>
#include <climits>
#include <fstream>
//
#include <clixx/clixx.hpp>
#include "defs.hpp"


using Duration = std::chrono::duration<fp64, std::milli>;
using timer = std::chrono::steady_clock;


namespace global_config
{
    constexpr const char* timeh_cpp = "/tmp/timeh_main.cpp";
    constexpr const char* timeh_out = "/tmp/timeh.out";

    constexpr const char* compiler = "g++";
    constexpr const char* cpp_version = "c++23";
    constexpr const char* flags = "-Wattributes -isystem ";
}
namespace globcfg = global_config;

namespace config
{
    std::string header;
    bool include_tree = false;
    bool compiler_stderr = false;
}
namespace cfg = config;



namespace cmd
{
    void defaultAction() {
        clixx::exitWith(
            "pass -h/--help to print usage\n"
        );
    }

    void help() {
        clixx::exitWith(
            "<header> :  print duration of the header to get compiled in an empty TU\n"
            "-t <header> :  print the tree of includes recursively\n"
        );
    }
};



int main(int argc, char** argv)
{
    clixx::CLI cli = {argc, argv};
    cli.parse();
    const auto& values = cli.values();
    const auto& options = cli.options();

    if (values.size() >= 1) {
        cfg::header = values.at(0).val();
    } else if (options.size() == 0) {
        cmd::defaultAction();
    }

    std::vector<usize> invalids;

    for (const auto& opt : options) {
        if (opt.is("-h", "--help")) cmd::help();

        else if (opt.is("-t", "--tree")) {
            cfg::include_tree = true;
        }

        else if (opt.is("-v", "--verbose")) {
            cfg::compiler_stderr = true;
        }

        else {
            clixx::exitWith(std::format("Unknown flag: {}", opt.name()).c_str());
        }
    }


    auto fout = std::fstream{globcfg::timeh_cpp, std::ios::out};
    if (!fout) std::println("[ERROR]: Could not create file '{}'", globcfg::timeh_cpp);


    constexpr const char* silencer =
        // { "" }
        { "2> /dev/null" }
    ;

    std::string command;

    if (cfg::include_tree)
    {
        command = std::format(
            "{} -include{} -std={} {} -I. -I./include -o {} -H -c {}",
            globcfg::compiler, cfg::header, globcfg::cpp_version, globcfg::flags, globcfg::timeh_out, globcfg::timeh_cpp
        );
    }
    else
    {
        command = std::format(
            "{} -include{} -std={} {} -I. -I./include -o {} -c {} {}",
            globcfg::compiler, cfg::header, globcfg::cpp_version, globcfg::flags, globcfg::timeh_out, globcfg::timeh_cpp,
            cfg::compiler_stderr ? "" : silencer
        );
    }

    bool is_header_a_path = false;
    if (cfg::header.starts_with('/') || cfg::header.starts_with('~')) {
        is_header_a_path = true;
    }


    auto start = timer::now();
    int command_failed = ::system(command.c_str());
    auto finish = timer::now();

    Duration elapsed = finish - start;

    if (!command_failed) {
        std::println("duration: \033[1m{}\033[3;34m ms\033[0m", elapsed.count());
    } else {
        std::println("Failed to compile header: \033[31m'{}'\033[0m!", cfg::header);
    }
}
