#include <windows.h>
#include <shellapi.h>
#include <string>
#include <map>
#include <sstream>
#include <commctrl.h>
#include <vector>

#include "core/bsphp_client.h"
#include "core/card_config.h"
#include "core/machine_id.h"
#include "core/string_utf8.h"

namespace {

constexpr int kEditW = 240;

enum ControlId {
    IDC_TAB = 3001,
    IDC_NOTICE_EDIT = 3002,
    IDC_STATUS_STATIC = 3003,
    IDC_LOG_EDIT = 3004,

    // Card tab controls (Tab0)
    IDC_CARD_INPUT_EDIT = 1001,
    IDC_CARD_PWD_EDIT = 1002,
    IDC_CARD_VERIFY_BTN = 2001,
    IDC_CARD_TESTNET_BTN = 2002,
    IDC_CARD_VERSION_BTN = 2003,
    IDC_CARD_RENEW_BTN = 2004,
    IDC_CARD_BUY_RECHARGE_BTN = 2005,
    IDC_CARD_BUY_STOCK_BTN = 2006,
    IDC_CARD_LOGOUT_BTN = 2007,

    // Machine tab controls (Tab1)
    IDC_MACHINE_CODE_EDIT = 2101,
    IDC_RADIO_MACHINE_VERIFY = 2102,
    IDC_RADIO_MACHINE_RECHARGE = 2103,
    IDC_MACHINE_VERIFY_BTN = 2201,
    IDC_MACHINE_TESTNET_BTN = 2202,
    IDC_MACHINE_VERSION_BTN = 2203,
    IDC_MACHINE_CONFIRM_RECHARGE_BTN = 2204,
    IDC_MACHINE_ONECLICK_RENEW_BTN = 2205,
    IDC_MACHINE_BUY_RECHARGE_BTN = 2206,
    IDC_MACHINE_BUY_STOCK_BTN = 2207,
    IDC_MACHINE_RECHARGE_CARD_NO_LABEL = 2210,
    IDC_MACHINE_RECHARGE_CARD_NO_EDIT = 2211, // ka
    IDC_MACHINE_RECHARGE_CARD_PWD_LABEL = 2212,
    IDC_MACHINE_RECHARGE_CARD_PWD_EDIT = 2213  // pwd
};

enum PanelControlId {
    IDC_PANEL_INFO_STATIC = 6001,
    IDC_PANEL_VIP_STATIC = 6002,
    IDC_PANEL_PWD_EDIT = 6003,
    IDC_PANEL_LOG_EDIT = 6004,

    IDC_P_DATE = 6101,
    IDC_P_LK = 6102,
    IDC_P_HEART = 6103,
    IDC_P_NOTICE = 6104,
    IDC_P_SVRDATE = 6105,
    IDC_P_VER = 6106,
    IDC_P_SOFT = 6107,
    IDC_P_URL = 6108,
    IDC_P_WEBURL = 6109,
    IDC_P_CUST_APP = 6110,
    IDC_P_CUST_VIP = 6111,
    IDC_P_CUST_LOGIN = 6112,
    IDC_P_GLOBAL = 6113,
    IDC_P_LA = 6114,
    IDC_P_LB = 6115,
    IDC_P_QUERY = 6116,
    IDC_P_INFO = 6117,
    IDC_P_BIND = 6118,
    IDC_P_UNBIND = 6119,
    IDC_P_WRENEW = 6120,
    IDC_P_WGEN = 6121,
    IDC_P_WSTOCK = 6122,
    IDC_P_LOGOUT = 6123,
};

struct AppState {
    bsphp::BsPhp client = bsphp_card_demo::MakeDemoClient();
    std::string machine_code = BsphpDemoMachineCodeUtf8();
    bool bootstrapped = false;
    std::string logged_card_id;
    std::string logged_card_pwd;
    std::string vip_expiry;
    HWND panel_hwnd = nullptr;
};

static void AppendLogW(HWND hEdit, const std::wstring& line) {
    if (!hEdit) return;
    int len = GetWindowTextLengthW(hEdit);
    std::wstring prefix;
    prefix.resize(static_cast<size_t>(len));
    if (len > 0) {
        GetWindowTextW(hEdit, &prefix[0], len + 1);
    }
    std::wstring next = prefix;
    next += line;
    next += L"\r\n";
    SetWindowTextW(hEdit, next.c_str());
    SendMessageW(hEdit, EM_SCROLLCARET, 0, 0);
}

static void OpenUrlW(const std::wstring& url) {
    if (url.empty()) return;
    ShellExecuteW(nullptr, L"open", url.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
}

static bool LoginIcSuccess(const bsphp::ApiResponse& r) {
    if (r.code == "1081") return true;
    return r.data.find("1081") != std::string::npos;
}

static std::wstring ToW(const std::string& s) {
    return WideFromUtf8(s);
}

static AppState* GetState(HWND hwnd) {
    return reinterpret_cast<AppState*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
}

static void SetState(HWND hwnd, AppState* st) {
    SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(st));
}

static void LogApi(HWND hwnd, const char* title, const bsphp::ApiResponse& r) {
    AppState* st = GetState(hwnd);
    (void)st;
    HWND hLog = GetDlgItem(hwnd, IDC_LOG_EDIT);
    std::wstringstream ss;
    ss << L"==" << ToW(title ? std::string(title) : std::string("")) << L"==\n";
    ss << L"code=" << ToW(r.code) << L"\n";
    ss << L"data=" << (r.data.empty() ? L"(empty)" : ToW(r.data)) << L"\n";
    AppendLogW(hLog, ss.str());
}

struct PanelState {
    AppState* mainSt = nullptr;
};

static void AppendPanelText(HWND hEdit, const std::wstring& block) {
    AppendLogW(hEdit, block);
}

static void AppendPanelResult(HWND hwnd, const wchar_t* title, const bsphp::ApiResponse& r) {
    HWND hLog = GetDlgItem(hwnd, IDC_PANEL_LOG_EDIT);
    if (!hLog) return;
    const std::wstring codeW = WideFromUtf8(r.code);
    const std::wstring dataW = r.data.empty() ? L"(empty)" : WideFromUtf8(r.data);
    std::wstringstream ss;
    ss << title << L"\r\ncode=" << codeW << L" data=" << dataW << L"\r\n";
    AppendPanelText(hLog, ss.str());
}

static std::string Utf8FromHwndEdit(HWND hEdit) {
    if (!hEdit) return {};
    const int len = GetWindowTextLengthW(hEdit);
    std::wstring w;
    w.resize(static_cast<size_t>(len));
    if (len > 0) GetWindowTextW(hEdit, &w[0], len + 1);
    return Utf8FromWide(w.c_str());
}

static void RefreshPanelHeader(HWND hwnd, AppState* st) {
    if (!st) return;
    const std::wstring cardW = WideFromUtf8(st->logged_card_id);
    const std::wstring vipW = st->vip_expiry.empty() ? L"-" : WideFromUtf8(st->vip_expiry);
    SetWindowTextW(GetDlgItem(hwnd, IDC_PANEL_INFO_STATIC), (L"当前卡号：" + cardW).c_str());
    SetWindowTextW(GetDlgItem(hwnd, IDC_PANEL_VIP_STATIC), (L"VIP 到期：" + vipW).c_str());
    SetWindowTextW(GetDlgItem(hwnd, IDC_PANEL_PWD_EDIT), WideFromUtf8(st->logged_card_pwd).c_str());
}

LRESULT CALLBACK PanelWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE: {
        auto* st = reinterpret_cast<AppState*>((reinterpret_cast<CREATESTRUCTW*>(lParam))->lpCreateParams);
        auto* ps = new PanelState();
        ps->mainSt = st;
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(ps));

        const int x = 16;
        const int w = 900;
        CreateWindowW(L"STATIC", L"", WS_CHILD | WS_VISIBLE, x, 12, w - 40, 22, hwnd, (HMENU)IDC_PANEL_INFO_STATIC, nullptr, nullptr);
        CreateWindowW(L"STATIC", L"", WS_CHILD | WS_VISIBLE, x, 36, w - 40, 22, hwnd, (HMENU)IDC_PANEL_VIP_STATIC, nullptr, nullptr);

        CreateWindowW(L"STATIC", L"卡密密码：", WS_CHILD | WS_VISIBLE, x, 62, 80, 22, hwnd, nullptr, nullptr, nullptr);
        CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL | ES_PASSWORD,
                       x + 85, 62, 240, 22, hwnd, (HMENU)IDC_PANEL_PWD_EDIT, nullptr, nullptr);

        auto mkBtn = [&](int bx, int by, int bw, const wchar_t* text, int id) {
            CreateWindowW(L"BUTTON", text, WS_CHILD | WS_VISIBLE, bx, by, bw, 26, hwnd, (HMENU)id, nullptr, nullptr);
        };

        int by = 92;
        int bx = x;
        mkBtn(bx, by, 100, L"刷新到期", IDC_P_DATE); bx += 106;
        mkBtn(bx, by, 100, L"登录状态", IDC_P_LK); bx += 106;
        mkBtn(bx, by, 80, L"心跳", IDC_P_HEART); bx += 86;
        mkBtn(bx, by, 80, L"公告", IDC_P_NOTICE); bx += 86;
        mkBtn(bx, by, 110, L"服务器时间", IDC_P_SVRDATE); bx += 116;
        mkBtn(bx, by, 80, L"版本", IDC_P_VER); bx += 86;
        mkBtn(bx, by, 100, L"软件描述", IDC_P_SOFT); bx += 106;
        mkBtn(bx, by, 80, L"url.in", IDC_P_URL); bx += 86;
        mkBtn(bx, by, 90, L"weburl.in", IDC_P_WEBURL);

        by += 34;
        bx = x;
        mkBtn(bx, by, 100, L"软件配置", IDC_P_CUST_APP); bx += 106;
        mkBtn(bx, by, 100, L"VIP配置", IDC_P_CUST_VIP); bx += 106;
        mkBtn(bx, by, 100, L"登录配置", IDC_P_CUST_LOGIN); bx += 106;
        mkBtn(bx, by, 100, L"全局配置", IDC_P_GLOBAL); bx += 106;
        mkBtn(bx, by, 80, L"逻辑A", IDC_P_LA); bx += 86;
        mkBtn(bx, by, 80, L"逻辑B", IDC_P_LB); bx += 86;
        mkBtn(bx, by, 100, L"激活查询", IDC_P_QUERY); bx += 106;
        mkBtn(bx, by, 80, L"卡信息", IDC_P_INFO); bx += 86;
        mkBtn(bx, by, 100, L"绑定本机", IDC_P_BIND); bx += 106;
        mkBtn(bx, by, 100, L"解除绑定", IDC_P_UNBIND);

        by += 34;
        mkBtn(x, by, 110, L"续费充值页", IDC_P_WRENEW);
        mkBtn(x + 116, by, 110, L"购买充值卡", IDC_P_WGEN);
        mkBtn(x + 232, by, 110, L"购买库存卡", IDC_P_WSTOCK);
        mkBtn(x + 350, by, 80, L"注销", IDC_P_LOGOUT);

        CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"",
                         WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY | WS_VSCROLL,
                         x, by + 34, w - 40, 360, hwnd, (HMENU)IDC_PANEL_LOG_EDIT, nullptr, nullptr);

        if (st) {
            RefreshPanelHeader(hwnd, st);
            AppendPanelText(GetDlgItem(hwnd, IDC_PANEL_LOG_EDIT), L"[panel] ready.");
        }
        return 0;
    }
    case WM_COMMAND: {
        auto* ps = reinterpret_cast<PanelState*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
        if (!ps || !ps->mainSt) break;
        auto* st = ps->mainSt;
        st->logged_card_pwd = Utf8FromHwndEdit(GetDlgItem(hwnd, IDC_PANEL_PWD_EDIT));

        switch (LOWORD(wParam)) {
        case IDC_P_DATE: {
            auto r = st->client.api_call("getdate.ic");
            if (!r.data.empty()) st->vip_expiry = r.data;
            RefreshPanelHeader(hwnd, st);
            AppendPanelResult(hwnd, L"刷新到期(getdate.ic)", r);
            break;
        }
        case IDC_P_LK: AppendPanelResult(hwnd, L"登录状态(getlkinfo.ic)", st->client.api_call("getlkinfo.ic")); break;
        case IDC_P_HEART: AppendPanelResult(hwnd, L"心跳(timeout.ic)", st->client.api_call("timeout.ic")); break;
        case IDC_P_NOTICE: AppendPanelResult(hwnd, L"公告(gg.in)", st->client.api_call("gg.in")); break;
        case IDC_P_SVRDATE: AppendPanelResult(hwnd, L"服务器时间(date.in)", st->client.api_call("date.in")); break;
        case IDC_P_VER: AppendPanelResult(hwnd, L"版本(v.in)", st->client.api_call("v.in")); break;
        case IDC_P_SOFT: AppendPanelResult(hwnd, L"软件描述(miao.in)", st->client.api_call("miao.in")); break;
        case IDC_P_URL: AppendPanelResult(hwnd, L"预设URL(url.in)", st->client.api_call("url.in")); break;
        case IDC_P_WEBURL: AppendPanelResult(hwnd, L"Web地址(weburl.in)", st->client.api_call("weburl.in")); break;
        case IDC_P_CUST_APP: AppendPanelResult(hwnd, L"软件配置(appcustom.in myapp)", st->client.api_call("appcustom.in", {{"info", "myapp"}})); break;
        case IDC_P_CUST_VIP: AppendPanelResult(hwnd, L"VIP配置(appcustom.in myvip)", st->client.api_call("appcustom.in", {{"info", "myvip"}})); break;
        case IDC_P_CUST_LOGIN: AppendPanelResult(hwnd, L"登录配置(appcustom.in mylogin)", st->client.api_call("appcustom.in", {{"info", "mylogin"}})); break;
        case IDC_P_GLOBAL: AppendPanelResult(hwnd, L"全局配置(globalinfo.in)", st->client.api_call("globalinfo.in")); break;
        case IDC_P_LA: AppendPanelResult(hwnd, L"逻辑A(logica.in)", st->client.api_call("logica.in")); break;
        case IDC_P_LB: AppendPanelResult(hwnd, L"逻辑B(logicb.in)", st->client.api_call("logicb.in")); break;
        case IDC_P_QUERY: AppendPanelResult(hwnd, L"激活查询(socard.in)", st->client.api_call("socard.in", {{"cardid", st->logged_card_id}})); break;
        case IDC_P_INFO:
            AppendPanelResult(hwnd, L"卡信息(getinfo.ic)",
                              st->client.api_call("getinfo.ic", {{"ic_carid", st->logged_card_id}, {"ic_pwd", st->logged_card_pwd}, {"info", "UserName"}}));
            break;
        case IDC_P_BIND:
            AppendPanelResult(hwnd, L"绑定本机(setcaron.ic)",
                              st->client.api_call("setcaron.ic", {{"key", st->machine_code}, {"icid", st->logged_card_id}, {"icpwd", st->logged_card_pwd}}));
            break;
        case IDC_P_UNBIND:
            AppendPanelResult(hwnd, L"解除绑定(setcarnot.ic)",
                              st->client.api_call("setcarnot.ic", {{"icid", st->logged_card_id}, {"icpwd", st->logged_card_pwd}}));
            break;
        case IDC_P_WRENEW: OpenUrlW(bsphp_card_demo::RenewSaleUrlW(WideFromUtf8(st->logged_card_id))); break;
        case IDC_P_WGEN: OpenUrlW(bsphp_card_demo::GenCardSaleUrlW()); break;
        case IDC_P_WSTOCK: OpenUrlW(bsphp_card_demo::StockSaleUrlW()); break;
        case IDC_P_LOGOUT:
            st->client.logout_ic();
            if (st->panel_hwnd && IsWindow(st->panel_hwnd)) {
                DestroyWindow(st->panel_hwnd);
                st->panel_hwnd = nullptr;
            }
            DestroyWindow(hwnd);
            break;
        default: break;
        }
        return 0;
    }
    case WM_DESTROY: {
        auto* ps = reinterpret_cast<PanelState*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
        if (ps) {
            if (ps->mainSt) ps->mainSt->panel_hwnd = nullptr;
            delete ps;
        }
        return 0;
    }
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

static void OpenPanelWindow(AppState* st) {
    if (!st) return;
    if (st->panel_hwnd && IsWindow(st->panel_hwnd)) {
        ShowWindow(st->panel_hwnd, SW_SHOW);
        SetForegroundWindow(st->panel_hwnd);
        return;
    }
    const wchar_t* kPanelClassName = L"BSPHPWinCardPanel";
    HINSTANCE hInst = GetModuleHandleW(nullptr);
    WNDCLASSW wc{};
    wc.lpfnWndProc = PanelWndProc;
    wc.hInstance = hInst;
    wc.lpszClassName = kPanelClassName;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    RegisterClassW(&wc);
    HWND hwnd = CreateWindowW(kPanelClassName, L"主控制面板 (Card)",
                               WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT,
                               980, 620, nullptr, nullptr, hInst, st);
    if (hwnd) st->panel_hwnd = hwnd;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE: {
        AppState* st = new AppState();
        SetState(hwnd, st);

        auto setVisible = [&](int id, bool visible) {
            HWND h = GetDlgItem(hwnd, id);
            if (h) ShowWindow(h, visible ? SW_SHOW : SW_HIDE);
        };

        const int x = 20;
        const int w = 640;

        // Notice area (top, similar to mac group box)
        CreateWindowW(L"BUTTON", L"公告", WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
                       x, 20, w, 90, hwnd, nullptr, nullptr, nullptr);
        HWND hNotice = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"",
                                        WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_AUTOVSCROLL |
                                            ES_READONLY | WS_VSCROLL,
                                        x + 10, 40, w - 20, 60, hwnd, (HMENU)IDC_NOTICE_EDIT, nullptr, nullptr);
        (void)hNotice;

        // TabControl
        HWND hTab = CreateWindowExW(0, WC_TABCONTROLW, L"",
                                     WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS,
                                     x, 120, w, 28, hwnd, (HMENU)IDC_TAB, nullptr, nullptr);
        TCITEMW ti{};
        ti.mask = TCIF_TEXT;
        ti.pszText = const_cast<LPWSTR>(L"制作卡密登陆模式");
        TabCtrl_InsertItem(hTab, 0, &ti);
        ti.pszText = const_cast<LPWSTR>(L"一键注册机器码账号");
        TabCtrl_InsertItem(hTab, 1, &ti);
        TabCtrl_SetCurSel(hTab, 0);

        const int contentY = 150;

        // ---- Card tab controls ----
        CreateWindowW(L"STATIC", L"卡串：", WS_CHILD | WS_VISIBLE, x, contentY, 80, 24, hwnd, nullptr, nullptr, nullptr);
        CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
                       x + 90, contentY, kEditW, 24, hwnd, (HMENU)IDC_CARD_INPUT_EDIT, nullptr, nullptr);

        CreateWindowW(L"STATIC", L"密码：", WS_CHILD | WS_VISIBLE, x, contentY + 32, 80, 24, hwnd, nullptr, nullptr, nullptr);
        CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL | ES_PASSWORD,
                       x + 90, contentY + 32, kEditW, 24, hwnd, (HMENU)IDC_CARD_PWD_EDIT, nullptr, nullptr);

        int yBtn = contentY + 70;
        CreateWindowW(L"BUTTON", L"验证使用", WS_CHILD | WS_VISIBLE, x, yBtn, 140, 30, hwnd, (HMENU)IDC_CARD_VERIFY_BTN, nullptr, nullptr);
        CreateWindowW(L"BUTTON", L"网络测试", WS_CHILD | WS_VISIBLE, x + 150, yBtn, 140, 30, hwnd, (HMENU)IDC_CARD_TESTNET_BTN, nullptr, nullptr);
        CreateWindowW(L"BUTTON", L"版本检测", WS_CHILD | WS_VISIBLE, x + 300, yBtn, 140, 30, hwnd, (HMENU)IDC_CARD_VERSION_BTN, nullptr, nullptr);

        yBtn += 40;
        CreateWindowW(L"BUTTON", L"续费充值", WS_CHILD | WS_VISIBLE, x, yBtn, 140, 30, hwnd, (HMENU)IDC_CARD_RENEW_BTN, nullptr, nullptr);
        CreateWindowW(L"BUTTON", L"购买充值卡", WS_CHILD | WS_VISIBLE, x + 150, yBtn, 140, 30, hwnd, (HMENU)IDC_CARD_BUY_RECHARGE_BTN, nullptr, nullptr);
        CreateWindowW(L"BUTTON", L"购买库存卡", WS_CHILD | WS_VISIBLE, x + 300, yBtn, 140, 30, hwnd, (HMENU)IDC_CARD_BUY_STOCK_BTN, nullptr, nullptr);

        CreateWindowW(L"STATIC", L"待操作", WS_CHILD | WS_VISIBLE, x, yBtn + 45, 560, 24, hwnd, (HMENU)IDC_STATUS_STATIC, nullptr, nullptr);

        // ---- Machine tab controls ----
        // code input
        CreateWindowW(L"STATIC", L"机器码：", WS_CHILD | WS_VISIBLE, x, contentY, 80, 24, hwnd, nullptr, nullptr, nullptr);
        CreateWindowW(L"EDIT", L"", WS_CHILD | ES_AUTOHSCROLL | WS_BORDER,
                       x + 90, contentY, kEditW, 24, hwnd, (HMENU)IDC_MACHINE_CODE_EDIT, nullptr, nullptr);
        // radios
        CreateWindowW(L"BUTTON", L"机器码验证使用", WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON | WS_GROUP,
                       x, contentY + 32, 160, 24, hwnd, (HMENU)IDC_RADIO_MACHINE_VERIFY, nullptr, nullptr);
        CreateWindowW(L"BUTTON", L"充值续费：", WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON,
                       x + 170, contentY + 32, 160, 24, hwnd, (HMENU)IDC_RADIO_MACHINE_RECHARGE, nullptr, nullptr);

        yBtn = contentY + 65;
        CreateWindowW(L"BUTTON", L"验证使用", WS_CHILD | WS_VISIBLE, x, yBtn, 140, 30, hwnd, (HMENU)IDC_MACHINE_VERIFY_BTN, nullptr, nullptr);
        CreateWindowW(L"BUTTON", L"网络测试", WS_CHILD | WS_VISIBLE, x + 150, yBtn, 140, 30, hwnd, (HMENU)IDC_MACHINE_TESTNET_BTN, nullptr, nullptr);
        CreateWindowW(L"BUTTON", L"版本检测", WS_CHILD | WS_VISIBLE, x + 300, yBtn, 140, 30, hwnd, (HMENU)IDC_MACHINE_VERSION_BTN, nullptr, nullptr);

        // recharge group (hidden by default)
        const int yRecharge = contentY + 105;
        CreateWindowW(L"STATIC", L"充值卡号：", WS_CHILD | WS_VISIBLE, x, yRecharge, 80, 24, hwnd, (HMENU)IDC_MACHINE_RECHARGE_CARD_NO_LABEL, nullptr, nullptr);
        CreateWindowW(L"EDIT", L"", WS_CHILD | WS_BORDER | ES_AUTOHSCROLL,
                       x + 90, yRecharge, kEditW, 24, hwnd, (HMENU)IDC_MACHINE_RECHARGE_CARD_NO_EDIT, nullptr, nullptr);

        CreateWindowW(L"STATIC", L"充值密码：", WS_CHILD | WS_VISIBLE, x, yRecharge + 32, 80, 24, hwnd, (HMENU)IDC_MACHINE_RECHARGE_CARD_PWD_LABEL, nullptr, nullptr);
        CreateWindowW(L"EDIT", L"", WS_CHILD | WS_BORDER | ES_AUTOHSCROLL | ES_PASSWORD,
                       x + 90, yRecharge + 32, kEditW, 24, hwnd, (HMENU)IDC_MACHINE_RECHARGE_CARD_PWD_EDIT, nullptr, nullptr);

        int yRechargeBtn = yRecharge + 70;
        CreateWindowW(L"BUTTON", L"确认充值", WS_CHILD | WS_VISIBLE, x, yRechargeBtn, 140, 30, hwnd, (HMENU)IDC_MACHINE_CONFIRM_RECHARGE_BTN, nullptr, nullptr);
        CreateWindowW(L"BUTTON", L"一键支付续费充值", WS_CHILD | WS_VISIBLE, x + 150, yRechargeBtn, 220, 30, hwnd, (HMENU)IDC_MACHINE_ONECLICK_RENEW_BTN, nullptr, nullptr);
        CreateWindowW(L"BUTTON", L"购买充值卡", WS_CHILD | WS_VISIBLE, x, yRechargeBtn + 40, 140, 30, hwnd, (HMENU)IDC_MACHINE_BUY_RECHARGE_BTN, nullptr, nullptr);
        CreateWindowW(L"BUTTON", L"购买库存卡", WS_CHILD | WS_VISIBLE, x + 150, yRechargeBtn + 40, 140, 30, hwnd, (HMENU)IDC_MACHINE_BUY_STOCK_BTN, nullptr, nullptr);

        // Log area
        // NOTE: When "充值续费" sub-mode is enabled, recharge controls extend much lower.
        // Place log below the recharge section to avoid overlap.
        int yLog = yRechargeBtn + 85;
        HWND hLog = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"",
                                     WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_AUTOVSCROLL |
                                         ES_READONLY | WS_VSCROLL,
                                     x, yLog, w, 220, hwnd, (HMENU)IDC_LOG_EDIT, nullptr, nullptr);

        // Initialize default state
        SetWindowTextW(GetDlgItem(hwnd, IDC_MACHINE_CODE_EDIT), WideFromUtf8(st->machine_code).c_str());
        CheckRadioButton(hwnd, IDC_RADIO_MACHINE_VERIFY, IDC_RADIO_MACHINE_RECHARGE, IDC_RADIO_MACHINE_VERIFY);

        // Hide machine tab controls initially
        setVisible(IDC_MACHINE_CODE_EDIT, false);
        setVisible(IDC_RADIO_MACHINE_VERIFY, false);
        setVisible(IDC_RADIO_MACHINE_RECHARGE, false);
        setVisible(IDC_MACHINE_VERIFY_BTN, false);
        setVisible(IDC_MACHINE_TESTNET_BTN, false);
        setVisible(IDC_MACHINE_VERSION_BTN, false);
        setVisible(IDC_MACHINE_RECHARGE_CARD_NO_LABEL, false);
        setVisible(IDC_MACHINE_RECHARGE_CARD_NO_EDIT, false);
        setVisible(IDC_MACHINE_RECHARGE_CARD_PWD_LABEL, false);
        setVisible(IDC_MACHINE_RECHARGE_CARD_PWD_EDIT, false);
        setVisible(IDC_MACHINE_CONFIRM_RECHARGE_BTN, false);
        setVisible(IDC_MACHINE_ONECLICK_RENEW_BTN, false);
        setVisible(IDC_MACHINE_BUY_RECHARGE_BTN, false);
        setVisible(IDC_MACHINE_BUY_STOCK_BTN, false);

        // Recharge group hidden when verify radio selected
        // (even when we show the machine tab later, we re-apply after selection change)
        setVisible(IDC_MACHINE_RECHARGE_CARD_NO_LABEL, false);
        setVisible(IDC_MACHINE_RECHARGE_CARD_NO_EDIT, false);
        setVisible(IDC_MACHINE_RECHARGE_CARD_PWD_LABEL, false);
        setVisible(IDC_MACHINE_RECHARGE_CARD_PWD_EDIT, false);
        setVisible(IDC_MACHINE_CONFIRM_RECHARGE_BTN, false);
        setVisible(IDC_MACHINE_ONECLICK_RENEW_BTN, false);
        setVisible(IDC_MACHINE_BUY_RECHARGE_BTN, false);
        setVisible(IDC_MACHINE_BUY_STOCK_BTN, false);

        st->bootstrapped = st->client.bootstrap();
        if (!st->bootstrapped) {
            AppendLogW(hLog, L"bootstrap failed.");
            setVisible(IDC_CARD_VERIFY_BTN, false);
            setVisible(IDC_CARD_TESTNET_BTN, false);
            setVisible(IDC_CARD_VERSION_BTN, false);
            setVisible(IDC_CARD_RENEW_BTN, false);
            setVisible(IDC_CARD_BUY_RECHARGE_BTN, false);
            setVisible(IDC_CARD_BUY_STOCK_BTN, false);
        } else {
            AppendLogW(hLog, L"bootstrap ok.");
            auto n = st->client.get_notice();
            std::wstring codeW = WideFromUtf8(n.code);
            std::wstring dataW = n.data.empty() ? L"(empty)" : WideFromUtf8(n.data);
            std::wstring line = std::wstring(L"[notice] code=") + codeW + L" data=" + dataW;
            AppendLogW(hLog, line);
            SetWindowTextW(GetDlgItem(hwnd, IDC_NOTICE_EDIT), dataW.c_str());
        }

        return 0;
    }
    case WM_NOTIFY: {
        if (LOWORD(wParam) == IDC_TAB) {
            LPNMHDR hdr = reinterpret_cast<LPNMHDR>(lParam);
            if (hdr && hdr->code == TCN_SELCHANGE) {
                int sel = TabCtrl_GetCurSel(GetDlgItem(hwnd, IDC_TAB));
                auto setVisible = [&](int id, bool visible) {
                    HWND h = GetDlgItem(hwnd, id);
                    if (h) ShowWindow(h, visible ? SW_SHOW : SW_HIDE);
                };
                const bool showCard = (sel == 0);
                // Card group
                setVisible(IDC_CARD_INPUT_EDIT, showCard);
                setVisible(IDC_CARD_PWD_EDIT, showCard);
                setVisible(IDC_CARD_VERIFY_BTN, showCard);
                setVisible(IDC_CARD_TESTNET_BTN, showCard);
                setVisible(IDC_CARD_VERSION_BTN, showCard);
                setVisible(IDC_CARD_RENEW_BTN, showCard);
                setVisible(IDC_CARD_BUY_RECHARGE_BTN, showCard);
                setVisible(IDC_CARD_BUY_STOCK_BTN, showCard);
                // Machine group
                setVisible(IDC_MACHINE_CODE_EDIT, !showCard);
                setVisible(IDC_RADIO_MACHINE_VERIFY, !showCard);
                setVisible(IDC_RADIO_MACHINE_RECHARGE, !showCard);
                bool machineRecharge = (sel == 1) && (IsDlgButtonChecked(hwnd, IDC_RADIO_MACHINE_RECHARGE) == BST_CHECKED);
                setVisible(IDC_MACHINE_VERIFY_BTN, !showCard && !machineRecharge);
                setVisible(IDC_MACHINE_TESTNET_BTN, !showCard && !machineRecharge);
                setVisible(IDC_MACHINE_VERSION_BTN, !showCard && !machineRecharge);
                setVisible(IDC_MACHINE_RECHARGE_CARD_NO_LABEL, !showCard && machineRecharge);
                setVisible(IDC_MACHINE_RECHARGE_CARD_NO_EDIT, !showCard && machineRecharge);
                setVisible(IDC_MACHINE_RECHARGE_CARD_PWD_LABEL, !showCard && machineRecharge);
                setVisible(IDC_MACHINE_RECHARGE_CARD_PWD_EDIT, !showCard && machineRecharge);
                setVisible(IDC_MACHINE_CONFIRM_RECHARGE_BTN, !showCard && machineRecharge);
                setVisible(IDC_MACHINE_ONECLICK_RENEW_BTN, !showCard && machineRecharge);
                setVisible(IDC_MACHINE_BUY_RECHARGE_BTN, !showCard && machineRecharge);
                setVisible(IDC_MACHINE_BUY_STOCK_BTN, !showCard && machineRecharge);
            }
        }
        break;
    }
    case WM_COMMAND: {
        AppState* st = GetState(hwnd);
        if (!st) break;

        auto getEditUtf8 = [&](int id) -> std::string {
            HWND h = GetDlgItem(hwnd, id);
            int len = GetWindowTextLengthW(h);
            std::wstring w;
            w.resize(static_cast<size_t>(len));
            if (len > 0) GetWindowTextW(h, &w[0], len + 1);
            return Utf8FromWide(w.c_str());
        };

        auto setStatus = [&](const std::wstring& s) {
            HWND h = GetDlgItem(hwnd, IDC_STATUS_STATIC);
            if (h) SetWindowTextW(h, s.c_str());
        };

        auto showRechargeGroup = [&](bool visible) {
            // Recharge subtab: hide verify subtab buttons.
            ShowWindow(GetDlgItem(hwnd, IDC_MACHINE_VERIFY_BTN), visible ? SW_HIDE : SW_SHOW);
            ShowWindow(GetDlgItem(hwnd, IDC_MACHINE_TESTNET_BTN), visible ? SW_HIDE : SW_SHOW);
            ShowWindow(GetDlgItem(hwnd, IDC_MACHINE_VERSION_BTN), visible ? SW_HIDE : SW_SHOW);
            ShowWindow(GetDlgItem(hwnd, IDC_MACHINE_RECHARGE_CARD_NO_LABEL), visible ? SW_SHOW : SW_HIDE);
            ShowWindow(GetDlgItem(hwnd, IDC_MACHINE_RECHARGE_CARD_NO_EDIT), visible ? SW_SHOW : SW_HIDE);
            ShowWindow(GetDlgItem(hwnd, IDC_MACHINE_RECHARGE_CARD_PWD_LABEL), visible ? SW_SHOW : SW_HIDE);
            ShowWindow(GetDlgItem(hwnd, IDC_MACHINE_RECHARGE_CARD_PWD_EDIT), visible ? SW_SHOW : SW_HIDE);
            ShowWindow(GetDlgItem(hwnd, IDC_MACHINE_CONFIRM_RECHARGE_BTN), visible ? SW_SHOW : SW_HIDE);
            ShowWindow(GetDlgItem(hwnd, IDC_MACHINE_ONECLICK_RENEW_BTN), visible ? SW_SHOW : SW_HIDE);
            ShowWindow(GetDlgItem(hwnd, IDC_MACHINE_BUY_RECHARGE_BTN), visible ? SW_SHOW : SW_HIDE);
            ShowWindow(GetDlgItem(hwnd, IDC_MACHINE_BUY_STOCK_BTN), visible ? SW_SHOW : SW_HIDE);
        };

        switch (LOWORD(wParam)) {
        case IDC_CARD_VERIFY_BTN: {
            std::string cardid = getEditUtf8(IDC_CARD_INPUT_EDIT);
            std::string cardpwd = getEditUtf8(IDC_CARD_PWD_EDIT);
            if (cardid.empty()) {
                MessageBoxW(hwnd, L"请输入卡串（卡密）。", L"info", MB_OK);
                break;
            }
            auto r = st->client.login_ic(cardid, cardpwd, st->machine_code, st->machine_code);
            HWND hLog = GetDlgItem(hwnd, IDC_LOG_EDIT);
            std::wstring codeW = WideFromUtf8(r.code);
            std::wstring dataW = r.data.empty() ? L"(empty)" : WideFromUtf8(r.data);
            AppendLogW(hLog, std::wstring(L"[login_ic] code=") + codeW + L" data=" + dataW);
            if (LoginIcSuccess(r) || !st->client.BSphpSeSsL.empty()) {
                st->logged_card_id = cardid;
                st->logged_card_pwd = cardpwd;
                auto expR = st->client.api_call("getdate.ic");
                std::wstring expCodeW = WideFromUtf8(expR.code);
                std::wstring expDataW = expR.data.empty() ? L"-" : WideFromUtf8(expR.data);
                AppendLogW(hLog, std::wstring(L"[getdate.ic] code=") + expCodeW + L" data=" + expDataW);
                st->vip_expiry = expR.data;
                setStatus(std::wstring(L"验证成功（卡密登陆模式），VIP到期：") + expDataW);
                OpenPanelWindow(st);
            } else {
                setStatus(L"验证失败（卡密登陆模式）。");
            }
            break;
        }
        case IDC_CARD_TESTNET_BTN: {
            auto r = st->client.connect();
            HWND hLog = GetDlgItem(hwnd, IDC_LOG_EDIT);
            std::wstring codeW = WideFromUtf8(r.code);
            std::wstring dataW = r.data.empty() ? L"(empty)" : WideFromUtf8(r.data);
            AppendLogW(hLog, std::wstring(L"[connect] code=") + codeW + L" data=" + dataW);
            setStatus(dataW == L"1" ? L"网络连接正常。" : L"网络连接异常。");
            break;
        }
        case IDC_CARD_VERSION_BTN: {
            auto r = st->client.get_version();
            HWND hLog = GetDlgItem(hwnd, IDC_LOG_EDIT);
            std::wstring codeW = WideFromUtf8(r.code);
            std::wstring dataW = r.data.empty() ? L"(empty)" : WideFromUtf8(r.data);
            AppendLogW(hLog, std::wstring(L"[get_version] code=") + codeW + L" data=" + dataW);
            setStatus(L"已获取版本（见日志）。");
            break;
        }
        case IDC_CARD_RENEW_BTN: {
            std::string user = getEditUtf8(IDC_CARD_INPUT_EDIT);
            if (user.empty()) {
                MessageBoxW(hwnd, L"请输入卡串（用于续费user参数）。", L"info", MB_OK);
                break;
            }
            OpenUrlW(bsphp_card_demo::RenewSaleUrlW(WideFromUtf8(user)));
            setStatus(L"打开续费充值页面。");
            break;
        }
        case IDC_CARD_BUY_RECHARGE_BTN: {
            OpenUrlW(bsphp_card_demo::GenCardSaleUrlW());
            setStatus(L"打开购买充值卡页面。");
            break;
        }
        case IDC_CARD_BUY_STOCK_BTN: {
            OpenUrlW(bsphp_card_demo::StockSaleUrlW());
            setStatus(L"打开购买库存卡页面。");
            break;
        }
        case IDC_MACHINE_VERIFY_BTN: {
            std::string id = getEditUtf8(IDC_MACHINE_CODE_EDIT);
            if (id.empty()) {
                setStatus(L"请输入机器码（账号）。");
                break;
            }
            auto feat = st->client.api_call("AddCardFeatures.key.ic",
                                             {{"carid", id},
                                              {"key", st->machine_code},
                                              {"maxoror", st->machine_code}});
            HWND hLog = GetDlgItem(hwnd, IDC_LOG_EDIT);
            AppendLogW(hLog, std::wstring(L"[AddCardFeatures.key.ic] code=") + WideFromUtf8(feat.code) + L" data=" +
                               (feat.data.empty() ? L"(empty)" : WideFromUtf8(feat.data)));

            bool featOk = (feat.code == "1011" || feat.code == "1081") ||
                           (feat.data.find("1081") != std::string::npos) ||
                           (feat.data.find("成功") != std::string::npos);
            if (!featOk) {
                setStatus(L"机器码特征添加失败（见日志）。");
                break;
            }

            auto r = st->client.login_ic(id, "", st->machine_code, st->machine_code);
            AppendLogW(hLog, std::wstring(L"[login_ic] code=") + WideFromUtf8(r.code) + L" data=" +
                               (r.data.empty() ? L"(empty)" : WideFromUtf8(r.data)));

            if (LoginIcSuccess(r) || !st->client.BSphpSeSsL.empty()) {
                st->logged_card_id = id;
                st->logged_card_pwd.clear();
                auto expR = st->client.api_call("getdate.ic");
                AppendLogW(hLog, std::wstring(L"[getdate.ic] code=") + WideFromUtf8(expR.code) + L" data=" +
                                   (expR.data.empty() ? L"-" : WideFromUtf8(expR.data)));
                st->vip_expiry = expR.data;
                setStatus(L"验证成功（机器码账号）。");
                OpenPanelWindow(st);
            } else {
                setStatus(L"机器码登录失败（见日志）。");
            }
            break;
        }
        case IDC_MACHINE_TESTNET_BTN: {
            auto r = st->client.connect();
            HWND hLog = GetDlgItem(hwnd, IDC_LOG_EDIT);
            AppendLogW(hLog, std::wstring(L"[connect] code=") + WideFromUtf8(r.code) + L" data=" +
                               (r.data.empty() ? L"(empty)" : WideFromUtf8(r.data)));
            setStatus(L"网络测试完成。");
            break;
        }
        case IDC_MACHINE_VERSION_BTN: {
            auto r = st->client.get_version();
            HWND hLog = GetDlgItem(hwnd, IDC_LOG_EDIT);
            AppendLogW(hLog, std::wstring(L"[get_version] code=") + WideFromUtf8(r.code) + L" data=" +
                               (r.data.empty() ? L"(empty)" : WideFromUtf8(r.data)));
            setStatus(L"版本检测完成。");
            break;
        }
        case IDC_MACHINE_CONFIRM_RECHARGE_BTN: {
            std::string icid = getEditUtf8(IDC_MACHINE_CODE_EDIT);
            std::string ka = getEditUtf8(IDC_MACHINE_RECHARGE_CARD_NO_EDIT);
            std::string pwd = getEditUtf8(IDC_MACHINE_RECHARGE_CARD_PWD_EDIT);
            if (icid.empty() || ka.empty()) {
                setStatus(L"请输入机器码账号和充值卡号。");
                break;
            }
            auto r = st->client.api_call("chong.ic",
                                          {{"icid", icid},
                                           {"ka", ka},
                                           {"pwd", pwd}});
            HWND hLog = GetDlgItem(hwnd, IDC_LOG_EDIT);
            AppendLogW(hLog, std::wstring(L"[chong.ic] code=") + WideFromUtf8(r.code) + L" data=" +
                               (r.data.empty() ? L"(empty)" : WideFromUtf8(r.data)));
            setStatus(L"确认充值完成（见日志）。");
            break;
        }
        case IDC_MACHINE_ONECLICK_RENEW_BTN: {
            std::string user = getEditUtf8(IDC_MACHINE_CODE_EDIT);
            if (user.empty()) {
                MessageBoxW(hwnd, L"请输入机器码（账号）用于续费。", L"info", MB_OK);
                break;
            }
            OpenUrlW(bsphp_card_demo::RenewSaleUrlW(WideFromUtf8(user)));
            setStatus(L"打开一键支付续费页面。");
            break;
        }
        case IDC_MACHINE_BUY_RECHARGE_BTN: {
            OpenUrlW(bsphp_card_demo::GenCardSaleUrlW());
            setStatus(L"打开购买充值卡页面。");
            break;
        }
        case IDC_MACHINE_BUY_STOCK_BTN: {
            OpenUrlW(bsphp_card_demo::StockSaleUrlW());
            setStatus(L"打开购买库存卡页面。");
            break;
        }
        case IDC_RADIO_MACHINE_VERIFY: {
            showRechargeGroup(false);
            break;
        }
        case IDC_RADIO_MACHINE_RECHARGE: {
            showRechargeGroup(true);
            break;
        }
        default:
            break;
        }

        return 0;
    }
    case WM_DESTROY: {
        AppState* st = GetState(hwnd);
        if (st) {
            if (st->panel_hwnd && IsWindow(st->panel_hwnd)) {
                DestroyWindow(st->panel_hwnd);
                st->panel_hwnd = nullptr;
            }
            delete st;
        }
        SetState(hwnd, nullptr);
        PostQuitMessage(0);
        return 0;
    }
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

} // namespace

int WINAPI wWinMain(HINSTANCE hInst, HINSTANCE, PWSTR, int nCmdShow) {
    const wchar_t* kClassName = L"BSPHPWinCardDemo";

    INITCOMMONCONTROLSEX icc{};
    icc.dwSize = sizeof(icc);
    icc.dwICC = ICC_TAB_CLASSES;
    InitCommonControlsEx(&icc);

    WNDCLASSW wc{};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInst;
    wc.lpszClassName = kClassName;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    RegisterClassW(&wc);

    HWND hwnd = CreateWindowW(kClassName, L"BSPHP Win32 Card Demo (no MFC)",
                               WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
                               720, 760, nullptr, nullptr, hInst, nullptr);
    if (!hwnd) return 0;

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessageW(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    return static_cast<int>(msg.wParam);
}

