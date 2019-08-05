#include "ringer_pjsip_pool.hpp"

#include "ringer_logger.hpp"
#include "ringer_pjsip_pool_factory.hpp"

namespace ringer {
RingerPjsipPool::RingerPjsipPool(const std::string& poolName,
                                 const pj_size_t    initialSize,
                                 const pj_size_t    incSize,
                                 pj_pool_callback   callback) {
    pool_ = pj_pool_create(&(((RingerPjsipPoolFactory::getInstance()).get())->factory),
                           poolName.c_str(),
                           initialSize,
                           incSize,
                           callback);
    if (NULL == pool_) {
        logger::error("Error creating pool");
        exit(1);
    }
}
RingerPjsipPool::~RingerPjsipPool() { pj_pool_release(pool_); }

pj_pool_t* RingerPjsipPool::get() { return pool_; }
}  // namespace ringer