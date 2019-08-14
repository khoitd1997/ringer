cmake_minimum_required(VERSION 3.13)
project(ringer)

link_directories(/usr/local/lib)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(CMAKE_CXX_STANDARD 17)
add_compile_options(-g)
set(target_name x86_64-unknown-linux-gnu)

link_libraries(pjsua2-${target_name}
               pjsua-${target_name}
               pjsip-ua-${target_name}
               pjsip-simple-${target_name}
               pjsip-${target_name}
               pjmedia-codec-${target_name}
               pjmedia-${target_name}
               pjmedia-codec-${target_name}
               pjmedia-audiodev-${target_name}
               pj-${target_name}
               pjnath-${target_name}
               gsmcodec-${target_name}
               speex-${target_name}
               ilbccodec-${target_name}
               srtp-${target_name}
               webrtc-${target_name}
               resample-${target_name}
               pjlib-util-${target_name}
               uuid
               asound
               m
               pthread)

add_subdirectory(common)
add_subdirectory(client)
add_subdirectory(server)
