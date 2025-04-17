cmake_minimum_required(VERSION 3.5)

project(ColPlanAlgo VERSION 0.1 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Поиск Qt с модулем Charts
find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets Charts Core Gui REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets Charts REQUIRED)

find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})
find_library(GLPK glpk)

# Находим LEMON
find_library(LEMON_LIBRARY NAMES emon libemon HINTS /usr/local/lib)
find_path(LEMON_INCLUDE_DIR lemon/list_graph.h HINTS /usr/local/include)

if (NOT LEMON_LIBRARY OR NOT LEMON_INCLUDE_DIR)
    message(FATAL_ERROR "LEMON library not found. Please install it.")
else()
    message(STATUS "Found LEMON: ${LEMON_LIBRARY}")
endif()

set(PROJECT_SOURCES
        main.cpp
        COI_3_7.hpp
        COI_3_9.hpp
        COI_3_1.hpp
        HungarianAlgo.hpp
        AuctionAlgo.hpp
        solving_LP.hpp
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(ColPlanAlgo
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
else()
    if(ANDROID)
        add_library(ColPlanAlgo SHARED
            ${PROJECT_SOURCES}
        )
    else()
        add_executable(ColPlanAlgo
            ${PROJECT_SOURCES}
        )
    endif()
endif()

if(CMAKE_BUILD_TYPE MATCHES Debug)
    # Добавляем определение DEBUG
    add_definitions(-DDEBUG)
else()
    # Убираем определение DEBUG для других конфигураций (например, Release)
    remove_definitions(-DDEBUG)
endif()

if(CMAKE_BUILD_TYPE MATCHES Release)
    # Добавляем определение DEBUG
    add_definitions(-DRELEASE)
else()
    # Убираем определение DEBUG для других конфигураций (например, Release)
    remove_definitions(-DRELEASE)
endif()

target_include_directories(ColPlanAlgo PRIVATE ${LEMON_INCLUDE_DIR})
# Подключение библиотек, включая Charts
target_link_libraries(ColPlanAlgo PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui Qt${QT_VERSION_MAJOR}::Charts ${GLPK})


set_target_properties(ColPlanAlgo PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(ColPlanAlgo)
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "KPDA")
    set(INSTALL_DESTDIR "/tmp")

    install(TARGETS ColPlanAlgo
        RUNTIME DESTINATION "${INSTALL_DESTDIR}"
        BUNDLE DESTINATION "${INSTALL_DESTDIR}"
        LIBRARY DESTINATION "${INSTALL_DESTDIR}"
    )
endif()
