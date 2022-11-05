#version 330 core
uniform sampler2D input1;

in vec2 uv;
out vec3 color;
void main() { 
  color = texture(input1, uv).xyz;
}
