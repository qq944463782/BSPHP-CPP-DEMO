#pragma once

#include "resource.h"

#include "bsphp_client.h"

#include <string>

class CPanelDlg;
class CCardTabDlg;
class CMachineTabDlg;

class CMainCardDlg : public CDialogEx {
public:
    explicit CMainCardDlg(CWnd* pParent = nullptr);

    enum { IDD = IDD_MAIN };

    void NotifyPanelClosed(CPanelDlg* p);
    void SetStatusFromPanel(const CString& text);
    void SetStatus(const CString& text);
    void OpenUrl(const wchar_t* url);
    void OpenOrFocusPanel(const std::string& id_utf8, const std::string& vip_utf8);
    std::string FetchVipExpiry();
    bsphp::BsPhp& client() { return client_; }

    static bool LoginIcSuccess(const bsphp::ApiResponse& r);
    static bool AddFeaturesOk(const bsphp::ApiResponse& r);

protected:
    void DoDataExchange(CDataExchange* pDX) override;
    BOOL OnInitDialog() override;

    afx_msg void OnMainTabChange(NMHDR* pnm, LRESULT* plr);

    DECLARE_MESSAGE_MAP()

private:
    bsphp::BsPhp client_;
    CPanelDlg* panel_ = nullptr;
    CCardTabDlg* page_card_ = nullptr;
    CMachineTabDlg* page_mach_ = nullptr;

    void ShowTabPage(int index);
};
