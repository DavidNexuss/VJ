cmake_minimum_required(VERSION 3.20)
project(glew)
file(GLOB_RECURSE SRCS *.c)
add_library(glew ${SRCS})
