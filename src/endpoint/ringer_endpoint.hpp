#pragma once

#include <pjsua2.hpp>

namespace ringer {
class RingerEndpoint {
   public:
    static RingerEndpoint& getInstance(const int port, const std::string& publicAddr);

    ~RingerEndpoint();

    pj::Endpoint* get();
    pj::Endpoint* operator->();

   private:
    RingerEndpoint(const int port, const std::string& publicAddr);

    pj::Endpoint ep_;
};
}  // namespace ringer