cmake_minimum_required(VERSION 3.10)

# set the project name
project(Elf_infector)

# add the executable
add_executable(infector infector.c payload.h payload.c)
add_executable(simple_executable sample.c)
add_executable(detector detector.c)

