#version 330 core
in vec2 vUV;
out vec4 color;
uniform sampler2D base;
uniform vec2 uv_scale;
uniform vec2 uv_offset;
uniform float hit = 0.0;

void main() { 
  vec2 st = vUV * uv_scale + uv_offset;
  color = texture2D(base,st);
  color.yz *= (2.0 - hit)  / 2.0;
}
