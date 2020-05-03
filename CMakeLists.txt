cmake_minimum_required(VERSION 3.16)
project(folder_sync_deamon C)

set(CMAKE_C_STANDARD 99)

add_executable(folder_sync_deamon main.c sync_functions.c sync_functions.h)