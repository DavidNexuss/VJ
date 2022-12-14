#version 330 core

layout (location = 0) in vec3 aPosition;
layout (location = 3) in vec3 aNormal;
layout (location = 4) in vec3 aTangent;
layout (location = 2) in vec2 aUV;

uniform mat4 uProjectionMatrix;
uniform mat4 uViewMatrix;
uniform mat4 uTransformMatrix;

out mat3 vTBN;
out vec2 vUV;
out vec3 vWorldPos;

void main() {
    
    gl_Position = (uProjectionMatrix * uViewMatrix * uTransformMatrix * vec4(aPosition,1.0));
    vWorldPos = (uTransformMatrix * vec4(aPosition,1.0)).xyz;
    vec3 T = normalize(vec3(uTransformMatrix * vec4(aTangent,   0.0)));
    vec3 N = normalize(vec3(uTransformMatrix * vec4(aNormal,    0.0)));
    vec3 B = cross(N,T);

    vTBN = mat3(T, B, N);
    vUV = aUV;
}
