#version 330 core
in vec2 vUV;
out vec4 color;
uniform sampler2D uBaseColor;

void main() { 
  color = texture2D(uBaseColor,vUV);
}
