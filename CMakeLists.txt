cmake_minimum_required(VERSION 3.14)
project(SimpSim C)

set(CMAKE_C_STANDARD 99)

add_executable(SimpSim src/main.c src/spacecat_vm.c src/spacecat_vm.h)