#version 330 core

layout (location = 0) in vec3 aPosition;
layout (location = 2) in vec2 aUV;

uniform mat4 uProjectionMatrix;
uniform mat4 uViewMatrix;
uniform mat4 uTransformMatrix;

out vec2 vUV;

void main() {
    
    gl_Position = (uProjectionMatrix * uViewMatrix * uTransformMatrix * vec4(aPosition,1.0));
    vUV = aUV;
}
