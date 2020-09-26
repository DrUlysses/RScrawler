cmake_minimum_required(VERSION 3.16)
set(PROJECT_NAME RScrawler)
project(${PROJECT_NAME} C CXX)
set(THREADS_PREFER_PTHREAD_FLAG ON)
set(CMAKE_CXX_STANDARD 17)
set(OPENSSL_USE_STATIC_LIBS TRUE)

#add_subdirectory(libbitdht/src/example)
#add_subdirectory(libbitdht/src/tests)
#add_subdirectory(libbitdht/src/bitdht)
#add_subdirectory(libbitdht/src/udp)
add_subdirectory(libbitdht/src)

include_directories(libbitdht/src/example)
include_directories(libbitdht/src/tests)
include_directories(libbitdht/src/bitdht)
include_directories(libbitdht/src/udp)
include_directories(libbitdht/src)

link_directories(libbitdht/src/example)
link_directories(libbitdht/src/tests)
link_directories(libbitdht/src/bitdht)
link_directories(libbitdht/src/udp)
link_directories(libbitdht/src)

add_executable(${PROJECT_NAME} main.cpp logger.cpp logger.h crawler.cpp crawler.h)

find_package(OpenSSL REQUIRED)
find_package(Threads REQUIRED)

target_link_libraries(${PROJECT_NAME} PUBLIC libbitdht Threads::Threads OpenSSL::Crypto)# example tests udp util bitdht)

set_target_properties(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX)