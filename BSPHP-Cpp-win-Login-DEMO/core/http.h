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
std::string send_data(const std::string& url,
    const std::string& data,
    const std::string& client_public_key,
    const std::string& server_private_key);

}  // namespace bsphp

