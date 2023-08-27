cmake_minimum_required(VERSION 3.16)
project(DHT VERSION 0.1.0)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 20)

Include(FetchContent)

FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v3.4.0
)

FetchContent_MakeAvailable(Catch2)

add_subdirectory(src)

include(CTest)
enable_testing()
