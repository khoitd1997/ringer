#include "ringer_user_database.hpp"

#include <vector>

namespace ringer {
namespace user_db {
namespace {
struct UserRecord {
    const std::string userName;
    std::string       location;
};
std::vector<UserRecord> userDB;

struct LocationRecord {
    const std::string location;
    std::string       host;
};
std::vector<LocationRecord> locationDB;
}  // namespace
void UserDB::changeUserLocation(const std::string& userName, const std::string& location) {
    for (const auto& userRecord : userDB) {
        if (userRecord.userName == userName) {
            userRecord.location = location;
            return;
        }
    }

    userDB.push_back(UserRecord{userName, location});
}

pj_str_t UserDB::getHostFromUserName(const pj_str_t& userName) {}
}  // namespace user_db
}  // namespace ringer