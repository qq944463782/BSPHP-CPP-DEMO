#include "pch.h"

#include "LoginDlg.h"

#include "ConsoleDlg.h"
#include "WebLoginDlg.h"

#include "login_demo_config.h"

#include "../core/string_utf8.h"
#include "../core/machine_id.h"
#include "../core/debug_log.h"

#include <urlmon.h>
#include <gdiplus.h>

#pragma comment(lib, "urlmon.lib")
#pragma comment(lib, "gdiplus.lib")

#include <ctime>
#include <cctype>
#include <CommCtrl.h>

BEGIN_MESSAGE_MAP(CLoginDlg, CDialogEx)
    ON_BN_CLICKED(IDC_BTN_CAPTCHA_REFRESH, &CLoginDlg::OnBnClickedCaptchaRefresh)
    ON_BN_CLICKED(IDC_BTN_CAPTCHA_REFRESH_1, &CLoginDlg::OnBnClickedCaptchaRefresh)
    ON_BN_CLICKED(IDC_BTN_CAPTCHA_REFRESH_2, &CLoginDlg::OnBnClickedCaptchaRefresh)
    ON_BN_CLICKED(IDC_BTN_CAPTCHA_REFRESH_3, &CLoginDlg::OnBnClickedCaptchaRefresh)
    ON_BN_CLICKED(IDC_BTN_CAPTCHA_REFRESH_4, &CLoginDlg::OnBnClickedCaptchaRefresh)
    ON_BN_CLICKED(IDC_BTN_CAPTCHA_REFRESH_5, &CLoginDlg::OnBnClickedCaptchaRefresh)
    ON_BN_CLICKED(IDC_BTN_CAPTCHA_REFRESH_8, &CLoginDlg::OnBnClickedCaptchaRefresh)
    ON_BN_CLICKED(IDC_BTN_CAPTCHA_REFRESH_9, &CLoginDlg::OnBnClickedCaptchaRefresh)
    ON_BN_CLICKED(IDC_BTN_CAPTCHA_REFRESH_10, &CLoginDlg::OnBnClickedCaptchaRefresh)
    ON_BN_CLICKED(IDC_BTN_CAPTCHA_REFRESH_12, &CLoginDlg::OnBnClickedCaptchaRefresh)
    ON_BN_CLICKED(IDC_BTN_NET_TEST, &CLoginDlg::OnBnClickedNetTest)
    ON_BN_CLICKED(IDC_BTN_ENDTIME, &CLoginDlg::OnBnClickedEndTime)
    ON_BN_CLICKED(IDC_BTN_VERSION, &CLoginDlg::OnBnClickedVersion)
    ON_BN_CLICKED(IDC_BTN_WEB_LOGIN, &CLoginDlg::OnBnClickedWebLogin)
    ON_BN_CLICKED(IDC_BTN_LOGIN, &CLoginDlg::OnBnClickedLogin)
    ON_BN_CLICKED(IDC_BTN_SEND_CODE, &CLoginDlg::OnBnClickedSendCode)
    ON_BN_CLICKED(IDC_BTN_TAB_ACTION, &CLoginDlg::OnBnClickedTabAction)
    ON_NOTIFY(TCN_SELCHANGE, IDC_TAB_MAIN, &CLoginDlg::OnTcnSelchangeMainTab)
    ON_WM_DESTROY()
END_MESSAGE_MAP()

static void DeleteOldBitmap(HBITMAP& bmp) {
    if (bmp) {
        DeleteObject(bmp);
        bmp = nullptr;
    }
}

namespace {
std::wstring GetTempFilePath(const wchar_t* fileName) {
    wchar_t path[MAX_PATH] = {};
    DWORD n = GetTempPathW(MAX_PATH, path);
    if (n == 0 || n > MAX_PATH) {
        return std::wstring(fileName);
    }
    std::wstring out = path;
    if (!out.empty() && out.back() != L'\\') out += L'\\';
    out += fileName;
    return out;
}

ULONG_PTR InitGdiplusOnce() {
    static ULONG_PTR token = 0;
    static bool inited = false;
    if (inited) return token;
    Gdiplus::GdiplusStartupInput input;
    Gdiplus::Status st = Gdiplus::GdiplusStartup(&token, &input, nullptr);
    (void)st;
    inited = true;
    return token;
}
}  // namespace

CLoginDlg::CLoginDlg()
    : CDialogEx(IDD_LOGIN_MAIN, nullptr)
    , client_(bsphp_login_demo::MakeDemoClient())
    , machine_code_(BsphpDemoMachineCodeUtf8()) {}

BOOL CLoginDlg::OnInitDialog() {
    CDialogEx::OnInitDialog();

    // Ensure common controls (TabCtrl) are registered.
    INITCOMMONCONTROLSEX icex;
    icex.dwSize = sizeof(icex);
    icex.dwICC = ICC_TAB_CLASSES;
    InitCommonControlsEx(&icex);

    SetServiceStatus(L"初始化中...");
    SetLoginStatus(L"-");
    InitTabs();
    SetControlsEnabled(FALSE);

    BootstrapAndInitUI();
    ApplyTabUI(0);

    SetControlsEnabled(TRUE);
    return TRUE;
}

void CLoginDlg::InitTabs() {
    CWnd* wnd = GetDlgItem(IDC_TAB_MAIN);
    if (!wnd) return;
    // 使用 Attach/Detach，确保 CTabCtrl 句柄与内部 m_hWnd 绑定正确。
    CTabCtrl tab;
    tab.Attach(wnd->GetSafeHwnd());
    tab.DeleteAllItems();
    tab.InsertItem(0, L"密码登录");
    tab.InsertItem(1, L"短信登录");
    tab.InsertItem(2, L"邮箱登录");
    tab.InsertItem(3, L"账号注册");
    tab.InsertItem(4, L"短信注册");
    tab.InsertItem(5, L"邮箱注册");
    tab.InsertItem(6, L"解绑");
    tab.InsertItem(7, L"充值");
    tab.InsertItem(8, L"短信找回");
    tab.InsertItem(9, L"邮箱找回");
    tab.InsertItem(10, L"找回密码");
    tab.InsertItem(11, L"修改密码");
    tab.InsertItem(12, L"意见反馈");
    tab.SetCurSel(0);
    tab.Invalidate();
    tab.RedrawWindow();
    tab.Detach();
}

void CLoginDlg::OnDestroy() {
    DeleteOldBitmap(captcha_bmp_);
    CDialogEx::OnDestroy();
}

void CLoginDlg::SetServiceStatus(const CString& text) {
    if (CWnd* p = GetDlgItem(IDC_ST_SERVICE_STATUS)) {
        p->SetWindowTextW(text);
    }
}

void CLoginDlg::SetLoginStatus(const CString& text) {
    if (CWnd* p = GetDlgItem(IDC_ST_LOGIN_STATUS)) {
        p->SetWindowTextW(text);
    }
}

void CLoginDlg::SetControlsEnabled(BOOL enabled) {
    for (int id = IDC_EDIT_F1; id <= IDC_EDIT_F8; ++id) {
        if (CWnd* p = GetDlgItem(id)) p->EnableWindow(enabled);
    }

    const int inputId = CaptchaInputIdForTab(current_tab_);
    if (inputId != -1) {
        if (CWnd* p = GetDlgItem(inputId)) p->EnableWindow(enabled && is_code_enabled_);
    }
    const int refreshBtnId = CaptchaRefreshBtnIdForTab(current_tab_);
    if (refreshBtnId != -1) {
        if (CWnd* p = GetDlgItem(refreshBtnId)) p->EnableWindow(enabled && is_code_enabled_);
    }

    if (CWnd* p = GetDlgItem(IDC_BTN_SEND_CODE)) p->EnableWindow(enabled && is_ready_);
    if (CWnd* p = GetDlgItem(IDC_BTN_NET_TEST)) p->EnableWindow(enabled && is_ready_);
    if (CWnd* p = GetDlgItem(IDC_BTN_ENDTIME)) p->EnableWindow(enabled && is_ready_);
    if (CWnd* p = GetDlgItem(IDC_BTN_VERSION)) p->EnableWindow(enabled && is_ready_);
    if (CWnd* p = GetDlgItem(IDC_BTN_WEB_LOGIN)) p->EnableWindow(enabled && is_ready_);
    if (CWnd* p = GetDlgItem(IDC_BTN_LOGIN)) p->EnableWindow(enabled && is_ready_);
    if (CWnd* p = GetDlgItem(IDC_BTN_TAB_ACTION)) p->EnableWindow(enabled && is_ready_);
}

bool CLoginDlg::CodeEnabledFromApiData(const std::string& data) {
    if (data.empty()) return true;  // Mac default
    std::string s = data;
    for (char& c : s) c = static_cast<char>(tolower(static_cast<unsigned char>(c)));
    return s.find("checked") != std::string::npos || s == "true" || s == "1";
}

void CLoginDlg::UpdateCaptchaVisibility() {
    // Layout-independent: only show the captcha set of the current tab.
    const int tabs[] = {0, 1, 2, 3, 4, 5, 8, 9, 10, 12};
    for (int tab : tabs) {
        const bool show = (tab == current_tab_) && is_code_enabled_;

        const int labelId = CaptchaLabelIdForTab(tab);
        if (labelId != -1) {
            if (CWnd* p = GetDlgItem(labelId)) p->ShowWindow(show ? SW_SHOW : SW_HIDE);
        }

        const int inputId = CaptchaInputIdForTab(tab);
        if (inputId != -1) {
            if (CWnd* p = GetDlgItem(inputId)) p->ShowWindow(show ? SW_SHOW : SW_HIDE);
        }

        const int picId = CaptchaPicIdForTab(tab);
        if (picId != -1) {
            if (CWnd* p = GetDlgItem(picId)) p->ShowWindow(show ? SW_SHOW : SW_HIDE);
        }

        const int refreshBtnId = CaptchaRefreshBtnIdForTab(tab);
        if (refreshBtnId != -1) {
            if (CWnd* p = GetDlgItem(refreshBtnId)) p->ShowWindow(show ? SW_SHOW : SW_HIDE);
        }
    }

    // Show OTP "发送验证码" only on OTP-style tabs.
    // Password login / register / recover password / feedback: no "send code".
    const bool showSendCode =
        (current_tab_ == 1 || current_tab_ == 2 || current_tab_ == 4 || current_tab_ == 5 ||
         current_tab_ == 8 || current_tab_ == 9);
    if (CWnd* p = GetDlgItem(IDC_BTN_SEND_CODE)) {
        p->ShowWindow(showSendCode ? SW_SHOW : SW_HIDE);
    }
}

void CLoginDlg::BootstrapAndInitUI() {
    try {
        is_ready_ = client_.bootstrap();
    } catch (...) {
        is_ready_ = false;
    }

    if (!is_ready_) {
        SetServiceStatus(L"服务未连接");
        SetLoginStatus(L"初始化失败");
        if (CWnd* notice = GetDlgItem(IDC_EDIT_NOTICE)) {
            notice->SetWindowTextW(L"初始化失败：无法连接或获取 BSphpSeSsL");
        }
        return;
    }

    SetServiceStatus(L"服务已连接");
    SetLoginStatus(L"等待登录");

    // Notice
    bsphp::ApiResponse n = client_.get_notice();
    if (CWnd* notice = GetDlgItem(IDC_EDIT_NOTICE)) {
        std::wstring wn = WideFromUtf8(n.data);
        notice->SetWindowTextW(wn.c_str());
    }

    // Code enabled switch
    bsphp::ApiResponse c = client_.get_code_enabled("INGES_LOGIN");
    is_code_enabled_ = CodeEnabledFromApiData(c.data);
    UpdateCaptchaVisibility();

    // Load captcha
    if (is_code_enabled_) {
        captcha_refresh_tick_ = static_cast<int>(time(nullptr));
        RefreshCaptchaImage();
    }
}

void CLoginDlg::RefreshCaptchaImage() {
    if (!is_ready_ || !is_code_enabled_) return;
    if (client_.BSphpSeSsL.empty()) return;

    std::string url = client_.code_url + client_.BSphpSeSsL + "&_=" + std::to_string(captcha_refresh_tick_);
    std::wstring wurl = WideFromUtf8(url);
    std::wstring tmp = GetTempFilePath(L"bsphp_captcha.png");

    LoadCaptchaByDownloadToStatic(wurl, tmp);
}

void CLoginDlg::LoadCaptchaByDownloadToStatic(const std::wstring& url, const std::wstring& tmpFile) {
    DeleteOldBitmap(captcha_bmp_);

    const int tab = current_tab_;
    const int picId = CaptchaPicIdForTab(tab);
    const int labelId = CaptchaLabelIdForTab(tab);
    const int inputId = CaptchaInputIdForTab(tab);
    const int refreshBtnId = CaptchaRefreshBtnIdForTab(tab);

    DebugLogW(L"[captcha] start tab=%d enabled=%d url=%s",
               tab,
               is_code_enabled_ ? 1 : 0,
               url.c_str());

    DebugLogW(L"[captcha] ids pic=%d label=%d input=%d refreshBtn=%d",
               picId, labelId, inputId, refreshBtnId);

    auto logCtrl = [&](int id, const wchar_t* name) {
        CWnd* p = GetDlgItem(id);
        if (!p) {
            DebugLogW(L"[captcha] ctrl %s id=%d not found", name, id);
            return;
        }
        DebugLogW(L"[captcha] ctrl %s id=%d found visible=%d", name, id, p->IsWindowVisible() ? 1 : 0);
    };
    if (picId != -1) logCtrl(picId, L"pic");
    if (labelId != -1) logCtrl(labelId, L"label");
    if (inputId != -1) logCtrl(inputId, L"input");
    if (refreshBtnId != -1) logCtrl(refreshBtnId, L"refresh");

    HRESULT hr = S_OK;
    try {
        hr = URLDownloadToFileW(nullptr, url.c_str(), tmpFile.c_str(), 0, nullptr);
    } catch (...) {
        DebugLogW(L"[captcha] URLDownloadToFileW threw exception");
        SetLoginStatus(L"验证码下载异常");
        // Even if download failed, force-show the area so we can confirm visibility.
        if (picId != -1) if (CWnd* p = GetDlgItem(picId)) p->ShowWindow(SW_SHOW);
        if (labelId != -1) if (CWnd* p = GetDlgItem(labelId)) p->ShowWindow(SW_SHOW);
        if (inputId != -1) if (CWnd* p = GetDlgItem(inputId)) p->ShowWindow(SW_SHOW);
        if (refreshBtnId != -1) if (CWnd* p = GetDlgItem(refreshBtnId)) p->ShowWindow(SW_SHOW);
        return;
    }

    if (FAILED(hr)) {
        DebugLogW(L"[captcha] URLDownloadToFileW failed hr=0x%08X", static_cast<unsigned long>(hr));
        SetLoginStatus(L"验证码下载失败");
        return;
    }

    InitGdiplusOnce();
    Gdiplus::Bitmap bmp(tmpFile.c_str());
    if (bmp.GetLastStatus() != Gdiplus::Ok) {
        SetLoginStatus(L"验证码图片解析失败");
        return;
    }

    HBITMAP hBmp = nullptr;
    Gdiplus::Color bg(0, 0, 0, 0);
    if (bmp.GetHBITMAP(bg, &hBmp) != Gdiplus::Ok || !hBmp) {
        SetLoginStatus(L"验证码位图生成失败");
        return;
    }

    captcha_bmp_ = hBmp;
    // Do NOT rely on CStatic subclassing/DDX_Control.
    // Use raw Win32 message to set bitmap image.
    if (CWnd* pic = GetDlgItem(picId)) {
        pic->ShowWindow(SW_SHOW);
        pic->BringWindowToTop();
        ::SendMessageW(pic->GetSafeHwnd(), STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)captcha_bmp_);
        pic->Invalidate();
        pic->RedrawWindow();
    } else {
        DebugLogW(L"[captcha] pic control not found, id=%d", picId);
    }

    // Ensure only current-tab captcha controls are visible (avoid overlap).
    UpdateCaptchaVisibility();
}

void CLoginDlg::OnBnClickedCaptchaRefresh() {
    if (!is_ready_ || !is_code_enabled_) return;
    const int nowTick = static_cast<int>(time(nullptr));
    if (current_tab_ >= 0 && current_tab_ < 13) {
        captcha_refresh_tick_by_tab_[current_tab_] = nowTick;
    }
    captcha_refresh_tick_ = nowTick;
    RefreshCaptchaImage();
}

void CLoginDlg::OnBnClickedNetTest() {
    if (!is_ready_) return;
    bsphp::ApiResponse r = client_.connect();
    // connect() returns ApiResponse with data == "1"
    if (r.data == "1") {
        SetServiceStatus(L"连接成功");
    } else {
        SetServiceStatus(L"连接失败");
    }
}

void CLoginDlg::OnBnClickedEndTime() {
    if (!is_ready_) return;
    bsphp::ApiResponse r = client_.get_end_time();
    std::wstring w = WideFromUtf8(r.data);
    CString s;
    s.Format(L"到期时间：%s", w.c_str());
    SetLoginStatus(s);
}

void CLoginDlg::OnBnClickedVersion() {
    if (!is_ready_) return;
    bsphp::ApiResponse v = client_.get_version();
    std::wstring w = WideFromUtf8(v.data);
    CString s;
    if (!v.data.empty()) {
        s.Format(L"当前版本：%s", w.c_str());
    } else {
        s = L"获取版本失败";
    }
    SetLoginStatus(s);
}

void CLoginDlg::OnBnClickedWebLogin() {
    if (!is_ready_) return;
    if (client_.BSphpSeSsL.empty()) {
        AfxMessageBox(L"尚未获取会话 BSphpSeSsL。");
        return;
    }
    std::string url = bsphp_login_demo::MakeWebLoginUrl(client_.BSphpSeSsL);
    std::wstring wurl = WideFromUtf8(url);
    if (wurl.empty() && !url.empty()) {
        wurl.assign(url.begin(), url.end());
    }
    CWebLoginDlg dlg(this, &client_, wurl);
    dlg.DoModal();
}

void CLoginDlg::NotifyWebLoginSucceeded() {
    SetLoginStatus(L"Web登录成功");
    OpenOrFocusConsole();
}

void CLoginDlg::OpenOrFocusConsole() {
    if (console_ && ::IsWindow(console_->GetSafeHwnd())) {
        console_->SetForegroundWindow();
        return;
    }

    // compute end time
    bsphp::ApiResponse r = client_.get_end_time();
    std::string endTime = r.data.empty() ? "-" : r.data;

    console_ = new CConsoleDlg(&client_, endTime, this);
    console_->Create(IDD_CONSOLE_WIN, this);
    console_->ShowWindow(SW_SHOW);
}

void CLoginDlg::OnBnClickedLogin() {
    if (!is_ready_) return;

    std::string user = GetEditUtf8(IDC_EDIT_F1);
    std::string pwd = GetEditUtf8(IDC_EDIT_F2);
    const int inputId = CaptchaInputIdForTab(current_tab_);
    std::string code = (is_code_enabled_ && inputId != -1) ? GetEditUtf8(inputId) : "";

    SetControlsEnabled(FALSE);

    bsphp::ApiResponse r = client_.login_lg(user, pwd, code, machine_code_, machine_code_);

    if (r.code == "1011" || r.code == "9908") {
        std::wstring msg = L"登录成功";
        if (r.code == "9908") {
            msg = L"使用已经到期（仍视为已登录）";
        }
        SetLoginStatus(msg.c_str());
        OpenOrFocusConsole();
    } else {
        std::wstring wc = WideFromUtf8(r.code);
        CString s = L"登录失败(code=";
        s += wc.c_str();
        s += L")";
        SetLoginStatus(s);
    }

    SetControlsEnabled(TRUE);
}

void CLoginDlg::OnConsoleLogoutCompleted() {
    // reset status
    is_ready_ = client_.BSphpSeSsL.size() > 0;
    SetServiceStatus(is_ready_ ? L"已重新初始化" : L"服务未连接");
    SetLoginStatus(L"已注销");
}

void CLoginDlg::OnTcnSelchangeMainTab(NMHDR* pNMHDR, LRESULT* pResult) {
    (void)pNMHDR;
    CWnd* wnd = GetDlgItem(IDC_TAB_MAIN);
    if (!wnd) return;
    CTabCtrl* tab = static_cast<CTabCtrl*>(wnd);
    const int sel = tab->GetCurSel();
    ApplyTabUI(sel);
    *pResult = 0;
}

CString CLoginDlg::GetEditText(int id) const {
    CString s;
    if (CWnd* p = const_cast<CLoginDlg*>(this)->GetDlgItem(id)) p->GetWindowTextW(s);
    return s;
}

std::string CLoginDlg::GetEditUtf8(int id) const {
    return Utf8FromWide(GetEditText(id).GetString());
}

void CLoginDlg::SetLabelText(int id, const wchar_t* text) {
    if (CWnd* p = GetDlgItem(id)) p->SetWindowTextW(text);
}

void CLoginDlg::SetControlVisible(int id, bool visible) {
    if (CWnd* p = GetDlgItem(id)) p->ShowWindow(visible ? SW_SHOW : SW_HIDE);
}

void CLoginDlg::SetEditPasswordStyle(int editId, bool password) {
    if (CEdit* e = dynamic_cast<CEdit*>(GetDlgItem(editId))) {
        e->SetPasswordChar(password ? L'*' : 0);
        e->Invalidate();
    }
}

void CLoginDlg::SetEditMultiLineStyle(int editId, bool multiline) {
    CWnd* p = GetDlgItem(editId);
    if (!p) return;
    LONG_PTR st = ::GetWindowLongPtr(p->GetSafeHwnd(), GWL_STYLE);
    if (multiline) {
        st |= ES_MULTILINE;
    } else {
        st &= ~ES_MULTILINE;
    }
    ::SetWindowLongPtr(p->GetSafeHwnd(), GWL_STYLE, st);
}

void CLoginDlg::ApplyTabUI(int tab) {
    // Save old tab captcha input before switching (per-tab independence).
    const int oldTab = current_tab_;
    if (oldTab >= 0 && oldTab < 13) {
        const int oldInputId = CaptchaInputIdForTab(oldTab);
        if (oldInputId != -1) {
            if (CWnd* p = GetDlgItem(oldInputId)) {
                CString cur;
                p->GetWindowTextW(cur);
                captcha_value_by_tab_[oldTab] = Utf8FromWide(cur.GetString());
            }
        }
    }

    current_tab_ = tab;
    for (int i = 0; i < 8; ++i) {
        SetLabelText(IDC_ST_F1 + i, L"");
        if (CWnd* e = GetDlgItem(IDC_EDIT_F1 + i)) e->SetWindowTextW(L"");
        SetControlVisible(IDC_ST_F1 + i, false);
        SetControlVisible(IDC_EDIT_F1 + i, false);
        SetEditPasswordStyle(IDC_EDIT_F1 + i, false);
        SetEditMultiLineStyle(IDC_EDIT_F1 + i, false);
    }
    SetControlVisible(IDC_CHK_OPTION1, false);
    SetControlVisible(IDC_BTN_SEND_CODE, false);
    if (CWnd* p = GetDlgItem(IDC_BTN_TAB_ACTION)) p->SetWindowTextW(L"执行当前Tab");

    auto showField = [&](int idx, const wchar_t* label, bool pwd = false) {
        SetLabelText(IDC_ST_F1 + idx, label);
        SetControlVisible(IDC_ST_F1 + idx, true);
        SetControlVisible(IDC_EDIT_F1 + idx, true);
        SetEditPasswordStyle(IDC_EDIT_F1 + idx, pwd);
    };

    switch (tab) {
    case 0:
        showField(0, L"账号");
        showField(1, L"密码", true);
        // 密码登录使用 Tab 内「登录」按钮（IDC_BTN_LOGIN），底部「执行当前 Tab」在此 Tab 隐藏。
        is_code_enabled_ = QueryCodeEnabledForTab(tab, true);
        break;
    case 1:
        showField(0, L"手机号");
        showField(1, L"区号");
        showField(2, L"短信码");
        showField(3, L"key");
        showField(4, L"maxoror");
        SetControlVisible(IDC_BTN_SEND_CODE, true);
        if (CWnd* p = GetDlgItem(IDC_BTN_TAB_ACTION)) p->SetWindowTextW(L"短信登录");
        is_code_enabled_ = QueryCodeEnabledForTab(tab, true);
        break;
    case 2:
        showField(0, L"邮箱");
        showField(1, L"邮箱码");
        showField(2, L"key");
        showField(3, L"maxoror");
        SetControlVisible(IDC_BTN_SEND_CODE, true);
        if (CWnd* p = GetDlgItem(IDC_BTN_TAB_ACTION)) p->SetWindowTextW(L"邮箱登录");
        is_code_enabled_ = QueryCodeEnabledForTab(tab, true);
        break;
    case 3:
        showField(0, L"账号");
        showField(1, L"密码", true);
        showField(2, L"确认密码", true);
        showField(3, L"手机号");
        showField(4, L"密保问题");
        showField(5, L"密保答案");
        showField(7, L"推广码");
        if (CWnd* p = GetDlgItem(IDC_BTN_TAB_ACTION)) p->SetWindowTextW(L"账号注册");
        is_code_enabled_ = QueryCodeEnabledForTab(tab, true);
        break;
    case 4:
        showField(0, L"账号");
        showField(1, L"手机号");
        showField(2, L"区号");
        showField(3, L"短信码");
        showField(4, L"密码", true);
        showField(5, L"确认密码", true);
        showField(6, L"key");
        SetControlVisible(IDC_BTN_SEND_CODE, true);
        if (CWnd* p = GetDlgItem(IDC_BTN_TAB_ACTION)) p->SetWindowTextW(L"短信注册");
        is_code_enabled_ = QueryCodeEnabledForTab(tab, true);
        break;
    case 5:
        showField(0, L"账号");
        showField(1, L"邮箱");
        showField(2, L"邮箱码");
        showField(3, L"密码", true);
        showField(4, L"确认密码", true);
        showField(5, L"key");
        SetControlVisible(IDC_BTN_SEND_CODE, true);
        if (CWnd* p = GetDlgItem(IDC_BTN_TAB_ACTION)) p->SetWindowTextW(L"邮箱注册");
        is_code_enabled_ = QueryCodeEnabledForTab(tab, true);
        break;
    case 6:
        showField(0, L"账号");
        showField(1, L"密码", true);
        if (CWnd* p = GetDlgItem(IDC_BTN_TAB_ACTION)) p->SetWindowTextW(L"解绑");
        is_code_enabled_ = false;
        break;
    case 7:
        showField(0, L"账号");
        showField(1, L"密码", true);
        showField(2, L"充值卡");
        showField(3, L"卡密", true);
        SetControlVisible(IDC_CHK_OPTION1, true);
        if (CWnd* p = GetDlgItem(IDC_BTN_TAB_ACTION)) p->SetWindowTextW(L"充值");
        is_code_enabled_ = false;
        break;
    case 8:
        showField(0, L"手机号");
        showField(1, L"区号");
        showField(2, L"短信码");
        showField(3, L"新密码", true);
        showField(4, L"确认密码", true);
        SetControlVisible(IDC_BTN_SEND_CODE, true);
        if (CWnd* p = GetDlgItem(IDC_BTN_TAB_ACTION)) p->SetWindowTextW(L"短信找回");
        is_code_enabled_ = QueryCodeEnabledForTab(tab, true);
        break;
    case 9:
        showField(0, L"邮箱");
        showField(1, L"邮箱码");
        showField(2, L"新密码", true);
        showField(3, L"确认密码", true);
        SetControlVisible(IDC_BTN_SEND_CODE, true);
        if (CWnd* p = GetDlgItem(IDC_BTN_TAB_ACTION)) p->SetWindowTextW(L"邮箱找回");
        is_code_enabled_ = QueryCodeEnabledForTab(tab, true);
        break;
    case 10:
        showField(0, L"账号");
        showField(1, L"新密码", true);
        showField(2, L"确认密码", true);
        showField(3, L"密保问题");
        showField(4, L"密保答案");
        if (CWnd* p = GetDlgItem(IDC_BTN_TAB_ACTION)) p->SetWindowTextW(L"找回密码");
        is_code_enabled_ = QueryCodeEnabledForTab(tab, true);
        break;
    case 11:
        showField(0, L"账号");
        showField(1, L"旧密码", true);
        showField(2, L"新密码", true);
        showField(3, L"确认密码", true);
        if (CWnd* p = GetDlgItem(IDC_BTN_TAB_ACTION)) p->SetWindowTextW(L"修改密码");
        is_code_enabled_ = false;
        break;
    case 12:
        showField(0, L"账号");
        showField(1, L"密码", true);
        showField(2, L"标题");
        showField(3, L"联系方式");
        showField(4, L"类型");
        showField(5, L"内容");
        SetEditMultiLineStyle(IDC_EDIT_F6, true);
        if (CWnd* p = GetDlgItem(IDC_BTN_TAB_ACTION)) p->SetWindowTextW(L"提交反馈");
        is_code_enabled_ = QueryCodeEnabledForTab(tab, true);
        break;
    default:
        break;
    }

    // 仅密码登录 Tab 显示 Tab 内「登录」；其它 Tab 使用底部「执行当前 Tab」。
    SetControlVisible(IDC_BTN_LOGIN, tab == 0);
    SetControlVisible(IDC_BTN_TAB_ACTION, tab != 0);

    // 密码登录：演示默认账号/密码（与常见 BSPHP 演示一致）
    if (tab == 0) {
        if (CWnd* p = GetDlgItem(IDC_EDIT_F1)) p->SetWindowTextW(L"admin");
        if (CWnd* p = GetDlgItem(IDC_EDIT_F2)) p->SetWindowTextW(L"admin");
    }

    // Default phone area: sms login/recover 用区号=86
    // SMS register (tab=4) uses IDC_EDIT_F3 (idx=2) as area.
    if (tab == 1 || tab == 8) {
        if (CWnd* p = GetDlgItem(IDC_EDIT_F2)) p->SetWindowTextW(L"86");
    }
    if (tab == 4) {
        if (CWnd* p = GetDlgItem(IDC_EDIT_F3)) p->SetWindowTextW(L"86");
    }
    if (tab == 1 || tab == 2) {
        std::wstring w = WideFromUtf8(machine_code_);
        if (tab == 1) {
            if (CWnd* p = GetDlgItem(IDC_EDIT_F4)) p->SetWindowTextW(w.c_str());
            if (CWnd* p = GetDlgItem(IDC_EDIT_F5)) p->SetWindowTextW(w.c_str());
        } else {
            if (CWnd* p = GetDlgItem(IDC_EDIT_F3)) p->SetWindowTextW(w.c_str());
            if (CWnd* p = GetDlgItem(IDC_EDIT_F4)) p->SetWindowTextW(w.c_str());
        }
    }
    if (tab == 4 || tab == 5) {
        std::wstring w = WideFromUtf8(machine_code_);
        if (CWnd* p = GetDlgItem(tab == 4 ? IDC_EDIT_F7 : IDC_EDIT_F6)) p->SetWindowTextW(w.c_str());
    }
    if (tab == 12) {
        if (CWnd* p = GetDlgItem(IDC_EDIT_F5)) p->SetWindowTextW(L"建议反馈");
    }

    // Restore captcha input for this tab and bind refresh tick.
    {
        const int inputId = CaptchaInputIdForTab(tab);
        if (inputId != -1) {
            if (CWnd* p = GetDlgItem(inputId)) {
                std::wstring w = WideFromUtf8(captcha_value_by_tab_[tab]);
                p->SetWindowTextW(w.c_str());
            }
        }
    }
    captcha_refresh_tick_ = (tab >= 0 && tab < 13) ? captcha_refresh_tick_by_tab_[tab] : 0;
    if (captcha_refresh_tick_ == 0) {
        // First time showing captcha for this tab.
        captcha_refresh_tick_ = static_cast<int>(time(nullptr));
        if (tab >= 0 && tab < 13) captcha_refresh_tick_by_tab_[tab] = captcha_refresh_tick_;
    }

    UpdateCaptchaVisibility();
    // When switching tabs and captcha is enabled, refresh the image so user sees the correct captcha.
    if (is_code_enabled_) {
        RefreshCaptchaImage();
    }
    SetControlsEnabled(TRUE);
}

bool CLoginDlg::QueryCodeEnabledForTab(int tab, bool fallback) const {
    std::string t;
    // Forced captcha tabs（图片验证码始终开启）:
    // - SMS login / Email login
    // - SMS register / Email register / SMS recover / Email recover
    if (tab == 1 || tab == 2 || tab == 4 || tab == 5 || tab == 8 || tab == 9) {
        return true;
    }

    // Interface-driven captcha tabs:
    // - Password login => INGES_LOGIN
    // - Account registration => INGES_RE
    // - Recover password => INGES_MACK
    // - Feedback => INGES_SAY
    switch (tab) {
    case 0:
        t = "INGES_LOGIN";
        break;
    case 3:
        t = "INGES_RE";
        break;
    case 10:
        t = "INGES_MACK";
        break;
    case 12:
        t = "INGES_SAY";
        break;
    default:
        return fallback;
    }
    bsphp::ApiResponse r = const_cast<CLoginDlg*>(this)->client_.get_code_enabled(t);
    if (r.data.empty()) return fallback;
    return CodeEnabledFromApiData(r.data);
}

int CLoginDlg::CaptchaInputIdForTab(int tab) const {
    switch (tab) {
    case 0: return IDC_EDIT_CAPTCHA_INPUT;
    case 1: return IDC_EDIT_CAPTCHA_INPUT_1;
    case 2: return IDC_EDIT_CAPTCHA_INPUT_2;
    case 3: return IDC_EDIT_CAPTCHA_INPUT_3;
    case 4: return IDC_EDIT_CAPTCHA_INPUT_4;
    case 5: return IDC_EDIT_CAPTCHA_INPUT_5;
    case 8: return IDC_EDIT_CAPTCHA_INPUT_8;
    case 9: return IDC_EDIT_CAPTCHA_INPUT_9;
    case 10: return IDC_EDIT_CAPTCHA_INPUT_10;
    case 12: return IDC_EDIT_CAPTCHA_INPUT_12;
    default: return -1;
    }
}

int CLoginDlg::CaptchaPicIdForTab(int tab) const {
    switch (tab) {
    case 0: return IDC_PIC_CAPTCHA;
    case 1: return IDC_PIC_CAPTCHA_1;
    case 2: return IDC_PIC_CAPTCHA_2;
    case 3: return IDC_PIC_CAPTCHA_3;
    case 4: return IDC_PIC_CAPTCHA_4;
    case 5: return IDC_PIC_CAPTCHA_5;
    case 8: return IDC_PIC_CAPTCHA_8;
    case 9: return IDC_PIC_CAPTCHA_9;
    case 10: return IDC_PIC_CAPTCHA_10;
    case 12: return IDC_PIC_CAPTCHA_12;
    default: return -1;
    }
}

int CLoginDlg::CaptchaLabelIdForTab(int tab) const {
    switch (tab) {
    case 0: return IDC_ST_CAPTCHA_LABEL_0;
    case 1: return IDC_ST_CAPTCHA_LABEL_1;
    case 2: return IDC_ST_CAPTCHA_LABEL_2;
    case 3: return IDC_ST_CAPTCHA_LABEL_3;
    case 4: return IDC_ST_CAPTCHA_LABEL_4;
    case 5: return IDC_ST_CAPTCHA_LABEL_5;
    case 8: return IDC_ST_CAPTCHA_LABEL_8;
    case 9: return IDC_ST_CAPTCHA_LABEL_9;
    case 10: return IDC_ST_CAPTCHA_LABEL_10;
    case 12: return IDC_ST_CAPTCHA_LABEL_12;
    default: return -1;
    }
}

int CLoginDlg::CaptchaRefreshBtnIdForTab(int tab) const {
    switch (tab) {
    case 0: return IDC_BTN_CAPTCHA_REFRESH;
    case 1: return IDC_BTN_CAPTCHA_REFRESH_1;
    case 2: return IDC_BTN_CAPTCHA_REFRESH_2;
    case 3: return IDC_BTN_CAPTCHA_REFRESH_3;
    case 4: return IDC_BTN_CAPTCHA_REFRESH_4;
    case 5: return IDC_BTN_CAPTCHA_REFRESH_5;
    case 8: return IDC_BTN_CAPTCHA_REFRESH_8;
    case 9: return IDC_BTN_CAPTCHA_REFRESH_9;
    case 10: return IDC_BTN_CAPTCHA_REFRESH_10;
    case 12: return IDC_BTN_CAPTCHA_REFRESH_12;
    default: return -1;
    }
}

void CLoginDlg::RunSendCode() {
    bsphp::ApiResponse r;
    if (current_tab_ == 1 || current_tab_ == 4 || current_tab_ == 8) {
        // 与 Mac DemoBSphpOTPScene：login / register / reset 一致（send_sms.lg 的 scene）
        std::string scene = current_tab_ == 4 ? "register" : (current_tab_ == 8 ? "reset" : "login");
        const int inputId = CaptchaInputIdForTab(current_tab_);
        std::string coode = (is_code_enabled_ && inputId != -1) ? GetEditUtf8(inputId) : "";
        // Tab1/8: F1=手机号 F2=区号 | Tab4 短信注册: F1=账号 F2=手机号 F3=区号（与 register_sms.lg / ApplyTabUI 一致）
        std::string mobile, area;
        if (current_tab_ == 4) {
            mobile = GetEditUtf8(IDC_EDIT_F2);
            area = GetEditUtf8(IDC_EDIT_F3);
        } else {
            mobile = GetEditUtf8(IDC_EDIT_F1);
            area = GetEditUtf8(IDC_EDIT_F2);
        }
        r = client_.api_call("send_sms.lg", {{"scene", scene}, {"mobile", mobile}, {"area", area}, {"coode", coode}});
    } else if (current_tab_ == 2 || current_tab_ == 5 || current_tab_ == 9) {
        std::string scene = current_tab_ == 5 ? "register" : (current_tab_ == 9 ? "reset" : "login");
        std::string email = current_tab_ == 5 ? GetEditUtf8(IDC_EDIT_F2) : GetEditUtf8(IDC_EDIT_F1);
        const int inputId = CaptchaInputIdForTab(current_tab_);
        std::string coode = (is_code_enabled_ && inputId != -1) ? GetEditUtf8(inputId) : "";
        r = client_.api_call("send_email.lg", {{"scene", scene}, {"email", email}, {"coode", coode}});
    } else {
        return;
    }
    SetLoginStatus(WideFromUtf8(r.data).c_str());
}

void CLoginDlg::RunCurrentTabAction() {
    bsphp::ApiResponse r;
    const int inputId = CaptchaInputIdForTab(current_tab_);
    std::string captcha_value = (is_code_enabled_ && inputId != -1) ? GetEditUtf8(inputId) : "";
    switch (current_tab_) {
    case 0: r = client_.login_lg(GetEditUtf8(IDC_EDIT_F1), GetEditUtf8(IDC_EDIT_F2), captcha_value, machine_code_, machine_code_); break;
    case 1: r = client_.api_call("login_sms.lg", {{"mobile", GetEditUtf8(IDC_EDIT_F1)}, {"area", GetEditUtf8(IDC_EDIT_F2)}, {"sms_code", GetEditUtf8(IDC_EDIT_F3)}, {"key", GetEditUtf8(IDC_EDIT_F4)}, {"maxoror", GetEditUtf8(IDC_EDIT_F5)}, {"coode", captcha_value}}); break;
    case 2: r = client_.api_call("login_email.lg", {{"email", GetEditUtf8(IDC_EDIT_F1)}, {"email_code", GetEditUtf8(IDC_EDIT_F2)}, {"key", GetEditUtf8(IDC_EDIT_F3)}, {"maxoror", GetEditUtf8(IDC_EDIT_F4)}, {"coode", captcha_value}}); break;
    case 3: r = client_.api_call("registration.lg", {{"user", GetEditUtf8(IDC_EDIT_F1)}, {"pwd", GetEditUtf8(IDC_EDIT_F2)}, {"pwdb", GetEditUtf8(IDC_EDIT_F3)}, {"mobile", GetEditUtf8(IDC_EDIT_F4)}, {"mibao_wenti", GetEditUtf8(IDC_EDIT_F5)}, {"mibao_daan", GetEditUtf8(IDC_EDIT_F6)}, {"coode", captcha_value}, {"extension", GetEditUtf8(IDC_EDIT_F8)}}); break;
    case 4: r = client_.api_call("register_sms.lg", {{"user", GetEditUtf8(IDC_EDIT_F1)}, {"mobile", GetEditUtf8(IDC_EDIT_F2)}, {"area", GetEditUtf8(IDC_EDIT_F3)}, {"sms_code", GetEditUtf8(IDC_EDIT_F4)}, {"pwd", GetEditUtf8(IDC_EDIT_F5)}, {"pwdb", GetEditUtf8(IDC_EDIT_F6)}, {"key", GetEditUtf8(IDC_EDIT_F7)}, {"coode", captcha_value}}); break;
    case 5: r = client_.api_call("register_email.lg", {{"user", GetEditUtf8(IDC_EDIT_F1)}, {"email", GetEditUtf8(IDC_EDIT_F2)}, {"email_code", GetEditUtf8(IDC_EDIT_F3)}, {"pwd", GetEditUtf8(IDC_EDIT_F4)}, {"pwdb", GetEditUtf8(IDC_EDIT_F5)}, {"key", GetEditUtf8(IDC_EDIT_F6)}, {"coode", captcha_value}}); break;
    case 6: r = client_.api_call("jiekey.lg", {{"user", GetEditUtf8(IDC_EDIT_F1)}, {"pwd", GetEditUtf8(IDC_EDIT_F2)}}); break;
    case 7: r = client_.api_call("chong.lg", {{"user", GetEditUtf8(IDC_EDIT_F1)}, {"userpwd", GetEditUtf8(IDC_EDIT_F2)}, {"ka", GetEditUtf8(IDC_EDIT_F3)}, {"pwd", GetEditUtf8(IDC_EDIT_F4)}, {"userset", (IsDlgButtonChecked(IDC_CHK_OPTION1) == BST_CHECKED) ? "1" : "0"}}); break;
    case 8: r = client_.api_call("resetpwd_sms.lg", {{"mobile", GetEditUtf8(IDC_EDIT_F1)}, {"area", GetEditUtf8(IDC_EDIT_F2)}, {"sms_code", GetEditUtf8(IDC_EDIT_F3)}, {"pwd", GetEditUtf8(IDC_EDIT_F4)}, {"pwdb", GetEditUtf8(IDC_EDIT_F5)}, {"coode", captcha_value}}); break;
    case 9: r = client_.api_call("resetpwd_email.lg", {{"email", GetEditUtf8(IDC_EDIT_F1)}, {"email_code", GetEditUtf8(IDC_EDIT_F2)}, {"pwd", GetEditUtf8(IDC_EDIT_F3)}, {"pwdb", GetEditUtf8(IDC_EDIT_F4)}, {"coode", captcha_value}}); break;
    case 10: r = client_.api_call("backto.lg", {{"user", GetEditUtf8(IDC_EDIT_F1)}, {"pwd", GetEditUtf8(IDC_EDIT_F2)}, {"pwdb", GetEditUtf8(IDC_EDIT_F3)}, {"wenti", GetEditUtf8(IDC_EDIT_F4)}, {"daan", GetEditUtf8(IDC_EDIT_F5)}, {"coode", captcha_value}}); break;
    case 11: r = client_.api_call("password.lg", {{"user", GetEditUtf8(IDC_EDIT_F1)}, {"pwd", GetEditUtf8(IDC_EDIT_F2)}, {"pwda", GetEditUtf8(IDC_EDIT_F3)}, {"pwdb", GetEditUtf8(IDC_EDIT_F4)}, {"img", captcha_value}}); break;
    case 12: r = client_.api_call("liuyan.in", {{"user", GetEditUtf8(IDC_EDIT_F1)}, {"pwd", GetEditUtf8(IDC_EDIT_F2)}, {"table", GetEditUtf8(IDC_EDIT_F3)}, {"qq", GetEditUtf8(IDC_EDIT_F4)}, {"leix", GetEditUtf8(IDC_EDIT_F5)}, {"txt", GetEditUtf8(IDC_EDIT_F6)}, {"coode", captcha_value}}); break;
    default: return;
    }
    if ((r.code == "1011" || r.code == "9908") && !r.sessl.empty()) {
        client_.BSphpSeSsL = r.sessl;
        OpenOrFocusConsole();
    }
    std::wstring wc = WideFromUtf8(r.code);
    std::wstring wd = WideFromUtf8(r.data);
    CString s = L"code=";
    s += wc.c_str();
    s += L" data=";
    s += wd.c_str();
    SetLoginStatus(s);
}

void CLoginDlg::OnBnClickedSendCode() {
    if (!is_ready_) return;
    RunSendCode();
}

void CLoginDlg::OnBnClickedTabAction() {
    if (!is_ready_) return;
    RunCurrentTabAction();
}


