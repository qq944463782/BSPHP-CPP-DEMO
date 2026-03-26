#pragma once

#include "bsphp_client.h"

#include <string>

namespace bsphp_login_demo {

// Creates BSPHPClient preconfigured with demo keys and captcha/image URLs (same as Mac demo).
bsphp::BsPhp MakeDemoClient();

// Web方式登陆入口（Mac 用 WKWebView 监控 hash，此处直接打开浏览器）。
std::string MakeWebLoginUrl(const std::string& bsphpSeSsL);

// 续费/购卡推广页（与 Mac demo_bsphp_console_window_view 一致）
std::wstring RenewSaleUrlW();
std::wstring GenCardSaleUrlW();
std::wstring StockSaleUrlW();

// Append &user=... from getuserinfo.lg data when possible
std::wstring RenewSaleUrlWithUserFromInfoDataW(const std::string& getuserinfo_data_utf8);

}  // namespace bsphp_login_demo

