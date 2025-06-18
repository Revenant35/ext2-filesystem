cmake_minimum_required(VERSION 3.31)
project(c_ext2_filesystem C)

set(CMAKE_C_STANDARD 23)

add_executable(c_ext2_filesystem main.c
        Superblock.c
        BlockGroup.c)
