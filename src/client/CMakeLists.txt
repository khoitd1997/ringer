add_subdirectory(endpoint)
add_subdirectory(account)
add_subdirectory(call)

add_executable(caller caller.cpp)
target_link_libraries(caller
                      PRIVATE ringer_logger
                              ringer_endpoint
                              ringer_account
                              ringer_call)

add_executable(answerrer answerrer.cpp)
target_link_libraries(answerrer
                      PRIVATE ringer_logger ringer_endpoint ringer_account)
