#version 330 core

layout (location = 0) in vec3 aPosition;
layout (location = 2) in vec2 aUV;

uniform mat4 uProjectionMatrix;
uniform mat4 uViewMatrix;
uniform mat4 uTransformMatrix;

out vec2 vUV;
uniform vec2 stScale = vec2(1.0);
uniform vec2 stOffset = vec2(0.0);

void main() {
    
    gl_Position = (uProjectionMatrix * uViewMatrix * uTransformMatrix * vec4(aPosition,1.0));

    float p = 16.0f / 512.0f;
    vec2 vUV = vec2(aUV.x * p , aUV.y * p);
    vUV = aUV * stScale + stOffset;
}
