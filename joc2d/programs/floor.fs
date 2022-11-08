#version 330 core
in vec2 vUV;
out vec4 color;
uniform sampler2D base;
uniform sampler2D input2;
uniform vec2 uv_scale;
uniform vec2 uv_offset;
uniform float uTime;
#define PI 3.1456


void main() { 
  vec2 st = vUV * uv_scale + uv_offset;
  vec3 c = texture(base, st).xyz;
  vec3 f = texture(input2, st).xyz;
  
  float s = smoothstep(-0.2,0.4,vUV.x) - 0.3;
  float a = max(0.0,s);
  color = vec4((c + f * a * 0.1) * a,a);
}
