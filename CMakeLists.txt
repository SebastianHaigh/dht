cmake_minimum_required(VERSION 3.16)
project(DHT VERSION 0.1.0)
set(CMAKE_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 20)

find_package(Catch2 3 REQUIRED)

add_subdirectory(src)

include(CTest)
enable_testing()
