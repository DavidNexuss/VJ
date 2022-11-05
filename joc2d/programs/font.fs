#version 330 core
uniform sampler2D base;
in vec2 vUV;
out vec4 color;
void main() { 
  color = texture2D(base, vUV);
}
