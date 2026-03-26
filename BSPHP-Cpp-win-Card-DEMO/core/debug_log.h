#pragma once

#include <windows.h>
#include <cstdarg>
#include <cstdio>
#include <string>

inline void DebugLogW(const wchar_t* fmt, ...) {
    wchar_t buf[1024] = {};
    va_list args;
    va_start(args, fmt);
    _vsnwprintf_s(buf, _countof(buf), _TRUNCATE, fmt, args);
    va_end(args);
    OutputDebugStringW(buf);
    OutputDebugStringW(L"\r\n");
}

inline std::wstring DebugUtf8ToWide(const std::string& s) {
    if (s.empty()) return std::wstring();
    int needed = MultiByteToWideChar(CP_UTF8, 0, s.c_str(), (int)s.size(), nullptr, 0);
    if (needed <= 0) return std::wstring();
    std::wstring w;
    w.resize((size_t)needed);
    MultiByteToWideChar(CP_UTF8, 0, s.c_str(), (int)s.size(), &w[0], needed);
    return w;
}

inline void DebugLogUtf8(const char* prefix, const std::string& utf8) {
    std::wstring w = DebugUtf8ToWide(utf8);
    if (prefix) {
        std::wstring wp = DebugUtf8ToWide(prefix);
        OutputDebugStringW(wp.c_str());
    }
    OutputDebugStringW(w.c_str());
    OutputDebugStringW(L"\r\n");
}

