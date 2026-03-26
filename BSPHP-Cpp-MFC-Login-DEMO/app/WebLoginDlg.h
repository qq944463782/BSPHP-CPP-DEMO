#pragma once

#include "resource.h"

#include "../core/bsphp_client.h"

#include <WebView2.h>

#include <string>

#include <wrl/client.h>

class CLoginDlg;

// 内嵌 WebView2：加载 Web 登录页，监听 #login=（与 Mac WebLoginWebView 一致），心跳 timeout.lg code=5031 视为已登录。
class CWebLoginDlg : public CDialogEx {
public:
    CWebLoginDlg(CLoginDlg* owner, bsphp::BsPhp* client, std::wstring navigate_url);

    enum { IDD = IDD_WEBLOGIN };

protected:
    BOOL OnInitDialog() override;
    afx_msg void OnDestroy();
    afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnBnClickedOpenBrowser();
    afx_msg void OnBnClickedHeartbeatBtn();
    // wParam: 1 = 先 Shutdown WebView 再通知（不得在 WebMessageReceived 内直接 Shutdown，会 remove 重入崩溃）
    afx_msg LRESULT OnWebLoginSuccess(WPARAM wParam, LPARAM lParam);
    // 从 WebMessageReceived 投递：避免在回调里同步阻塞 api_call，导致页面闪白/不渲染
    afx_msg LRESULT OnWebLoginHeartbeat(WPARAM wParam, LPARAM lParam);
    // 在非 UI 线程收到 WebView2 异步回调时投递，否则控件不创建/页面不渲染
    afx_msg LRESULT OnWebLoginCreateController(WPARAM wParam, LPARAM lParam);
    afx_msg LRESULT OnWebLoginFinishAttach(WPARAM wParam, LPARAM lParam);

    DECLARE_MESSAGE_MAP()

private:
    static const UINT WM_WEBLOGIN_SUCCESS;
    static const UINT WM_WEBLOGIN_HEARTBEAT;
    static const UINT WM_WEBLOGIN_CREATE_CONTROLLER;
    static const UINT WM_WEBLOGIN_FINISH_ATTACH;

    CLoginDlg* owner_ = nullptr;
    bsphp::BsPhp* client_ = nullptr;
    std::wstring navigate_url_;

    Microsoft::WRL::ComPtr<ICoreWebView2Environment> pending_env_;
    Microsoft::WRL::ComPtr<ICoreWebView2Controller> pending_controller_;
    Microsoft::WRL::ComPtr<ICoreWebView2Controller> controller_;
    Microsoft::WRL::ComPtr<ICoreWebView2> webview_;

    EventRegistrationToken web_msg_token_{};
    EventRegistrationToken nav_completed_token_{};
    EventRegistrationToken nav_starting_token_{};

    bool webview_shutdown_ = false;
    bool heartbeat_in_progress_ = false;

    void ResizeWebView();
    void SetupWebView();
    void ApplyWebViewDefaults();
    void ApplyRasterizationScale();
    void ApplyDefaultBackgroundColor();
    HRESULT RequestCreateWebView2Environment(const wchar_t* userDataFolderOpt);
    void StartCreateWebViewController(ICoreWebView2Environment* env);
    void FinishAttachCoreWebView(ICoreWebView2Controller* controller);
    void TryHeartbeatAfterLoginHash();
    void ShutdownWebView();

    // SS_BLACKFRAME STATIC 作 WebView2 父 HWND 在部分环境下子 HWND 不绘制；改用专用子窗口作宿主
    HWND webview_host_hwnd_ = nullptr;
    static bool RegisterWebViewHostWindowClass();
    bool CreateWebViewHostWindow();
};
