#version 330 core

uniform mat4 uProjectionMatrix;
uniform mat4 uViewMatrix;

layout (location = 0) in vec3 aVertex;

out vec3 pos;
void main()
{
    gl_Position = (uProjectionMatrix * uViewMatrix * vec4(aVertex,1.0));
    pos = aVertex;
}
