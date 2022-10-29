#version 330 core
uniform sampler2D input;

in vec2 uv;
out vec3 color;
void main() { 
  color = texture(input, uv).xyz;
}
