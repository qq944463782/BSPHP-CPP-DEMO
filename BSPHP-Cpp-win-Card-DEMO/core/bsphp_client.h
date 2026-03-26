#pragma once
/*
 * BSPHP API 客户端 | BSPHP API 客戶端 | BSPHP API Client
 */

#include <string>
#include <map>
#include <vector>

namespace bsphp {

struct ApiResponse {
    std::string code;
    std::string data;
    std::string appsafecode;
    std::string sessl;
};

class BsPhp {
public:
    BsPhp(const std::string& url,
          const std::string& mutual_key,
          const std::string& server_private_key = "",
          const std::string& client_public_key = "");

    ApiResponse connect();
    ApiResponse get_se_ssl();
    ApiResponse get_version();
    ApiResponse get_notice();

    std::string login(const std::string& user, const std::string& password,
        const std::string& coode = "", const std::string& key = "",
        const std::string& maxoror = "");

    std::string reg(const std::string& user, const std::string& pwd, const std::string& pwdb,
        const std::string& coode, const std::string& mobile,
        const std::string& mibao_wenti, const std::string& mibao_daan,
        const std::string& qq = "", const std::string& mail = "",
        const std::string& key = "", const std::string& extension = "");

    ApiResponse api_call(const std::string& api,
        const std::map<std::string, std::string>& params = {});

    bool bootstrap();

    ApiResponse login_ic(const std::string& icid, const std::string& icpwd,
        const std::string& key, const std::string& maxoror);

    ApiResponse logout_ic();

    std::string code_url;
    std::string BSphpSeSsL;

private:
    std::string _send_data(const std::string& api,
        const std::map<std::string, std::string>& attach_param = {});

    std::string url_;
    std::string mutual_key_;
    std::string server_private_key_;
    std::string client_public_key_;
};

}  // namespace bsphp

