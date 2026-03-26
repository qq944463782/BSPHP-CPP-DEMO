#include "pch.h"

#include "ConsoleDlg.h"

#include "LoginDlg.h"

#include "../core/login_demo_config.h"
#include "../core/string_utf8.h"

#include <shellapi.h>

namespace {

// 与 Mac BSPHPUserInfoField 一致（getuserinfo.lg 的 info）
const char* const kUserInfoFieldKeys[] = {
    "UserName",
    "UserUID",
    "UserReDate",
    "UserReIp",
    "UserIsLock",
    "UserLogInDate",
    "UserLogInIp",
    "UserVipDate",
    "UserKey",
    "Class_Nane",
    "Class_Mark",
    "UserQQ",
    "UserMAIL",
    "UserPayZhe",
    "UserTreasury",
    "UserMobile",
    "UserRMB",
    "UserPoint",
    "Usermibao_wenti",
    "UserVipWhether",
    "UserVipDateSurplus_DAY",
    "UserVipDateSurplus_H",
    "UserVipDateSurplus_I",
    "UserVipDateSurplus_S",
};

const wchar_t* const kUserInfoFieldLabels[] = {
    L"用户名称",
    L"用户UID",
    L"激活时间",
    L"激活时Ip",
    L"用户状态",
    L"登录时间",
    L"登录Ip",
    L"到期时",
    L"绑定特征",
    L"用户分组名称",
    L"用户分组别名",
    L"用户QQ",
    L"用户邮箱",
    L"购卡折扣",
    L"是否代理",
    L"电话",
    L"帐号金额",
    L"帐号积分",
    L"密保问题",
    L"vip是否到期",
    L"到期倒计时-天",
    L"到期倒计时-时",
    L"到期倒计时-分",
    L"到期倒计时-秒",
};

constexpr int kUserInfoFieldCount =
    static_cast<int>(sizeof(kUserInfoFieldKeys) / sizeof(kUserInfoFieldKeys[0]));
static_assert(sizeof(kUserInfoFieldKeys) / sizeof(kUserInfoFieldKeys[0]) ==
                  sizeof(kUserInfoFieldLabels) / sizeof(kUserInfoFieldLabels[0]),
              "userinfo field tables");

}  // namespace

BEGIN_MESSAGE_MAP(CConsoleDlg, CDialogEx)
    ON_WM_CLOSE()
    ON_BN_CLICKED(IDC_BTN_CONSOLE_ENDTIME, &CConsoleDlg::OnConsoleEndTime)
    ON_BN_CLICKED(IDC_BTN_CONSOLE_VERSION, &CConsoleDlg::OnConsoleVersion)
    ON_BN_CLICKED(IDC_BTN_CONSOLE_LOGOUT, &CConsoleDlg::OnConsoleLogout)
    ON_BN_CLICKED(IDC_BTN_CONSOLE_SERVER_DATE, &CConsoleDlg::OnConsoleServerDate)
    ON_BN_CLICKED(IDC_BTN_CONSOLE_WEB_URL, &CConsoleDlg::OnConsoleWebUrl)
    ON_BN_CLICKED(IDC_BTN_CONSOLE_GLOBAL_INFO, &CConsoleDlg::OnConsoleGlobalInfo)
    ON_BN_CLICKED(IDC_BTN_CONSOLE_HEARTBEAT, &CConsoleDlg::OnConsoleHeartbeat)
    ON_BN_CLICKED(IDC_BTN_CONSOLE_USERINFO, &CConsoleDlg::OnConsoleUserInfo)
    ON_BN_CLICKED(IDC_BTN_CONSOLE_PRESET_URL, &CConsoleDlg::OnConsolePresetUrl)
    ON_BN_CLICKED(IDC_BTN_CONSOLE_NOTICE, &CConsoleDlg::OnConsoleNotice)
    ON_BN_CLICKED(IDC_BTN_CONSOLE_CODE_ALL, &CConsoleDlg::OnConsoleCodeAll)
    ON_BN_CLICKED(IDC_BTN_CONSOLE_CODE_LOGIN, &CConsoleDlg::OnConsoleCodeLogin)
    ON_BN_CLICKED(IDC_BTN_CONSOLE_CODE_REG, &CConsoleDlg::OnConsoleCodeReg)
    ON_BN_CLICKED(IDC_BTN_CONSOLE_CODE_BACK, &CConsoleDlg::OnConsoleCodeBack)
    ON_BN_CLICKED(IDC_BTN_CONSOLE_CODE_SAY, &CConsoleDlg::OnConsoleCodeSay)
    ON_BN_CLICKED(IDC_BTN_CONSOLE_LOGIC_A, &CConsoleDlg::OnConsoleLogicA)
    ON_BN_CLICKED(IDC_BTN_CONSOLE_LOGIC_B, &CConsoleDlg::OnConsoleLogicB)
    ON_BN_CLICKED(IDC_BTN_CONSOLE_LOGIC_INFOA, &CConsoleDlg::OnConsoleLogicInfoA)
    ON_BN_CLICKED(IDC_BTN_CONSOLE_LOGIC_INFOB, &CConsoleDlg::OnConsoleLogicInfoB)
    ON_BN_CLICKED(IDC_BTN_CONSOLE_APP_APP, &CConsoleDlg::OnConsoleAppApp)
    ON_BN_CLICKED(IDC_BTN_CONSOLE_APP_VIP, &CConsoleDlg::OnConsoleAppVip)
    ON_BN_CLICKED(IDC_BTN_CONSOLE_APP_LOGIN, &CConsoleDlg::OnConsoleAppLogin)
    ON_BN_CLICKED(IDC_BTN_CONSOLE_MIAO, &CConsoleDlg::OnConsoleMiao)
    ON_BN_CLICKED(IDC_BTN_CONSOLE_USERKEY, &CConsoleDlg::OnConsoleUserKey)
    ON_BN_CLICKED(IDC_BTN_CONSOLE_USERINFO_FIELD, &CConsoleDlg::OnConsoleUserInfoField)
    ON_LBN_DBLCLK(IDC_LIST_CONSOLE_USERINFO_FIELDS, &CConsoleDlg::OnLbnDblclkUserInfoFields)
    ON_BN_CLICKED(IDC_BTN_CONSOLE_RENEW, &CConsoleDlg::OnConsoleRenew)
    ON_BN_CLICKED(IDC_BTN_CONSOLE_BUYCARD, &CConsoleDlg::OnConsoleBuycard)
    ON_BN_CLICKED(IDC_BTN_CONSOLE_STOCK, &CConsoleDlg::OnConsoleStock)
END_MESSAGE_MAP()

CConsoleDlg::CConsoleDlg(bsphp::BsPhp* client, const std::string& end_time_utf8, CLoginDlg* owner)
    : CDialogEx(IDD_CONSOLE_WIN, owner)
    , client_(client)
    , end_time_utf8_(end_time_utf8)
    , owner_(owner) {}

BOOL CConsoleDlg::OnInitDialog() {
    CDialogEx::OnInitDialog();

    if (CWnd* st = GetDlgItem(IDC_ST_CONSOLE_STATUS)) {
        std::wstring w = WideFromUtf8(end_time_utf8_);
        CString s;
        s.Format(L"到期时间：%s", w.c_str());
        st->SetWindowTextW(s);
    }

    if (CListBox* lb = (CListBox*)GetDlgItem(IDC_LIST_CONSOLE_USERINFO_FIELDS)) {
        for (int i = 0; i < kUserInfoFieldCount; ++i) {
            lb->AddString(kUserInfoFieldLabels[i]);
        }
    }

    AppendLog("Console ready.");
    return TRUE;
}

void CConsoleDlg::PostNcDestroy() {
    // modeless: delete after close
    delete this;
    CDialogEx::PostNcDestroy();
}

void CConsoleDlg::OnClose() {
    DestroyWindow();
}

void CConsoleDlg::AppendLog(const std::string& utf8_line) {
    CWnd* p = GetDlgItem(IDC_EDIT_CONSOLE_LOG);
    if (!p) {
        return;
    }

    CString cur;
    p->GetWindowTextW(cur);
    std::wstring w = WideFromUtf8(utf8_line);
    CString add(w.c_str());
    add += L"\r\n";
    cur += add;
    p->SetWindowTextW(cur);
}

void CConsoleDlg::OpenUrlW(const std::wstring& url) {
    if (url.empty()) {
        return;
    }
    ShellExecuteW(nullptr, L"open", url.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
}

void CConsoleDlg::RefreshEndTime() {
    if (!client_) {
        return;
    }
    bsphp::ApiResponse r = client_->get_end_time();
    if (!r.data.empty()) {
        end_time_utf8_ = r.data;
    }
    if (CWnd* st = GetDlgItem(IDC_ST_CONSOLE_STATUS)) {
        std::wstring w = WideFromUtf8(end_time_utf8_);
        CString s;
        s.Format(L"到期时间：%s", w.c_str());
        st->SetWindowTextW(s);
    }
}

void CConsoleDlg::OnConsoleEndTime() {
    RefreshEndTime();
    if (!client_) {
        return;
    }
    bsphp::ApiResponse r = client_->get_end_time();
    AppendLog(std::string("vipdate.lg: code=") + r.code + " data=" + r.data);
}

void CConsoleDlg::OnConsoleVersion() {
    if (!client_) {
        return;
    }
    bsphp::ApiResponse v = client_->get_version();
    AppendLog(std::string("v.in: code=") + v.code + " data=" + v.data);
}

void CConsoleDlg::OnConsoleLogout() {
    if (!client_) {
        return;
    }
    bsphp::ApiResponse r = client_->logout_lg();
    AppendLog(std::string("cancellation.lg: code=") + r.code + " data=" + r.data);

    if (owner_) {
        owner_->OnConsoleLogoutCompleted();
    }
    DestroyWindow();
}

void CConsoleDlg::OnConsoleServerDate() {
    if (!client_) return;
    bsphp::ApiResponse r = client_->api_call("date.in");
    AppendLog(std::string("date.in: code=") + r.code + " data=" + r.data);
}

void CConsoleDlg::OnConsolePresetUrl() {
    if (!client_) return;
    bsphp::ApiResponse r = client_->api_call("url.in");
    AppendLog(std::string("url.in: code=") + r.code + " data=" + r.data);
}

void CConsoleDlg::OnConsoleNotice() {
    if (!client_) return;
    bsphp::ApiResponse r = client_->get_notice();
    AppendLog(std::string("gg.in: code=") + r.code + " data=" + r.data);
}

void CConsoleDlg::OnConsoleWebUrl() {
    if (!client_) return;
    bsphp::ApiResponse r = client_->api_call("weburl.in");
    AppendLog(std::string("weburl.in: code=") + r.code + " data=" + r.data);
}

void CConsoleDlg::OnConsoleGlobalInfo() {
    if (!client_) return;
    bsphp::ApiResponse r = client_->api_call("globalinfo.in");
    AppendLog(std::string("globalinfo.in: code=") + r.code + " data=" + r.data);
}

void CConsoleDlg::OnConsoleCodeAll() {
    if (!client_) return;
    bsphp::ApiResponse r = client_->get_code_enabled("INGES_LOGIN|INGES_RE|INGES_MACK|INGES_SAY");
    AppendLog(std::string("getsetimag.in (all): code=") + r.code + " data=" + r.data);
}

void CConsoleDlg::OnConsoleCodeLogin() {
    if (!client_) return;
    bsphp::ApiResponse r = client_->get_code_enabled("INGES_LOGIN");
    AppendLog(std::string("getsetimag.in INGES_LOGIN: code=") + r.code + " data=" + r.data);
}

void CConsoleDlg::OnConsoleCodeReg() {
    if (!client_) return;
    bsphp::ApiResponse r = client_->get_code_enabled("INGES_RE");
    AppendLog(std::string("getsetimag.in INGES_RE: code=") + r.code + " data=" + r.data);
}

void CConsoleDlg::OnConsoleCodeBack() {
    if (!client_) return;
    bsphp::ApiResponse r = client_->get_code_enabled("INGES_MACK");
    AppendLog(std::string("getsetimag.in INGES_MACK: code=") + r.code + " data=" + r.data);
}

void CConsoleDlg::OnConsoleCodeSay() {
    if (!client_) return;
    bsphp::ApiResponse r = client_->get_code_enabled("INGES_SAY");
    AppendLog(std::string("getsetimag.in INGES_SAY: code=") + r.code + " data=" + r.data);
}

void CConsoleDlg::OnConsoleLogicA() {
    if (!client_) return;
    bsphp::ApiResponse r = client_->api_call("logica.in");
    AppendLog(std::string("logica.in: code=") + r.code + " data=" + r.data);
}

void CConsoleDlg::OnConsoleLogicB() {
    if (!client_) return;
    bsphp::ApiResponse r = client_->api_call("logicb.in");
    AppendLog(std::string("logicb.in: code=") + r.code + " data=" + r.data);
}

void CConsoleDlg::OnConsoleLogicInfoA() {
    if (!client_) return;
    bsphp::ApiResponse r = client_->api_call("logicinfoa.in");
    AppendLog(std::string("logicinfoa.in: code=") + r.code + " data=" + r.data);
}

void CConsoleDlg::OnConsoleLogicInfoB() {
    if (!client_) return;
    bsphp::ApiResponse r = client_->api_call("logicinfob.in");
    AppendLog(std::string("logicinfob.in: code=") + r.code + " data=" + r.data);
}

void CConsoleDlg::OnConsoleAppApp() {
    if (!client_) return;
    bsphp::ApiResponse r = client_->api_call("appcustom.in", {{"info", "myapp"}});
    AppendLog(std::string("appcustom.in myapp: code=") + r.code + " data=" + r.data);
}

void CConsoleDlg::OnConsoleAppVip() {
    if (!client_) return;
    bsphp::ApiResponse r = client_->api_call("appcustom.in", {{"info", "myvip"}});
    AppendLog(std::string("appcustom.in myvip: code=") + r.code + " data=" + r.data);
}

void CConsoleDlg::OnConsoleAppLogin() {
    if (!client_) return;
    bsphp::ApiResponse r = client_->api_call("appcustom.in", {{"info", "mylogin"}});
    AppendLog(std::string("appcustom.in mylogin: code=") + r.code + " data=" + r.data);
}

void CConsoleDlg::OnConsoleMiao() {
    if (!client_) return;
    bsphp::ApiResponse r = client_->api_call("miao.in");
    AppendLog(std::string("miao.in: code=") + r.code + " data=" + r.data);
}

void CConsoleDlg::OnConsoleHeartbeat() {
    if (!client_) return;
    bsphp::ApiResponse r = client_->api_call("timeout.lg");
    AppendLog(std::string("timeout.lg (heartbeat): code=") + r.code + " data=" + r.data);
}

void CConsoleDlg::OnConsoleUserInfo() {
    if (!client_) return;
    bsphp::ApiResponse r = client_->api_call("getuserinfo.lg");
    AppendLog(std::string("getuserinfo.lg: code=") + r.code + " data=" + r.data);
}

void CConsoleDlg::OnConsoleUserKey() {
    if (!client_) return;
    bsphp::ApiResponse r = client_->api_call("userkey.lg");
    AppendLog(std::string("userkey.lg: code=") + r.code + " data=" + r.data);
}

void CConsoleDlg::QueryUserInfoField() {
    if (!client_) {
        return;
    }
    CListBox* lb = (CListBox*)GetDlgItem(IDC_LIST_CONSOLE_USERINFO_FIELDS);
    if (!lb) {
        return;
    }
    int sel = lb->GetCurSel();
    if (sel == LB_ERR || sel < 0 || sel >= kUserInfoFieldCount) {
        AppendLog(u8"请在列表中选择一项用户信息字段。");
        return;
    }
    const char* key = kUserInfoFieldKeys[sel];
    bsphp::ApiResponse r = client_->api_call("getuserinfo.lg", {{"info", key}});
    AppendLog(std::string("getuserinfo.lg info=") + key + " code=" + r.code + " data=" + r.data);
}

void CConsoleDlg::OnConsoleUserInfoField() {
    QueryUserInfoField();
}

void CConsoleDlg::OnLbnDblclkUserInfoFields() {
    QueryUserInfoField();
}

void CConsoleDlg::OnConsoleRenew() {
    if (!client_) return;
    bsphp::ApiResponse r = client_->api_call("getuserinfo.lg", {{"info", "UserName"}});
    std::wstring url = bsphp_login_demo::RenewSaleUrlWithUserFromInfoDataW(r.data);
    OpenUrlW(url);
    AppendLog(std::string("Open renew URL (user from getuserinfo UserName). getuserinfo: code=") + r.code + " data=" + r.data);
}

void CConsoleDlg::OnConsoleBuycard() {
    OpenUrlW(bsphp_login_demo::GenCardSaleUrlW());
    AppendLog("Open buy card URL (gencard).");
}

void CConsoleDlg::OnConsoleStock() {
    OpenUrlW(bsphp_login_demo::StockSaleUrlW());
    AppendLog("Open stock card URL (salecard).");
}
