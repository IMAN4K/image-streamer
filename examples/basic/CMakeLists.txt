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

set(EXE_NAME Basic)
add_executable(${EXE_NAME} src/main.cpp)

if (TARGET ImageStreamer)
    target_link_libraries(${EXE_NAME} PRIVATE ImageStreamer)
endif()

# Qt
find_package(QT NAMES Qt6 Qt5 COMPONENTS Core Network Widgets Gui REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Network Widgets Gui REQUIRED)
target_link_libraries(${EXE_NAME} PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Network
    Qt${QT_VERSION_MAJOR}::Widgets
    )
