#include "pch.h"

#include "MainCardDlg.h"

class CCardMfcApp : public CWinApp {
public:
    BOOL InitInstance() override;
};

CCardMfcApp theApp;

BOOL CCardMfcApp::InitInstance() {
    CWinApp::InitInstance();
    SetRegistryKey(_T("BSPHP-Card-Mfc-Demo"));
    CMainCardDlg dlg;
    m_pMainWnd = &dlg;
    dlg.DoModal();
    return FALSE;
}
