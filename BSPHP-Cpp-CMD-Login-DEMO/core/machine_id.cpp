#include "machine_id.h"

#include "string_utf8.h"

#include <Windows.h>

#include <fstream>
#include <random>
#include <sstream>

namespace {

std::string GeneratePersistedUuid() {
    wchar_t path[MAX_PATH];
    if (GetEnvironmentVariableW(L"LOCALAPPDATA", path, MAX_PATH) == 0) {
        return "demo-machine-fallback";
    }
    std::wstring dir = std::wstring(path) + L"\\BSPHPCardMfcDemo";
    CreateDirectoryW(dir.c_str(), nullptr);
    std::wstring file = dir + L"\\machine_id.txt";
    std::ifstream in(file);
    std::string line;
    if (in && std::getline(in, line) && line.size() > 8) {
        return line;
    }
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<unsigned> d(0, 15);
    std::ostringstream oss;
    for (int i = 0; i < 32; ++i) {
        oss << "0123456789abcdef"[d(gen)];
    }
    std::string uuid = oss.str();
    std::ofstream out(file, std::ios::trunc);
    if (out) {
        out << uuid;
    }
    return uuid;
}

}  // namespace

std::string BsphpDemoMachineCodeUtf8() {
    HKEY hkey = nullptr;
    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Cryptography", 0,
            KEY_READ | KEY_WOW64_64KEY, &hkey) != ERROR_SUCCESS) {
        return GeneratePersistedUuid();
    }
    wchar_t buf[256] = {};
    DWORD sz = sizeof(buf);
    LSTATUS st = RegGetValueW(hkey, nullptr, L"MachineGuid", RRF_RT_REG_SZ, nullptr, buf, &sz);
    RegCloseKey(hkey);
    if (st != ERROR_SUCCESS || buf[0] == L'\0') {
        return GeneratePersistedUuid();
    }
    return Utf8FromWide(buf);
}

