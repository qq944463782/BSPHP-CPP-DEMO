#include <iostream>
#include <string>
#include <map>
#include <sstream>

#include <windows.h>
#include <shellapi.h>

// Local-only core (no cross-directory includes).
#include "core/bsphp_client.cpp"
#include "core/crypto_http.cpp"
#include "core/login_demo_config.cpp"
#include "core/machine_id.cpp"

namespace {

constexpr wchar_t kIoctlServiceName[] = L"BSPHPSystemLoginDemoDrv";
constexpr wchar_t kIoctlDeviceName[] = L"\\\\.\\BSPHPSystemLoginDemo";
constexpr DWORD IOCTL_BSPHP_LOGIN_PASS =
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x911, METHOD_BUFFERED, FILE_ANY_ACCESS);
constexpr DWORD IOCTL_BSPHP_LOGIN_SMS =
    CTL_CODE(FILE_DEVICE_UNKNOWN, 0x912, METHOD_BUFFERED, FILE_ANY_ACCESS);

struct IoctlLoginPassReq {
    wchar_t user[64];
    wchar_t pwd[64];
    wchar_t machine_code[128];
};

struct IoctlLoginSmsReq {
    wchar_t mobile[32];
    wchar_t area[8];
    wchar_t sms_code[16];
    wchar_t machine_code[128];
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

bool CodeEnabledFromApiData(const std::string& data) {
    // Mirror MFC logic (CLoginDlg::CodeEnabledFromApiData).
    if (data.empty()) return true;  // Mac default
    std::string s = data;
    for (char& c : s) c = static_cast<char>(tolower(static_cast<unsigned char>(c)));
    return s.find("checked") != std::string::npos || s == "true" || s == "1";
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

void RunIoctlLoginPass(const std::string& user, const std::string& pwd, const std::string& machine) {
    HANDLE dev = CreateFileW(kIoctlDeviceName, GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING,
                             FILE_ATTRIBUTE_NORMAL, nullptr);
    if (dev == INVALID_HANDLE_VALUE) {
        std::cout << "[ioctl] open device failed, err=" << GetLastError() << "\n";
        return;
    }
    IoctlLoginPassReq req{};
    FillWideFromUtf8Input(user, req.user, _countof(req.user));
    FillWideFromUtf8Input(pwd, req.pwd, _countof(req.pwd));
    FillWideFromUtf8Input(machine, req.machine_code, _countof(req.machine_code));
    DWORD bytes = 0;
    BOOL ok = DeviceIoControl(dev, IOCTL_BSPHP_LOGIN_PASS, &req, sizeof(req), nullptr, 0, &bytes, nullptr);
    std::cout << "[ioctl login pass] " << (ok ? "OK" : "FAIL")
              << " bytes=" << bytes << " err=" << (ok ? 0 : GetLastError()) << "\n";
    CloseHandle(dev);
}

void RunIoctlLoginSms(const std::string& mobile, const std::string& area, const std::string& sms,
                      const std::string& machine) {
    HANDLE dev = CreateFileW(kIoctlDeviceName, GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING,
                             FILE_ATTRIBUTE_NORMAL, nullptr);
    if (dev == INVALID_HANDLE_VALUE) {
        std::cout << "[ioctl] open device failed, err=" << GetLastError() << "\n";
        return;
    }
    IoctlLoginSmsReq req{};
    FillWideFromUtf8Input(mobile, req.mobile, _countof(req.mobile));
    FillWideFromUtf8Input(area, req.area, _countof(req.area));
    FillWideFromUtf8Input(sms, req.sms_code, _countof(req.sms_code));
    FillWideFromUtf8Input(machine, req.machine_code, _countof(req.machine_code));
    DWORD bytes = 0;
    BOOL ok = DeviceIoControl(dev, IOCTL_BSPHP_LOGIN_SMS, &req, sizeof(req), nullptr, 0, &bytes, nullptr);
    std::cout << "[ioctl login sms] " << (ok ? "OK" : "FAIL")
              << " bytes=" << bytes << " err=" << (ok ? 0 : GetLastError()) << "\n";
    CloseHandle(dev);
}

void PrintHelp() {
    std::cout << "\nCommands:\n"
        << "  help                 show commands\n"
        << "  endtime              get end time (vipdate.lg)\n"
        << "  version              v.in\n"
        << "  weburl               weburl.in\n"
        << "  globalinfo           globalinfo.in\n"
        << "  heartbeat            timeout.lg\n"
        << "  userinfo             getuserinfo.lg\n"
        << "  codeall              get_code_enabled (all types)\n"
        << "  code_login           get_code_enabled (INGES_LOGIN)\n"
        << "  code_reg             get_code_enabled (INGES_RE)\n"
        << "  code_back            get_code_enabled (INGES_MACK)\n"
        << "  code_say             get_code_enabled (INGES_SAY)\n"
        << "  renew                renew URL (requires getuserinfo UserName)\n"
        << "  buygen               open buy-gen URL (gencard)\n"
        << "  buystock             open buy-stock URL (salecard)\n"
        << "  logout               logout_lg\n"
        << "  ioctl_install        install/start driver service\n"
        << "  ioctl_login          send DeviceIoControl login(pass) request\n"
        << "  ioctl_sms            send DeviceIoControl login(sms) request\n"
        << "  api <name> [k=v...]  call api_call with optional params\n"
        << "  exit                 quit\n\n";
}

}  // namespace

int main() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    std::cout << "BSPHP-Cpp-System-Login-DEMO (Console, full crypto flow)\n";

    bsphp::BsPhp client = bsphp_login_demo::MakeDemoClient();
    if (!client.bootstrap()) {
        std::cerr << "bootstrap failed (connect or get session BSphpSeSsL)\n";
        return 1;
    }

    PrintApiResponse("get_notice (gg.in)", client.get_notice());

    // Query whether captcha is enabled for password login.
    auto code = client.get_code_enabled("INGES_LOGIN");
    PrintApiResponse("get_code_enabled(INGES_LOGIN)", code);
    const bool need_captcha = CodeEnabledFromApiData(code.data);

    std::string user = ReadLine("user (default admin): ");
    if (user.empty()) user = "admin";
    std::string pwd = ReadLine("password (default admin): ");
    if (pwd.empty()) pwd = "admin";

    std::string coode;
    if (need_captcha) {
        const long long tick = static_cast<long long>(time(nullptr));
        const std::string url = client.code_url + client.BSphpSeSsL + "&_=" + std::to_string(tick);
        std::cout << "Captcha required. Open this URL and input coode:\n" << url << "\n";
        coode = ReadLine("coode: ");
    }

    const std::string machine_code = BsphpDemoMachineCodeUtf8();
    PrintApiResponse("login_lg", client.login_lg(user, pwd, coode, machine_code, machine_code));

    if (!(client.BSphpSeSsL.size() > 0)) {
        std::cout << "Login failed (no session). Exit.\n";
        return 2;
    }

    PrintHelp();
    while (true) {
        std::cout << "\n> ";
        std::string line;
        if (!std::getline(std::cin, line)) break;
        if (line == "exit" || line == "quit" || line == "q") {
            break;
        }
        if (line == "help") {
            PrintHelp();
            continue;
        }

        std::istringstream iss(line);
        std::string cmd;
        iss >> cmd;

        if (cmd == "endtime") {
            PrintApiResponse("get_end_time", client.get_end_time());
        } else if (cmd == "version") {
            PrintApiResponse("v.in", client.get_version());
        } else if (cmd == "weburl") {
            PrintApiResponse("weburl.in", client.api_call("weburl.in"));
        } else if (cmd == "globalinfo") {
            PrintApiResponse("globalinfo.in", client.api_call("globalinfo.in"));
        } else if (cmd == "heartbeat") {
            PrintApiResponse("timeout.lg (heartbeat)", client.api_call("timeout.lg"));
        } else if (cmd == "userinfo") {
            PrintApiResponse("getuserinfo.lg", client.api_call("getuserinfo.lg"));
        } else if (cmd == "codeall") {
            PrintApiResponse("get_code_enabled(all)", client.get_code_enabled("INGES_LOGIN|INGES_RE|INGES_MACK|INGES_SAY"));
        } else if (cmd == "code_login") {
            PrintApiResponse("get_code_enabled(INGES_LOGIN)", client.get_code_enabled("INGES_LOGIN"));
        } else if (cmd == "code_reg") {
            PrintApiResponse("get_code_enabled(INGES_RE)", client.get_code_enabled("INGES_RE"));
        } else if (cmd == "code_back") {
            PrintApiResponse("get_code_enabled(INGES_MACK)", client.get_code_enabled("INGES_MACK"));
        } else if (cmd == "code_say") {
            PrintApiResponse("get_code_enabled(INGES_SAY)", client.get_code_enabled("INGES_SAY"));
        } else if (cmd == "renew") {
            // Mirror CConsoleDlg::OnConsoleRenew.
            auto r = client.api_call("getuserinfo.lg", {{"info", "UserName"}});
            const std::wstring url = bsphp_login_demo::RenewSaleUrlWithUserFromInfoDataW(r.data);
            OpenUrlW(url);
            PrintApiResponse("getuserinfo.lg (UserName) for renew", r);
            std::cout << "Opened renew URL.\n";
        } else if (cmd == "buygen") {
            OpenUrlW(bsphp_login_demo::GenCardSaleUrlW());
            std::cout << "Opened buy-gen URL.\n";
        } else if (cmd == "buystock") {
            OpenUrlW(bsphp_login_demo::StockSaleUrlW());
            std::cout << "Opened buy-stock URL.\n";
        } else if (cmd == "logout") {
            PrintApiResponse("logout_lg", client.logout_lg());
            std::cout << "Logged out.\n";
            break;
        } else if (cmd == "ioctl_install") {
            std::string p = ReadLine("driver .sys full path: ");
            bool ok = InstallAndStartIoctlDriver(WideFromUtf8(p));
            std::cout << (ok ? "[ioctl] driver start ok\n" : "[ioctl] driver start fail\n");
        } else if (cmd == "ioctl_login") {
            const std::string u = ReadLine("ioctl user: ");
            const std::string p = ReadLine("ioctl pwd: ");
            const std::string m = ReadLine("ioctl machine_code: ");
            RunIoctlLoginPass(u, p, m);
        } else if (cmd == "ioctl_sms") {
            const std::string mob = ReadLine("ioctl mobile: ");
            const std::string area = ReadLine("ioctl area(86): ");
            const std::string sms = ReadLine("ioctl sms_code: ");
            const std::string m = ReadLine("ioctl machine_code: ");
            RunIoctlLoginSms(mob, area.empty() ? "86" : area, sms, m);
        } else if (cmd == "api") {
            std::string apiName;
            iss >> apiName;
            if (apiName.empty()) {
                std::cout << "Usage: api <name> [k=v...]\n";
                continue;
            }
            std::map<std::string, std::string> params;
            std::string kv;
            while (iss >> kv) {
                auto eq = kv.find('=');
                if (eq == std::string::npos) continue;
                std::string k = kv.substr(0, eq);
                std::string v = kv.substr(eq + 1);
                if (!k.empty()) params[k] = v;
            }
            PrintApiResponse(std::string("api_call(") + apiName + ")", client.api_call(apiName, params));
        } else {
            std::cout << "Unknown command. Type `help`.\n";
        }
    }

    return 0;
}

