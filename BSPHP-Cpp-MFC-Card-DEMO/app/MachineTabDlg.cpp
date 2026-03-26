#include "pch.h"

#include "MachineTabDlg.h"

#include "MainCardDlg.h"

#include "machine_id.h"
#include "string_utf8.h"
#include "card_config.h"

CMachineTabDlg::CMachineTabDlg(CMainCardDlg* owner) : CDialogEx(IDD_TAB_MACHINE, owner), owner_(owner) {}

void CMachineTabDlg::DoDataExchange(CDataExchange* pDX) {
    CDialogEx::DoDataExchange(pDX);
}

BOOL CMachineTabDlg::OnInitDialog() {
    CDialogEx::OnInitDialog();
    CheckRadioButton(IDC_RADIO_MACH_USE, IDC_RADIO_MACH_RENEW, IDC_RADIO_MACH_USE);
    SetDlgItemText(IDC_EDIT_MACH, CString(WideFromUtf8(BsphpDemoMachineCodeUtf8()).c_str()));
    // Runtime text to match mac demo wording.
    SetDlgItemText(IDC_ST_MACH_TITLE, L"机器码账号模式");
    SetDlgItemText(IDC_RADIO_MACH_USE, L"机器码验证使用");
    SetDlgItemText(IDC_RADIO_MACH_RENEW, L"机器码充值续费");
    SetDlgItemText(IDC_ST_MACH_LBL, L"机器码：");
    SetDlgItemText(IDC_ST_CHONG1, L"充值卡号：");
    SetDlgItemText(IDC_ST_CHONG2, L"充值密码：");
    SetDlgItemText(IDC_BTN_MACH_VERIFY, L"验证使用");
    SetDlgItemText(IDC_BTN_MNET, L"网络测试");
    SetDlgItemText(IDC_BTN_MVER, L"版本检测");
    SetDlgItemText(IDC_BTN_CHONG_OK, L"确认充值");
    SetDlgItemText(IDC_BTN_MACH_PAYWEB, L"一键续费充值");
    SetDlgItemText(IDC_BTN_MACH_GEN, L"购买充值卡");
    SetDlgItemText(IDC_BTN_MACH_STOCK, L"购买库存卡");
    ApplySubModeUI();
    return TRUE;
}

BEGIN_MESSAGE_MAP(CMachineTabDlg, CDialogEx)
    ON_BN_CLICKED(IDC_RADIO_MACH_USE, &CMachineTabDlg::OnSubMode)
    ON_BN_CLICKED(IDC_RADIO_MACH_RENEW, &CMachineTabDlg::OnSubMode)
    ON_BN_CLICKED(IDC_BTN_MACH_VERIFY, &CMachineTabDlg::OnMachVerify)
    ON_BN_CLICKED(IDC_BTN_MNET, &CMachineTabDlg::OnMachNet)
    ON_BN_CLICKED(IDC_BTN_MVER, &CMachineTabDlg::OnMachVer)
    ON_BN_CLICKED(IDC_BTN_CHONG_OK, &CMachineTabDlg::OnChongOk)
    ON_BN_CLICKED(IDC_BTN_MACH_PAYWEB, &CMachineTabDlg::OnMachPayWeb)
    ON_BN_CLICKED(IDC_BTN_MACH_GEN, &CMachineTabDlg::OnMachGen)
    ON_BN_CLICKED(IDC_BTN_MACH_STOCK, &CMachineTabDlg::OnMachStock)
END_MESSAGE_MAP()

void CMachineTabDlg::ApplySubModeUI() {
    const BOOL use = IsDlgButtonChecked(IDC_RADIO_MACH_USE) == BST_CHECKED;
    int show_verify[] = {IDC_BTN_MACH_VERIFY, IDC_BTN_MNET, IDC_BTN_MVER};
    int show_renew[] = {IDC_ST_CHONG1, IDC_EDIT_CHONG_KA, IDC_ST_CHONG2, IDC_EDIT_CHONG_PWD,
                        IDC_BTN_CHONG_OK, IDC_BTN_MACH_PAYWEB, IDC_BTN_MACH_GEN, IDC_BTN_MACH_STOCK};
    for (int id : show_verify) {
        if (CWnd* w = GetDlgItem(id)) w->ShowWindow(use ? SW_SHOW : SW_HIDE);
    }
    for (int id : show_renew) {
        if (CWnd* w = GetDlgItem(id)) w->ShowWindow(use ? SW_HIDE : SW_SHOW);
    }
}

void CMachineTabDlg::OnSubMode() {
    ApplySubModeUI();
    const BOOL use = IsDlgButtonChecked(IDC_RADIO_MACH_USE) == BST_CHECKED;
    AfxMessageBox(use ? L"切换：机器码验证使用" : L"切换：机器码充值续费");
}

void CMachineTabDlg::OnMachVerify() {
    if (!owner_) return;

    CString m;
    GetDlgItemText(IDC_EDIT_MACH, m);
    m.Trim();
    if (m.IsEmpty()) {
        owner_->SetStatus(L"【机器码】请输入机器码（账号）");
        AfxMessageBox(L"【机器码】请输入机器码（账号）");
        return;
    }
    std::string id = Utf8FromWide(m.GetString());
    std::string glob = BsphpDemoMachineCodeUtf8();
    bsphp::ApiResponse feat = owner_->client().api_call(
        "AddCardFeatures.key.ic", {{"carid", id}, {"key", glob}, {"maxoror", glob}});
    CString featCodeW = CString(WideFromUtf8(feat.code).c_str());
    CString featDataW = feat.data.empty() ? L"(empty)" : CString(WideFromUtf8(feat.data).c_str());
    CString line;
    line.Format(L"[AddCardFeatures.key.ic]\r\ncode=%s data=%s",
                featCodeW.GetString(), featDataW.GetString());
    owner_->SetStatus(line);
    AfxMessageBox(line);
    if (!CMainCardDlg::AddFeaturesOk(feat)) return;

    bsphp::ApiResponse r = owner_->client().login_ic(id, "", glob, glob);
    CString rCodeW = CString(WideFromUtf8(r.code).c_str());
    CString rDataW = r.data.empty() ? L"(empty)" : CString(WideFromUtf8(r.data).c_str());
    CString loginLine;
    loginLine.Format(L"[login.ic]\r\ncode=%s data=%s", rCodeW.GetString(), rDataW.GetString());
    owner_->SetStatus(loginLine);
    AfxMessageBox(loginLine);
    if (CMainCardDlg::LoginIcSuccess(r)) {
        std::string exp = owner_->FetchVipExpiry();
        owner_->OpenOrFocusPanel(id, exp);
        owner_->SetStatus(L"验证成功（机器码账号），主控制面板已在新窗口打开");
    }
}

void CMachineTabDlg::OnMachNet() {
    if (!owner_) return;
    bsphp::ApiResponse c = owner_->client().connect();
    CString codeW = CString(WideFromUtf8(c.code).c_str());
    CString dataW = c.data.empty() ? L"(empty)" : CString(WideFromUtf8(c.data).c_str());
    CString msg;
    msg.Format(L"网络测试\r\ncode=%s data=%s", codeW.GetString(), dataW.GetString());
    owner_->SetStatus(msg);
    AfxMessageBox(msg);
}

void CMachineTabDlg::OnMachVer() {
    if (!owner_) return;
    bsphp::ApiResponse v = owner_->client().get_version();
    CString codeW = CString(WideFromUtf8(v.code).c_str());
    CString dataW = v.data.empty() ? L"(empty)" : CString(WideFromUtf8(v.data).c_str());
    CString s;
    s.Format(L"版本检测\r\ncode=%s data=%s", codeW.GetString(), dataW.GetString());
    owner_->SetStatus(s);
    AfxMessageBox(s);
}

void CMachineTabDlg::OnChongOk() {
    if (!owner_) return;

    CString m;
    GetDlgItemText(IDC_EDIT_MACH, m);
    m.Trim();
    if (m.IsEmpty()) {
        owner_->SetStatus(L"【机器码】请输入机器码（账号）");
        AfxMessageBox(L"【机器码】请输入机器码（账号）");
        return;
    }
    CString ka;
    GetDlgItemText(IDC_EDIT_CHONG_KA, ka);
    ka.Trim();
    if (ka.IsEmpty()) {
        owner_->SetStatus(L"请输入充值卡号");
        AfxMessageBox(L"请输入充值卡号");
        return;
    }
    CString pwd;
    GetDlgItemText(IDC_EDIT_CHONG_PWD, pwd);
    pwd.Trim();

    std::string icid = Utf8FromWide(m.GetString());
    std::string ka8 = Utf8FromWide(ka.GetString());
    std::string pw8 = Utf8FromWide(pwd.GetString());
    bsphp::ApiResponse r =
        owner_->client().api_call("chong.ic", {{"icid", icid}, {"ka", ka8}, {"pwd", pw8}});
    CString rCodeW = CString(WideFromUtf8(r.code).c_str());
    CString rDataW = r.data.empty() ? L"(empty)" : CString(WideFromUtf8(r.data).c_str());
    CString line;
    line.Format(L"[chong.ic]\r\ncode=%s data=%s", rCodeW.GetString(), rDataW.GetString());
    owner_->SetStatus(line);
    AfxMessageBox(line);
}

void CMachineTabDlg::OnMachPayWeb() {
    if (!owner_) return;
    CString m;
    GetDlgItemText(IDC_EDIT_MACH, m);
    std::wstring w(m.GetString());
    AfxMessageBox(L"打开一键续费充值页面。");
    owner_->OpenUrl(bsphp_card_demo::RenewSaleUrlW(w).c_str());
}

void CMachineTabDlg::OnMachGen() {
    if (!owner_) return;
    AfxMessageBox(L"打开购买充值卡页面。");
    owner_->OpenUrl(bsphp_card_demo::GenCardSaleUrlW().c_str());
}

void CMachineTabDlg::OnMachStock() {
    if (!owner_) return;
    AfxMessageBox(L"打开购买库存卡页面。");
    owner_->OpenUrl(bsphp_card_demo::StockSaleUrlW().c_str());
}

