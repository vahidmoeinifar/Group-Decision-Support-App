cmake_minimum_required(VERSION 3.21)

project(GroupDecision VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# MSVC: read source files as UTF-8, so names like "Wiśniewski" in SeedData.cpp come out right
if(MSVC)
    add_compile_options(/utf-8)
endif()

find_package(Qt6 6.4 REQUIRED COMPONENTS Core Sql Network Quick QuickControls2 Test)

# Turns on AUTOMOC and other Qt defaults for the whole project
qt_standard_project_setup()

enable_testing()

add_subdirectory(core)     # decision engine, plain C++
add_subdirectory(backend)  # SQLite storage, accounts, rules
add_subdirectory(app)      # the QML desktop app
add_subdirectory(tests)    # Qt Test unit tests
