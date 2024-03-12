cmake_minimum_required(VERSION 3.11.4)
project(main LANGUAGES CXX)

find_package(Curses REQUIRED)
include_directories(${CURSES_INCLUDE_DIR})

set(SOURCES main.cpp)

add_executable(main ${SOURCES})
target_link_libraries(main ${CURSES_LIBRARIES})
target_link_libraries(main ncurses++w)
