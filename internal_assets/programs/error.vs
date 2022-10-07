#version 330 
layout (location = 0) in vec3 aVertex;
layout (location = 2) in vec2 aUV;
layout (location = 3) in vec3 aNormal;

uniform mat4 uProjectionMatrix;
uniform mat4 uViewMatrix;
uniform mat4 uTransformMatrix;

out vec3 globalPosition;

void main()
{
    globalPosition = (uTransformMatrix * vec4(aVertex,1.0)).xyz;
    gl_Position = (uProjectionMatrix * uViewMatrix * uTransformMatrix * vec4(aVertex,1.0));
}
