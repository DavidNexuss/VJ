#version 330 core
in vec2 vUV;
out vec4 color;
uniform sampler2D input;
uniform vec2 uv_scale;
uniform vec2 uv_offset;

void main() { 
  vec2 st = vUV * uv_scale + uv_offset;
  color = texture2D(input,st);
}
