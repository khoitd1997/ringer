#pragma once

#include <string>
#include "pj/types.h"

namespace ringer {
namespace user_db {
// location here is the room name not the host
void changeUserLocation(const std::string& userName, const std::string& location);

// host is the IP of the client of the room the user is in
pj_str_t getHostFromUserName(const pj_str_t& userName);
}  // namespace user_db
}  // namespace ringer