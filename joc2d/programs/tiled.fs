#version 330 core
uniform sampler2D base;
in vec2 vUV;
out vec4 color;
uniform vec4 add;
uniform vec4 mul;
void main() { 
  color = texture2D(base, vUV) * mul + add;
}
