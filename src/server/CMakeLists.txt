add_subdirectory(user_database)
add_subdirectory(proxy)

add_executable(ringer_proxy_server ringer_proxy_server.cpp)
target_link_libraries(ringer_proxy_server PRIVATE ringer_logger ringer_proxy)
