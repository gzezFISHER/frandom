cmake_minimum_required(VERSION 3.29)

set(CMAKE_CXX_STANDARD 17)

SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O2 -Wall -Werror")

project(frandom)

include_directories(./3rd)

add_executable(frandom main.cc)