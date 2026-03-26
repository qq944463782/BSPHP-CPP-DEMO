#include "pch.h"

#include "MainCardDlg.h"

#include "PanelDlg.h"
#include "CardTabDlg.h"
#include "MachineTabDlg.h"
#include "card_config.h"
#include "machine_id.h"
#include "string_utf8.h"

#include <Shellapi.h>
#include <string>


CMainCardDlg::CMainCardDlg(CWnd* pParent) : CDialogEx(IDD_MAIN, pParent), client_(bsphp_card_demo::MakeDemoClient()) {}

void CMainCardDlg::DoDataExchange(CDataExchange* pDX) {
    CDialogEx::DoDataExchange(pDX);
}

void CMainCardDlg::SetStatus(const CString& text) {
    CWnd* p = GetDlgItem(IDC_STATIC_STATUS);
    if (!p || !::IsWindow(p->GetSafeHwnd())) return;
    p->SetWindowTextW(text);
}

void CMainCardDlg::SetStatusFromPanel(const CString& text) {
    SetStatus(text);
}

void CMainCardDlg::NotifyPanelClosed(CPanelDlg* p) {
    if (panel_ == p) {
        panel_ = nullptr;
    }
}

void CMainCardDlg::OpenUrl(const wchar_t* url) {
    ShellExecuteW(nullptr, L"open", url, nullptr, nullptr, SW_SHOWNORMAL);
}

bool CMainCardDlg::LoginIcSuccess(const bsphp::ApiResponse& r) {
    if (r.code == "1081") {
        return true;
    }
    return r.data.find("1081") != std::string::npos;
}

bool CMainCardDlg::AddFeaturesOk(const bsphp::ApiResponse& r) {
    if (r.code == "1011" || r.code == "1081") {
        return true;
    }
    if (r.data.find("1081") != std::string::npos) {
        return true;
    }
    if (r.data.find(u8"\u6210\u529f") != std::string::npos) {
        return true;
    }
    return false;
}

std::string CMainCardDlg::FetchVipExpiry() {
    bsphp::ApiResponse d = client_.api_call("getdate.ic");
    return d.data.empty() ? "-" : d.data;
}

void CMainCardDlg::ShowTabPage(int index) {
    if (page_card_ && ::IsWindow(page_card_->GetSafeHwnd())) {
        page_card_->ShowWindow(index == 0 ? SW_SHOW : SW_HIDE);
    }
    if (page_mach_ && ::IsWindow(page_mach_->GetSafeHwnd())) {
        page_mach_->ShowWindow(index == 1 ? SW_SHOW : SW_HIDE);
    }
}

void CMainCardDlg::OpenOrFocusPanel(const std::string& id_utf8, const std::string& vip_utf8) {
    if (panel_ && ::IsWindow(panel_->GetSafeHwnd())) {
        panel_->SetForegroundWindow();
        return;
    }
    auto* dlg = new CPanelDlg(this);
    panel_ = dlg;
    dlg->Setup(&client_, id_utf8, vip_utf8);
    dlg->Create(IDD_PANEL, this);
    dlg->ShowWindow(SW_SHOW);
}

BOOL CMainCardDlg::OnInitDialog() {
    CDialogEx::OnInitDialog();
    SetWindowText(L"BSPHP 卡密演示 (MFC)");
    // Use runtime text to avoid any .rc encoding issues.
    SetDlgItemText(IDC_GRP_NOTICE, L"公告");

    auto* tab = (CTabCtrl*)GetDlgItem(IDC_TAB_MAIN);
    if (tab) {
        tab->InsertItem(0, L"\u5236\u4f5c\u5361\u5bc6\u767b\u9646\u6a21\u5f0f");
        tab->InsertItem(1, L"\u4e00\u952e\u6ce8\u518c\u673a\u5668\u7801\u8d26\u53f7");
    }

    // Create child pages and place them below the tab.
    // Note: IDD_TAB_CARD / IDD_TAB_MACHINE templates are wider than the main dialog.
    // So we must size/position child pages based on the actual tab area to avoid clipping.
    CRect tabRc;
    if (CWnd* tabWnd = GetDlgItem(IDC_TAB_MAIN)) {
        tabWnd->GetWindowRect(&tabRc);
        ScreenToClient(&tabRc);
    } else {
        tabRc = CRect(10, 78, 10 + 528, 78 + 20);
    }

    CRect clientRc;
    GetClientRect(&clientRc);

    const int pageX = tabRc.left;
    const int pageY = tabRc.bottom + 6;
    const int pageW = (tabRc.Width() > 0) ? tabRc.Width() : 528;
    // Use a safe fallback to avoid invalid sizes during dialog init.
    const int calcH = clientRc.bottom - pageY - 2;
    const int pageH = (calcH > 0) ? calcH : 330;

    page_card_ = new CCardTabDlg(this);
    page_card_->Create(IDD_TAB_CARD, this);
    page_card_->MoveWindow(pageX, pageY, pageW, pageH);
    page_card_->ShowWindow(SW_SHOW);

    page_mach_ = new CMachineTabDlg(this);
    page_mach_->Create(IDD_TAB_MACHINE, this);
    page_mach_->MoveWindow(pageX, pageY, pageW, pageH);
    page_mach_->ShowWindow(SW_HIDE);

    if (!client_.bootstrap()) {
        SetStatus(L"\u521d\u59cb\u5316\u5931\u8d25\uff08\u8fde\u63a5\u6216 BSphpSeSsL\uff09");
    } else {
        bsphp::ApiResponse n = client_.get_notice();
        SetDlgItemText(IDC_EDIT_NOTICE,
            n.data.empty() ? L"\u6682\u65e0\u516c\u544a" : CString(WideFromUtf8(n.data).c_str()));
        // "工程提示" 弹窗（alert）：显示返回的 code / data
        {
            CString codeW = CString(WideFromUtf8(n.code).c_str());
            std::wstring wd = WideFromUtf8(n.data);
            if (wd.empty()) wd = L"\u6682\u65e0\u516c\u544a";
            CString dataW = CString(wd.c_str());
            CString s;
            s.Format(L"code=%s data=%s", codeW.GetString(), dataW.GetString());
            AfxMessageBox(s);
        }
        SetStatus(L"\u5f85\u64cd\u4f5c");
    }

    ShowTabPage(0);
    return TRUE;
}

void CMainCardDlg::OnMainTabChange(NMHDR*, LRESULT* plr) {
    int sel = 0;
    if (auto* tab = (CTabCtrl*)GetDlgItem(IDC_TAB_MAIN)) {
        sel = tab->GetCurSel();
    }
    ShowTabPage(sel);
    *plr = 0;
}

BEGIN_MESSAGE_MAP(CMainCardDlg, CDialogEx)
    ON_NOTIFY(TCN_SELCHANGE, IDC_TAB_MAIN, &CMainCardDlg::OnMainTabChange)
END_MESSAGE_MAP()
