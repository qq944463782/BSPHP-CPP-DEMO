#pragma once

#include "resource.h"

class CMainCardDlg;

class CMachineTabDlg : public CDialogEx {
public:
    explicit CMachineTabDlg(CMainCardDlg* owner);

    enum { IDD = IDD_TAB_MACHINE };

    void ApplySubModeUI();

protected:
    BOOL OnInitDialog() override;
    void DoDataExchange(CDataExchange* pDX) override;

    afx_msg void OnSubMode();
    afx_msg void OnMachVerify();
    afx_msg void OnMachNet();
    afx_msg void OnMachVer();
    afx_msg void OnChongOk();
    afx_msg void OnMachPayWeb();
    afx_msg void OnMachGen();
    afx_msg void OnMachStock();

    DECLARE_MESSAGE_MAP()

private:
    CMainCardDlg* owner_ = nullptr;
};

