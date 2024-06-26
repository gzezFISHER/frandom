cmake_minimum_required(VERSION 3.21)

project(frandom)

add_executable(frandom main.cc)

target_include_directories(frandom PRIVATE ./3rd)
target_compile_features(frandom PRIVATE cxx_std_20)

target_compile_options(frandom PRIVATE
	$<$<CXX_COMPILER_ID:MSVC>:/W4>
	$<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-Wall>
)
