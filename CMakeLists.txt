cmake_minimum_required(VERSION 3.28)

set(CMAKE_CXX_STANDARD 20)

SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O2 -Wall")

project(frandom)

include_directories(./3rd)

add_executable(frandom main.cc)