#pragma once

#include <pjsua2.hpp>

namespace ringer {
class RingerPjsipPool {
   public:
    RingerPjsipPool(const std::string& poolName,
                    const pj_size_t    initialSize = 4000,
                    const pj_size_t    incSize     = 4000,
                    pj_pool_callback   callback    = nullptr);
    ~RingerPjsipPool();

    pj_pool_t* get();

   private:
    pj_pool_t* pool_;
};  // namespace ringer
}  // namespace ringer