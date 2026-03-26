#pragma once

#include "bsphp_client.h"

#include <string>

/** Same demo keys as bsphp.mac.demo.card ContentView BSPHPCardConfig. */
namespace bsphp_card_demo {

bsphp::BsPhp MakeDemoClient();

std::wstring RenewSaleUrlW(const std::wstring& user_utf16);
std::wstring GenCardSaleUrlW();
std::wstring StockSaleUrlW();

}  // namespace bsphp_card_demo

