#version 440 core

in vec2 vUV;
uniform sampler2D uBaseColor;
out vec3 color;

void main() { 
    color = texture2D(uBaseColor, vUV).xyz;
}
