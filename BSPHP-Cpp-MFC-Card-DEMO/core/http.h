#pragma once
/*
 * BSPHP HTTP 加密通信模块 | BSPHP HTTP 加密通訊模組 | BSPHP HTTP Encryption Module
 */

#include <string>

namespace bsphp {

// MD5 十六进制摘要 | MD5 十六進制摘要 | MD5 hex digest
std::string md5_hex(const std::string& input);

// 发送数据包 (与 Python http.send_data 一致)
// 發送資料包 (與 Python http.send_data 一致)
// Send data packet (same as Python http.send_data)
// url: 服务器地址 | 伺服器位址 | Server URL
// data: 请求内容 (已含 appsafecode 的 urlencoded 参数字符串) | 請求內容 (已含 appsafecode 的 urlencoded 參數) | Request body (urlencoded params including appsafecode)
// client_public_key: RSA 公钥 (Base64 PEM 或 DER) | RSA 公鑰 (Base64 PEM 或 DER) | RSA public key (Base64 PEM or DER)
// server_private_key: RSA 私钥 (Base64 PEM 或 DER) | RSA 私鑰 (Base64 PEM 或 DER) | RSA private key (Base64 PEM or DER)
// 返回: 解析后的 JSON response 字符串，失败返回空 | 返回: 解析後的 JSON response 字串，失敗返回空 | Returns: parsed JSON response string, empty on failure
std::string send_data(const std::string& url,
    const std::string& data,
    const std::string& client_public_key,
    const std::string& server_private_key);

}  // namespace bsphp
