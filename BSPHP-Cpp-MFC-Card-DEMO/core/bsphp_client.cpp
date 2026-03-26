/*
 * BSPHP 业务逻辑实现 | BSPHP 業務邏輯實作 | BSPHP Business Logic Implementation
 */
#include "bsphp_client.h"
#include "http.h"
#include <ctime>
#include <sstream>
#include <iomanip>
#include <cstring>

#include "debug_log.h"
#include <vector>
#include <windows.h>

namespace bsphp {

// 当前进程可执行文件 MD5（BSPHP 参数 md5；后台若开启软件校验需与后台登记一致）。首次调用读盘并缓存。
static std::string md5_hex_of_current_exe_cached() {
    static std::string cached;
    static bool inited = false;
    if (inited) return cached;
    inited = true;

    wchar_t path[MAX_PATH];
    if (GetModuleFileNameW(nullptr, path, MAX_PATH) == 0) return cached;

    HANDLE h = CreateFileW(path, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING,
                           FILE_ATTRIBUTE_NORMAL, nullptr);
    if (h == INVALID_HANDLE_VALUE) return cached;

    LARGE_INTEGER sz{};
    if (!GetFileSizeEx(h, &sz) || sz.QuadPart <= 0) {
        CloseHandle(h);
        return cached;
    }
    const long long kMaxBytes = 256LL * 1024 * 1024;
    if (sz.QuadPart > kMaxBytes) {
        CloseHandle(h);
        return cached;
    }
    const DWORD len = static_cast<DWORD>(sz.QuadPart);
    std::vector<char> buf(len);
    DWORD read = 0;
    if (!ReadFile(h, buf.data(), len, &read, nullptr) || read != len) {
        CloseHandle(h);
        return cached;
    }
    CloseHandle(h);
    cached = md5_hex(std::string(buf.data(), buf.size()));
    return cached;
}

static std::string get_time_str(const char* fmt) {
    time_t t = time(nullptr);
    struct tm tm;
    localtime_s(&tm, &t);
    char buf[64];
    if (strcmp(fmt, "date#time") == 0) {
        sprintf_s(buf, "%04d-%02d-%02d#%02d:%02d:%02d",
            tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
            tm.tm_hour, tm.tm_min, tm.tm_sec);
    } else {
        sprintf_s(buf, "%04d-%02d-%02d %02d:%02d:%02d",
            tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
            tm.tm_hour, tm.tm_min, tm.tm_sec);
    }
    return std::string(buf);
}

static std::string url_encode_value(const std::string& value) {
    std::ostringstream escaped;
    escaped.fill('0');
    escaped << std::hex;
    for (unsigned char c : value) {
        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
            escaped << static_cast<char>(c);
        } else if (c == ' ') {
            escaped << '+';
        } else {
            escaped << '%' << std::setw(2) << static_cast<int>(c);
        }
    }
    return escaped.str();
}

static std::string urlencode(const std::map<std::string, std::string>& params) {
    std::ostringstream oss;
    bool first = true;
    for (const auto& p : params) {
        if (!first) oss << '&';
        oss << url_encode_value(p.first) << '=' << url_encode_value(p.second);
        first = false;
    }
    return oss.str();
}

// XML 解析：提取 <tag>value</tag> 中的 value | XML 解析：擷取 <tag>value</tag> 中的 value | XML parse: extract value from <tag>value</tag>
static std::string extract_xml_tag(const std::string& xml, const std::string& tag) {
    std::string open_tag = "<" + tag + ">";
    std::string close_tag = "</" + tag + ">";
    size_t start = xml.find(open_tag);
    if (start == std::string::npos) return "";
    start += open_tag.size();
    size_t end = xml.find(close_tag, start);
    if (end == std::string::npos) return "";
    return xml.substr(start, end - start);
}

// 判断响应是否为 XML 格式 | 判斷回應是否為 XML 格式 | Check if response is XML format
static bool is_xml_response(const std::string& raw) {
    size_t p = raw.find_first_not_of(" \t\n\r");
    return (p != std::string::npos && (raw.compare(p, 5, "<?xml") == 0 || raw.compare(p, 9, "<response") == 0));
}

static ApiResponse parse_response(const std::string& raw) {
    ApiResponse r;
    if (raw.empty()) return r;
    if (is_xml_response(raw)) {
        r.code = extract_xml_tag(raw, "code");
        r.data = extract_xml_tag(raw, "data");
        r.appsafecode = extract_xml_tag(raw, "appsafecode");
        r.sessl = extract_xml_tag(raw, "SeSsL");
    } else {
        auto extract_json = [&raw](const std::string& key) -> std::string {
            std::string search = "\"" + key + "\"";
            size_t pos = raw.find(search);
            if (pos == std::string::npos) return "";
            pos = raw.find(':', pos);
            if (pos == std::string::npos) return "";
            pos = raw.find_first_not_of(" \t\n\r:", pos + 1);
            if (pos == std::string::npos) return "";
            if (raw[pos] == '"') {
                size_t s = pos + 1, e = raw.find('"', s);
                return (e == std::string::npos) ? "" : raw.substr(s, e - s);
            }
            if (raw[pos] >= '0' && raw[pos] <= '9') {
                size_t e = pos;
                while (e < raw.size() && (isdigit(raw[e]) || raw[e] == '.')) ++e;
                return raw.substr(pos, e - pos);
            }
            if (raw[pos] == '{' || raw[pos] == '[') {
                int depth = 1;
                size_t s = pos;
                char open = raw[pos], close = (open == '{') ? '}' : ']';
                ++pos;
                while (pos < raw.size() && depth > 0) {
                    if (raw[pos] == open) ++depth;
                    else if (raw[pos] == close) --depth;
                    ++pos;
                }
                return raw.substr(s, pos - s);
            }
            return "";
        };
        r.code = extract_json("code");
        r.data = extract_json("data");
        r.appsafecode = extract_json("appsafecode");
        r.sessl = extract_json("SeSsL");
    }
    return r;
}

BsPhp::BsPhp(const std::string& url,
             const std::string& mutual_key,
             const std::string& server_private_key,
             const std::string& client_public_key)
    : url_(url),
      mutual_key_(mutual_key),
      server_private_key_(server_private_key),
      client_public_key_(client_public_key) {
}

std::string BsPhp::_send_data(const std::string& api,
                              const std::map<std::string, std::string>& attach_param) {
    std::string appsafecode = md5_hex(get_time_str(""));
    std::map<std::string, std::string> param = {
        {"api", api},
        {"BSphpSeSsL", BSphpSeSsL},
        {"date", get_time_str("date#time")},
        {"md5", md5_hex_of_current_exe_cached()},
        {"mutualkey", mutual_key_},
        {"appsafecode", appsafecode}
    };
    for (const auto& p : attach_param) {
        param[p.first] = p.second;
    }
    std::string data_str = urlencode(param);

    DebugLogUtf8("[api] call api=", api);
    DebugLogUtf8("[api] urlencoded=", data_str);

    std::string raw = send_data(url_, data_str, client_public_key_, server_private_key_);
    if (raw.empty()) return "";

    DebugLogUtf8("[api] raw(decrypted)=", raw);

    ApiResponse resp = parse_response(raw);
    DebugLogUtf8("[api] parsed code=", resp.code);
    DebugLogUtf8("[api] parsed data=", resp.data);
    DebugLogUtf8("[api] parsed sessl=", resp.sessl);
    if (!resp.appsafecode.empty() && resp.appsafecode != appsafecode) {
        return "";  // appsafecode 安全参数验证不通过 | appsafecode 安全參數驗證不通過 | appsafecode security validation failed
    }
    return raw;
}

ApiResponse BsPhp::connect() {
    std::string result = _send_data("internet.in");
    if (result.empty()) return ApiResponse();
    return parse_response(result);
}

ApiResponse BsPhp::get_se_ssl() {
    std::string result = _send_data("BSphpSeSsL.in");
    if (result.empty()) return ApiResponse();
    return parse_response(result);
}

ApiResponse BsPhp::get_version() {
    std::string result = _send_data("v.in");
    if (result.empty()) return ApiResponse();
    return parse_response(result);
}

ApiResponse BsPhp::get_notice() {
    std::string result = _send_data("gg.in");
    if (result.empty()) return ApiResponse();
    return parse_response(result);
}

std::string BsPhp::login(const std::string& user, const std::string& password,
                         const std::string& coode, const std::string& key,
                         const std::string& maxoror) {
    std::map<std::string, std::string> param = {
        {"user", user},
        {"pwd", password},
        {"key", key},
        {"coode", coode},
        {"maxoror", maxoror}
    };
    std::string result = _send_data("login.lg", param);
    if (result.empty()) return u8"系统错误，登录失败！";
    ApiResponse resp = parse_response(result);
    if (resp.code != "1011") return resp.data;  // 登录失败返回错误信息
    BSphpSeSsL = resp.sessl;  // 登录成功更新 session
    return resp.data;
}

std::string BsPhp::reg(const std::string& user, const std::string& pwd, const std::string& pwdb,
                       const std::string& coode, const std::string& mobile,
                       const std::string& mibao_wenti, const std::string& mibao_daan,
                       const std::string& qq, const std::string& mail,
                       const std::string& key, const std::string& extension) {
    std::map<std::string, std::string> param = {
        {"user", user},
        {"pwd", pwd},
        {"pwdb", pwdb},
        {"qq", qq},
        {"mail", mail},
        {"key", key},
        {"coode", coode},
        {"mobile", mobile},
        {"mibao_wenti", mibao_wenti},
        {"mibao_daan", mibao_daan},
        {"extension", extension}
    };
    std::string result = _send_data("registration.lg", param);
    if (result.empty()) return u8"系统错误，注册失败！";
    ApiResponse resp = parse_response(result);
    return resp.data;
}

ApiResponse BsPhp::api_call(const std::string& api,
                            const std::map<std::string, std::string>& params) {
    std::string result = _send_data(api, params);
    if (result.empty()) {
        return ApiResponse();
    }
    return parse_response(result);
}

bool BsPhp::bootstrap() {
    ApiResponse c = connect();
    if (c.data != "1") {
        return false;
    }
    ApiResponse s = get_se_ssl();
    if (!s.sessl.empty()) {
        BSphpSeSsL = s.sessl;
    } else if (!s.data.empty()) {
        BSphpSeSsL = s.data;
    }
    return !BSphpSeSsL.empty();
}

ApiResponse BsPhp::login_ic(const std::string& icid, const std::string& icpwd,
                              const std::string& key, const std::string& maxoror) {
    std::map<std::string, std::string> param = {
        {"icid", icid},
        {"icpwd", icpwd},
        {"key", key},
        {"maxoror", maxoror}
    };
    std::string result = _send_data("login.ic", param);
    if (result.empty()) {
        return ApiResponse();
    }
    ApiResponse resp = parse_response(result);
    if (resp.code == "1011" || resp.code == "9908" || resp.code == "1081") {
        if (!resp.sessl.empty()) {
            BSphpSeSsL = resp.sessl;
        }
    }
    return resp;
}

ApiResponse BsPhp::logout_ic() {
    ApiResponse r = api_call("cancellation.ic");
    BSphpSeSsL.clear();
    if (!bootstrap()) {
        return ApiResponse();
    }
    return r;
}

}  // namespace bsphp
