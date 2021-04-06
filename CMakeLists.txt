cmake_minimum_required(VERSION 3.13)

project(raytracing_iow)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fopenmp -Wconversion -O3 -Ofast -fno-fast-math -std=gnu++17 -Wall -Wno-undef")
set(CMAKE_CXX_FLAGS"${CMAKE_CXX_FLAGS} -fopenmp -Wconversion -O3 -Ofast -fno-fast-math -std=gnu++17 -Wall -Wno-undef -fsanitize=address")
add_executable(riow main.cpp)
