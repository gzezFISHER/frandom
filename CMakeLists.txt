cmake_minimum_required(VERSION 3.29)

set(CMAKE_CXX_STANDARD 17)

project(frandom)

include_directories(./3rd)

add_executable(frandom main.cc)