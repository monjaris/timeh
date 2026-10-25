#include <chrono>
//
#include <clixx/clixx.hpp>
#include <tinyfmt/tfmt.hpp>
#include <unistd.h>
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

    constexpr const char* silencer =
        // { "" }
        { "2> /dev/null" }
    ;
}
namespace globcfg = global_config;


namespace config
{
    // values
    std::string header;

    // options
    bool measure_time    = false;  // -t/--time
    bool grep_includes   = false;  // -g/--grep
    bool print_includes  = false;  // -i/--includes
    bool verbose         = false;  // -v/--verbose
}
namespace cfg = config;


namespace util
{
    template<class... T>
    int shellExec(tfmt::format_string<T...> fmt, T&&... args) {
        return ::system(tfmt::format(fmt, std::forward<T>(args)...).c_str());
    }


    struct [[nodiscard]] ShellExecInfo {
        int exit_status;
        fp64 duration;
    };

    template<class... T>
    ShellExecInfo shellExecTime(tfmt::format_string<T...> fmt, T&&... args) {
        auto start = timer::now();
        int status = ::system(tfmt::format(fmt, std::forward<T>(args)...).c_str());
        auto finish = timer::now();
        return { status, std::chrono::duration_cast<Duration>(finish - start).count() };
    }


    Own<FILE> openFile(const char* path) {
        if (FILE* file = ::fopen(path, "w+")) {
            return {file, ::fclose};
        }
        else {
            clixx::exitWith("Could not open file");
        }
    }

    bool pathExists(const char* path) {
        return ::access(path, F_OK) == 0;
    }

    std::optional<std::string> findHeader(const std::string& header_name) {
        if (pathExists(header_name.c_str())) {
            return header_name;
        } else {
            std::string header = std::string("/usr/include/c++/16/") + header_name;

            if (header.empty()) return std::nullopt;
            else {
                return header;
            }
        }
    }

    void argumentRequiredIf(bool cond) {
        if (cond) clixx::exitWith("Expected <header-name>\n");
    }
}


namespace cmd
{
    void defaultAction() {
        clixx::exitWith(
            "pass -h/--help to print usage\n"
        );
    }

    void help() {
        clixx::exitWith(
            "<header>:  print duration of the header to get compiled in an empty TU\n"
            "-t/--time <header>:      (default option)\n"
            "-g/--grep <header>:      using ugrep, print all '#include' instances\n"
            "-i/--includes <header>:  first preprocess the header and then print all '#include' instances\n"
            "-v/--verbose:            enable verbose output\n"
        );
    }


    void measureCompileTime(const std::string& header, bool compiler_stderr) {
        util::argumentRequiredIf(header.empty());
        (void) util::openFile(globcfg::timeh_cpp);

        auto res = util::shellExecTime(
            "{} -include{} -std={} {} -I. -I./include -o {} -c {} {}",
            globcfg::compiler, header, globcfg::cpp_version, globcfg::flags, globcfg::timeh_out, globcfg::timeh_cpp,
            (compiler_stderr ? (""):(globcfg::silencer))
        );

        if (res.exit_status != 0) tfmt::println("Failed to process header '{}'", header);
        else tfmt::println("duration: \033[1m{}\033[3;34m ms\033[0m", res.duration);
    }


    void grepIncludes(const std::string& header) {
        util::argumentRequiredIf(header.empty());

        auto found_header = util::findHeader(header);
        if (!found_header.has_value()) {
            clixx::exitWith(tfmt::format("Could not find header '{}'\n", header).c_str());
        }

        util::shellExec(
            "grep -E '^[[:space:]]*#[[:space:]]*include[[:space:]]*[<\"]' '{}'",
            found_header.value()
        );
    }

    void preprocPrintIncludes(const std::string header) {
        util::argumentRequiredIf(header.empty());

        auto found_header = util::findHeader(header);
        if (!found_header.has_value()) {
            clixx::exitWith(tfmt::format("Could not find header '{}'\n", header).c_str());
        }

        util::shellExec(
            "g++ -E -x c++ '{}' 2>/dev/null | "
            "grep -E '^#[[:space:]]*[0-9]+[[:space:]]+\"' | "
            "sed -E 's/^#[[:space:]]*[0-9]+[[:space:]]+\"([^\"]+)\".*/\\1/' | "
            "sort -u",
            found_header.value()
        );
    }
};



int main(int argc, char** argv)
{
    clixx::CLI cli = {argc, argv};
    cli.parse();
    const auto& values = cli.values();
    const auto& options = cli.options();

    if (auto value_count = values.size()) {
        if (value_count != 2) {
            cfg::header = values.at(0).val();
        } else {
            clixx::exitWith(tfmt::format(
                "Unexpected trailing argument(s). pass -h/--help for usage\n"
            ).c_str(), 1);
        }
    } else if (options.size() == 0) {
        cmd::defaultAction();
    }

    if (options.empty()) {
        cfg::measure_time = true;
    }

    for (const auto& opt : options) {
        if (opt.is("-h", "--help")) cmd::help();

        else if (opt.is("-t", "--time")) {
            cfg::measure_time = true;
        }

        else if (opt.is("-v", "--verbose")) {
            cfg::verbose = true;
        }

        else if (opt.is("-g", "--grep")) {
            cfg::grep_includes = true;
        }

        else if (opt.is("-i", "--includes")) {
            cfg::print_includes = true;
        }

        else {
            clixx::exitWith(tfmt::format("Unknown flag: {}\n", opt.name()).c_str(), 1);
        }
    }


    if (cfg::measure_time) {
        cmd::measureCompileTime(cfg::header, cfg::verbose);
    }
    if (cfg::grep_includes) {
        cmd::grepIncludes(cfg::header);
    }
    if (cfg::print_includes) {
        cmd::preprocPrintIncludes(cfg::header);
    }
}
