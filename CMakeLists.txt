cmake_minimum_required(VERSION 3.21)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

project(action-queue-cutscene-sfml3-desktop CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(WIN32)
    set(SFML_DIR "C:/libs/sfml-3.1.0-mingw/lib/cmake/SFML")
else()
    set(SFML_DIR "$ENV{HOME}/libs/sfml-3.1.0-ubuntu/lib/cmake/SFML")
endif()

# Tell CMake to use static SFML libraries
set(SFML_STATIC_LIBRARIES TRUE)

find_package(SFML 3 REQUIRED COMPONENTS Graphics Window System CONFIG)

add_executable(app src/main.cpp)

if(WIN32)
    target_compile_options(app PRIVATE -Os)
    target_link_options(app PRIVATE -static -s -static-libgcc -static-libstdc++)

    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        target_link_options(app PRIVATE -mconsole)
    else()
        target_link_options(app PRIVATE -mwindows)
    endif()
endif()

target_link_libraries(app PRIVATE
    SFML::Graphics
    SFML::Window
    SFML::System
)
