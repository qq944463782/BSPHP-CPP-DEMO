#include "pch.h"

#include "LoginDlg.h"

class CLoginMfcApp : public CWinApp {
public:
    BOOL InitInstance() override;
    int ExitInstance() override;
};

static CLoginMfcApp theApp;

BOOL CLoginMfcApp::InitInstance() {
    CWinApp::InitInstance();

    SetRegistryKey(_T("BSPHP-Cpp-MFC-Login-DEMO"));

    CLoginDlg dlg;
    m_pMainWnd = &dlg;
    dlg.DoModal();
    return FALSE;
}

int CLoginMfcApp::ExitInstance() {
    return CWinApp::ExitInstance();
}

