#pragma once
//
#include <cstdio>
#include <cstdlib>
//
#include <chrono>
#include <string>
#include <optional>
#include <utility>
#include <inplace_vector>
//
#include <unistd.h>
//
#include <tinyfmt/tfmt.hpp>
//
#include "defs.hpp"
NAMESPACE(util)


using Duration = std::chrono::duration<real64, std::milli>;
using timer = std::chrono::steady_clock;



template<class ValueType>
class [[nodiscard]] Own
{
    ValueType* m_data;
    int (*m_cleaner)(ValueType*);
    bool m_destructor_dont_release = false;

public:
    using ValueT = ValueType;

    Own(const Own&) = delete;
    Own& operator= (const Own&) = delete;
    ~Own() { if (!m_destructor_dont_release) this->release(m_cleaner); }

    Own (ValueT* ptr, int (*invokable)(ValueT*))
    {
        m_data = ptr;
        m_cleaner = invokable;
    }

    template<class Invokable>
    void release(Invokable invokable) {
        m_destructor_dont_release = true;
        invokable(m_data);
    }

    const ValueT* data() const {
        return m_data;
    }

    // overload
    ValueT* data() {
        return m_data;
    }
};



template<int exit_code = 0, class... Args>
void exitWith(tfmt::format_string<Args...> fmt, Args... args) {
    tfmt::println(fmt, std::forward<Args>(args)...);
    ::exit(exit_code);
}


template<class... T>
int shellExec(tfmt::format_string<T...> fmt, T&&... args) {
    return ::system(tfmt::format(fmt, std::forward<T>(args)...).c_str());
}


struct [[nodiscard]] ShellExecInfo {
    int exit_status;
    real64 duration;
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
        tfmt::println("Could not open file {}", path);
        ::exit(1);
    }
}

bool pathExists(const char* path) {
    return ::access(path, F_OK) == 0;
}


std::string gccVersion() {
    Own<FILE> pipe = {::popen("gcc --version", "r"), ::pclose};
    if (pipe.data() == nullptr) exitWith("Running pipe command failed");

    std::array<char, 32> buffer;
    if (::fgets(buffer.data(), buffer.size(), pipe.data()) == nullptr) {
        exitWith("Failed to read GCC version");
    }

    constexpr usize BEGIN_POS = 10;  // gcc output puts version number at 10, currently
    constexpr usize MAX_DIGITS = 2;

    return { buffer.data() + BEGIN_POS, buffer.data() + BEGIN_POS + MAX_DIGITS };
}


std::optional<std::string> findHeader(const std::string& header_name, const std::string& gcc_ver) {
    if (pathExists(header_name.c_str())) {
        return header_name;
    }
    else {
        std::string header;
        header.reserve(64);
        (((header += "/usr/include/c++/") += gcc_ver) += "/") += header_name;

        if (header.empty()) return std::nullopt;
        else return header;
    }
}

void argumentRequiredIf(bool cond) {
    if (cond) exitWith("Expected <header-name>\n");
}


NAMESPACE_END()
