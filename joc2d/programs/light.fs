#version 330 core
in vec2 vUV;
out vec4 color;

uniform vec2 uv_scale = vec2(1.0);
uniform vec2 uv_offset = vec2(0.0);
uniform vec3 tint_color = vec3(-0.3,0.5,1.2);
uniform float pixelize = 40.0;
uniform float uTime;

void main() { 
  vec2 st = (vUV * uv_scale + uv_offset) - vec2(0.5);
  st = floor(st * pixelize) / pixelize;
  float d = length(st) * 10.0;
  d = d * d;
  float s = min(0.4,1.0 / d);
  color = vec4(tint_color * s,s);
}
