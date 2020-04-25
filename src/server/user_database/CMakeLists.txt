add_library(ringer_user_database ringer_user_database.cpp)
target_include_directories(ringer_user_database
                           PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(ringer_user_database PRIVATE ringer_pjsip_util)
