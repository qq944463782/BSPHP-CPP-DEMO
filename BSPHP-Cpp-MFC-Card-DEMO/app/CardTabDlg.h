#pragma once

#include "resource.h"

class CMainCardDlg;

class CCardTabDlg : public CDialogEx {
public:
    explicit CCardTabDlg(CMainCardDlg* owner);

    enum { IDD = IDD_TAB_CARD };

protected:
    BOOL OnInitDialog() override;
    void DoDataExchange(CDataExchange* pDX) override;

    afx_msg void OnCardVerify();
    afx_msg void OnCardNet();
    afx_msg void OnCardVer();
    afx_msg void OnCardRenewWeb();
    afx_msg void OnCardBuyGen();
    afx_msg void OnCardBuyStock();

    DECLARE_MESSAGE_MAP()

private:
    CMainCardDlg* owner_ = nullptr;
};

