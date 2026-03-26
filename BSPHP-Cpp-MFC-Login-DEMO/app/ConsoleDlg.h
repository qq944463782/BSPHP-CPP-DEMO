#pragma once

#include "afxwin.h"

#include <string>

#include "resource.h"

#include "../core/bsphp_client.h"

class CLoginDlg;

class CConsoleDlg : public CDialogEx {
public:
    CConsoleDlg(bsphp::BsPhp* client, const std::string& end_time_utf8, CLoginDlg* owner);

    enum { IDD = IDD_CONSOLE_WIN };

protected:
    BOOL OnInitDialog() override;
    void PostNcDestroy() override;

    afx_msg void OnClose();
    afx_msg void OnConsoleEndTime();
    afx_msg void OnConsoleVersion();
    afx_msg void OnConsoleLogout();
    afx_msg void OnConsoleServerDate();
    afx_msg void OnConsoleWebUrl();
    afx_msg void OnConsoleGlobalInfo();
    afx_msg void OnConsoleHeartbeat();
    afx_msg void OnConsoleUserInfo();
    afx_msg void OnConsolePresetUrl();
    afx_msg void OnConsoleNotice();
    afx_msg void OnConsoleCodeAll();
    afx_msg void OnConsoleCodeLogin();
    afx_msg void OnConsoleCodeReg();
    afx_msg void OnConsoleCodeBack();
    afx_msg void OnConsoleCodeSay();
    afx_msg void OnConsoleLogicA();
    afx_msg void OnConsoleLogicB();
    afx_msg void OnConsoleLogicInfoA();
    afx_msg void OnConsoleLogicInfoB();
    afx_msg void OnConsoleAppApp();
    afx_msg void OnConsoleAppVip();
    afx_msg void OnConsoleAppLogin();
    afx_msg void OnConsoleMiao();
    afx_msg void OnConsoleUserKey();
    afx_msg void OnConsoleUserInfoField();
    afx_msg void OnLbnDblclkUserInfoFields();
    afx_msg void OnConsoleRenew();
    afx_msg void OnConsoleBuycard();
    afx_msg void OnConsoleStock();

    DECLARE_MESSAGE_MAP()

private:
    bsphp::BsPhp* client_ = nullptr;
    std::string end_time_utf8_;
    CLoginDlg* owner_ = nullptr;

    void AppendLog(const std::string& utf8_line);
    void RefreshEndTime();
    void QueryUserInfoField();
    static void OpenUrlW(const std::wstring& url);
};
