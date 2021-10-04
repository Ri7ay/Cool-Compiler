cmake_minimum_required(VERSION 3.0.0)
project(CoolCompiler VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-Wall -Werror")

set(CMAKE_CPP_STAN)

add_executable(Lexer assignment_2/lexer.cpp)
