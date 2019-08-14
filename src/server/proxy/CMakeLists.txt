add_library(proxy proxy.cpp)
target_include_directories(proxy PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(proxy PRIVATE ringer_logger)
