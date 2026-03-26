#include <windows.h>
#include <shellapi.h>
#include <commctrl.h>

#include <winhttp.h>
#include <gdiplus.h>

#pragma comment(lib, "winhttp.lib")
#pragma comment(lib, "gdiplus.lib")

#include <cstring>

#include <string>
#include <map>
#include <sstream>
#include <vector>
#include <ctime>

#include "core/bsphp_client.h"
#include "core/login_demo_config.h"
#include "core/machine_id.h"
#include "core/string_utf8.h"

namespace {

static ULONG_PTR g_gdiplusToken = 0;

enum class TabIndex : int {
    Login = 0,
    SmsLogin,
    EmailLogin,
    Register,
    SmsRegister,
    EmailRegister,
    Unbind,
    Recharge,
    SmsRecover,
    EmailRecover,
    RecoverPassword,
    ChangePassword,
    Feedback,
    Count
};

enum ControlId {
    IDC_TAB = 3001,
    IDC_NOTICE_EDIT = 3002,
    IDC_LOG_EDIT = 3003,

    // ---- Common buttons (shown on some tabs) ----
    IDC_BTN_TESTNET = 4001,
    IDC_BTN_VERSION = 4002,
    IDC_BTN_ENDTIME = 4003,
    IDC_BTN_WEBLOGIN = 4004,
    IDC_BTN_LOGOUT = 4005,

    // ---- Login (账号密码) ----
    IDC_LOGIN_USER = 1001,
    IDC_LOGIN_PWD = 1002,
    IDC_LOGIN_CODE_LABEL = 1003,
    IDC_LOGIN_CODE = 1004,
    IDC_LOGIN_CODE_REFRESH = 1005,
    IDC_LOGIN_CODE_PIC = 1007,
    IDC_LOGIN_SUBMIT = 1006,

    // ---- Register (账号密码注册) ----
    IDC_REG_USER = 1101,
    IDC_REG_PWD = 1102,
    IDC_REG_PWD2 = 1103,
    IDC_REG_QQ = 1104,
    IDC_REG_MAIL = 1105,
    IDC_REG_MOBILE = 1106,
    IDC_REG_QUESTION = 1107,
    IDC_REG_ANSWER = 1108,
    IDC_REG_CODE_LABEL = 1109,
    IDC_REG_CODE = 1110,
    IDC_REG_CODE_REFRESH = 1111,
    IDC_REG_CODE_PIC = 1114,
    IDC_REG_EXTENSION = 1112,
    IDC_REG_SUBMIT = 1113,

    // ---- Unbind ----
    IDC_UNBIND_USER = 1201,
    IDC_UNBIND_PWD = 1202,
    IDC_UNBIND_SUBMIT = 1203,

    // ---- Recharge ----
    IDC_PAY_USER = 1301,
    IDC_PAY_USERPWD = 1302,
    IDC_PAY_USERSET = 1303, // checkbox
    IDC_PAY_KA = 1304,
    IDC_PAY_PWD = 1305,
    IDC_PAY_SUBMIT = 1306,

    // ---- RecoverPassword (密保找回) ----
    IDC_BACK_USER = 1401,
    IDC_BACK_QUESTION = 1402,
    IDC_BACK_ANSWER = 1403,
    IDC_BACK_PWD = 1404,
    IDC_BACK_PWD2 = 1405,
    IDC_BACK_CODE_LABEL = 1406,
    IDC_BACK_CODE = 1407,
    IDC_BACK_CODE_REFRESH = 1408,
    IDC_BACK_CODE_PIC = 1410,
    IDC_BACK_SUBMIT = 1409,

    // ---- ChangePassword ----
    IDC_CHG_USER = 1501,
    IDC_CHG_OLD = 1502,
    IDC_CHG_NEW = 1503,
    IDC_CHG_NEW2 = 1504,
    IDC_CHG_IMG = 1505,
    IDC_CHG_SUBMIT = 1506,

    // ---- Feedback ----
    IDC_FB_USER = 1601,
    IDC_FB_PWD = 1602,
    IDC_FB_TITLE = 1603,
    IDC_FB_CONTACT = 1604,
    IDC_FB_TYPE = 1605,
    IDC_FB_TEXT = 1606, // multiline
    IDC_FB_CODE_LABEL = 1607,
    IDC_FB_CODE = 1608,
    IDC_FB_CODE_REFRESH = 1609,
    IDC_FB_CODE_PIC = 1611,
    IDC_FB_SUBMIT = 1610,

    // ---- SMS common ----
    IDC_SMS_MOBILE = 1701,
    IDC_SMS_AREA = 1702,
    IDC_SMS_CODE = 1703,
    IDC_SMS_KEY_LABEL = 17031,
    IDC_SMS_KEY = 1704,
    IDC_SMS_MAXOROR_LABEL = 17032,
    IDC_SMS_MAXOROR = 1705,
    IDC_SMS_USER_LABEL = 17033,
    IDC_SMS_IMG = 1706,
    IDC_SMS_IMG_REFRESH = 1707,
    IDC_SMS_SEND = 1708,
    IDC_SMS_SUBMIT = 1709,
    IDC_SMS_IMG_PIC = 1713,
    IDC_SMS_USER = 1710,
    IDC_SMS_PWD_LABEL = 17034,
    IDC_SMS_PWD = 1711,
    IDC_SMS_PWD2 = 1712,

    // ---- Email common ----
    IDC_EMAIL_ADDR = 1801,
    IDC_EMAIL_CODE = 1802,
    IDC_EMAIL_KEY_LABEL = 18031,
    IDC_EMAIL_KEY = 1803,
    IDC_EMAIL_MAXOROR_LABEL = 18032,
    IDC_EMAIL_MAXOROR = 1804,
    IDC_EMAIL_IMG = 1805,
    IDC_EMAIL_IMG_REFRESH = 1806,
    IDC_EMAIL_SEND = 1807,
    IDC_EMAIL_SUBMIT = 1808,
    IDC_EMAIL_USER_LABEL = 18033,
    IDC_EMAIL_USER = 1809,
    IDC_EMAIL_PWD_LABEL = 18034,
    IDC_EMAIL_PWD = 1810,
    IDC_EMAIL_PWD2 = 1811,
    IDC_EMAIL_IMG_PIC = 1812,
};

enum ConsoleControlId {
    IDC_CONSOLE_LOG_EDIT = 5001,
    IDC_CONSOLE_BTN_NOTICE = 5002,
    IDC_CONSOLE_BTN_ENDTIME = 5003,
    IDC_CONSOLE_BTN_VERSION = 5004,
    IDC_CONSOLE_BTN_HEARTBEAT = 5005,
    IDC_CONSOLE_BTN_LOGOUT = 5006,
    IDC_CONSOLE_INFO_EDIT = 5007,
    IDC_CONSOLE_BTN_USERINFO = 5008,
    IDC_CONSOLE_BTN_RENEW = 5009,
    IDC_CONSOLE_BTN_BUY = 5010,
    IDC_CONSOLE_BTN_STOCK = 5011,
    IDC_CONSOLE_BTN_WEBLOGIN = 5012,
    IDC_CONSOLE_BTN_COPY_MACHINE = 5013,
    IDC_CONSOLE_BTN_COPY_SESSION = 5014,
    IDC_CONSOLE_CHK_AUTOHB = 5015,
    IDC_CONSOLE_EDIT_INTERVAL = 5016,
    IDC_CONSOLE_BTN_APPCUSTOM_APP = 5017,
    IDC_CONSOLE_BTN_APPCUSTOM_VIP = 5018,
    IDC_CONSOLE_BTN_APPCUSTOM_LOGIN = 5019,
};

struct AppState {
    bsphp::BsPhp client = bsphp_login_demo::MakeDemoClient();
    std::string machine_code = BsphpDemoMachineCodeUtf8();
    bool code_login = true;
    bool code_reg = true;
    bool code_back = true;
    bool code_say = true;
    bool logged_in = false;

    // Tab page containers (children of main window). Hiding a page hides all its controls (incl. static labels).
    HWND page_login = nullptr;
    HWND page_sms = nullptr;
    HWND page_email = nullptr;
    HWND page_register = nullptr;
    HWND page_unbind = nullptr;
    HWND page_recharge = nullptr;
    HWND page_recover = nullptr;
    HWND page_change = nullptr;
    HWND page_feedback = nullptr;

    // Captcha cached bitmaps so we can replace old ones safely.
    std::map<int, HBITMAP> captchaBitmaps;

    // Console window handle (opened after successful login).
    HWND console_hwnd = nullptr;
};

static AppState* GetState(HWND hwnd) {
    return reinterpret_cast<AppState*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
}
static void SetState(HWND hwnd, AppState* st) {
    SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(st));
}

static LRESULT CALLBACK PageForwardSubclassProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam,
                                                UINT_PTR uIdSubclass, DWORD_PTR dwRefData) {
    (void)uIdSubclass;
    (void)dwRefData;
    if (uMsg == WM_COMMAND || uMsg == WM_NOTIFY) {
        HWND parent = GetParent(hWnd);
        if (parent) {
            SendMessageW(parent, uMsg, wParam, lParam);
            return 0;
        }
    }
    return DefSubclassProc(hWnd, uMsg, wParam, lParam);
}

static void SubclassPageForForwarding(HWND page) {
    if (!page) return;
    SetWindowSubclass(page, PageForwardSubclassProc, 1, 0);
}

static HWND GetAnyDlgItem(HWND mainHwnd, int id) {
    if (!mainHwnd) return nullptr;
    if (HWND h = GetDlgItem(mainHwnd, id)) return h;
    auto* st = GetState(mainHwnd);
    if (!st) return nullptr;
    HWND pages[] = {st->page_login,   st->page_sms,     st->page_email,  st->page_register, st->page_unbind,
                    st->page_recharge, st->page_recover, st->page_change, st->page_feedback};
    for (HWND p : pages) {
        if (!p) continue;
        if (HWND h = GetDlgItem(p, id)) return h;
    }
    return nullptr;
}

static std::wstring W(const std::string& s) { return WideFromUtf8(s); }

static void AppendLogW(HWND hEdit, const std::wstring& line) {
    if (!hEdit) return;
    int len = GetWindowTextLengthW(hEdit);
    std::wstring prefix;
    prefix.resize(static_cast<size_t>(len));
    if (len > 0) GetWindowTextW(hEdit, &prefix[0], len + 1);
    std::wstring next = prefix;
    next += line;
    next += L"\r\n";
    SetWindowTextW(hEdit, next.c_str());
    SendMessageW(hEdit, EM_SCROLLCARET, 0, 0);
}

static std::string Utf8FromEdit(HWND hwnd, int id) {
    HWND h = GetAnyDlgItem(hwnd, id);
    if (!h) return {};
    int len = GetWindowTextLengthW(h);
    std::wstring w;
    w.resize(static_cast<size_t>(len));
    if (len > 0) GetWindowTextW(h, &w[0], len + 1);
    return Utf8FromWide(w.c_str());
}

static void SetEditW(HWND hwnd, int id, const std::wstring& text) {
    HWND h = GetAnyDlgItem(hwnd, id);
    if (h) SetWindowTextW(h, text.c_str());
}

static void OpenUrlW(const std::wstring& url) {
    if (url.empty()) return;
    ShellExecuteW(nullptr, L"open", url.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
}

static std::wstring MakeCaptchaUrl(const bsphp::BsPhp& client) {
    if (client.code_url.empty() || client.BSphpSeSsL.empty()) return {};
    const long long tick = static_cast<long long>(time(nullptr));
    std::string url = client.code_url + client.BSphpSeSsL + "&_=" + std::to_string(tick);
    return WideFromUtf8(url);
}

static bool DownloadUrlToBytesWinHttp(const std::wstring& url, std::vector<unsigned char>& out) {
    out.clear();

    URL_COMPONENTSW uc{};
    uc.dwStructSize = sizeof(uc);
    wchar_t scheme[16] = {};
    wchar_t hostName[256] = {};
    wchar_t urlPath[2048] = {};

    uc.lpszScheme = scheme;
    uc.dwSchemeLength = static_cast<DWORD>(_countof(scheme));
    uc.lpszHostName = hostName;
    uc.dwHostNameLength = static_cast<DWORD>(_countof(hostName));
    uc.lpszUrlPath = urlPath;
    uc.dwUrlPathLength = static_cast<DWORD>(_countof(urlPath));

    if (!WinHttpCrackUrl(url.c_str(), 0, 0, &uc)) return false;

    const bool isHttps = (uc.nScheme == INTERNET_SCHEME_HTTPS);

    HINTERNET hSession = WinHttpOpen(L"BSPHPWin32", WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
                                     WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
    if (!hSession) return false;

    HINTERNET hConnect = WinHttpConnect(hSession, hostName, uc.nPort, 0);
    if (!hConnect) {
        WinHttpCloseHandle(hSession);
        return false;
    }

    DWORD flags = isHttps ? WINHTTP_FLAG_SECURE : 0;
    HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"GET", urlPath, nullptr, WINHTTP_NO_REFERER,
                                             WINHTTP_DEFAULT_ACCEPT_TYPES, flags);
    if (!hRequest) {
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return false;
    }

    BOOL ok = WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, nullptr, 0, 0, 0);
    if (!ok) {
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return false;
    }

    ok = WinHttpReceiveResponse(hRequest, nullptr);
    if (!ok) {
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return false;
    }

    DWORD dwSize = 0;
    while (WinHttpQueryDataAvailable(hRequest, &dwSize) && dwSize) {
        std::vector<unsigned char> buf(dwSize);
        DWORD dwDownloaded = 0;
        if (!WinHttpReadData(hRequest, buf.data(), dwSize, &dwDownloaded)) break;
        if (dwDownloaded) {
            out.insert(out.end(), buf.begin(), buf.begin() + dwDownloaded);
        }
    }

    WinHttpCloseHandle(hRequest);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);
    return !out.empty();
}

static void UpdateCaptchaPicture(HWND hwnd, AppState* st, int picId, const std::wstring& url) {
    if (!st) return;
    if (url.empty()) return;
    HWND hPic = GetAnyDlgItem(hwnd, picId);
    if (!hPic) return;

    const wchar_t* kFailText = L"验证码加载失败";

    std::vector<unsigned char> bytes;
    if (!DownloadUrlToBytesWinHttp(url, bytes)) {
        SetWindowTextW(hPic, kFailText);
        return;
    }

    HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, bytes.size());
    if (!hMem) return;
    void* p = GlobalLock(hMem);
    if (!p) {
        GlobalFree(hMem);
        SetWindowTextW(hPic, kFailText);
        return;
    }
    memcpy(p, bytes.data(), bytes.size());
    GlobalUnlock(hMem);

    IStream* stream = nullptr;
    if (CreateStreamOnHGlobal(hMem, TRUE, &stream) != S_OK) {
        GlobalFree(hMem);
        SetWindowTextW(hPic, kFailText);
        return;
    }

    Gdiplus::Bitmap src(stream);
    if (src.GetLastStatus() != Gdiplus::Ok || src.GetWidth() == 0 || src.GetHeight() == 0) {
        stream->Release();
        SetWindowTextW(hPic, kFailText);
        return;
    }

    RECT rc{};
    GetClientRect(hPic, &rc);
    const int w = rc.right - rc.left;
    const int h = rc.bottom - rc.top;
    if (w <= 0 || h <= 0) {
        stream->Release();
        SetWindowTextW(hPic, kFailText);
        return;
    }

    Gdiplus::Bitmap dest(w, h, PixelFormat32bppARGB);
    Gdiplus::Graphics g(&dest);
    g.SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);
    g.Clear(Gdiplus::Color(0, 0, 0, 0));
    g.DrawImage(&src, 0, 0, w, h);

    HBITMAP hbmp = nullptr;
    dest.GetHBITMAP(Gdiplus::Color(0, 0, 0, 0), &hbmp);
    stream->Release();
    if (!hbmp) {
        SetWindowTextW(hPic, kFailText);
        return;
    }

    auto it = st->captchaBitmaps.find(picId);
    if (it != st->captchaBitmaps.end()) {
        if (it->second) DeleteObject(it->second);
        it->second = hbmp;
    } else {
        st->captchaBitmaps[picId] = hbmp;
    }

    SendMessageW(hPic, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hbmp);
}

static bool CodeEnabledFromApiData(const std::string& data) {
    if (data.empty()) return true;
    std::string s = data;
    for (char& c : s) c = static_cast<char>(tolower(static_cast<unsigned char>(c)));
    return s.find("checked") != std::string::npos || s == "true" || s == "1";
}

static void ShowMany(HWND hwnd, const std::vector<int>& ids, bool show) {
    for (int id : ids) {
        HWND h = GetAnyDlgItem(hwnd, id);
        if (h) ShowWindow(h, show ? SW_SHOW : SW_HIDE);
    }
}

static void EnableMany(HWND hwnd, const std::vector<int>& ids, bool en) {
    for (int id : ids) {
        HWND h = GetAnyDlgItem(hwnd, id);
        if (h) EnableWindow(h, en ? TRUE : FALSE);
    }
}

static void ApplyTabVisibility(HWND hwnd) {
    auto* st = GetState(hwnd);
    if (!st) return;

    const int sel = TabCtrl_GetCurSel(GetDlgItem(hwnd, IDC_TAB));
    const TabIndex tab = static_cast<TabIndex>(sel);

    // Hide all pages first.
    auto hidePage = [](HWND h) { if (h) ShowWindow(h, SW_HIDE); };
    hidePage(st->page_login);
    hidePage(st->page_sms);
    hidePage(st->page_email);
    hidePage(st->page_register);
    hidePage(st->page_unbind);
    hidePage(st->page_recharge);
    hidePage(st->page_recover);
    hidePage(st->page_change);
    hidePage(st->page_feedback);

    // Common buttons
    ShowMany(hwnd, {IDC_BTN_TESTNET, IDC_BTN_VERSION, IDC_BTN_ENDTIME, IDC_BTN_WEBLOGIN}, true);
    // Logout button: hide on Login tab; only show after successful login.
    ShowMany(hwnd, {IDC_BTN_LOGOUT}, (st->logged_in && tab != TabIndex::Login));

    // Show selected page.
    auto showPage = [](HWND h) { if (h) ShowWindow(h, SW_SHOW); };
    switch (tab) {
    case TabIndex::Login: showPage(st->page_login); break;
    case TabIndex::SmsLogin:
    case TabIndex::SmsRegister:
    case TabIndex::SmsRecover:
        showPage(st->page_sms);
        break;
    case TabIndex::EmailLogin:
    case TabIndex::EmailRegister:
    case TabIndex::EmailRecover:
        showPage(st->page_email);
        break;
    case TabIndex::Register: showPage(st->page_register); break;
    case TabIndex::Unbind: showPage(st->page_unbind); break;
    case TabIndex::Recharge: showPage(st->page_recharge); break;
    case TabIndex::RecoverPassword: showPage(st->page_recover); break;
    case TabIndex::ChangePassword: showPage(st->page_change); break;
    case TabIndex::Feedback: showPage(st->page_feedback); break;
    default:
        showPage(st->page_login);
        break;
    }

    // Mode-specific visibility inside the shared SMS/Email pages.
    if (tab == TabIndex::SmsLogin) {
        ShowMany(hwnd, {IDC_SMS_KEY_LABEL, IDC_SMS_KEY, IDC_SMS_MAXOROR_LABEL, IDC_SMS_MAXOROR}, true);
        ShowMany(hwnd, {IDC_SMS_USER, IDC_SMS_PWD, IDC_SMS_PWD2}, false);
        ShowMany(hwnd, {IDC_SMS_USER_LABEL, IDC_SMS_PWD_LABEL}, false);
    } else if (tab == TabIndex::SmsRegister) {
        ShowMany(hwnd, {IDC_SMS_KEY_LABEL, IDC_SMS_KEY}, true);
        ShowMany(hwnd, {IDC_SMS_USER, IDC_SMS_PWD, IDC_SMS_PWD2}, true);
        ShowMany(hwnd, {IDC_SMS_USER_LABEL, IDC_SMS_PWD_LABEL}, true);
        ShowMany(hwnd, {IDC_SMS_MAXOROR_LABEL, IDC_SMS_MAXOROR}, false);
    } else if (tab == TabIndex::SmsRecover) {
        ShowMany(hwnd, {IDC_SMS_KEY_LABEL, IDC_SMS_KEY, IDC_SMS_MAXOROR_LABEL, IDC_SMS_MAXOROR, IDC_SMS_USER_LABEL, IDC_SMS_USER}, false);
        ShowMany(hwnd, {IDC_SMS_PWD, IDC_SMS_PWD2}, true);
        ShowMany(hwnd, {IDC_SMS_PWD_LABEL}, true);
    }

    if (tab == TabIndex::EmailLogin) {
        ShowMany(hwnd, {IDC_EMAIL_KEY_LABEL, IDC_EMAIL_KEY, IDC_EMAIL_MAXOROR_LABEL, IDC_EMAIL_MAXOROR}, true);
        ShowMany(hwnd, {IDC_EMAIL_USER, IDC_EMAIL_PWD, IDC_EMAIL_PWD2}, false);
        ShowMany(hwnd, {IDC_EMAIL_USER_LABEL, IDC_EMAIL_PWD_LABEL}, false);
    } else if (tab == TabIndex::EmailRegister) {
        ShowMany(hwnd, {IDC_EMAIL_KEY_LABEL, IDC_EMAIL_KEY}, true);
        ShowMany(hwnd, {IDC_EMAIL_USER, IDC_EMAIL_PWD, IDC_EMAIL_PWD2}, true);
        ShowMany(hwnd, {IDC_EMAIL_USER_LABEL, IDC_EMAIL_PWD_LABEL}, true);
        ShowMany(hwnd, {IDC_EMAIL_MAXOROR_LABEL, IDC_EMAIL_MAXOROR}, false);
    } else if (tab == TabIndex::EmailRecover) {
        ShowMany(hwnd, {IDC_EMAIL_KEY_LABEL, IDC_EMAIL_KEY, IDC_EMAIL_MAXOROR_LABEL, IDC_EMAIL_MAXOROR, IDC_EMAIL_USER_LABEL, IDC_EMAIL_USER}, false);
        ShowMany(hwnd, {IDC_EMAIL_PWD, IDC_EMAIL_PWD2}, true);
        ShowMany(hwnd, {IDC_EMAIL_PWD_LABEL}, true);
    }
}

static void InitCodeSwitches(HWND hwnd, AppState* st) {
    HWND hLog = GetAnyDlgItem(hwnd, IDC_LOG_EDIT);
    auto all = st->client.get_code_enabled("INGES_LOGIN|INGES_RE|INGES_MACK|INGES_SAY");
    AppendLogW(hLog, std::wstring(L"[get_code_enabled all] code=") + W(all.code) + L" data=" + (all.data.empty() ? L"(empty)" : W(all.data)));
    // data like: checked|checked|checked|checked (order: login|reg|back|say)
    std::string s = all.data;
    std::vector<std::string> parts;
    std::string cur;
    for (char c : s) {
        if (c == '|') { parts.push_back(cur); cur.clear(); }
        else cur.push_back(c);
    }
    parts.push_back(cur);
    if (parts.size() >= 1) st->code_login = CodeEnabledFromApiData(parts[0]);
    if (parts.size() >= 2) st->code_reg = CodeEnabledFromApiData(parts[1]);
    if (parts.size() >= 3) st->code_back = CodeEnabledFromApiData(parts[2]);
    if (parts.size() >= 4) st->code_say = CodeEnabledFromApiData(parts[3]);

    // Apply show/hide for image captcha fields that are fixed to each tab.
    ShowMany(hwnd, {IDC_LOGIN_CODE_LABEL, IDC_LOGIN_CODE, IDC_LOGIN_CODE_REFRESH, IDC_LOGIN_CODE_PIC}, st->code_login);
    ShowMany(hwnd, {IDC_REG_CODE_LABEL, IDC_REG_CODE, IDC_REG_CODE_REFRESH, IDC_REG_CODE_PIC}, st->code_reg);
    ShowMany(hwnd, {IDC_BACK_CODE_LABEL, IDC_BACK_CODE, IDC_BACK_CODE_REFRESH, IDC_BACK_CODE_PIC}, st->code_back);
    ShowMany(hwnd, {IDC_FB_CODE_LABEL, IDC_FB_CODE, IDC_FB_CODE_REFRESH, IDC_FB_CODE_PIC}, st->code_say);
}

static void AppendApi(HWND hwnd, const wchar_t* title, const bsphp::ApiResponse& r) {
    HWND hLog = GetAnyDlgItem(hwnd, IDC_LOG_EDIT);
    std::wstring codeW = W(r.code);
    std::wstring dataW = r.data.empty() ? L"(empty)" : W(r.data);
    AppendLogW(hLog, std::wstring(L"[") + title + L"] code=" + codeW + L" data=" + dataW);
}

static bsphp::ApiResponse ApiCall(bsphp::BsPhp& c, const std::string& api, const std::map<std::string, std::string>& p) {
    return c.api_call(api, p);
}

struct ConsoleState {
    AppState* mainSt = nullptr;
    std::string lastUserInfoData;
    UINT_PTR hbTimerId = 0;
};

static void AppendConsoleApi(HWND hwnd, const wchar_t* title, const bsphp::ApiResponse& r) {
    HWND hLog = GetDlgItem(hwnd, IDC_CONSOLE_LOG_EDIT);
    const std::wstring codeW = W(r.code);
    const std::wstring dataW = r.data.empty() ? L"(empty)" : W(r.data);
    AppendLogW(hLog, std::wstring(L"[") + title + L"] code=" + codeW + L" data=" + dataW);
}

static void RefreshConsoleHeaderInfo(HWND hwnd, ConsoleState* cs) {
    if (!cs || !cs->mainSt) return;
    auto* ms = cs->mainSt;
    std::wstringstream ss;
    ss << L"machine_code: " << W(ms->machine_code) << L"\r\n";
    ss << L"BSphpSeSsL: " << (ms->client.BSphpSeSsL.empty() ? L"(empty)" : W(ms->client.BSphpSeSsL)) << L"\r\n";
    ss << L"logged_in: " << (ms->logged_in ? L"true" : L"false") << L"\r\n";
    if (!cs->lastUserInfoData.empty()) {
        ss << L"getuserinfo.lg data: " << W(cs->lastUserInfoData) << L"\r\n";
    }
    SetWindowTextW(GetDlgItem(hwnd, IDC_CONSOLE_INFO_EDIT), ss.str().c_str());
}

static void SetClipboardTextW(HWND hwnd, const std::wstring& text) {
    if (!OpenClipboard(hwnd)) return;
    EmptyClipboard();
    const size_t bytes = (text.size() + 1) * sizeof(wchar_t);
    HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, bytes);
    if (hMem) {
        void* p = GlobalLock(hMem);
        if (p) {
            memcpy(p, text.c_str(), bytes);
            GlobalUnlock(hMem);
            SetClipboardData(CF_UNICODETEXT, hMem);
        } else {
            GlobalFree(hMem);
        }
    }
    CloseClipboard();
}

static int IntFromEditOr(HWND hwnd, int id, int fallback) {
    wchar_t buf[64] = {};
    GetWindowTextW(GetDlgItem(hwnd, id), buf, (int)_countof(buf));
    int v = _wtoi(buf);
    return v > 0 ? v : fallback;
}

LRESULT CALLBACK ConsoleWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE: {
        auto* cs = new ConsoleState();
        auto* mainSt = reinterpret_cast<AppState*>((reinterpret_cast<CREATESTRUCTW*>(lParam))->lpCreateParams);
        cs->mainSt = mainSt;
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(cs));

        const int x = 16;
        const int w = 712;
        const int hLogH = 275;

        CreateWindowW(L"STATIC", L"登录后控制台（参考 Mac）", WS_CHILD | WS_VISIBLE, x, 10, 360, 22, hwnd, nullptr, nullptr, nullptr);

        // Header info (machine_code/session/userinfo)
        CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"",
                         WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY | WS_VSCROLL,
                         x, 35, w, 90, hwnd, (HMENU)IDC_CONSOLE_INFO_EDIT, nullptr, nullptr);

        // Actions row (similar to Mac console actions)
        int bx = x;
        CreateWindowW(L"BUTTON", L"公告", WS_CHILD | WS_VISIBLE, bx, 132, 90, 28, hwnd, (HMENU)IDC_CONSOLE_BTN_NOTICE, nullptr, nullptr);
        bx += 95;
        CreateWindowW(L"BUTTON", L"到期", WS_CHILD | WS_VISIBLE, bx, 132, 90, 28, hwnd, (HMENU)IDC_CONSOLE_BTN_ENDTIME, nullptr, nullptr);
        bx += 95;
        CreateWindowW(L"BUTTON", L"版本", WS_CHILD | WS_VISIBLE, bx, 132, 90, 28, hwnd, (HMENU)IDC_CONSOLE_BTN_VERSION, nullptr, nullptr);
        bx += 95;
        CreateWindowW(L"BUTTON", L"用户信息", WS_CHILD | WS_VISIBLE, bx, 132, 110, 28, hwnd, (HMENU)IDC_CONSOLE_BTN_USERINFO, nullptr, nullptr);
        bx += 115;
        CreateWindowW(L"BUTTON", L"心跳", WS_CHILD | WS_VISIBLE, bx, 132, 90, 28, hwnd, (HMENU)IDC_CONSOLE_BTN_HEARTBEAT, nullptr, nullptr);
        bx += 95;
        CreateWindowW(L"BUTTON", L"Web登录", WS_CHILD | WS_VISIBLE, bx, 132, 90, 28, hwnd, (HMENU)IDC_CONSOLE_BTN_WEBLOGIN, nullptr, nullptr);
        CreateWindowW(L"BUTTON", L"注销", WS_CHILD | WS_VISIBLE, x + w - 90, 132, 90, 28, hwnd, (HMENU)IDC_CONSOLE_BTN_LOGOUT, nullptr, nullptr);

        // Sales/promo row (open web pages like Mac console window view)
        CreateWindowW(L"BUTTON", L"续费", WS_CHILD | WS_VISIBLE, x, 165, 90, 28, hwnd, (HMENU)IDC_CONSOLE_BTN_RENEW, nullptr, nullptr);
        CreateWindowW(L"BUTTON", L"购买卡密", WS_CHILD | WS_VISIBLE, x + 95, 165, 110, 28, hwnd, (HMENU)IDC_CONSOLE_BTN_BUY, nullptr, nullptr);
        CreateWindowW(L"BUTTON", L"库存卡密", WS_CHILD | WS_VISIBLE, x + 210, 165, 110, 28, hwnd, (HMENU)IDC_CONSOLE_BTN_STOCK, nullptr, nullptr);

        // Utility row
        CreateWindowW(L"BUTTON", L"复制机器码", WS_CHILD | WS_VISIBLE, x + 340, 165, 110, 28, hwnd, (HMENU)IDC_CONSOLE_BTN_COPY_MACHINE, nullptr, nullptr);
        CreateWindowW(L"BUTTON", L"复制会话", WS_CHILD | WS_VISIBLE, x + 455, 165, 95, 28, hwnd, (HMENU)IDC_CONSOLE_BTN_COPY_SESSION, nullptr, nullptr);
        CreateWindowW(L"BUTTON", L"自动心跳", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX, x + 560, 170, 90, 20, hwnd, (HMENU)IDC_CONSOLE_CHK_AUTOHB, nullptr, nullptr);
        CreateWindowW(L"EDIT", L"30", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_NUMBER, x + 655, 167, 40, 22, hwnd, (HMENU)IDC_CONSOLE_EDIT_INTERVAL, nullptr, nullptr);

        // appcustom.in (same as MFC/Mac demo: myapp/myvip/mylogin)
        CreateWindowW(L"BUTTON", L"App配置", WS_CHILD | WS_VISIBLE, x + 340, 195, 90, 28, hwnd, (HMENU)IDC_CONSOLE_BTN_APPCUSTOM_APP, nullptr, nullptr);
        CreateWindowW(L"BUTTON", L"VIP配置", WS_CHILD | WS_VISIBLE, x + 435, 195, 90, 28, hwnd, (HMENU)IDC_CONSOLE_BTN_APPCUSTOM_VIP, nullptr, nullptr);
        CreateWindowW(L"BUTTON", L"登录配置", WS_CHILD | WS_VISIBLE, x + 530, 195, 90, 28, hwnd, (HMENU)IDC_CONSOLE_BTN_APPCUSTOM_LOGIN, nullptr, nullptr);

        CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"",
                         WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY | WS_VSCROLL,
                         x, 235, w, hLogH, hwnd, (HMENU)IDC_CONSOLE_LOG_EDIT, nullptr, nullptr);

        auto* ms = cs->mainSt;
        if (ms) {
            AppendLogW(GetDlgItem(hwnd, IDC_CONSOLE_LOG_EDIT), L"[console] ready.");
            RefreshConsoleHeaderInfo(hwnd, cs);
            AppendConsoleApi(hwnd, L"get_notice", ms->client.get_notice());
            AppendConsoleApi(hwnd, L"vipdate.lg", ms->client.get_end_time());
            AppendConsoleApi(hwnd, L"v.in", ms->client.get_version());
        } else {
            AppendLogW(GetDlgItem(hwnd, IDC_CONSOLE_LOG_EDIT), L"[console] missing main state.");
        }
        return 0;
    }

    case WM_TIMER: {
        auto* cs = reinterpret_cast<ConsoleState*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
        if (!cs || !cs->mainSt) break;
        if (wParam == cs->hbTimerId) {
            AppendConsoleApi(hwnd, L"auto_heartbeat(vipdate.lg)", cs->mainSt->client.get_end_time());
        }
        break;
    }

    case WM_COMMAND: {
        auto* cs = reinterpret_cast<ConsoleState*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
        if (!cs || !cs->mainSt) break;
        auto* ms = cs->mainSt;

        switch (LOWORD(wParam)) {
        case IDC_CONSOLE_BTN_NOTICE:
            AppendConsoleApi(hwnd, L"get_notice", ms->client.get_notice());
            break;
        case IDC_CONSOLE_BTN_ENDTIME:
            AppendConsoleApi(hwnd, L"vipdate.lg", ms->client.get_end_time());
            break;
        case IDC_CONSOLE_BTN_VERSION:
            AppendConsoleApi(hwnd, L"v.in", ms->client.get_version());
            break;
        case IDC_CONSOLE_BTN_USERINFO: {
            auto r = ms->client.api_call("getuserinfo.lg");
            cs->lastUserInfoData = r.data;
            AppendConsoleApi(hwnd, L"getuserinfo.lg", r);
            RefreshConsoleHeaderInfo(hwnd, cs);
            break;
        }
        case IDC_CONSOLE_BTN_HEARTBEAT:
            // For demo: heartbeat shows end time again.
            AppendConsoleApi(hwnd, L"heartbeat(vipdate.lg)", ms->client.get_end_time());
            break;
        case IDC_CONSOLE_BTN_WEBLOGIN: {
            if (ms->client.BSphpSeSsL.empty()) {
                AppendLogW(GetDlgItem(hwnd, IDC_CONSOLE_LOG_EDIT), L"[web_login] session empty.");
                break;
            }
            std::wstring url = W(bsphp_login_demo::MakeWebLoginUrl(ms->client.BSphpSeSsL));
            OpenUrlW(url);
            AppendLogW(GetDlgItem(hwnd, IDC_CONSOLE_LOG_EDIT), std::wstring(L"[open_url] ") + url);
            break;
        }
        case IDC_CONSOLE_BTN_RENEW: {
            std::wstring url = cs->lastUserInfoData.empty() ? bsphp_login_demo::RenewSaleUrlW()
                                                            : bsphp_login_demo::RenewSaleUrlWithUserFromInfoDataW(cs->lastUserInfoData);
            OpenUrlW(url);
            AppendLogW(GetDlgItem(hwnd, IDC_CONSOLE_LOG_EDIT), std::wstring(L"[open_url] ") + url);
            break;
        }
        case IDC_CONSOLE_BTN_BUY: {
            std::wstring url = bsphp_login_demo::GenCardSaleUrlW();
            OpenUrlW(url);
            AppendLogW(GetDlgItem(hwnd, IDC_CONSOLE_LOG_EDIT), std::wstring(L"[open_url] ") + url);
            break;
        }
        case IDC_CONSOLE_BTN_STOCK: {
            std::wstring url = bsphp_login_demo::StockSaleUrlW();
            OpenUrlW(url);
            AppendLogW(GetDlgItem(hwnd, IDC_CONSOLE_LOG_EDIT), std::wstring(L"[open_url] ") + url);
            break;
        }
        case IDC_CONSOLE_BTN_COPY_MACHINE: {
            SetClipboardTextW(hwnd, W(ms->machine_code));
            AppendLogW(GetDlgItem(hwnd, IDC_CONSOLE_LOG_EDIT), L"[clipboard] machine_code copied.");
            break;
        }
        case IDC_CONSOLE_BTN_COPY_SESSION: {
            SetClipboardTextW(hwnd, W(ms->client.BSphpSeSsL));
            AppendLogW(GetDlgItem(hwnd, IDC_CONSOLE_LOG_EDIT), L"[clipboard] session copied.");
            break;
        }
        case IDC_CONSOLE_CHK_AUTOHB: {
            const bool on = (SendMessageW(GetDlgItem(hwnd, IDC_CONSOLE_CHK_AUTOHB), BM_GETCHECK, 0, 0) == BST_CHECKED);
            if (on) {
                const int sec = IntFromEditOr(hwnd, IDC_CONSOLE_EDIT_INTERVAL, 30);
                cs->hbTimerId = 1;
                SetTimer(hwnd, cs->hbTimerId, (UINT)(sec * 1000), nullptr);
                AppendLogW(GetDlgItem(hwnd, IDC_CONSOLE_LOG_EDIT), L"[auto_heartbeat] enabled.");
            } else {
                if (cs->hbTimerId) {
                    KillTimer(hwnd, cs->hbTimerId);
                    cs->hbTimerId = 0;
                }
                AppendLogW(GetDlgItem(hwnd, IDC_CONSOLE_LOG_EDIT), L"[auto_heartbeat] disabled.");
            }
            break;
        }
        case IDC_CONSOLE_BTN_APPCUSTOM_APP: {
            AppendConsoleApi(hwnd, L"appcustom.in info=myapp", ms->client.api_call("appcustom.in", {{"info", "myapp"}}));
            break;
        }
        case IDC_CONSOLE_BTN_APPCUSTOM_VIP: {
            AppendConsoleApi(hwnd, L"appcustom.in info=myvip", ms->client.api_call("appcustom.in", {{"info", "myvip"}}));
            break;
        }
        case IDC_CONSOLE_BTN_APPCUSTOM_LOGIN: {
            AppendConsoleApi(hwnd, L"appcustom.in info=mylogin", ms->client.api_call("appcustom.in", {{"info", "mylogin"}}));
            break;
        }
        case IDC_CONSOLE_BTN_LOGOUT: {
            AppendConsoleApi(hwnd, L"cancellation.lg", ms->client.logout_lg());
            DestroyWindow(hwnd);
            break;
        }
        }
        break;
    }

    case WM_DESTROY: {
        auto* cs = reinterpret_cast<ConsoleState*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
        if (cs) {
            if (cs->hbTimerId) {
                KillTimer(hwnd, cs->hbTimerId);
                cs->hbTimerId = 0;
            }
            if (cs->mainSt) cs->mainSt->console_hwnd = nullptr;
            delete cs;
        }
        return 0;
    }
    }
    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

static void OpenConsoleWindow(AppState* st) {
    if (!st) return;

    if (st->console_hwnd && IsWindow(st->console_hwnd)) {
        ShowWindow(st->console_hwnd, SW_SHOW);
        SetForegroundWindow(st->console_hwnd);
        return;
    }

    const wchar_t* kConsoleClassName = L"BSPHPWinLoginConsoleDemo";
    HINSTANCE hInst = GetModuleHandleW(nullptr);

    // Register class once (ignore failure if already registered).
    WNDCLASSW wc{};
    wc.lpfnWndProc = ConsoleWndProc;
    wc.hInstance = hInst;
    wc.lpszClassName = kConsoleClassName;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    RegisterClassW(&wc);

    HWND hwnd = CreateWindowW(kConsoleClassName, L"BSPHP Console (after login)",
                               WS_OVERLAPPEDWINDOW | WS_VISIBLE, CW_USEDEFAULT, CW_USEDEFAULT,
                               760, 560, nullptr, nullptr, hInst, st);
    if (hwnd) st->console_hwnd = hwnd;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE: {
        auto* st = new AppState();
        SetState(hwnd, st);

        const int x = 20;
        const int w = 820;
        const int pageX = x;
        const int commonY = 155;
        const int pageY = commonY + 45;
        const int pageW = w;
        const int pageH = 295;

        // Notice
        CreateWindowW(L"BUTTON", L"公告", WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
                       x, 20, w, 90, hwnd, nullptr, nullptr, nullptr);
        CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"",
                         WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY | WS_VSCROLL,
                         x + 10, 40, w - 20, 60, hwnd, (HMENU)IDC_NOTICE_EDIT, nullptr, nullptr);

        // Tabs
        HWND hTab = CreateWindowExW(0, WC_TABCONTROLW, L"",
                                     WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS,
                                     x, 120, w, 28, hwnd, (HMENU)IDC_TAB, nullptr, nullptr);
        TCITEMW ti{};
        ti.mask = TCIF_TEXT;
        const wchar_t* names[] = {
            L"登录", L"短信登录", L"邮箱登录", L"注册", L"短信注册", L"邮箱注册", L"解绑", L"充值", L"短信找回", L"邮箱找回", L"找回密码", L"修改密码", L"意见反馈"
        };
        for (int i = 0; i < static_cast<int>(TabIndex::Count); ++i) {
            ti.pszText = const_cast<LPWSTR>(names[i]);
            TabCtrl_InsertItem(hTab, i, &ti);
        }
        TabCtrl_SetCurSel(hTab, 0);

        // Page containers (hide/show per tab to avoid leftover controls).
        st->page_login = CreateWindowW(L"STATIC", L"", WS_CHILD, pageX, pageY, pageW, pageH, hwnd, nullptr, nullptr, nullptr);
        st->page_sms = CreateWindowW(L"STATIC", L"", WS_CHILD, pageX, pageY, pageW, pageH, hwnd, nullptr, nullptr, nullptr);
        st->page_email = CreateWindowW(L"STATIC", L"", WS_CHILD, pageX, pageY, pageW, pageH, hwnd, nullptr, nullptr, nullptr);
        st->page_register = CreateWindowW(L"STATIC", L"", WS_CHILD, pageX, pageY, pageW, pageH, hwnd, nullptr, nullptr, nullptr);
        st->page_unbind = CreateWindowW(L"STATIC", L"", WS_CHILD, pageX, pageY, pageW, pageH, hwnd, nullptr, nullptr, nullptr);
        st->page_recharge = CreateWindowW(L"STATIC", L"", WS_CHILD, pageX, pageY, pageW, pageH, hwnd, nullptr, nullptr, nullptr);
        st->page_recover = CreateWindowW(L"STATIC", L"", WS_CHILD, pageX, pageY, pageW, pageH, hwnd, nullptr, nullptr, nullptr);
        st->page_change = CreateWindowW(L"STATIC", L"", WS_CHILD, pageX, pageY, pageW, pageH, hwnd, nullptr, nullptr, nullptr);
        st->page_feedback = CreateWindowW(L"STATIC", L"", WS_CHILD, pageX, pageY, pageW, pageH, hwnd, nullptr, nullptr, nullptr);

        // Forward button clicks / notifications from page containers to main WndProc.
        SubclassPageForForwarding(st->page_login);
        SubclassPageForForwarding(st->page_sms);
        SubclassPageForForwarding(st->page_email);
        SubclassPageForForwarding(st->page_register);
        SubclassPageForForwarding(st->page_unbind);
        SubclassPageForForwarding(st->page_recharge);
        SubclassPageForForwarding(st->page_recover);
        SubclassPageForForwarding(st->page_change);
        SubclassPageForForwarding(st->page_feedback);

        // Log
        CreateWindowW(L"STATIC", L"日志：", WS_CHILD | WS_VISIBLE, x, 505, 60, 22, hwnd, nullptr, nullptr, nullptr);
        CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"",
                         WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY | WS_VSCROLL,
                         x, 530, w, 220, hwnd, (HMENU)IDC_LOG_EDIT, nullptr, nullptr);

        // Common buttons (toolbar under tabs)
        int bx = x;
        CreateWindowW(L"BUTTON", L"测试网络", WS_CHILD | WS_VISIBLE, bx, commonY, 120, 28, hwnd, (HMENU)IDC_BTN_TESTNET, nullptr, nullptr);
        bx += 130;
        CreateWindowW(L"BUTTON", L"获取版本", WS_CHILD | WS_VISIBLE, bx, commonY, 120, 28, hwnd, (HMENU)IDC_BTN_VERSION, nullptr, nullptr);
        bx += 130;
        CreateWindowW(L"BUTTON", L"检测到期", WS_CHILD | WS_VISIBLE, bx, commonY, 120, 28, hwnd, (HMENU)IDC_BTN_ENDTIME, nullptr, nullptr);
        bx += 130;
        CreateWindowW(L"BUTTON", L"Web方式登陆", WS_CHILD | WS_VISIBLE, bx, commonY, 140, 28, hwnd, (HMENU)IDC_BTN_WEBLOGIN, nullptr, nullptr);
        // Logout on the right side of toolbar (visibility controlled by ApplyTabVisibility)
        CreateWindowW(L"BUTTON", L"注销登陆", WS_CHILD | WS_VISIBLE, x + w - 120, commonY, 120, 28, hwnd, (HMENU)IDC_BTN_LOGOUT, nullptr, nullptr);

        // ---- Login tab controls ----
        CreateWindowW(L"STATIC", L"登录账号：", WS_CHILD | WS_VISIBLE, 0, 5, 90, 22, st->page_login, nullptr, nullptr, nullptr);
        CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL, 95, 5, 300, 22, st->page_login, (HMENU)IDC_LOGIN_USER, nullptr, nullptr);
        CreateWindowW(L"STATIC", L"登录密码：", WS_CHILD | WS_VISIBLE, 0, 35, 90, 22, st->page_login, nullptr, nullptr, nullptr);
        CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL | ES_PASSWORD, 95, 35, 300, 22, st->page_login, (HMENU)IDC_LOGIN_PWD, nullptr, nullptr);
        CreateWindowW(L"STATIC", L"验 证 码：", WS_CHILD | WS_VISIBLE, 0, 65, 90, 22, st->page_login, (HMENU)IDC_LOGIN_CODE_LABEL, nullptr, nullptr);
        CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL, 95, 65, 200, 22, st->page_login, (HMENU)IDC_LOGIN_CODE, nullptr, nullptr);
        CreateWindowW(L"BUTTON", L"刷新", WS_CHILD | WS_VISIBLE, 305, 65, 90, 22, st->page_login, (HMENU)IDC_LOGIN_CODE_REFRESH, nullptr, nullptr);
        CreateWindowW(L"STATIC", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | SS_CENTERIMAGE | SS_BITMAP, 410, 60, 180, 80, st->page_login, (HMENU)IDC_LOGIN_CODE_PIC, nullptr, nullptr);
        CreateWindowW(L"BUTTON", L"登录", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON, 95, 150, 140, 28, st->page_login, (HMENU)IDC_LOGIN_SUBMIT, nullptr, nullptr);

        // Default demo credentials.
        SetWindowTextW(GetDlgItem(st->page_login, IDC_LOGIN_USER), L"admin");
        SetWindowTextW(GetDlgItem(st->page_login, IDC_LOGIN_PWD), L"admin");

        // ---- Register tab controls ----
        int y = 5;
        CreateWindowW(L"STATIC", L"注册账号：", WS_CHILD | WS_VISIBLE, 0, y, 90, 22, st->page_register, nullptr, nullptr, nullptr);
        CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL, 95, y, 300, 22, st->page_register, (HMENU)IDC_REG_USER, nullptr, nullptr);
        y += 30;
        CreateWindowW(L"STATIC", L"注册密码：", WS_CHILD | WS_VISIBLE, 0, y, 90, 22, st->page_register, nullptr, nullptr, nullptr);
        CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL | ES_PASSWORD, 95, y, 145, 22, st->page_register, (HMENU)IDC_REG_PWD, nullptr, nullptr);
        CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL | ES_PASSWORD, 250, y, 145, 22, st->page_register, (HMENU)IDC_REG_PWD2, nullptr, nullptr);
        y += 30;
        CreateWindowW(L"STATIC", L"QQ/邮箱：", WS_CHILD | WS_VISIBLE, 0, y, 90, 22, st->page_register, nullptr, nullptr, nullptr);
        CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL, 95, y, 145, 22, st->page_register, (HMENU)IDC_REG_QQ, nullptr, nullptr);
        CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL, 250, y, 145, 22, st->page_register, (HMENU)IDC_REG_MAIL, nullptr, nullptr);
        y += 30;
        CreateWindowW(L"STATIC", L"手机号码：", WS_CHILD | WS_VISIBLE, 0, y, 90, 22, st->page_register, nullptr, nullptr, nullptr);
        CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL, 95, y, 300, 22, st->page_register, (HMENU)IDC_REG_MOBILE, nullptr, nullptr);
        y += 30;
        CreateWindowW(L"STATIC", L"密保问题：", WS_CHILD | WS_VISIBLE, 0, y, 90, 22, st->page_register, nullptr, nullptr, nullptr);
        HWND hQ = CreateWindowW(WC_COMBOBOXW, L"", WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST, 95, y, 300, 240, st->page_register, (HMENU)IDC_REG_QUESTION, nullptr, nullptr);
        (void)hQ;
        const wchar_t* q[] = {L"你最喜欢的颜色？", L"你最喜欢的食物？", L"你最喜欢的城市？", L"你的生日？", L"你的手机号后四位？"};
        for (auto* it : q) SendMessageW(hQ, CB_ADDSTRING, 0, (LPARAM)it);
        SendMessageW(hQ, CB_SETCURSEL, 0, 0);
        CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL, 405, y, 240, 22, st->page_register, (HMENU)IDC_REG_ANSWER, nullptr, nullptr);
        y += 30;
        CreateWindowW(L"STATIC", L"验 证 码：", WS_CHILD | WS_VISIBLE, 0, y, 90, 22, st->page_register, (HMENU)IDC_REG_CODE_LABEL, nullptr, nullptr);
        CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL, 95, y, 200, 22, st->page_register, (HMENU)IDC_REG_CODE, nullptr, nullptr);
        CreateWindowW(L"BUTTON", L"刷新", WS_CHILD | WS_VISIBLE, 305, y, 90, 22, st->page_register, (HMENU)IDC_REG_CODE_REFRESH, nullptr, nullptr);
        CreateWindowW(L"STATIC", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | SS_CENTERIMAGE | SS_BITMAP, 410, y, 180, 48, st->page_register, (HMENU)IDC_REG_CODE_PIC, nullptr, nullptr);
        y += 55;
        CreateWindowW(L"STATIC", L"推广码：", WS_CHILD | WS_VISIBLE, 0, y, 90, 22, st->page_register, nullptr, nullptr, nullptr);
        CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL, 95, y, 300, 22, st->page_register, (HMENU)IDC_REG_EXTENSION, nullptr, nullptr);
        y += 35;
        CreateWindowW(L"BUTTON", L"注册", WS_CHILD | WS_VISIBLE, 95, y, 140, 28, st->page_register, (HMENU)IDC_REG_SUBMIT, nullptr, nullptr);

        // ---- Unbind ----
        CreateWindowW(L"STATIC", L"登录账号：", WS_CHILD | WS_VISIBLE, 0, 5, 90, 22, st->page_unbind, nullptr, nullptr, nullptr);
        CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL, 95, 5, 300, 22, st->page_unbind, (HMENU)IDC_UNBIND_USER, nullptr, nullptr);
        CreateWindowW(L"STATIC", L"登录密码：", WS_CHILD | WS_VISIBLE, 0, 35, 90, 22, st->page_unbind, nullptr, nullptr, nullptr);
        CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL | ES_PASSWORD, 95, 35, 300, 22, st->page_unbind, (HMENU)IDC_UNBIND_PWD, nullptr, nullptr);
        CreateWindowW(L"BUTTON", L"解绑", WS_CHILD | WS_VISIBLE, 95, 70, 140, 28, st->page_unbind, (HMENU)IDC_UNBIND_SUBMIT, nullptr, nullptr);

        // ---- Recharge ----
        CreateWindowW(L"STATIC", L"充值账号：", WS_CHILD | WS_VISIBLE, 0, 5, 90, 22, st->page_recharge, nullptr, nullptr, nullptr);
        CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL, 95, 5, 300, 22, st->page_recharge, (HMENU)IDC_PAY_USER, nullptr, nullptr);
        CreateWindowW(L"STATIC", L"登录密码：", WS_CHILD | WS_VISIBLE, 0, 35, 90, 22, st->page_recharge, nullptr, nullptr, nullptr);
        CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL | ES_PASSWORD, 95, 35, 300, 22, st->page_recharge, (HMENU)IDC_PAY_USERPWD, nullptr, nullptr);
        CreateWindowW(L"STATIC", L"充值卡号：", WS_CHILD | WS_VISIBLE, 0, 65, 90, 22, st->page_recharge, nullptr, nullptr, nullptr);
        CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL, 95, 65, 300, 22, st->page_recharge, (HMENU)IDC_PAY_KA, nullptr, nullptr);
        CreateWindowW(L"STATIC", L"充值密码：", WS_CHILD | WS_VISIBLE, 0, 95, 90, 22, st->page_recharge, nullptr, nullptr, nullptr);
        CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL | ES_PASSWORD, 95, 95, 300, 22, st->page_recharge, (HMENU)IDC_PAY_PWD, nullptr, nullptr);
        CreateWindowW(L"BUTTON", L"是否需要验证密码(防充错)", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX, 95, 125, 260, 22, st->page_recharge, (HMENU)IDC_PAY_USERSET, nullptr, nullptr);
        SendMessageW(GetDlgItem(hwnd, IDC_PAY_USERSET), BM_SETCHECK, BST_CHECKED, 0);
        CreateWindowW(L"BUTTON", L"充值", WS_CHILD | WS_VISIBLE, 95, 155, 140, 28, st->page_recharge, (HMENU)IDC_PAY_SUBMIT, nullptr, nullptr);

        // ---- Recover password (密保) ----
        CreateWindowW(L"STATIC", L"登录账号：", WS_CHILD | WS_VISIBLE, 0, 5, 90, 22, st->page_recover, nullptr, nullptr, nullptr);
        CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL, 95, 5, 300, 22, st->page_recover, (HMENU)IDC_BACK_USER, nullptr, nullptr);
        CreateWindowW(L"STATIC", L"密保问题：", WS_CHILD | WS_VISIBLE, 0, 35, 90, 22, st->page_recover, nullptr, nullptr, nullptr);
        HWND hBQ = CreateWindowW(WC_COMBOBOXW, L"", WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST, 95, 35, 300, 240, st->page_recover, (HMENU)IDC_BACK_QUESTION, nullptr, nullptr);
        for (auto* it : q) SendMessageW(hBQ, CB_ADDSTRING, 0, (LPARAM)it);
        SendMessageW(hBQ, CB_SETCURSEL, 0, 0);
        CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL, 405, 35, 240, 22, st->page_recover, (HMENU)IDC_BACK_ANSWER, nullptr, nullptr);
        CreateWindowW(L"STATIC", L"新密码：", WS_CHILD | WS_VISIBLE, 0, 65, 90, 22, st->page_recover, nullptr, nullptr, nullptr);
        CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL | ES_PASSWORD, 95, 65, 145, 22, st->page_recover, (HMENU)IDC_BACK_PWD, nullptr, nullptr);
        CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL | ES_PASSWORD, 250, 65, 145, 22, st->page_recover, (HMENU)IDC_BACK_PWD2, nullptr, nullptr);
        CreateWindowW(L"STATIC", L"验 证 码：", WS_CHILD | WS_VISIBLE, 0, 95, 90, 22, st->page_recover, (HMENU)IDC_BACK_CODE_LABEL, nullptr, nullptr);
        CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL, 95, 95, 200, 22, st->page_recover, (HMENU)IDC_BACK_CODE, nullptr, nullptr);
        CreateWindowW(L"BUTTON", L"刷新", WS_CHILD | WS_VISIBLE, 305, 95, 90, 22, st->page_recover, (HMENU)IDC_BACK_CODE_REFRESH, nullptr, nullptr);
        CreateWindowW(L"STATIC", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | SS_CENTERIMAGE | SS_BITMAP, 410, 95, 180, 48, st->page_recover, (HMENU)IDC_BACK_CODE_PIC, nullptr, nullptr);
        CreateWindowW(L"BUTTON", L"找回密码", WS_CHILD | WS_VISIBLE, 95, 160, 140, 28, st->page_recover, (HMENU)IDC_BACK_SUBMIT, nullptr, nullptr);

        // ---- Change password ----
        CreateWindowW(L"STATIC", L"登录账号：", WS_CHILD | WS_VISIBLE, 0, 5, 90, 22, st->page_change, nullptr, nullptr, nullptr);
        CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL, 95, 5, 300, 22, st->page_change, (HMENU)IDC_CHG_USER, nullptr, nullptr);
        CreateWindowW(L"STATIC", L"旧密码：", WS_CHILD | WS_VISIBLE, 0, 35, 90, 22, st->page_change, nullptr, nullptr, nullptr);
        CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL | ES_PASSWORD, 95, 35, 300, 22, st->page_change, (HMENU)IDC_CHG_OLD, nullptr, nullptr);
        CreateWindowW(L"STATIC", L"新密码：", WS_CHILD | WS_VISIBLE, 0, 65, 90, 22, st->page_change, nullptr, nullptr, nullptr);
        CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL | ES_PASSWORD, 95, 65, 145, 22, st->page_change, (HMENU)IDC_CHG_NEW, nullptr, nullptr);
        CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL | ES_PASSWORD, 250, 65, 145, 22, st->page_change, (HMENU)IDC_CHG_NEW2, nullptr, nullptr);
        CreateWindowW(L"STATIC", L"img：", WS_CHILD | WS_VISIBLE, 0, 95, 90, 22, st->page_change, nullptr, nullptr, nullptr);
        CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL, 95, 95, 300, 22, st->page_change, (HMENU)IDC_CHG_IMG, nullptr, nullptr);
        CreateWindowW(L"BUTTON", L"修改密码", WS_CHILD | WS_VISIBLE, 95, 130, 140, 28, st->page_change, (HMENU)IDC_CHG_SUBMIT, nullptr, nullptr);

        // ---- Feedback ----
        CreateWindowW(L"STATIC", L"账号：", WS_CHILD | WS_VISIBLE, 0, 5, 90, 22, st->page_feedback, nullptr, nullptr, nullptr);
        CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL, 95, 5, 300, 22, st->page_feedback, (HMENU)IDC_FB_USER, nullptr, nullptr);
        CreateWindowW(L"STATIC", L"密码：", WS_CHILD | WS_VISIBLE, 0, 35, 90, 22, st->page_feedback, nullptr, nullptr, nullptr);
        CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL | ES_PASSWORD, 95, 35, 300, 22, st->page_feedback, (HMENU)IDC_FB_PWD, nullptr, nullptr);
        CreateWindowW(L"STATIC", L"标题：", WS_CHILD | WS_VISIBLE, 0, 65, 90, 22, st->page_feedback, nullptr, nullptr, nullptr);
        CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL, 95, 65, 300, 22, st->page_feedback, (HMENU)IDC_FB_TITLE, nullptr, nullptr);
        CreateWindowW(L"STATIC", L"联系：", WS_CHILD | WS_VISIBLE, 0, 95, 90, 22, st->page_feedback, nullptr, nullptr, nullptr);
        CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL, 95, 95, 300, 22, st->page_feedback, (HMENU)IDC_FB_CONTACT, nullptr, nullptr);
        CreateWindowW(L"STATIC", L"类型：", WS_CHILD | WS_VISIBLE, 0, 125, 90, 22, st->page_feedback, nullptr, nullptr, nullptr);
        HWND hFT = CreateWindowW(WC_COMBOBOXW, L"", WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST, 95, 125, 300, 240, st->page_feedback, (HMENU)IDC_FB_TYPE, nullptr, nullptr);
        const wchar_t* ft[] = {L"建议", L"BUG", L"投诉", L"其他"};
        for (auto* it : ft) SendMessageW(hFT, CB_ADDSTRING, 0, (LPARAM)it);
        SendMessageW(hFT, CB_SETCURSEL, 0, 0);
        CreateWindowW(L"STATIC", L"内容：", WS_CHILD | WS_VISIBLE, 0, 155, 90, 22, st->page_feedback, nullptr, nullptr, nullptr);
        CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"", WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_AUTOVSCROLL | WS_VSCROLL,
                         95, 155, 420, 50, st->page_feedback, (HMENU)IDC_FB_TEXT, nullptr, nullptr);
        CreateWindowW(L"STATIC", L"验 证 码：", WS_CHILD | WS_VISIBLE, 0, 215, 90, 22, st->page_feedback, (HMENU)IDC_FB_CODE_LABEL, nullptr, nullptr);
        CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL, 95, 215, 200, 22, st->page_feedback, (HMENU)IDC_FB_CODE, nullptr, nullptr);
        CreateWindowW(L"BUTTON", L"刷新", WS_CHILD | WS_VISIBLE, 305, 215, 90, 22, st->page_feedback, (HMENU)IDC_FB_CODE_REFRESH, nullptr, nullptr);
        CreateWindowW(L"STATIC", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | SS_CENTERIMAGE | SS_BITMAP, 410, 215, 180, 48, st->page_feedback, (HMENU)IDC_FB_CODE_PIC, nullptr, nullptr);
        CreateWindowW(L"BUTTON", L"提交", WS_CHILD | WS_VISIBLE, 95, 265, 140, 28, st->page_feedback, (HMENU)IDC_FB_SUBMIT, nullptr, nullptr);

        // Prefill related tabs that require user/pwd.
        SetEditW(hwnd, IDC_UNBIND_USER, L"admin");
        SetEditW(hwnd, IDC_UNBIND_PWD, L"admin");
        SetEditW(hwnd, IDC_PAY_USER, L"admin");
        SetEditW(hwnd, IDC_PAY_USERPWD, L"admin");
        SetEditW(hwnd, IDC_FB_USER, L"admin");
        SetEditW(hwnd, IDC_FB_PWD, L"admin");

        // ---- SMS controls (shared, placed in same coordinates) ----
        CreateWindowW(L"STATIC", L"手机号码：", WS_CHILD | WS_VISIBLE, 0, 5, 90, 22, st->page_sms, nullptr, nullptr, nullptr);
        CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL, 95, 5, 300, 22, st->page_sms, (HMENU)IDC_SMS_MOBILE, nullptr, nullptr);
        CreateWindowW(L"STATIC", L"区号：", WS_CHILD | WS_VISIBLE, 0, 35, 90, 22, st->page_sms, nullptr, nullptr, nullptr);
        CreateWindowW(L"EDIT", L"86", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL, 95, 35, 300, 22, st->page_sms, (HMENU)IDC_SMS_AREA, nullptr, nullptr);
        CreateWindowW(L"STATIC", L"图片验证码：", WS_CHILD | WS_VISIBLE, 0, 65, 90, 22, st->page_sms, nullptr, nullptr, nullptr);
        CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL, 95, 65, 200, 22, st->page_sms, (HMENU)IDC_SMS_IMG, nullptr, nullptr);
        CreateWindowW(L"BUTTON", L"刷新", WS_CHILD | WS_VISIBLE, 305, 65, 90, 22, st->page_sms, (HMENU)IDC_SMS_IMG_REFRESH, nullptr, nullptr);
        CreateWindowW(L"STATIC", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | SS_CENTERIMAGE | SS_BITMAP, 410, 65, 180, 48, st->page_sms, (HMENU)IDC_SMS_IMG_PIC, nullptr, nullptr);
        CreateWindowW(L"BUTTON", L"发送验证码", WS_CHILD | WS_VISIBLE, 595, 65, 110, 22, st->page_sms, (HMENU)IDC_SMS_SEND, nullptr, nullptr);
        CreateWindowW(L"STATIC", L"短信验证码：", WS_CHILD | WS_VISIBLE, 0, 95, 90, 22, st->page_sms, nullptr, nullptr, nullptr);
        CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL, 95, 95, 300, 22, st->page_sms, (HMENU)IDC_SMS_CODE, nullptr, nullptr);
        CreateWindowW(L"STATIC", L"key：", WS_CHILD | WS_VISIBLE, 0, 125, 90, 22, st->page_sms, (HMENU)IDC_SMS_KEY_LABEL, nullptr, nullptr);
        CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL, 95, 125, 300, 22, st->page_sms, (HMENU)IDC_SMS_KEY, nullptr, nullptr);
        CreateWindowW(L"STATIC", L"maxoror：", WS_CHILD | WS_VISIBLE, 0, 155, 90, 22, st->page_sms, (HMENU)IDC_SMS_MAXOROR_LABEL, nullptr, nullptr);
        CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL, 95, 155, 300, 22, st->page_sms, (HMENU)IDC_SMS_MAXOROR, nullptr, nullptr);
        CreateWindowW(L"STATIC", L"账号：", WS_CHILD | WS_VISIBLE, 0, 185, 90, 22, st->page_sms, (HMENU)IDC_SMS_USER_LABEL, nullptr, nullptr);
        CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL, 95, 185, 300, 22, st->page_sms, (HMENU)IDC_SMS_USER, nullptr, nullptr);
        CreateWindowW(L"STATIC", L"密码：", WS_CHILD | WS_VISIBLE, 0, 215, 90, 22, st->page_sms, (HMENU)IDC_SMS_PWD_LABEL, nullptr, nullptr);
        CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL | ES_PASSWORD, 95, 215, 145, 22, st->page_sms, (HMENU)IDC_SMS_PWD, nullptr, nullptr);
        CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL | ES_PASSWORD, 250, 215, 145, 22, st->page_sms, (HMENU)IDC_SMS_PWD2, nullptr, nullptr);
        CreateWindowW(L"BUTTON", L"提交", WS_CHILD | WS_VISIBLE, 95, 250, 140, 28, st->page_sms, (HMENU)IDC_SMS_SUBMIT, nullptr, nullptr);

        // ---- Email controls (shared) ----
        CreateWindowW(L"STATIC", L"邮箱地址：", WS_CHILD | WS_VISIBLE, 0, 5, 90, 22, st->page_email, nullptr, nullptr, nullptr);
        CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL, 95, 5, 300, 22, st->page_email, (HMENU)IDC_EMAIL_ADDR, nullptr, nullptr);
        CreateWindowW(L"STATIC", L"图片验证码：", WS_CHILD | WS_VISIBLE, 0, 35, 90, 22, st->page_email, nullptr, nullptr, nullptr);
        CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL, 95, 35, 200, 22, st->page_email, (HMENU)IDC_EMAIL_IMG, nullptr, nullptr);
        CreateWindowW(L"BUTTON", L"刷新", WS_CHILD | WS_VISIBLE, 305, 35, 90, 22, st->page_email, (HMENU)IDC_EMAIL_IMG_REFRESH, nullptr, nullptr);
        CreateWindowW(L"STATIC", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | SS_CENTERIMAGE | SS_BITMAP, 410, 35, 180, 48, st->page_email, (HMENU)IDC_EMAIL_IMG_PIC, nullptr, nullptr);
        CreateWindowW(L"BUTTON", L"发送验证码", WS_CHILD | WS_VISIBLE, 595, 35, 110, 22, st->page_email, (HMENU)IDC_EMAIL_SEND, nullptr, nullptr);
        CreateWindowW(L"STATIC", L"邮箱验证码：", WS_CHILD | WS_VISIBLE, 0, 65, 90, 22, st->page_email, nullptr, nullptr, nullptr);
        CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL, 95, 65, 300, 22, st->page_email, (HMENU)IDC_EMAIL_CODE, nullptr, nullptr);
        CreateWindowW(L"STATIC", L"key：", WS_CHILD | WS_VISIBLE, 0, 95, 90, 22, st->page_email, (HMENU)IDC_EMAIL_KEY_LABEL, nullptr, nullptr);
        CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL, 95, 95, 300, 22, st->page_email, (HMENU)IDC_EMAIL_KEY, nullptr, nullptr);
        CreateWindowW(L"STATIC", L"maxoror：", WS_CHILD | WS_VISIBLE, 0, 125, 90, 22, st->page_email, (HMENU)IDC_EMAIL_MAXOROR_LABEL, nullptr, nullptr);
        CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL, 95, 125, 300, 22, st->page_email, (HMENU)IDC_EMAIL_MAXOROR, nullptr, nullptr);
        CreateWindowW(L"STATIC", L"账号：", WS_CHILD | WS_VISIBLE, 0, 155, 90, 22, st->page_email, (HMENU)IDC_EMAIL_USER_LABEL, nullptr, nullptr);
        CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL, 95, 155, 300, 22, st->page_email, (HMENU)IDC_EMAIL_USER, nullptr, nullptr);
        CreateWindowW(L"STATIC", L"密码：", WS_CHILD | WS_VISIBLE, 0, 185, 90, 22, st->page_email, (HMENU)IDC_EMAIL_PWD_LABEL, nullptr, nullptr);
        CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL | ES_PASSWORD, 95, 185, 145, 22, st->page_email, (HMENU)IDC_EMAIL_PWD, nullptr, nullptr);
        CreateWindowW(L"EDIT", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL | ES_PASSWORD, 250, 185, 145, 22, st->page_email, (HMENU)IDC_EMAIL_PWD2, nullptr, nullptr);
        CreateWindowW(L"BUTTON", L"提交", WS_CHILD | WS_VISIBLE, 95, 220, 140, 28, st->page_email, (HMENU)IDC_EMAIL_SUBMIT, nullptr, nullptr);

        // Bootstrap + notice
        HWND hLog = GetDlgItem(hwnd, IDC_LOG_EDIT);
        st->client.bootstrap();
        auto n = st->client.get_notice();
        SetEditW(hwnd, IDC_NOTICE_EDIT, n.data.empty() ? L"" : W(n.data));
        AppendApi(hwnd, L"get_notice", n);

        // Init code switches
        InitCodeSwitches(hwnd, st);

        // Default key/maxoror fill for otp tabs
        SetEditW(hwnd, IDC_SMS_KEY, W(st->machine_code));
        SetEditW(hwnd, IDC_SMS_MAXOROR, W(st->machine_code));
        SetEditW(hwnd, IDC_EMAIL_KEY, W(st->machine_code));
        SetEditW(hwnd, IDC_EMAIL_MAXOROR, W(st->machine_code));

        ApplyTabVisibility(hwnd);

        // Default tab: show captcha image immediately (when enabled).
        if (st->code_login) {
            UpdateCaptchaPicture(hwnd, st, IDC_LOGIN_CODE_PIC, MakeCaptchaUrl(st->client));
        }
        return 0;
    }

    case WM_NOTIFY: {
        if (LOWORD(wParam) == IDC_TAB) {
            LPNMHDR hdr = reinterpret_cast<LPNMHDR>(lParam);
            if (hdr && hdr->code == TCN_SELCHANGE) {
                ApplyTabVisibility(hwnd);
            }
        }
        break;
    }

    case WM_COMMAND: {
        auto* st = GetState(hwnd);
        if (!st) break;
        HWND hLog = GetDlgItem(hwnd, IDC_LOG_EDIT);

        auto capUrl = [&]() -> std::wstring { return MakeCaptchaUrl(st->client); };

        switch (LOWORD(wParam)) {
        // Common
        case IDC_BTN_TESTNET: {
            AppendApi(hwnd, L"connect", st->client.connect());
            break;
        }
        case IDC_BTN_VERSION: {
            AppendApi(hwnd, L"v.in", st->client.get_version());
            break;
        }
        case IDC_BTN_ENDTIME: {
            AppendApi(hwnd, L"vipdate.lg", st->client.get_end_time());
            break;
        }
        case IDC_BTN_WEBLOGIN: {
            if (st->client.BSphpSeSsL.empty()) {
                MessageBoxW(hwnd, L"Session empty, please bootstrap first.", L"info", MB_OK);
                break;
            }
            OpenUrlW(W(bsphp_login_demo::MakeWebLoginUrl(st->client.BSphpSeSsL)));
            AppendLogW(hLog, L"[web_login] opened.");
            break;
        }
        case IDC_BTN_LOGOUT: {
            AppendApi(hwnd, L"cancellation.lg", st->client.logout_lg());
            st->logged_in = false;
            if (st->console_hwnd && IsWindow(st->console_hwnd)) {
                DestroyWindow(st->console_hwnd);
                st->console_hwnd = nullptr;
            }
            ApplyTabVisibility(hwnd);
            break;
        }

        // Login
        case IDC_LOGIN_CODE_REFRESH: {
            UpdateCaptchaPicture(hwnd, st, IDC_LOGIN_CODE_PIC, capUrl());
            AppendLogW(hLog, std::wstring(L"[captcha_url] ") + capUrl());
            break;
        }
        case IDC_LOGIN_SUBMIT: {
            std::string user = Utf8FromEdit(hwnd, IDC_LOGIN_USER);
            std::string pwd = Utf8FromEdit(hwnd, IDC_LOGIN_PWD);
            std::string coode = st->code_login ? Utf8FromEdit(hwnd, IDC_LOGIN_CODE) : "";
            auto r = st->client.login_lg(user, pwd, coode, st->machine_code, st->machine_code);
            AppendApi(hwnd, L"login.lg", r);
            if (r.code == "1011" || r.code == "9908") {
                st->logged_in = true;
                ApplyTabVisibility(hwnd);
                OpenConsoleWindow(st);
            }
            break;
        }

        // Register
        case IDC_REG_CODE_REFRESH: {
            UpdateCaptchaPicture(hwnd, st, IDC_REG_CODE_PIC, capUrl());
            AppendLogW(hLog, std::wstring(L"[captcha_url] ") + capUrl());
            break;
        }
        case IDC_REG_SUBMIT: {
            std::string user = Utf8FromEdit(hwnd, IDC_REG_USER);
            std::string pwd = Utf8FromEdit(hwnd, IDC_REG_PWD);
            std::string pwdb = Utf8FromEdit(hwnd, IDC_REG_PWD2);
            std::string qq = Utf8FromEdit(hwnd, IDC_REG_QQ);
            std::string mail = Utf8FromEdit(hwnd, IDC_REG_MAIL);
            std::string mobile = Utf8FromEdit(hwnd, IDC_REG_MOBILE);
            std::string answer = Utf8FromEdit(hwnd, IDC_REG_ANSWER);
            std::string ext = Utf8FromEdit(hwnd, IDC_REG_EXTENSION);
            std::string coode = st->code_reg ? Utf8FromEdit(hwnd, IDC_REG_CODE) : "";

            int sel = (int)SendMessageW(GetDlgItem(hwnd, IDC_REG_QUESTION), CB_GETCURSEL, 0, 0);
            wchar_t buf[256] = {};
            SendMessageW(GetDlgItem(hwnd, IDC_REG_QUESTION), CB_GETLBTEXT, sel, (LPARAM)buf);
            std::string question = Utf8FromWide(buf);

            AppendApi(hwnd, L"registration.lg",
                      ApiCall(st->client, "registration.lg",
                              {{"user", user},
                               {"pwd", pwd},
                               {"pwdb", pwdb},
                               {"coode", coode},
                               {"mobile", mobile},
                               {"mibao_wenti", question},
                               {"mibao_daan", answer},
                               {"qq", qq},
                               {"mail", mail},
                               {"extension", ext}}));
            break;
        }

        // Unbind
        case IDC_UNBIND_SUBMIT: {
            AppendApi(hwnd, L"jiekey.lg",
                      ApiCall(st->client, "jiekey.lg", {{"user", Utf8FromEdit(hwnd, IDC_UNBIND_USER)}, {"pwd", Utf8FromEdit(hwnd, IDC_UNBIND_PWD)}}));
            break;
        }

        // Recharge
        case IDC_PAY_SUBMIT: {
            bool userset = (SendMessageW(GetDlgItem(hwnd, IDC_PAY_USERSET), BM_GETCHECK, 0, 0) == BST_CHECKED);
            AppendApi(hwnd, L"chong.lg",
                      ApiCall(st->client, "chong.lg",
                              {{"user", Utf8FromEdit(hwnd, IDC_PAY_USER)},
                               {"userpwd", Utf8FromEdit(hwnd, IDC_PAY_USERPWD)},
                               {"userset", userset ? "1" : "0"},
                               {"ka", Utf8FromEdit(hwnd, IDC_PAY_KA)},
                               {"pwd", Utf8FromEdit(hwnd, IDC_PAY_PWD)}}));
            break;
        }

        // Recover (密保)
        case IDC_BACK_CODE_REFRESH: {
            UpdateCaptchaPicture(hwnd, st, IDC_BACK_CODE_PIC, capUrl());
            AppendLogW(hLog, std::wstring(L"[captcha_url] ") + capUrl());
            break;
        }
        case IDC_BACK_SUBMIT: {
            int sel = (int)SendMessageW(GetDlgItem(hwnd, IDC_BACK_QUESTION), CB_GETCURSEL, 0, 0);
            wchar_t buf[256] = {};
            SendMessageW(GetDlgItem(hwnd, IDC_BACK_QUESTION), CB_GETLBTEXT, sel, (LPARAM)buf);
            std::string wenti = Utf8FromWide(buf);
            std::string coode = st->code_back ? Utf8FromEdit(hwnd, IDC_BACK_CODE) : "";
            AppendApi(hwnd, L"backto.lg",
                      ApiCall(st->client, "backto.lg",
                              {{"user", Utf8FromEdit(hwnd, IDC_BACK_USER)},
                               {"pwd", Utf8FromEdit(hwnd, IDC_BACK_PWD)},
                               {"pwdb", Utf8FromEdit(hwnd, IDC_BACK_PWD2)},
                               {"wenti", wenti},
                               {"daan", Utf8FromEdit(hwnd, IDC_BACK_ANSWER)},
                               {"coode", coode}}));
            break;
        }

        // Change password
        case IDC_CHG_SUBMIT: {
            AppendApi(hwnd, L"password.lg",
                      ApiCall(st->client, "password.lg",
                              {{"user", Utf8FromEdit(hwnd, IDC_CHG_USER)},
                               {"pwd", Utf8FromEdit(hwnd, IDC_CHG_OLD)},
                               {"pwda", Utf8FromEdit(hwnd, IDC_CHG_NEW)},
                               {"pwdb", Utf8FromEdit(hwnd, IDC_CHG_NEW2)},
                               {"img", Utf8FromEdit(hwnd, IDC_CHG_IMG)}}));
            break;
        }

        // Feedback
        case IDC_FB_CODE_REFRESH: {
            UpdateCaptchaPicture(hwnd, st, IDC_FB_CODE_PIC, capUrl());
            AppendLogW(hLog, std::wstring(L"[captcha_url] ") + capUrl());
            break;
        }
        case IDC_FB_SUBMIT: {
            int sel = (int)SendMessageW(GetDlgItem(hwnd, IDC_FB_TYPE), CB_GETCURSEL, 0, 0);
            wchar_t buf[256] = {};
            SendMessageW(GetDlgItem(hwnd, IDC_FB_TYPE), CB_GETLBTEXT, sel, (LPARAM)buf);
            std::string leix = Utf8FromWide(buf);
            std::string coode = st->code_say ? Utf8FromEdit(hwnd, IDC_FB_CODE) : "";
            AppendApi(hwnd, L"liuyan.in",
                      ApiCall(st->client, "liuyan.in",
                              {{"user", Utf8FromEdit(hwnd, IDC_FB_USER)},
                               {"pwd", Utf8FromEdit(hwnd, IDC_FB_PWD)},
                               {"table", Utf8FromEdit(hwnd, IDC_FB_TITLE)},
                               {"qq", Utf8FromEdit(hwnd, IDC_FB_CONTACT)},
                               {"leix", leix},
                               {"txt", Utf8FromEdit(hwnd, IDC_FB_TEXT)},
                               {"coode", coode}}));
            break;
        }

        // SMS OTP: refresh image
        case IDC_SMS_IMG_REFRESH: {
            UpdateCaptchaPicture(hwnd, st, IDC_SMS_IMG_PIC, capUrl());
            AppendLogW(hLog, std::wstring(L"[captcha_url] ") + capUrl());
            break;
        }
        case IDC_SMS_SEND: {
            // scene depends on tab
            TabIndex tab = static_cast<TabIndex>(TabCtrl_GetCurSel(GetDlgItem(hwnd, IDC_TAB)));
            std::string scene = (tab == TabIndex::SmsRegister) ? "register" : (tab == TabIndex::SmsRecover) ? "reset" : "login";
            std::string area = Utf8FromEdit(hwnd, IDC_SMS_AREA);
            if (area.empty()) area = "86";
            AppendApi(hwnd, L"send_sms.lg",
                      ApiCall(st->client, "send_sms.lg",
                              {{"scene", scene},
                               {"mobile", Utf8FromEdit(hwnd, IDC_SMS_MOBILE)},
                               {"area", area},
                               {"coode", Utf8FromEdit(hwnd, IDC_SMS_IMG)}}));
            break;
        }
        case IDC_SMS_SUBMIT: {
            TabIndex tab = static_cast<TabIndex>(TabCtrl_GetCurSel(GetDlgItem(hwnd, IDC_TAB)));
            std::string area = Utf8FromEdit(hwnd, IDC_SMS_AREA);
            if (area.empty()) area = "86";
            if (tab == TabIndex::SmsLogin) {
                AppendApi(hwnd, L"login_sms.lg",
                          ApiCall(st->client, "login_sms.lg",
                                  {{"mobile", Utf8FromEdit(hwnd, IDC_SMS_MOBILE)},
                                   {"area", area},
                                   {"sms_code", Utf8FromEdit(hwnd, IDC_SMS_CODE)},
                                   {"key", Utf8FromEdit(hwnd, IDC_SMS_KEY)},
                                   {"maxoror", Utf8FromEdit(hwnd, IDC_SMS_MAXOROR)},
                                   {"coode", Utf8FromEdit(hwnd, IDC_SMS_IMG)}}));
            } else if (tab == TabIndex::SmsRegister) {
                AppendApi(hwnd, L"register_sms.lg",
                          ApiCall(st->client, "register_sms.lg",
                                  {{"user", Utf8FromEdit(hwnd, IDC_SMS_USER)},
                                   {"mobile", Utf8FromEdit(hwnd, IDC_SMS_MOBILE)},
                                   {"area", area},
                                   {"sms_code", Utf8FromEdit(hwnd, IDC_SMS_CODE)},
                                   {"pwd", Utf8FromEdit(hwnd, IDC_SMS_PWD)},
                                   {"pwdb", Utf8FromEdit(hwnd, IDC_SMS_PWD2)},
                                   {"key", Utf8FromEdit(hwnd, IDC_SMS_KEY)},
                                   {"coode", Utf8FromEdit(hwnd, IDC_SMS_IMG)}}));
            } else if (tab == TabIndex::SmsRecover) {
                AppendApi(hwnd, L"resetpwd_sms.lg",
                          ApiCall(st->client, "resetpwd_sms.lg",
                                  {{"mobile", Utf8FromEdit(hwnd, IDC_SMS_MOBILE)},
                                   {"area", area},
                                   {"sms_code", Utf8FromEdit(hwnd, IDC_SMS_CODE)},
                                   {"pwd", Utf8FromEdit(hwnd, IDC_SMS_PWD)},
                                   {"pwdb", Utf8FromEdit(hwnd, IDC_SMS_PWD2)},
                                   {"coode", Utf8FromEdit(hwnd, IDC_SMS_IMG)}}));
            }
            break;
        }

        // Email OTP
        case IDC_EMAIL_IMG_REFRESH: {
            UpdateCaptchaPicture(hwnd, st, IDC_EMAIL_IMG_PIC, capUrl());
            AppendLogW(hLog, std::wstring(L"[captcha_url] ") + capUrl());
            break;
        }
        case IDC_EMAIL_SEND: {
            TabIndex tab = static_cast<TabIndex>(TabCtrl_GetCurSel(GetDlgItem(hwnd, IDC_TAB)));
            std::string scene = (tab == TabIndex::EmailRegister) ? "register" : (tab == TabIndex::EmailRecover) ? "reset" : "login";
            AppendApi(hwnd, L"send_email.lg",
                      ApiCall(st->client, "send_email.lg",
                              {{"scene", scene},
                               {"email", Utf8FromEdit(hwnd, IDC_EMAIL_ADDR)},
                               {"coode", Utf8FromEdit(hwnd, IDC_EMAIL_IMG)}}));
            break;
        }
        case IDC_EMAIL_SUBMIT: {
            TabIndex tab = static_cast<TabIndex>(TabCtrl_GetCurSel(GetDlgItem(hwnd, IDC_TAB)));
            if (tab == TabIndex::EmailLogin) {
                AppendApi(hwnd, L"login_email.lg",
                          ApiCall(st->client, "login_email.lg",
                                  {{"email", Utf8FromEdit(hwnd, IDC_EMAIL_ADDR)},
                                   {"email_code", Utf8FromEdit(hwnd, IDC_EMAIL_CODE)},
                                   {"key", Utf8FromEdit(hwnd, IDC_EMAIL_KEY)},
                                   {"maxoror", Utf8FromEdit(hwnd, IDC_EMAIL_MAXOROR)},
                                   {"coode", Utf8FromEdit(hwnd, IDC_EMAIL_IMG)}}));
            } else if (tab == TabIndex::EmailRegister) {
                AppendApi(hwnd, L"register_email.lg",
                          ApiCall(st->client, "register_email.lg",
                                  {{"user", Utf8FromEdit(hwnd, IDC_EMAIL_USER)},
                                   {"email", Utf8FromEdit(hwnd, IDC_EMAIL_ADDR)},
                                   {"email_code", Utf8FromEdit(hwnd, IDC_EMAIL_CODE)},
                                   {"pwd", Utf8FromEdit(hwnd, IDC_EMAIL_PWD)},
                                   {"pwdb", Utf8FromEdit(hwnd, IDC_EMAIL_PWD2)},
                                   {"key", Utf8FromEdit(hwnd, IDC_EMAIL_KEY)},
                                   {"coode", Utf8FromEdit(hwnd, IDC_EMAIL_IMG)}}));
            } else if (tab == TabIndex::EmailRecover) {
                AppendApi(hwnd, L"resetpwd_email.lg",
                          ApiCall(st->client, "resetpwd_email.lg",
                                  {{"email", Utf8FromEdit(hwnd, IDC_EMAIL_ADDR)},
                                   {"email_code", Utf8FromEdit(hwnd, IDC_EMAIL_CODE)},
                                   {"pwd", Utf8FromEdit(hwnd, IDC_EMAIL_PWD)},
                                   {"pwdb", Utf8FromEdit(hwnd, IDC_EMAIL_PWD2)},
                                   {"coode", Utf8FromEdit(hwnd, IDC_EMAIL_IMG)}}));
            }
            break;
        }
        }
        return 0;
    }

    case WM_DESTROY: {
        auto* st = GetState(hwnd);
        if (st) {
            if (st->console_hwnd && IsWindow(st->console_hwnd)) {
                DestroyWindow(st->console_hwnd);
                st->console_hwnd = nullptr;
            }
            for (auto& kv : st->captchaBitmaps) {
                if (kv.second) DeleteObject(kv.second);
            }
            st->captchaBitmaps.clear();
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
    const wchar_t* kClassName = L"BSPHPWinLoginDemo";

    INITCOMMONCONTROLSEX icc{};
    icc.dwSize = sizeof(icc);
    icc.dwICC = ICC_TAB_CLASSES;
    InitCommonControlsEx(&icc);

    // Needed for decoding captcha images.
    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    if (g_gdiplusToken == 0) {
        Gdiplus::GdiplusStartup(&g_gdiplusToken, &gdiplusStartupInput, nullptr);
    }

    WNDCLASSW wc{};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInst;
    wc.lpszClassName = kClassName;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    RegisterClassW(&wc);

    HWND hwnd = CreateWindowW(kClassName, L"BSPHP Win32 Login Demo (no MFC)",
                               WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
                               900, 820, nullptr, nullptr, hInst, nullptr);
    if (!hwnd) return 0;

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessageW(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    if (g_gdiplusToken != 0) {
        Gdiplus::GdiplusShutdown(g_gdiplusToken);
        g_gdiplusToken = 0;
    }
    return static_cast<int>(msg.wParam);
}

