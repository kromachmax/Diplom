cmake_minimum_required(VERSION 3.5)

project(ColPlanAlgo VERSION 0.1 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Поиск Qt с модулем Charts
find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets Charts Core Gui REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets Charts REQUIRED)

set(PROJECT_SOURCES
        main.cpp
        COI_3_7.hpp
        COI_3_9.hpp
        COI_3_6.hpp
        COI_3_4.hpp
        AssignmentAlgo.hpp
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

# Подключение библиотек, включая Charts
target_link_libraries(ColPlanAlgo PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Core Qt${QT_VERSION_MAJOR}::Gui Qt${QT_VERSION_MAJOR}::Charts)

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
