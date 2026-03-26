#pragma once

#include "resource.h"

#include "../core/bsphp_client.h"
#include "../core/machine_id.h"

#include <string>
#include <vector>

class CConsoleDlg;

class CLoginDlg : public CDialogEx {
public:
    CLoginDlg();

    enum { IDD = IDD_LOGIN_MAIN };

    // Called from console after logout.
    void OnConsoleLogoutCompleted();

    // WebView2 内嵌登录成功（timeout.lg code=5031）后由 CWebLoginDlg 调用。
    void NotifyWebLoginSucceeded();

protected:
    BOOL OnInitDialog() override;
    afx_msg void OnDestroy();

    afx_msg void OnBnClickedCaptchaRefresh();
    afx_msg void OnBnClickedNetTest();
    afx_msg void OnBnClickedEndTime();
    afx_msg void OnBnClickedVersion();
    afx_msg void OnBnClickedWebLogin();
    afx_msg void OnBnClickedLogin();
    afx_msg void OnBnClickedSendCode();
    afx_msg void OnBnClickedTabAction();
    afx_msg void OnTcnSelchangeMainTab(NMHDR* pNMHDR, LRESULT* pResult);

    DECLARE_MESSAGE_MAP()

private:
    bsphp::BsPhp client_;
    std::string machine_code_;

    bool is_ready_ = false;
    bool is_code_enabled_ = true;
    int captcha_refresh_tick_ = 0;
    int current_tab_ = 0;
    // Captcha state per tab:
    // - captcha_value_by_tab_[tab] keeps user's input for that specific tab.
    // - captcha_refresh_tick_by_tab_[tab] keeps refresh trigger timestamp for that specific tab.
    // This prevents "global shared captcha" behavior when switching tabs.
    std::string captcha_value_by_tab_[13];
    int captcha_refresh_tick_by_tab_[13] = {};

    CConsoleDlg* console_ = nullptr;

    // keep last bitmap to avoid leaks
    HBITMAP captcha_bmp_ = nullptr;
    int captcha_input_id_ = IDC_EDIT_CAPTCHA_INPUT;

    void SetServiceStatus(const CString& text);
    void SetLoginStatus(const CString& text);
    void SetControlsEnabled(BOOL enabled);
    void BootstrapAndInitUI();
    void RefreshCaptchaImage();
    void LoadCaptchaByDownloadToStatic(const std::wstring& url, const std::wstring& tmpFile);
    void UpdateCaptchaVisibility();

    static bool CodeEnabledFromApiData(const std::string& data);

    void OpenOrFocusConsole();
    void ApplyTabUI(int tab);
    CString GetEditText(int id) const;
    std::string GetEditUtf8(int id) const;
    void SetLabelText(int id, const wchar_t* text);
    void SetControlVisible(int id, bool visible);
    void SetEditPasswordStyle(int editId, bool password);
    void SetEditMultiLineStyle(int editId, bool multiline);
    void RunCurrentTabAction();
    void RunSendCode();
    void InitTabs();
    bool QueryCodeEnabledForTab(int tab, bool fallback) const;

    int CaptchaInputIdForTab(int tab) const;
    int CaptchaPicIdForTab(int tab) const;
    int CaptchaLabelIdForTab(int tab) const;
    int CaptchaRefreshBtnIdForTab(int tab) const;
};

