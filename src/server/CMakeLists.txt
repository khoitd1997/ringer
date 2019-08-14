add_subdirectory(proxy)

add_executable(proxy_server proxy_server.cpp)
target_link_libraries(proxy_server PRIVATE ringer_logger proxy)
