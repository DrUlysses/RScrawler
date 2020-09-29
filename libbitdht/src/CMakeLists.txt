cmake_minimum_required(VERSION 3.16)
set(PROJECT_NAME RScrawler)
set(LIB_NAME libbitdht)
set(THREADS_PREFER_PTHREAD_FLAG ON)
set(OPENSSL_USE_STATIC_LIBS TRUE)

project(${PROJECT_NAME} C CXX)

set(CMAKE_CXX_STANDARD 17)

#add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/bitdht)
#add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/example)
#add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/tests)
#add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/udp)
#add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/util)

#file(GLOB_RECURSE libbitdht_SRC "*.h" "*.cc")

#add_library(${LIB_NAME} ${libbitdht_SRC})

add_library(${LIB_NAME}
    bitdht/bdiface.h
    bitdht/bencode.h
    bitdht/bdobj.h
    bitdht/bdmsgs.h
    bitdht/bdpeer.h
    bitdht/bdquery.h
    bitdht/bdhash.h
    bitdht/bdstore.h
    bitdht/bdnode.h
    bitdht/bdmanager.h
    bitdht/bdstddht.h
    bitdht/bdhistory.h
    util/bdnet.h
    util/bdthreads.h
    util/bdrandom.h
    util/bdfile.h
    util/bdstring.h
    udp/udplayer.h
    udp/udpstack.h
    udp/udpbitdht.h
    bitdht/bdconnection.h
    bitdht/bdfilter.h
    bitdht/bdaccount.h
    bitdht/bdquerymgr.h
    util/bdbloom.h
    bitdht/bdfriendlist.h
    bitdht/bencode.c
    bitdht/bdobj.cc
    bitdht/bdmsgs.cc
    bitdht/bdpeer.cc
    bitdht/bdquery.cc
    bitdht/bdhash.cc
    bitdht/bdstore.cc
    bitdht/bdnode.cc
    bitdht/bdmanager.cc
    bitdht/bdstddht.cc
    bitdht/bdhistory.cc
    util/bdnet.cc
    util/bdthreads.cc
    util/bdrandom.cc
    util/bdfile.cc
    util/bdstring.cc
    udp/udplayer.cc
    udp/udpstack.cc
    udp/udpbitdht.cc
    bitdht/bdconnection.cc
    bitdht/bdfilter.cc
    bitdht/bdaccount.cc
    bitdht/bdquerymgr.cc
    util/bdbloom.cc
    bitdht/bdfriendlist.cc
    example/bdhandler.cc
    example/bdhandler.h
    rsdebug.cc
    rsdebug.h
    rsdeprecate.h
    rserrno.cc
    rserrno.h
    rsids.h
    rsmemory.cc
    rsmemory.h
    rsrandom.cc
    rsrandom.h
    rsthreads.cc
    rsthreads.h
    stacktrace.h
)

#add_library(${LIB_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/.. ${CMAKE_CURRENT_SOURCE_DIR}/bitdht ${CMAKE_CURRENT_SOURCE_DIR}/example ${CMAKE_CURRENT_SOURCE_DIR}/tests ${CMAKE_CURRENT_SOURCE_DIR}/udp ${CMAKE_CURRENT_SOURCE_DIR}/util)

find_package(OpenSSL REQUIRED)
find_package(Threads REQUIRED)

target_include_directories(${LIB_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/bitdht PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/example PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/tests PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/udp PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/util)

#target_link_libraries(${LIB_NAME} PRIVATE bitdht example tests udp util)

target_link_libraries(${LIB_NAME} Threads::Threads OpenSSL::Crypto)

set_target_properties(${LIB_NAME} PROPERTIES LINKER_LANGUAGE CXX)