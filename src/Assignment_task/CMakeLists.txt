cmake_minimum_required(VERSION 3.5)

project(Assignment_task LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(CMAKE_BUILD_TYPE MATCHES Debug)
    add_definitions(-DDEBUG)
else()
    remove_definitions(-DDEBUG)
endif()

configure_file(${CMAKE_SOURCE_DIR}/index.html ${CMAKE_BINARY_DIR}/index.html COPYONLY)

add_executable(Assignment_task HungarianAlgo.hpp AuctionAlgo.hpp main.cpp)

if(WIN32)
    target_link_libraries(Assignment_task PRIVATE ws2_32)
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "KPDA")
    set(INSTALL_DESTDIR "/tmp")

    install(TARGETS Assignment_task
        RUNTIME DESTINATION "${INSTALL_DESTDIR}"
        BUNDLE DESTINATION "${INSTALL_DESTDIR}"
        LIBRARY DESTINATION "${INSTALL_DESTDIR}"
    )
endif()
