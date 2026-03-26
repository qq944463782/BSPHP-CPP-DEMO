#include "pch.h"

#include "PanelDlg.h"

#include "MainCardDlg.h"
#include "card_config.h"
#include "machine_id.h"
#include "string_utf8.h"

#include <Shellapi.h>

CPanelDlg::CPanelDlg(CWnd* pParent) : CDialogEx(IDD_PANEL, pParent) {}

void CPanelDlg::Setup(bsphp::BsPhp* client, const std::string& logged_id, const std::string& vip_expiry_utf8) {
    client_ = client;
    logged_id_ = logged_id;
    vip_expiry_ = vip_expiry_utf8;
}

void CPanelDlg::DoDataExchange(CDataExchange* pDX) {
    CDialogEx::DoDataExchange(pDX);
}

BOOL CPanelDlg::OnInitDialog() {
    CDialogEx::OnInitDialog();
    CString title;
    title.Format(L"\u4e3b\u63a7\u5236\u9762\u677f — %s", CString(WideFromUtf8(logged_id_).c_str()));
    SetWindowText(title);
    CString line1, line2;
    line1.Format(L"\u5f53\u524d\u5361\u53f7\uff1a%s", CString(WideFromUtf8(logged_id_).c_str()));
    line2.Format(L"VIP \u5230\u671f\uff1a%s", CString(WideFromUtf8(vip_expiry_).c_str()));
    SetDlgItemText(IDC_STATIC_PANEL_CARD, line1);
    SetDlgItemText(IDC_STATIC_PANEL_VIP, line2);
    return TRUE;
}

void CPanelDlg::PostNcDestroy() {
    if (CMainCardDlg* p = dynamic_cast<CMainCardDlg*>(GetParent())) {
        if (::IsWindow(p->GetSafeHwnd())) {
            p->NotifyPanelClosed(this);
        }
    }
    delete this;
}

void CPanelDlg::OnClose() {
    if (is_closing_) return;
    is_closing_ = true;
    DestroyWindow();
}

void CPanelDlg::OpenUrl(const wchar_t* url) {
    ShellExecuteW(nullptr, L"open", url, nullptr, nullptr, SW_SHOWNORMAL);
}

void CPanelDlg::AppendResult(const wchar_t* title, const bsphp::ApiResponse& r) {
    CString dataW = r.data.empty() ? L"(empty)" : CString(WideFromUtf8(r.data).c_str());
    CString codeW = CString(WideFromUtf8(r.code).c_str());
    CString block;
    // alert 显示：code=xxx data=xxxx
    block.Format(L"%s\r\ncode=%s data=%s\r\n\r\n", title, codeW.GetString(), dataW.GetString());
    CWnd* p = GetDlgItem(IDC_EDIT_PANEL_LOG);
    if (!p) {
        AfxMessageBox(block);
        return;
    }
    if (!::IsWindow(p->GetSafeHwnd())) {
        AfxMessageBox(block);
        return;
    }
    CString cur;
    p->GetWindowText(cur);
    cur += block;
    p->SetWindowText(cur);
    AfxMessageBox(block);
}

void CPanelDlg::OnPDate() {
    if (!client_) {
        return;
    }
    bsphp::ApiResponse r = client_->api_call("getdate.ic");
    if (!r.data.empty()) {
        vip_expiry_ = r.data;
        CString line2;
        line2.Format(L"VIP \u5230\u671f\uff1a%s", CString(WideFromUtf8(vip_expiry_).c_str()));
        SetDlgItemText(IDC_STATIC_PANEL_VIP, line2);
    }
    AppendResult(L"\u5237\u65b0\u5230\u671f", r);
}

void CPanelDlg::OnPLk() {
    if (!client_) {
        return;
    }
    AppendResult(L"\u767b\u5f55\u72b6\u6001", client_->api_call("getlkinfo.ic"));
}

void CPanelDlg::OnPHeart() {
    if (!client_) {
        return;
    }
    AppendResult(L"\u5fc3\u8df3", client_->api_call("timeout.ic"));
}

void CPanelDlg::OnPNotice() {
    if (!client_) {
        return;
    }
    AppendResult(L"\u516c\u544a", client_->api_call("gg.in"));
}

void CPanelDlg::OnPSvrdate() {
    if (!client_) {
        return;
    }
    AppendResult(L"\u670d\u52a1\u5668\u65f6\u95f4", client_->api_call("date.in"));
}

void CPanelDlg::OnPVer() {
    if (!client_) {
        return;
    }
    AppendResult(L"\u7248\u672c", client_->api_call("v.in"));
}

void CPanelDlg::OnPSoft() {
    if (!client_) {
        return;
    }
    AppendResult(L"\u8f6f\u4ef6\u63cf\u8ff0", client_->api_call("miao.in"));
}

void CPanelDlg::OnPUrl() {
    if (!client_) {
        return;
    }
    AppendResult(L"\u9884\u8bbeURL", client_->api_call("url.in"));
}

void CPanelDlg::OnPWeburl() {
    if (!client_) {
        return;
    }
    AppendResult(L"Web\u5730\u5740", client_->api_call("weburl.in"));
}

void CPanelDlg::OnPCustApp() {
    if (!client_) {
        return;
    }
    AppendResult(L"\u8f6f\u4ef6\u914d\u7f6e",
        client_->api_call("appcustom.in", {{"info", "myapp"}}));
}

void CPanelDlg::OnPCustVip() {
    if (!client_) {
        return;
    }
    AppendResult(L"VIP\u914d\u7f6e", client_->api_call("appcustom.in", {{"info", "myvip"}}));
}

void CPanelDlg::OnPCustLogin() {
    if (!client_) {
        return;
    }
    AppendResult(L"\u767b\u5f55\u914d\u7f6e", client_->api_call("appcustom.in", {{"info", "mylogin"}}));
}

void CPanelDlg::OnPGlobal() {
    if (!client_) {
        return;
    }
    AppendResult(L"\u5168\u5c40\u914d\u7f6e", client_->api_call("globalinfo.in"));
}

void CPanelDlg::OnPLa() {
    if (!client_) {
        return;
    }
    AppendResult(L"\u903b\u8f91A", client_->api_call("logica.in"));
}

void CPanelDlg::OnPLb() {
    if (!client_) {
        return;
    }
    AppendResult(L"\u903b\u8f91B", client_->api_call("logicb.in"));
}

void CPanelDlg::OnPQuery() {
    if (!client_) {
        return;
    }
    AppendResult(L"\u6fc0\u6d3b\u67e5\u8be2", client_->api_call("socard.in", {{"cardid", logged_id_}}));
}

void CPanelDlg::OnPInfo() {
    if (!client_) {
        return;
    }
    CString pwd;
    GetDlgItemText(IDC_EDIT_PANEL_PWD, pwd);
    std::string p = Utf8FromWide(pwd.GetString());
    AppendResult(L"\u5361\u4fe1\u606f",
        client_->api_call("getinfo.ic",
            {{"ic_carid", logged_id_}, {"ic_pwd", p}, {"info", "UserName"}}));
}

void CPanelDlg::OnPBind() {
    if (!client_) {
        return;
    }
    CString pwd;
    GetDlgItemText(IDC_EDIT_PANEL_PWD, pwd);
    std::string p = Utf8FromWide(pwd.GetString());
    std::string key = BsphpDemoMachineCodeUtf8();
    AppendResult(L"\u7ed1\u5b9a\u672c\u673a",
        client_->api_call("setcaron.ic", {{"key", key}, {"icid", logged_id_}, {"icpwd", p}}));
}

void CPanelDlg::OnPUnbind() {
    if (!client_) {
        return;
    }
    CString pwd;
    GetDlgItemText(IDC_EDIT_PANEL_PWD, pwd);
    std::string p = Utf8FromWide(pwd.GetString());
    AppendResult(L"\u89e3\u9664\u7ed1\u5b9a",
        client_->api_call("setcarnot.ic", {{"icid", logged_id_}, {"icpwd", p}}));
}

void CPanelDlg::OnPWrenew() {
    AfxMessageBox(L"\u5f00\u542f\u7eed\u8d39\u5145\u503c\u94fe\u63a5\u3002");
    OpenUrl(bsphp_card_demo::RenewSaleUrlW(WideFromUtf8(logged_id_)).c_str());
}

void CPanelDlg::OnPWgen() {
    AfxMessageBox(L"\u5f00\u542f\u8d2d\u4e70\u5145\u503c\u5361\u94fe\u63a5\u3002");
    OpenUrl(bsphp_card_demo::GenCardSaleUrlW().c_str());
}

void CPanelDlg::OnPWstock() {
    AfxMessageBox(L"\u5f00\u542f\u8d2d\u4e70\u5e93\u5b58\u5361\u94fe\u63a5\u3002");
    OpenUrl(bsphp_card_demo::StockSaleUrlW().c_str());
}

void CPanelDlg::OnPLogout() {
    if (client_) {
        client_->logout_ic();
    }
    if (CMainCardDlg* p = dynamic_cast<CMainCardDlg*>(GetParent())) {
        p->SetStatusFromPanel(L"\u5df2\u6ce8\u9500");
    }
    AfxMessageBox(L"\u5df2\u6ce8\u9500\u3002");
    DestroyWindow();
}

BEGIN_MESSAGE_MAP(CPanelDlg, CDialogEx)
    ON_WM_CLOSE()
    ON_BN_CLICKED(IDC_P_DATE, &CPanelDlg::OnPDate)
    ON_BN_CLICKED(IDC_P_LK, &CPanelDlg::OnPLk)
    ON_BN_CLICKED(IDC_P_HEART, &CPanelDlg::OnPHeart)
    ON_BN_CLICKED(IDC_P_NOTICE, &CPanelDlg::OnPNotice)
    ON_BN_CLICKED(IDC_P_SVRDATE, &CPanelDlg::OnPSvrdate)
    ON_BN_CLICKED(IDC_P_VER, &CPanelDlg::OnPVer)
    ON_BN_CLICKED(IDC_P_SOFT, &CPanelDlg::OnPSoft)
    ON_BN_CLICKED(IDC_P_URL, &CPanelDlg::OnPUrl)
    ON_BN_CLICKED(IDC_P_WEBURL, &CPanelDlg::OnPWeburl)
    ON_BN_CLICKED(IDC_P_CUST_APP, &CPanelDlg::OnPCustApp)
    ON_BN_CLICKED(IDC_P_CUST_VIP, &CPanelDlg::OnPCustVip)
    ON_BN_CLICKED(IDC_P_CUST_LOGIN, &CPanelDlg::OnPCustLogin)
    ON_BN_CLICKED(IDC_P_GLOBAL, &CPanelDlg::OnPGlobal)
    ON_BN_CLICKED(IDC_P_LA, &CPanelDlg::OnPLa)
    ON_BN_CLICKED(IDC_P_LB, &CPanelDlg::OnPLb)
    ON_BN_CLICKED(IDC_P_QUERY, &CPanelDlg::OnPQuery)
    ON_BN_CLICKED(IDC_P_INFO, &CPanelDlg::OnPInfo)
    ON_BN_CLICKED(IDC_P_BIND, &CPanelDlg::OnPBind)
    ON_BN_CLICKED(IDC_P_UNBIND, &CPanelDlg::OnPUnbind)
    ON_BN_CLICKED(IDC_P_WRENEW, &CPanelDlg::OnPWrenew)
    ON_BN_CLICKED(IDC_P_WGEN, &CPanelDlg::OnPWgen)
    ON_BN_CLICKED(IDC_P_WSTOCK, &CPanelDlg::OnPWstock)
    ON_BN_CLICKED(IDC_P_LOGOUT, &CPanelDlg::OnPLogout)
END_MESSAGE_MAP()
