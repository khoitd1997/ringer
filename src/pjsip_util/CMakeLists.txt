add_library(ringer_pjsip_util ringer_pjsip_pool.cpp
            ringer_pjsip_pool_factory.cpp)
target_include_directories(ringer_pjsip_util PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(ringer_pjsip_util PRIVATE ringer_logger)
