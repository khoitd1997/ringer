#pragma once

#include <string>

#include "pj/types.h"

namespace ringer {
std::string pjStrToStr(const pj_str_t& pjStr);
pj_str_t    strToPjStr(const std::string& str);
}  // namespace ringer