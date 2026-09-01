cmake_minimum_required(VERSION 3.22.1)

project("gembound")

add_library(
    gembound
    SHARED
    game.cpp
)

find_library(
    log-lib
    log
)

target_link_libraries(
    gembound
    ${log-lib}
)
