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
        << "  exit                 quit\n\n";
}

bool LoginIcSuccess(const bsphp::ApiResponse& r) {
    // Mirror MFC logic: success when r.code == 1081 or r.data contains 1081.
    if (r.code == "1081") return true;
    return r.data.find("1081") != std::string::npos;
}

}  // namespace

int main() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    std::cout << "BSPHP-Cpp-CMD-Card-DEMO (Console)\n";

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
        } else {
            std::cout << "Unknown command. Type `help`.\n";
        }
    }

    return 0;
}

