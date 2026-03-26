#pragma once

#include <Windows.h>

#include <string>

inline std::string Utf8FromWide(const wchar_t* w) {
    if (!w || !*w) {
        return {};
    }
    int n = WideCharToMultiByte(CP_UTF8, 0, w, -1, nullptr, 0, nullptr, nullptr);
    if (n <= 0) {
        return {};
    }
    // n includes the terminating '\0' because cchWideChar is -1.
    std::string out;
    out.resize(static_cast<size_t>(n), '\0');
    // C++14/older STL: std::string::data() may be const char*; WideCharToMultiByte wants LPSTR.
    WideCharToMultiByte(CP_UTF8, 0, w, -1, &out[0], n, nullptr, nullptr);
    // Drop the trailing '\0' so callers don't see embedded terminators.
    if (!out.empty() && out.back() == '\0') {
        out.pop_back();
    }
    return out;
}

inline std::wstring WideFromUtf8(const std::string& u8) {
    if (u8.empty()) {
        return {};
    }
    int n = MultiByteToWideChar(CP_UTF8, 0, u8.data(), static_cast<int>(u8.size()), nullptr, 0);
    if (n <= 0) {
        return {};
    }
    std::wstring w(static_cast<size_t>(n), L'\0');
    MultiByteToWideChar(CP_UTF8, 0, u8.data(), static_cast<int>(u8.size()), &w[0], n);
    return w;
}
