#version 330 core

in vec2 aPosition;
in float type;
in float emissionTime;

uniform mat4 uProjectionMatrix;
uniform mat4 uViewMatrix;

out float vEmission;
void main() {
    
    gl_Position = (uProjectionMatrix * uViewMatrix * vec4(vec3(aPosition,0.0),1.0));
    vEmission = emissionTime;
}
