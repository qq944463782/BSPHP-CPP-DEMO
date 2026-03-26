#pragma once
/*
 * BSPHP API 客户端 | BSPHP API 客戶端 | BSPHP API Client
 */

#include <string>
#include <map>
#include <vector>

namespace bsphp {

// API 响应结构 | API 回應結構 | API response structure
struct ApiResponse {
    std::string code;       // 状态码 | 狀態碼 | Status code
    std::string data;       // 数据内容 | 資料內容 | Data content
    std::string appsafecode;// 安全校验码 | 安全校驗碼 | Security check code
    std::string sessl;      // 会话标识 | 會話識別碼 | Session identifier
};

// BSPHP 主类 | BSPHP 主類別 | BSPHP main class
class BsPhp {
public:
    // url: API 地址 | API 位址 | API URL
    // mutual_key: 互验密钥 | 互驗密鑰 | Mutual authentication key
    // server_private_key: 服务端 RSA 私钥 | 伺服器 RSA 私鑰 | Server RSA private key
    // client_public_key: 客户端 RSA 公钥 | 用戶端 RSA 公鑰 | Client RSA public key
    BsPhp(const std::string& url,
          const std::string& mutual_key,
          const std::string& server_private_key = "",
          const std::string& client_public_key = "");

    ApiResponse connect();      // 连接测试 | 連線測試 | Connection test
    ApiResponse get_se_ssl();   // 获取会话 | 取得會話 | Get session
    ApiResponse get_version();  // 获取版本 | 取得版本 | Get version
    ApiResponse get_notice();   // 获取公告 | 取得公告 | Get notice
    std::string login(const std::string& user, const std::string& password,
        const std::string& coode = "", const std::string& key = "",
        const std::string& maxoror = "");  // 登录 | 登入 | Login

    // 注册 | 註冊 | Register
    // user: 注册账号 | 註冊帳號 | Username
    // pwd: 密码 | 密碼 | Password
    // pwdb: 再次密码 | 再次密碼 | Confirm password
    // coode: 验证码 | 驗證碼 | Captcha
    // mobile: 手机号码 | 手機號碼 | Mobile number
    // mibao_wenti: 密保问题 | 密保問題 | Security question
    // mibao_daan: 密保答案 | 密保答案 | Security answer
    // qq, mail, key, extension: 可选 | 可選 | Optional
    std::string reg(const std::string& user, const std::string& pwd, const std::string& pwdb,
        const std::string& coode, const std::string& mobile,
        const std::string& mibao_wenti, const std::string& mibao_daan,
        const std::string& qq = "", const std::string& mail = "",
        const std::string& key = "", const std::string& extension = "");

    /** AppEn generic (.in / .ic), parsed response. */
    ApiResponse api_call(const std::string& api,
        const std::map<std::string, std::string>& params = {});

    /** internet.in + BSphpSeSsL.in */
    bool bootstrap();

    /** login.ic; refresh BSphpSeSsL on codes 1011 / 9908 / 1081 when SeSsL present. */
    ApiResponse login_ic(const std::string& icid, const std::string& icpwd,
        const std::string& key, const std::string& maxoror);

    /** cancellation.ic then re-bootstrap session. */
    ApiResponse logout_ic();

    std::string code_url;   // 验证码图片地址 | 驗證碼圖片位址 | Captcha image URL
    std::string BSphpSeSsL; // session token，获取后需手动设置供后续 API 使用 | session token，取得後需手動設定供後續 API 使用 | session token, set manually after get_se_ssl() for subsequent API calls

private:
    std::string _send_data(const std::string& api,
        const std::map<std::string, std::string>& attach_param = {});

    std::string url_;
    std::string mutual_key_;
    std::string server_private_key_;
    std::string client_public_key_;
};

}  // namespace bsphp
