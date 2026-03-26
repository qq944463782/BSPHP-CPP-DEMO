/*
 * BSPHP demo configuration (login)
 * Derived from Mac bsphp.mac.demo ContentView.swift.
 */
#include "login_demo_config.h"

#include <windows.h>

#include <cstdio>
#include <string>

namespace bsphp_login_demo {
namespace {

static std::string UrlEncodeUtf8Bytes(const std::string& utf8) {
    std::string out;
    for (unsigned char c : utf8) {
        if ((c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') ||
            c == '-' || c == '_' || c == '.' || c == '~') {
            out += static_cast<char>(c);
        } else if (c == ' ') {
            out += '+';
        } else {
            char buf[8];
            sprintf_s(buf, "%%%02X", c);
            out += buf;
        }
    }
    return out;
}

}  // namespace
namespace {

// Server private key (Base64 DER, PKCS#8)
const char kServerKey[] =
    "MIIEqAIBADANBgkqhkiG9w0BAQEFAASCBJIwggSOAgEAAoH+DEr7H5BhMwRA9ZWXVftcCWHznBdl0gQBu5617qSe9in+uloF1sC64Ybdc8Q0JwQkGQANC5PnMqPLgXXIfnl7/LpnQ/BvghQI5cr/4DEezRKrmQaXgYfXHL3woVw7JIsLpPTGa7Ar9S6SEH8RcPIbZjlPVRZPwV3RgWgox2/4lkXsmopqD+mEtOI/ntvti147nEpK2c7cdtCU5M2hQSlIXsTWvri88RTYJ/CtopBOXarUkNBfpWGImiYGsmbZI+YZ6uU0wSYlq8huu+pkTseUUiymzmv8Rpg3coi7YU+pszvB9wnQ1Rz6Z/B6Z3WN7d6OP7f9w0Q0WvgrsKcEJhMCAwEAAQKB/gHa5t6yiRiL0cm902K0VgVMdNjfZww0cpZ/svDaguqfF8PDhhIMb6dNFOo9d6lTpKbpLQ7MOR2ZPkLBJYqAhsdy0dac2BcHMviKk+afQwirgp3LMt3nQ/0gZMnVA0/Wc+Fm1vK1WUzcxEodAuLKhnv8tg4fGdYSdGVU9KJ0MU1bKQZXv0CAIhJYWsiCa5y5bFO7K+ia+UIVBHcvITQLzlgEm+Z/X6ye5cws4pWbk8+spsBDvweb5jpelbkCYs5C5TRNIWXk7+QxTXTg1vrcsmZRcmpRJq7sOd3faZltNHTIlB3HhWnsf47Bz334j9RtU8iqonbuBmcnYbD3+bvBAn891RGdAl+rVU/sJ2kPXmV4eqJOwJfbi8o1WYDp4GcK0ThjrZ1pmaZMj2WTjb3QX1VUoi+7l3389KzzDn0VXLKXZvGxmLikA1FWuuLUmwfNTxyxtGTBVeZCEaQ2lEJuaDGsK0oLi4Bo8ELfQw6JFK7jlgtTlflcYcul99P9BThDAn8y5TpSQy8/07LCgMMZOgJomYzQUmd14Zn2VQLH1u1Z4v2CPlOzGanDt7mmGZCew7iMSO1P0TrwDIreKzYyERuVvZti/IFHH1+J1hAbvk9SJGmdt46W5lyIp3xjdR2QmiK+hSsc8HF9R+zPaSe9yGA8+FwxLRfo0snGP3MC3aXxAn4n2iyABgejZlkc3EnanfzIqkHygC9gUbkCqa1tEDVZw3+Uv1G1vlJxBftyHuk4ZDmbUu1w+zM41nqiLbRxEE4LR06AKO7Yx0qlm86XOVTN/y9/WcWW1saRzs0IYIZwordhQIV463DYMgLn41B7Cdmu1gZ22TLfWCjpz9HSQosCfwMJu9l9OSzOLjV+CidPVyV3RPiKcrKOrOoPWQMkyTY8XnWP0t82APQ121cW35Mai8GT+NZy3tnFZeStH6cNbmAZ2VSnTfA45zMLHBsL2SBGHCfV9ST8yzk9BifJreIb0UceG9y2XY/k4zXeSQkDFPuOt7IXxv2W14SF9Q+Ou4ECfzfRP1hXPwq2w4YJ8sLmqWJT+3aMDucei5MJEAJNifZWhdW0GIrlKRSbhIgLAunxq+KK+mAPqqWw7Prsa21JbXSe3gugusu5d6ESURvLENRKI+Pp9TgRESsydeLy8VcPKRJ5/Ct7/p6QB3A+7F/iPNE2GagGffG9i7e+OdcToYQ=";

// Client public key (Base64 DER)
const char kClientKey[] =
    "MIIBHjANBgkqhkiG9w0BAQEFAAOCAQsAMIIBBgKB/g26m2hYtESqcKW+95Lr+PfCd4bwHW2Z+mM0/vcKQ5j/ZGMigqkgl3QXCEcsCaw0KFSmqAPtLbrl6p5Sp+ZUSYEYQhSxAajE5qRCd3k0r/MIQQanBaOALkP71/u6U2SZhrTXd05n1wQo6ojMH/xVunBOFOa/Eon/Y5FVh6GiJpwwDkFzTlnecmff7Y+VDqRhZ7vu2CQjApOx23N6DiFEmVZYEb/efyASngoZ+3A/DSB5cwbaYVZ21EhPe/GNcwtUleFHn+d4vb0cvolO3Gyw6ObceOT/Q7E3k8ejIml6vPKzmRdtw0FXGOJTclx1CjShRDfXoUjFGyXHy3sZs9VLAgMBAAE=";

// AppEn API endpoint (Mac demo uses https)
const char kBSPHPURL[] =
    "https://demo.bsphp.com/AppEn.php?appid=8888888&m=95e87faf2f6e41babddaef60273489e1&lang=0";

const char kMutualKey[] = "6600cfcd5ac01b9bb3f2460eb416daa8";

// Captcha image prefix (append session token, then &_=refreshTick)
const char kCodeURLPrefix[] = "https://demo.bsphp.com/index.php?m=coode&sessl=";

// Web login prefix (Mac: kBSPHPWebLoginURL + bsPhpSeSsL)
const char kWebLoginURLPrefix[] =
    "https://demo.bsphp.com/index.php?m=webapi&c=software_auth&a=index&daihao=8888888&BSphpSeSsL=";

}  // namespace

bsphp::BsPhp MakeDemoClient() {
    bsphp::BsPhp c(kBSPHPURL, kMutualKey, kServerKey, kClientKey);
    c.code_url = kCodeURLPrefix;
    return c;
}

std::string MakeWebLoginUrl(const std::string& bsphpSeSsL) {
    return std::string(kWebLoginURLPrefix) + bsphpSeSsL;
}

// Mac: kBSPHPRenewURL / kBSPHPRenewCardURL / kBSPHPRenewStockCardURL
const wchar_t kRenewSaleUrl[] =
    L"https://demo.bsphp.com/index.php?m=webapi&c=salecard_renew&a=index&daihao=8888888";
const wchar_t kGenCardSaleUrl[] =
    L"https://demo.bsphp.com/index.php?m=webapi&c=salecard_gencard&a=index&daihao=8888888";
const wchar_t kStockSaleUrl[] =
    L"https://demo.bsphp.com/index.php?m=webapi&c=salecard_salecard&a=index&daihao=8888888";

std::wstring RenewSaleUrlW() {
    return std::wstring(kRenewSaleUrl);
}

std::wstring GenCardSaleUrlW() {
    return std::wstring(kGenCardSaleUrl);
}

std::wstring StockSaleUrlW() {
    return std::wstring(kStockSaleUrl);
}

std::wstring RenewSaleUrlWithUserFromInfoDataW(const std::string& getuserinfo_data_utf8) {
    const char kBaseUtf8[] =
        "https://demo.bsphp.com/index.php?m=webapi&c=salecard_renew&a=index&daihao=8888888";
    std::string s = getuserinfo_data_utf8;
    while (!s.empty() && (s.back() == '\r' || s.back() == '\n' || s.back() == ' ' || s.back() == '\t')) {
        s.pop_back();
    }
    std::string user;
    const size_t eq = s.find('=');
    if (eq != std::string::npos) {
        user = s.substr(eq + 1);
    } else {
        user = s;
    }
    while (!user.empty() && (user.front() == ' ' || user.front() == '\t')) {
        user.erase(user.begin());
    }
    std::string url_utf8(kBaseUtf8);
    if (!user.empty()) {
        url_utf8 += "&user=";
        url_utf8 += UrlEncodeUtf8Bytes(user);
    }
    int n = MultiByteToWideChar(CP_UTF8, 0, url_utf8.c_str(), -1, nullptr, 0);
    if (n <= 0) {
        return RenewSaleUrlW();
    }
    std::wstring w;
    w.resize(static_cast<size_t>(n - 1));
    MultiByteToWideChar(CP_UTF8, 0, url_utf8.c_str(), -1, &w[0], n);
    return w;
}

}  // namespace bsphp_login_demo
