message("-- Linking Cool")
add_library(assimp assimp.dll)
set_target_properties(assimp PROPERTIES LINKER_LANGUAGE C)

add_library(glew32 glew32.dll)
set_target_properties(glew32 PROPERTIES LINKER_LANGUAGE C)

add_library(glfw glfw3.dll glfw3.lib)
set_target_properties(glfw PROPERTIES LINKER_LANGUAGE C)

add_library(al OpenAL32.dll)
set_target_properties(al PROPERTIES LINKER_LANGUAGE C)

add_library(gl opengl32.dll)
set_target_properties(gl PROPERTIES LINKER_LANGUAGE C)