#pragma once

#include "resource.h"

#include "bsphp_client.h"

#include <string>

class CMainCardDlg;

class CPanelDlg : public CDialogEx {
public:
    explicit CPanelDlg(CWnd* pParent = nullptr);

    enum { IDD = IDD_PANEL };

    void Setup(bsphp::BsPhp* client, const std::string& logged_id, const std::string& vip_expiry_utf8);

protected:
    void DoDataExchange(CDataExchange* pDX) override;
    BOOL OnInitDialog() override;
    void PostNcDestroy() override;

    afx_msg void OnClose();
    afx_msg void OnPDate();
    afx_msg void OnPLk();
    afx_msg void OnPHeart();
    afx_msg void OnPNotice();
    afx_msg void OnPSvrdate();
    afx_msg void OnPVer();
    afx_msg void OnPSoft();
    afx_msg void OnPUrl();
    afx_msg void OnPWeburl();
    afx_msg void OnPCustApp();
    afx_msg void OnPCustVip();
    afx_msg void OnPCustLogin();
    afx_msg void OnPGlobal();
    afx_msg void OnPLa();
    afx_msg void OnPLb();
    afx_msg void OnPQuery();
    afx_msg void OnPInfo();
    afx_msg void OnPBind();
    afx_msg void OnPUnbind();
    afx_msg void OnPWrenew();
    afx_msg void OnPWgen();
    afx_msg void OnPWstock();
    afx_msg void OnPLogout();

    DECLARE_MESSAGE_MAP()

private:
    bsphp::BsPhp* client_ = nullptr;
    std::string logged_id_;
    std::string vip_expiry_;
    bool is_closing_ = false;

    void AppendResult(const wchar_t* title, const bsphp::ApiResponse& r);
    void OpenUrl(const wchar_t* url);
};
