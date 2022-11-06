#version 330 core
uniform sampler2D base;

in vec2 uv;
out vec3 color;
void main() { 
  color = texture(base, uv).xyz;
}
