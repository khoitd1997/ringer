#pragma once

#include "pjsua2.hpp"

namespace ringer {
class RingerPjsipPoolFactory {
   public:
    static RingerPjsipPoolFactory& getInstance();

    ~RingerPjsipPoolFactory();

    pj_caching_pool* get();

   private:
    RingerPjsipPoolFactory(const pj_pool_factory_policy* policy, pj_size_t maxCapcity);

    pj_caching_pool cachingPool_;
};
}  // namespace ringer