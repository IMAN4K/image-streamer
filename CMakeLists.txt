# This is a JPEG image streamer implementation over UDP using Qt's GUI backend
# Copyright (C) 2022 Iman Ahmadvand
#
# This is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# It is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.

cmake_minimum_required(VERSION 3.10)

project(ImageStreamer
    VERSION 1.0.0
    LANGUAGES C CXX
    )

set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(${PROJECT_NAME} SHARED)

# Source
file(GLOB_RECURSE hxxFiles ${CMAKE_CURRENT_SOURCE_DIR}/src/*.h)
file(GLOB_RECURSE cxxFiles ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
target_sources(${PROJECT_NAME} PRIVATE ${hxxFiles} ${cxxFiles})
target_include_directories(${PROJECT_NAME} PUBLIC src)

# Qt
find_package(QT NAMES Qt6 Qt5 COMPONENTS Core Network Widgets Gui REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Network Widgets Gui REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Network
    Qt${QT_VERSION_MAJOR}::Widgets
    )

# Platforms
if (WIN32)
    find_package(Qt${QT_VERSION_MAJOR} COMPONENTS WinExtras REQUIRED)
    target_link_libraries(${PROJECT_NAME} PRIVATE
        Qt${QT_VERSION_MAJOR}::WinExtras
        )
elseif(UNIX)
    target_link_libraries(${PROJECT_NAME} PRIVATE X11 Xext Xfixes)
endif()

# Examples
file(GLOB childs ${CMAKE_CURRENT_SOURCE_DIR}/examples/*)
foreach(child ${childs})
    if(IS_DIRECTORY ${child})
        add_subdirectory(${child})
    endif()
endforeach()

