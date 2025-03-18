cmake_minimum_required(VERSION 3.5)

project(Assignment_task LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(Assignment_task main.cpp)

if(CMAKE_SYSTEM_NAME STREQUAL "KPDA")
    set(INSTALL_DESTDIR "/tmp")

    install(TARGETS Assignment_task
        RUNTIME DESTINATION "${INSTALL_DESTDIR}"
        BUNDLE DESTINATION "${INSTALL_DESTDIR}"
        LIBRARY DESTINATION "${INSTALL_DESTDIR}"
    )
endif()
