#version 330 core
in vec2 vUV;
out vec4 color;
uniform sampler2D input1;

void main() { 
  color = texture2D(input1,vUV);
}
