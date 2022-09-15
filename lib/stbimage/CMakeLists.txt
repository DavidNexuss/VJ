project(stbimage)

file(GLOB_RECURSE SRCS *.cpp *.h)
add_library(stbimage ${SRCS})
