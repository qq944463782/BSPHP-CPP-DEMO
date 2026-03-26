#include "pch.h"

#include "CardTabDlg.h"

#include "MainCardDlg.h"

#include "machine_id.h"
#include "string_utf8.h"
#include "card_config.h"

CCardTabDlg::CCardTabDlg(CMainCardDlg* owner) : CDialogEx(IDD_TAB_CARD, owner), owner_(owner) {}

void CCardTabDlg::DoDataExchange(CDataExchange* pDX) {
    CDialogEx::DoDataExchange(pDX);
}

BOOL CCardTabDlg::OnInitDialog() {
    CDialogEx::OnInitDialog();
    // Runtime text to keep .rc strictly ASCII/UTF-8-safe.
    SetDlgItemText(IDC_ST_CARD1, L"卡串：");
    SetDlgItemText(IDC_ST_CARD2, L"密码：");
    SetDlgItemText(IDC_BTN_CARD_VERIFY, L"验证使用");
    SetDlgItemText(IDC_BTN_CARD_NET, L"网络测试");
    SetDlgItemText(IDC_BTN_CARD_VER, L"版本检测");
    SetDlgItemText(IDC_BTN_CARD_RENEW_WEB, L"续费充值");
    SetDlgItemText(IDC_BTN_CARD_BUY_GEN, L"购买充值卡");
    SetDlgItemText(IDC_BTN_CARD_BUY_STOCK, L"购买库存卡");
    return TRUE;
}

BEGIN_MESSAGE_MAP(CCardTabDlg, CDialogEx)
    ON_BN_CLICKED(IDC_BTN_CARD_VERIFY, &CCardTabDlg::OnCardVerify)
    ON_BN_CLICKED(IDC_BTN_CARD_NET, &CCardTabDlg::OnCardNet)
    ON_BN_CLICKED(IDC_BTN_CARD_VER, &CCardTabDlg::OnCardVer)
    ON_BN_CLICKED(IDC_BTN_CARD_RENEW_WEB, &CCardTabDlg::OnCardRenewWeb)
    ON_BN_CLICKED(IDC_BTN_CARD_BUY_GEN, &CCardTabDlg::OnCardBuyGen)
    ON_BN_CLICKED(IDC_BTN_CARD_BUY_STOCK, &CCardTabDlg::OnCardBuyStock)
END_MESSAGE_MAP()

void CCardTabDlg::OnCardVerify() {
    if (!owner_) return;

    CString card, pwd;
    GetDlgItemText(IDC_EDIT_CARD, card);
    GetDlgItemText(IDC_EDIT_CARD_PWD, pwd);
    card.Trim();
    pwd.Trim();
    if (card.IsEmpty()) {
        owner_->SetStatus(L"请输入卡串");
        AfxMessageBox(L"请输入卡串");
        return;
    }

    std::string mc = BsphpDemoMachineCodeUtf8();
    std::string ic = Utf8FromWide(card.GetString());
    std::string pw = Utf8FromWide(pwd.GetString());

    bsphp::ApiResponse r = owner_->client().login_ic(ic, pw, mc, mc);
    CString codeW = CString(WideFromUtf8(r.code).c_str());
    CString dataW = r.data.empty() ? L"(empty)" : CString(WideFromUtf8(r.data).c_str());
    CString msg;
    msg.Format(L"code=%s data=%s", codeW.GetString(), dataW.GetString());
    owner_->SetStatus(msg);
    AfxMessageBox(msg);
    if (CMainCardDlg::LoginIcSuccess(r)) {
        std::string exp = owner_->FetchVipExpiry();
        owner_->OpenOrFocusPanel(ic, exp);
        owner_->SetStatus(L"验证成功，主控制面板已在新窗口打开");
    }
}

void CCardTabDlg::OnCardNet() {
    if (!owner_) return;
    bsphp::ApiResponse c = owner_->client().connect();
    CString codeW = CString(WideFromUtf8(c.code).c_str());
    CString dataW = c.data.empty() ? L"(empty)" : CString(WideFromUtf8(c.data).c_str());
    CString msg;
    msg.Format(L"网络测试\r\ncode=%s data=%s", codeW.GetString(), dataW.GetString());
    owner_->SetStatus(msg);
    AfxMessageBox(msg);
}

void CCardTabDlg::OnCardVer() {
    if (!owner_) return;
    bsphp::ApiResponse v = owner_->client().get_version();
    CString codeW = CString(WideFromUtf8(v.code).c_str());
    CString dataW = v.data.empty() ? L"(empty)" : CString(WideFromUtf8(v.data).c_str());
    CString s;
    s.Format(L"版本检测\r\ncode=%s data=%s", codeW.GetString(), dataW.GetString());
    owner_->SetStatus(s);
    AfxMessageBox(s);
}

void CCardTabDlg::OnCardRenewWeb() {
    if (!owner_) return;
    CString card;
    GetDlgItemText(IDC_EDIT_CARD, card);
    std::wstring w(card.GetString());
    AfxMessageBox(L"打开续费充值页面。");
    owner_->OpenUrl(bsphp_card_demo::RenewSaleUrlW(w).c_str());
}

void CCardTabDlg::OnCardBuyGen() {
    if (!owner_) return;
    AfxMessageBox(L"打开购买充值卡页面。");
    owner_->OpenUrl(bsphp_card_demo::GenCardSaleUrlW().c_str());
}

void CCardTabDlg::OnCardBuyStock() {
    if (!owner_) return;
    AfxMessageBox(L"打开购买库存卡页面。");
    owner_->OpenUrl(bsphp_card_demo::StockSaleUrlW().c_str());
}

