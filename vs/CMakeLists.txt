cmake_minimum_required(VERSION 3.20)
project(Shambhala)
file(GLOB_RECURSE SRC ../src/*.cpp ../src/*.hpp)
include_directories(../src src)
include_directories(inc)
include_directories(../lib lib)
include_directories(../lib/imgui lib/imgui)
add_library(shambhala ${SRC})
add_subdirectory(../lib/imgui lib/imgui)
add_subdirectory(../lib/imguiText lib/imguiText)
add_subdirectory(../lib/stbimage lib/stbimage)
add_subdirectory(../lib/lightmapper lib/lightmapper)
add_subdirectory(../lib/xatlas lib/xatlas)

target_include_directories(shambhala PUBLIC include ../src)
target_link_libraries(shambhala imgui imguiText stbimage lightmapper xatlas)

file(GLOB_RECURSE JOC ../joc_src/*.cpp ../joc_src/*.hpp)


add_executable(joc ${JOC})
target_compile_definitions(joc PUBLIC GLFW)
target_compile_definitions(joc PUBLIC EDITOR)
target_link_libraries(joc shambhala stbimage ${GLEW_LIBRARIES} ${ASSIMP_LIBRARIES} ${OpeanAL_LIBRARIES} ${GLUT_LIBRARIES} ${GLM_LIBRARIES} ${GLFW_LIBRARIES})

add_executable(joc_final ${JOC})
target_compile_definitions(joc_final PUBLIC GLFW)
target_link_libraries(joc_final shambhala stbimage ${GLEW_LIBRARIES} ${ASSIMP_LIBRARIES} ${OpeanAL_LIBRARIES} ${GLUT_LIBRARIES} ${GLM_LIBRARIES} ${GLFW_LIBRARIES})

add_executable(joc_glut ${JOC})
target_compile_definitions(joc_glut PUBLIC GLUT)
target_link_libraries(joc_glut shambhala stbimage ${GLEW_LIBRARIES} ${ASSIMP_LIBRARIES} ${OpeanAL_LIBRARIES} ${GLUT_LIBRARIES} ${GLM_LIBRARIES} ${GLFW_LIBRARIES})

set(CMAKE_CXX_STANDARD 17)
