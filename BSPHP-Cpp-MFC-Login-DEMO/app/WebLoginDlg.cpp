#include "pch.h"

#include "WebLoginDlg.h"

#include "LoginDlg.h"

#include "../core/debug_log.h"

#include <wrl.h>

#include <objbase.h>

#include <shellapi.h>

namespace {

// 与 Mac WebLoginWebView：hash 含 login= 时通知宿主；WebView2 使用 chrome.webview.postMessage
const wchar_t kHashMonitorScript[] =
    LR"((function(){
  function checkHash(){
    var h = window.location.hash || '';
    if (h.indexOf('login=') !== -1 && window.chrome && window.chrome.webview) {
      window.chrome.webview.postMessage(h);
    }
  }
  window.addEventListener('hashchange', checkHash);
  if (document.readyState === 'complete') checkHash();
  else window.addEventListener('load', checkHash);
})();)";

}  // namespace

const UINT CWebLoginDlg::WM_WEBLOGIN_SUCCESS = WM_APP + 88;
const UINT CWebLoginDlg::WM_WEBLOGIN_HEARTBEAT = WM_APP + 89;
const UINT CWebLoginDlg::WM_WEBLOGIN_CREATE_CONTROLLER = WM_APP + 86;
const UINT CWebLoginDlg::WM_WEBLOGIN_FINISH_ATTACH = WM_APP + 85;

BEGIN_MESSAGE_MAP(CWebLoginDlg, CDialogEx)
    ON_WM_SIZE()
    ON_WM_SHOWWINDOW()
    ON_WM_DESTROY()
    ON_BN_CLICKED(IDC_BTN_WEBLOGIN_OPEN_BROWSER, &CWebLoginDlg::OnBnClickedOpenBrowser)
    ON_BN_CLICKED(IDC_BTN_WEBLOGIN_HEARTBEAT, &CWebLoginDlg::OnBnClickedHeartbeatBtn)
    ON_MESSAGE(WM_WEBLOGIN_SUCCESS, &CWebLoginDlg::OnWebLoginSuccess)
    ON_MESSAGE(WM_WEBLOGIN_HEARTBEAT, &CWebLoginDlg::OnWebLoginHeartbeat)
    ON_MESSAGE(WM_WEBLOGIN_CREATE_CONTROLLER, &CWebLoginDlg::OnWebLoginCreateController)
    ON_MESSAGE(WM_WEBLOGIN_FINISH_ATTACH, &CWebLoginDlg::OnWebLoginFinishAttach)
END_MESSAGE_MAP()

CWebLoginDlg::CWebLoginDlg(CLoginDlg* owner, bsphp::BsPhp* client, std::wstring navigate_url)
    : CDialogEx(IDD_WEBLOGIN, owner)
    , owner_(owner)
    , client_(client)
    , navigate_url_(std::move(navigate_url)) {}

BOOL CWebLoginDlg::OnInitDialog() {
    CDialogEx::OnInitDialog();

    SetWindowTextW(L"Web登录");

    if (!GetDlgItem(IDC_WEBLOGIN_HOST) || !client_) {
        EndDialog(IDCANCEL);
        return TRUE;
    }

    if (CWnd* urlEdit = GetDlgItem(IDC_EDIT_WEBLOGIN_URL)) {
        urlEdit->SetWindowTextW(navigate_url_.c_str());
    }
    DebugLogW(L"[WebLogin] navigate_url=%s", navigate_url_.c_str());

    if (!CreateWebViewHostWindow()) {
        AfxMessageBox(L"无法创建 WebView 宿主窗口。");
        EndDialog(IDCANCEL);
        return TRUE;
    }

    // 独立用户数据目录，避免默认路径告警并减少与系统浏览器配置冲突
    wchar_t tempRoot[MAX_PATH] = {};
    wchar_t userDataFolder[MAX_PATH] = {};
    if (GetTempPathW(MAX_PATH, tempRoot) > 0) {
        swprintf_s(userDataFolder, L"%sBSPHPLoginWebView2", tempRoot);
        CreateDirectoryW(userDataFolder, nullptr);
    }

    const wchar_t* udf = (userDataFolder[0] != L'\0') ? userDataFolder : nullptr;

    HRESULT hr = RequestCreateWebView2Environment(udf);
    if (FAILED(hr)) {
        DebugLogW(L"[WebLogin] RequestCreateWebView2Environment(custom UDF) hr=0x%08lX, retry default user data folder",
                  (unsigned long)hr);
        hr = RequestCreateWebView2Environment(nullptr);
    }
    if (FAILED(hr)) {
        CString msg;
        msg.Format(
            L"CreateCoreWebView2EnvironmentWithOptions 调用失败。\nHRESULT=0x%08lX\n\n"
            L"请安装 Microsoft Edge WebView2 Runtime（Evergreen）：\n"
            L"https://developer.microsoft.com/microsoft-edge/webview2/\n\n"
            L"若已安装仍失败，可尝试以管理员运行或检查杀毒/权限。",
            (unsigned long)hr);
        AfxMessageBox(msg, MB_ICONWARNING);
        DebugLogW(L"[WebLogin] RequestCreateWebView2Environment failed hr=0x%08lX", (unsigned long)hr);
        EndDialog(IDCANCEL);
    }
    return TRUE;
}

HRESULT CWebLoginDlg::RequestCreateWebView2Environment(const wchar_t* userDataFolderOpt) {
    const HWND dlgHwnd = GetSafeHwnd();
    const HWND hostHwnd = webview_host_hwnd_;
    if (userDataFolderOpt && userDataFolderOpt[0] != L'\0') {
        DebugLogW(L"[WebLogin] RequestCreateWebView2Environment userDataFolder=%s", userDataFolderOpt);
    } else {
        DebugLogW(L"[WebLogin] RequestCreateWebView2Environment userDataFolder=<default>");
    }
    return CreateCoreWebView2EnvironmentWithOptions(
        nullptr,
        userDataFolderOpt,
        nullptr,
        Microsoft::WRL::Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>(
            [dlgHwnd, hostHwnd, this](HRESULT result, ICoreWebView2Environment* env) -> HRESULT {
                if (!::IsWindow(dlgHwnd) || !::IsWindow(hostHwnd)) {
                    return E_ABORT;
                }
                if (FAILED(result) || !env) {
                    CString msg;
                    msg.Format(L"WebView2 环境异步创建失败。\nHRESULT=0x%08lX\n\n请确认已安装 WebView2 Runtime。",
                               (unsigned long)result);
                    AfxMessageBox(msg, MB_ICONWARNING);
                    DebugLogW(L"[WebLogin] env completed callback FAILED hr=0x%08lX", (unsigned long)result);
                    ::PostMessageW(dlgHwnd, WM_COMMAND, MAKEWPARAM(IDCANCEL, BN_CLICKED), 0);
                    return result;
                }
                const DWORD uiThread = ::GetWindowThreadProcessId(dlgHwnd, nullptr);
                const DWORD cur = ::GetCurrentThreadId();
                DebugLogW(L"[WebLogin] env callback thread=%lu uiThread=%lu (defer CREATE_CONTROLLER)", (unsigned long)cur,
                          (unsigned long)uiThread);
                pending_env_.Attach(env);
                if (!::PostMessageW(dlgHwnd, WM_WEBLOGIN_CREATE_CONTROLLER, 0, 0)) {
                    DebugLogW(L"[WebLogin] PostMessage CREATE_CONTROLLER failed");
                    pending_env_.Reset();
                }
                return S_OK;
            })
            .Get());
}

void CWebLoginDlg::StartCreateWebViewController(ICoreWebView2Environment* env) {
    if (!env || !::IsWindow(GetSafeHwnd()) || !::IsWindow(webview_host_hwnd_)) {
        return;
    }
    const HWND dlgHwnd = GetSafeHwnd();
    const HWND hostHwnd = webview_host_hwnd_;
    const HRESULT hrQueue =
        env->CreateCoreWebView2Controller(
            hostHwnd,
            Microsoft::WRL::Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
                [dlgHwnd, hostHwnd, this](HRESULT result2, ICoreWebView2Controller* controller) -> HRESULT {
                    if (!::IsWindow(dlgHwnd) || !::IsWindow(hostHwnd)) {
                        if (controller) {
                            controller->Release();
                        }
                        return E_ABORT;
                    }
                    if (FAILED(result2) || !controller) {
                        CString msg;
                        msg.Format(L"WebView2 控件异步创建失败。\nHRESULT=0x%08lX", (unsigned long)result2);
                        AfxMessageBox(msg, MB_ICONWARNING);
                        DebugLogW(L"[WebLogin] CreateCoreWebView2Controller completed FAILED hr=0x%08lX",
                                  (unsigned long)result2);
                        ::PostMessageW(dlgHwnd, WM_COMMAND, MAKEWPARAM(IDCANCEL, BN_CLICKED), 0);
                        return result2;
                    }
                    pending_controller_.Attach(controller);
                    if (!::PostMessageW(dlgHwnd, WM_WEBLOGIN_FINISH_ATTACH, 0, 0)) {
                        DebugLogW(L"[WebLogin] PostMessage FINISH_ATTACH failed");
                        pending_controller_.Reset();
                    }
                    return S_OK;
                })
                .Get());
    if (FAILED(hrQueue)) {
        CString msg;
        msg.Format(
            L"CreateCoreWebView2Controller 调用失败（未能排队创建）。\nHRESULT=0x%08lX\n\n"
            L"请确认宿主窗口有效，或关闭占用 WebView2 用户目录的其他实例后重试。",
            (unsigned long)hrQueue);
        AfxMessageBox(msg, MB_ICONWARNING);
        DebugLogW(L"[WebLogin] CreateCoreWebView2Controller sync FAILED hr=0x%08lX", (unsigned long)hrQueue);
    }
}

void CWebLoginDlg::FinishAttachCoreWebView(ICoreWebView2Controller* controller) {
    if (!controller || !::IsWindow(GetSafeHwnd()) || !::IsWindow(webview_host_hwnd_)) {
        if (controller) {
            controller->Release();
        }
        return;
    }
    controller_.Attach(controller);
    ICoreWebView2* wv = nullptr;
    HRESULT hr2 = controller_->get_CoreWebView2(&wv);
    if (FAILED(hr2) || !wv) {
        CString msg;
        msg.Format(L"无法获取 CoreWebView2。\nHRESULT=0x%08lX", (unsigned long)hr2);
        AfxMessageBox(msg, MB_ICONWARNING);
        DebugLogW(L"[WebLogin] get_CoreWebView2 FAILED hr=0x%08lX", (unsigned long)hr2);
        ::PostMessageW(GetSafeHwnd(), WM_COMMAND, MAKEWPARAM(IDCANCEL, BN_CLICKED), 0);
        return;
    }
    webview_.Attach(wv);
    ApplyRasterizationScale();
    ApplyDefaultBackgroundColor();
    controller_->put_IsVisible(TRUE);
    ApplyWebViewDefaults();
    SetupWebView();
    ResizeWebView();
    DebugLogW(L"[WebLogin] calling Navigate() len=%zu", navigate_url_.size());
    HRESULT nhr = webview_->Navigate(navigate_url_.c_str());
    if (FAILED(nhr)) {
        DebugLogW(L"[WebLogin] Navigate failed hr=0x%08lX", (unsigned long)nhr);
    }
}

void CWebLoginDlg::ApplyWebViewDefaults() {
    if (!webview_) {
        return;
    }
    Microsoft::WRL::ComPtr<ICoreWebView2Settings> settings;
    if (SUCCEEDED(webview_->get_Settings(settings.GetAddressOf())) && settings) {
        settings->put_IsScriptEnabled(TRUE);
        settings->put_IsWebMessageEnabled(TRUE);
        settings->put_AreDefaultScriptDialogsEnabled(TRUE);
    }
}

void CWebLoginDlg::ApplyRasterizationScale() {
    if (!controller_ || !webview_host_hwnd_) {
        return;
    }
    Microsoft::WRL::ComPtr<ICoreWebView2Controller3> c3;
    if (FAILED(controller_.As(&c3))) {
        return;
    }
    UINT dpi = ::GetDpiForWindow(webview_host_hwnd_);
    if (dpi == 0) {
        dpi = 96;
    }
    const double scale = static_cast<double>(dpi) / 96.0;
    c3->put_RasterizationScale(scale);
    DebugLogW(L"[WebLogin] RasterizationScale=%.3f dpi=%u", scale, dpi);
}

void CWebLoginDlg::ApplyDefaultBackgroundColor() {
    if (!controller_) {
        return;
    }
    Microsoft::WRL::ComPtr<ICoreWebView2Controller2> c2;
    if (FAILED(controller_.As(&c2))) {
        return;
    }
    const COREWEBVIEW2_COLOR white{255, 255, 255, 255};
    c2->put_DefaultBackgroundColor(white);
}

LRESULT CWebLoginDlg::OnWebLoginCreateController(WPARAM, LPARAM) {
    Microsoft::WRL::ComPtr<ICoreWebView2Environment> env = pending_env_;
    pending_env_.Reset();
    if (env && ::IsWindow(GetSafeHwnd()) && ::IsWindow(webview_host_hwnd_)) {
        StartCreateWebViewController(env.Get());
    }
    return 0;
}

LRESULT CWebLoginDlg::OnWebLoginFinishAttach(WPARAM, LPARAM) {
    if (!pending_controller_) {
        return 0;
    }
    ICoreWebView2Controller* raw = pending_controller_.Detach();
    FinishAttachCoreWebView(raw);
    return 0;
}

void CWebLoginDlg::SetupWebView() {
    if (!webview_ || webview_shutdown_) {
        return;
    }

    const HWND dlgHwnd = GetSafeHwnd();

    HRESULT hr = webview_->AddScriptToExecuteOnDocumentCreated(kHashMonitorScript, nullptr);
    (void)hr;

    webview_->add_NavigationStarting(
        Microsoft::WRL::Callback<ICoreWebView2NavigationStartingEventHandler>(
            [dlgHwnd](ICoreWebView2* /*sender*/, ICoreWebView2NavigationStartingEventArgs* args) -> HRESULT {
                if (!::IsWindow(dlgHwnd) || !args) {
                    return S_OK;
                }
                LPWSTR uri = nullptr;
                if (SUCCEEDED(args->get_Uri(&uri)) && uri) {
                    DebugLogW(L"[WebLogin] NavigationStarting uri=%s", uri);
                    CoTaskMemFree(uri);
                }
                return S_OK;
            })
            .Get(),
        &nav_starting_token_);

    webview_->add_WebMessageReceived(
        Microsoft::WRL::Callback<ICoreWebView2WebMessageReceivedEventHandler>(
            [dlgHwnd, this](ICoreWebView2* /*sender*/, ICoreWebView2WebMessageReceivedEventArgs* args) -> HRESULT {
                if (!::IsWindow(dlgHwnd)) {
                    return S_OK;
                }
                if (webview_shutdown_ || !webview_ || !args) {
                    return S_OK;
                }
                wchar_t* msg = nullptr;
                if (SUCCEEDED(args->TryGetWebMessageAsString(&msg)) && msg) {
                    std::wstring w(msg);
                    CoTaskMemFree(msg);
                    if (w.find(L"login=") != std::wstring::npos) {
                        // 勿在 WebMessageReceived 内同步 api_call（阻塞 WebView2 线程，易导致白屏/闪一下）
                        ::PostMessageW(dlgHwnd, WM_WEBLOGIN_HEARTBEAT, 0, 0);
                    }
                }
                return S_OK;
            })
            .Get(),
        &web_msg_token_);

    webview_->add_NavigationCompleted(
        Microsoft::WRL::Callback<ICoreWebView2NavigationCompletedEventHandler>(
            [dlgHwnd, this](ICoreWebView2* /*sender*/, ICoreWebView2NavigationCompletedEventArgs* args) -> HRESULT {
                if (!::IsWindow(dlgHwnd)) {
                    return S_OK;
                }
                if (args) {
                    BOOL ok = FALSE;
                    args->get_IsSuccess(&ok);
                    COREWEBVIEW2_WEB_ERROR_STATUS err = COREWEBVIEW2_WEB_ERROR_STATUS_UNKNOWN;
                    args->get_WebErrorStatus(&err);
                    DebugLogW(L"[WebLogin] NavigationCompleted IsSuccess=%d WebErrorStatus=%d", (int)ok, (int)err);
                }
                if (webview_shutdown_ || !webview_) {
                    return S_OK;
                }
                ResizeWebView();
                webview_->ExecuteScript(
                    LR"((function(){var h=window.location.hash||'';if(h.indexOf('login=')!==-1&&window.chrome&&window.chrome.webview)window.chrome.webview.postMessage(h);})();)",
                    nullptr);
                return S_OK;
            })
            .Get(),
        &nav_completed_token_);
}

void CWebLoginDlg::TryHeartbeatAfterLoginHash() {
    if (!client_ || webview_shutdown_ || heartbeat_in_progress_) {
        return;
    }
    heartbeat_in_progress_ = true;
    bsphp::ApiResponse r = client_->api_call("timeout.lg");
    std::wstring codeW(r.code.begin(), r.code.end());
    DebugLogW(L"[WebLogin] timeout.lg code=%s (5031=已登录，将关闭本窗口并打开控制台)", codeW.c_str());
    if (r.code == "5031") {
        // 禁止在本回调内 ShutdownWebView（会 remove_WebMessageReceived，属同回调重入，易 AV）
        if (::IsWindow(GetSafeHwnd())) {
            ::PostMessageW(GetSafeHwnd(), WM_WEBLOGIN_SUCCESS, 1, 0);
        }
    }
    heartbeat_in_progress_ = false;
}

LRESULT CWebLoginDlg::OnWebLoginHeartbeat(WPARAM, LPARAM) {
    TryHeartbeatAfterLoginHash();
    return 0;
}

void CWebLoginDlg::ShutdownWebView() {
    if (webview_shutdown_) {
        return;
    }
    webview_shutdown_ = true;

    Microsoft::WRL::ComPtr<ICoreWebView2> wv = webview_;
    Microsoft::WRL::ComPtr<ICoreWebView2Controller> ctl = controller_;

    if (wv) {
        if (nav_starting_token_.value != 0) {
            wv->remove_NavigationStarting(nav_starting_token_);
            nav_starting_token_.value = 0;
        }
        if (web_msg_token_.value != 0) {
            wv->remove_WebMessageReceived(web_msg_token_);
            web_msg_token_.value = 0;
        }
        if (nav_completed_token_.value != 0) {
            wv->remove_NavigationCompleted(nav_completed_token_);
            nav_completed_token_.value = 0;
        }
    }

    webview_.Reset();

    if (ctl) {
        ctl->Close();
    }
    controller_.Reset();
}

bool CWebLoginDlg::RegisterWebViewHostWindowClass() {
    static const wchar_t kClass[] = L"BSPHPWebViewHostW";
    static bool registered = false;
    if (registered) {
        return true;
    }
    WNDCLASSEXW wc = {};
    wc.cbSize = sizeof(wc);
    if (::GetClassInfoExW(AfxGetInstanceHandle(), kClass, &wc)) {
        registered = true;
        return true;
    }
    wc.style = CS_DBLCLKS;
    wc.lpfnWndProc = ::DefWindowProcW;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = AfxGetInstanceHandle();
    wc.hCursor = ::LoadCursorW(nullptr, IDC_ARROW);
    wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    wc.lpszClassName = kClass;
    if (!::RegisterClassExW(&wc)) {
        return false;
    }
    registered = true;
    return true;
}

bool CWebLoginDlg::CreateWebViewHostWindow() {
    if (webview_host_hwnd_ && ::IsWindow(webview_host_hwnd_)) {
        return true;
    }
    if (!RegisterWebViewHostWindowClass()) {
        return false;
    }
    CWnd* placeholder = GetDlgItem(IDC_WEBLOGIN_HOST);
    if (!placeholder) {
        return false;
    }
    CRect rc{};
    placeholder->GetWindowRect(&rc);
    ScreenToClient(&rc);
    placeholder->ShowWindow(SW_HIDE);

    static const wchar_t kClass[] = L"BSPHPWebViewHostW";
    webview_host_hwnd_ = ::CreateWindowExW(
        WS_EX_CONTROLPARENT,
        kClass,
        L"",
        WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
        rc.left,
        rc.top,
        rc.Width(),
        rc.Height(),
        GetSafeHwnd(),
        nullptr,
        AfxGetInstanceHandle(),
        nullptr);
    if (!webview_host_hwnd_ || !::IsWindow(webview_host_hwnd_)) {
        webview_host_hwnd_ = nullptr;
        placeholder->ShowWindow(SW_SHOW);
        return false;
    }
    ::SetWindowPos(webview_host_hwnd_, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
    return true;
}

void CWebLoginDlg::ResizeWebView() {
    if (webview_shutdown_ || !controller_ || !GetSafeHwnd()) {
        return;
    }
    if (!::IsWindow(GetSafeHwnd())) {
        return;
    }
    if (!::IsWindow(webview_host_hwnd_)) {
        return;
    }
    CWnd* placeholder = GetDlgItem(IDC_WEBLOGIN_HOST);
    if (!placeholder) {
        return;
    }
    CRect rc{};
    placeholder->GetWindowRect(&rc);
    ScreenToClient(&rc);
    const int w = rc.Width();
    const int h = rc.Height();
    if (w <= 0 || h <= 0) {
        DebugLogW(L"[WebLogin] ResizeWebView: placeholder rect is zero (%dx%d), skip put_Bounds", w, h);
        return;
    }
    ::MoveWindow(webview_host_hwnd_, rc.left, rc.top, w, h, TRUE);
    ::SetWindowPos(webview_host_hwnd_, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);

    RECT rcClient{};
    ::GetClientRect(webview_host_hwnd_, &rcClient);
    controller_->put_Bounds(rcClient);
    DebugLogW(L"[WebLogin] ResizeWebView: put_Bounds client %ldx%ld", (long)(rcClient.right - rcClient.left),
              (long)(rcClient.bottom - rcClient.top));
}

void CWebLoginDlg::OnShowWindow(BOOL bShow, UINT nStatus) {
    CDialogEx::OnShowWindow(bShow, nStatus);
    if (bShow) {
        ResizeWebView();
        DebugLogW(L"[WebLogin] OnShowWindow: resized WebView bounds");
    }
}

void CWebLoginDlg::OnSize(UINT nType, int cx, int cy) {
    CDialogEx::OnSize(nType, cx, cy);
    ResizeWebView();
}

void CWebLoginDlg::OnDestroy() {
    pending_env_.Reset();
    pending_controller_.Reset();
    ShutdownWebView();
    CDialogEx::OnDestroy();
}

LRESULT CWebLoginDlg::OnWebLoginSuccess(WPARAM wParam, LPARAM) {
    if (wParam == 1) {
        ShutdownWebView();
    }
    if (owner_) {
        owner_->NotifyWebLoginSucceeded();
    }
    EndDialog(IDOK);
    return 0;
}

void CWebLoginDlg::OnBnClickedOpenBrowser() {
    if (navigate_url_.empty()) {
        return;
    }
    const HINSTANCE r = ::ShellExecuteW(nullptr, L"open", navigate_url_.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
    if (reinterpret_cast<INT_PTR>(r) <= 32) {
        CString msg;
        msg.Format(L"无法用系统默认浏览器打开链接（ShellExecute 返回值 %d）。", static_cast<int>(reinterpret_cast<INT_PTR>(r)));
        AfxMessageBox(msg);
        return;
    }
    DebugLogW(L"[WebLogin] ShellExecute 已打开系统浏览器: %s", navigate_url_.c_str());
}

void CWebLoginDlg::OnBnClickedHeartbeatBtn() {
    TryHeartbeatAfterLoginHash();
}
