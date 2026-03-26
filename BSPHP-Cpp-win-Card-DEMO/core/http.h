#pragma once

#include <string>

namespace bsphp {

std::string md5_hex(const std::string& input);

std::string send_data(const std::string& url,
    const std::string& data,
    const std::string& client_public_key,
    const std::string& server_private_key);

}  // namespace bsphp

