cmake_minimum_required(VERSION 3.16)
project(Srowser VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets WebEngineCore WebEngineWidgets Network)

add_executable(Srowser
    main.cpp
    privacyengine.h
    privacyengine.cpp
    browserwindow.h
    browserwindow.cpp
    settingsdialog.h
    settingsdialog.cpp
    extensionmanager.h
    extensionmanager.cpp
)

target_link_libraries(Srowser PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::WebEngineCore
    Qt6::WebEngineWidgets
    Qt6::Network
)