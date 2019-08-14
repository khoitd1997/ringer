add_library(ringer_call ringer_call.cpp)
target_include_directories(ringer_call PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(ringer_call PUBLIC ringer_account PRIVATE ringer_logger)
