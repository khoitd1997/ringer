#include "ringer_pjsip_pool_factory.hpp"

namespace ringer {
RingerPjsipPoolFactory& RingerPjsipPoolFactory::getInstance() {
    static RingerPjsipPoolFactory singleton{nullptr, 1024 * 1024};
    return singleton;
}

pj_caching_pool* RingerPjsipPoolFactory::get() { return &cachingPool_; }

RingerPjsipPoolFactory::RingerPjsipPoolFactory(const pj_pool_factory_policy* policy,
                                               pj_size_t                     maxCapcity) {
    pj_caching_pool_init(&cachingPool_, policy, maxCapcity);
}

RingerPjsipPoolFactory::~RingerPjsipPoolFactory() { pj_caching_pool_destroy(&cachingPool_); }
}  // namespace ringer