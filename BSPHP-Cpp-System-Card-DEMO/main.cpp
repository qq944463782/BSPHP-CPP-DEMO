#include <iostream>
#include <string>
#include <map>
#include <limits>

#include <windows.h>
#include <shellapi.h>

// Local-only core (no cross-directory includes).
#include "core/bsphp_client.cpp"
#include "core/crypto_http.cpp"
#include "core/card_config.cpp"
#include "core/machine_id.cpp"
#include "core/string_utf8.h"

namespace {

constexpr wchar_t kIoctlServiceName[] = L"BSPHPSystemCardDemoDrv";
constexpr wchar_t kIoctlDeviceName[] = L"\\\\.\\BSPHPSystemCardDemo";
constexpr DWORD IOCTL_BSPHP_CARD_VERIFY =
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x901, METHOD_BUFFERED, FILE_ANY_ACCESS);
constexpr DWORD IOCTL_BSPHP_CARD_RECHARGE =
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x902, METHOD_BUFFERED, FILE_ANY_ACCESS);

struct IoctlCardVerifyReq {
    wchar_t card_id[64];
    wchar_t card_pwd[64];
    wchar_t machine_code[128];
};

struct IoctlCardRechargeReq {
    wchar_t card_id[64];
    wchar_t recharge_no[64];
    wchar_t recharge_pwd[64];
};

void PrintApiResponse(const std::string& title, const bsphp::ApiResponse& r) {
    std::cout << "== " << title << " ==" << "\n";
    std::cout << "code=" << (r.code.empty() ? "(empty)" : r.code) << "\n";
    std::cout << "data=" << (r.data.empty() ? "(empty)" : r.data) << "\n";
    if (!r.appsafecode.empty()) {
        std::cout << "appsafecode=" << r.appsafecode << "\n";
    }
    if (!r.sessl.empty()) {
        std::cout << "sessl=" << r.sessl << "\n";
    }
}

void OpenUrlW(const std::wstring& url) {
    if (url.empty()) return;
    ShellExecuteW(nullptr, L"open", url.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
}

std::string ReadLine(const char* prompt) {
    std::cout << prompt;
    std::string s;
    std::getline(std::cin, s);
    return s;
}

void PrintHelp() {
    std::cout << "\nCommands:\n"
        << "  help                 show commands\n"
        << "  verify               login_ic (need card icid/icpwd)\n"
        << "  getdate              getdate.ic (vip expiry)\n"
        << "  getlkinfo            getlkinfo.ic\n"
        << "  heart                timeout.ic\n"
        << "  notice               gg.in\n"
        << "  socard               socard.in (needs verified)\n"
        << "  info                 getinfo.ic (ask icpwd)\n"
        << "  bind                 setcaron.ic (ask icpwd)\n"
        << "  unbind               setcarnot.ic (ask icpwd)\n"
        << "  renew                open salecard_renew URL\n"
        << "  buygen               open salecard_gencard URL\n"
        << "  buystock             open salecard_salecard URL\n"
        << "  logout               logout_ic\n"
        << "  ioctl_install        install/start driver service\n"
        << "  ioctl_verify         send DeviceIoControl verify request\n"
        << "  ioctl_recharge       send DeviceIoControl recharge request\n"
        << "  exit                 quit\n\n";
}

bool LoginIcSuccess(const bsphp::ApiResponse& r) {
    // Mirror MFC logic: success when r.code == 1081 or r.data contains 1081.
    if (r.code == "1081") return true;
    return r.data.find("1081") != std::string::npos;
}

bool InstallAndStartIoctlDriver(const std::wstring& driver_sys_path) {
    SC_HANDLE scm = OpenSCManagerW(nullptr, nullptr, SC_MANAGER_ALL_ACCESS);
    if (!scm) return false;
    SC_HANDLE svc = CreateServiceW(
        scm, kIoctlServiceName, kIoctlServiceName, SERVICE_ALL_ACCESS, SERVICE_KERNEL_DRIVER,
        SERVICE_DEMAND_START, SERVICE_ERROR_NORMAL, driver_sys_path.c_str(),
        nullptr, nullptr, nullptr, nullptr, nullptr);
    if (!svc && GetLastError() == ERROR_SERVICE_EXISTS) {
        svc = OpenServiceW(scm, kIoctlServiceName, SERVICE_ALL_ACCESS);
    }
    if (!svc) {
        CloseServiceHandle(scm);
        return false;
    }
    StartServiceW(svc, 0, nullptr);
    DWORD ec = GetLastError();
    CloseServiceHandle(svc);
    CloseServiceHandle(scm);
    return ec == ERROR_SUCCESS || ec == ERROR_SERVICE_ALREADY_RUNNING;
}

bool FillWideFromUtf8Input(const std::string& in, wchar_t* out, size_t cap) {
    if (!out || cap == 0) return false;
    std::wstring w = WideFromUtf8(in);
    wcsncpy_s(out, cap, w.c_str(), _TRUNCATE);
    return true;
}

void RunIoctlVerify(const std::string& card, const std::string& pwd, const std::string& machine) {
    HANDLE dev = CreateFileW(kIoctlDeviceName, GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING,
                             FILE_ATTRIBUTE_NORMAL, nullptr);
    if (dev == INVALID_HANDLE_VALUE) {
        std::cout << "[ioctl] open device failed, err=" << GetLastError() << "\n";
        return;
    }
    IoctlCardVerifyReq req{};
    FillWideFromUtf8Input(card, req.card_id, _countof(req.card_id));
    FillWideFromUtf8Input(pwd, req.card_pwd, _countof(req.card_pwd));
    FillWideFromUtf8Input(machine, req.machine_code, _countof(req.machine_code));
    DWORD bytes = 0;
    BOOL ok = DeviceIoControl(dev, IOCTL_BSPHP_CARD_VERIFY, &req, sizeof(req), nullptr, 0, &bytes, nullptr);
    std::cout << "[ioctl verify] " << (ok ? "OK" : "FAIL")
              << " bytes=" << bytes << " err=" << (ok ? 0 : GetLastError()) << "\n";
    CloseHandle(dev);
}

void RunIoctlRecharge(const std::string& card, const std::string& no, const std::string& pwd) {
    HANDLE dev = CreateFileW(kIoctlDeviceName, GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING,
                             FILE_ATTRIBUTE_NORMAL, nullptr);
    if (dev == INVALID_HANDLE_VALUE) {
        std::cout << "[ioctl] open device failed, err=" << GetLastError() << "\n";
        return;
    }
    IoctlCardRechargeReq req{};
    FillWideFromUtf8Input(card, req.card_id, _countof(req.card_id));
    FillWideFromUtf8Input(no, req.recharge_no, _countof(req.recharge_no));
    FillWideFromUtf8Input(pwd, req.recharge_pwd, _countof(req.recharge_pwd));
    DWORD bytes = 0;
    BOOL ok = DeviceIoControl(dev, IOCTL_BSPHP_CARD_RECHARGE, &req, sizeof(req), nullptr, 0, &bytes, nullptr);
    std::cout << "[ioctl recharge] " << (ok ? "OK" : "FAIL")
              << " bytes=" << bytes << " err=" << (ok ? 0 : GetLastError()) << "\n";
    CloseHandle(dev);
}

}  // namespace

int main() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    std::cout << "BSPHP-Cpp-System-Card-DEMO (Console, full crypto flow)\n";

    bsphp::BsPhp client = bsphp_card_demo::MakeDemoClient();
    if (!client.bootstrap()) {
        std::cerr << "bootstrap failed (connect or get session BSphpSeSsL)\n";
        return 1;
    }

    auto notice = client.get_notice();
    PrintApiResponse("get_notice (gg.in)", notice);

    std::string machine_code = BsphpDemoMachineCodeUtf8();
    std::string logged_icid;

    PrintHelp();
    while (true) {
        std::cout << "\n> ";
        std::string cmd;
        if (!std::getline(std::cin, cmd)) break;
        if (cmd == "exit" || cmd == "quit" || cmd == "q") {
            break;
        } else if (cmd == "help") {
            PrintHelp();
        } else if (cmd == "verify") {
            std::string icid = ReadLine("icid(card): ");
            std::string icpwd = ReadLine("icpwd(password): ");
            if (icid.empty()) {
                std::cout << "icid empty.\n";
                continue;
            }
            bsphp::ApiResponse r = client.login_ic(icid, icpwd, machine_code, machine_code);
            PrintApiResponse("login_ic", r);
            if (LoginIcSuccess(r)) {
                logged_icid = icid;
                auto d = client.api_call("getdate.ic");
                std::cout << "VIP expiry (getdate.ic.data) = " << (d.data.empty() ? "(empty)" : d.data) << "\n";
            }
        } else if (cmd == "getdate") {
            auto r = client.api_call("getdate.ic");
            PrintApiResponse("getdate.ic", r);
        } else if (cmd == "getlkinfo") {
            auto r = client.api_call("getlkinfo.ic");
            PrintApiResponse("getlkinfo.ic", r);
        } else if (cmd == "heart") {
            auto r = client.api_call("timeout.ic");
            PrintApiResponse("timeout.ic", r);
        } else if (cmd == "notice") {
            auto r = client.api_call("gg.in");
            PrintApiResponse("gg.in", r);
        } else if (cmd == "socard") {
            if (logged_icid.empty()) {
                std::cout << "Please verify first.\n";
                continue;
            }
            auto r = client.api_call("socard.in", {{"cardid", logged_icid}});
            PrintApiResponse("socard.in", r);
        } else if (cmd == "info") {
            if (logged_icid.empty()) {
                std::cout << "Please verify first.\n";
                continue;
            }
            std::string icpwd = ReadLine("icpwd(for getinfo.ic): ");
            if (icpwd.empty()) {
                std::cout << "icpwd empty.\n";
                continue;
            }
            auto r = client.api_call("getinfo.ic", {
                {"ic_carid", logged_icid},
                {"ic_pwd", icpwd},
                {"info", "UserName"},
            });
            PrintApiResponse("getinfo.ic (UserName)", r);
        } else if (cmd == "bind") {
            if (logged_icid.empty()) {
                std::cout << "Please verify first.\n";
                continue;
            }
            std::string icpwd = ReadLine("icpwd(for setcaron.ic): ");
            if (icpwd.empty()) {
                std::cout << "icpwd empty.\n";
                continue;
            }
            auto r = client.api_call("setcaron.ic", {
                {"key", machine_code},
                {"icid", logged_icid},
                {"icpwd", icpwd},
            });
            PrintApiResponse("setcaron.ic", r);
        } else if (cmd == "unbind") {
            if (logged_icid.empty()) {
                std::cout << "Please verify first.\n";
                continue;
            }
            std::string icpwd = ReadLine("icpwd(for setcarnot.ic): ");
            if (icpwd.empty()) {
                std::cout << "icpwd empty.\n";
                continue;
            }
            auto r = client.api_call("setcarnot.ic", {
                {"icid", logged_icid},
                {"icpwd", icpwd},
            });
            PrintApiResponse("setcarnot.ic", r);
        } else if (cmd == "renew") {
            if (logged_icid.empty()) {
                std::cout << "Please verify first (need icid for renew URL).\n";
                continue;
            }
            OpenUrlW(bsphp_card_demo::RenewSaleUrlW(WideFromUtf8(logged_icid)));
            std::cout << "Opened renew URL.\n";
        } else if (cmd == "buygen") {
            OpenUrlW(bsphp_card_demo::GenCardSaleUrlW());
            std::cout << "Opened buy-gen URL.\n";
        } else if (cmd == "buystock") {
            OpenUrlW(bsphp_card_demo::StockSaleUrlW());
            std::cout << "Opened buy-stock URL.\n";
        } else if (cmd == "logout") {
            auto r = client.logout_ic();
            PrintApiResponse("logout_ic", r);
            logged_icid.clear();
        } else if (cmd == "ioctl_install") {
            std::string p = ReadLine("driver .sys full path: ");
            bool ok = InstallAndStartIoctlDriver(WideFromUtf8(p));
            std::cout << (ok ? "[ioctl] driver start ok\n" : "[ioctl] driver start fail\n");
        } else if (cmd == "ioctl_verify") {
            const std::string card = ReadLine("ioctl card_id: ");
            const std::string pwd = ReadLine("ioctl card_pwd: ");
            const std::string machine = ReadLine("ioctl machine_code: ");
            RunIoctlVerify(card, pwd, machine);
        } else if (cmd == "ioctl_recharge") {
            const std::string card = ReadLine("ioctl card_id: ");
            const std::string no = ReadLine("ioctl recharge_no: ");
            const std::string pwd = ReadLine("ioctl recharge_pwd: ");
            RunIoctlRecharge(card, no, pwd);
        } else {
            std::cout << "Unknown command. Type `help`.\n";
        }
    }

    return 0;
}

