#include "ringer_pjsip_type_util.hpp"

#include <cstring>

namespace ringer {
std::string pjStrToStr(const pj_str_t& pjStr) {
    return std::string{pjStr.ptr, static_cast<size_t>(pjStr.slen)};
}
pj_str_t strToPjStr(const std::string& str) {
    // TODO(kd): Resolve undeleted buffer
    auto buf = new char[str.size()];
    strcpy(buf, str.c_str());
    return pj_str(buf);
}
}  // namespace ringer