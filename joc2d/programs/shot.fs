#version 330 core
in vec2 vUV;
flat in int vInstanceID;

out vec4 color;

uniform vec2 uv_scale = vec2(1.0);
uniform vec2 uv_offset = vec2(0.0);

uniform float pixelize = 80.0;
uniform float uTime;

uniform int type[500];

vec4 light(vec2 st, vec3 c, float po) { 

  float d = length(st) * 10.0;
  d = pow(d,po);
  d = 1.0 / d;
  return vec4(d * c, d);
}

void main() { 
  int t = type[vInstanceID];

  vec2 st = (vUV * uv_scale + uv_offset) - vec2(0.5);
  st = floor(st * pixelize) / pixelize;

  vec3 tint_color;
  vec3 inner_tint_color;
  if(t == 1) { 
    tint_color = vec3(-0.3,0.5,1.2);
    inner_tint_color = vec3(-0.3,0.8,0.1);
  } else { 
    tint_color = vec3(0.5,1.2,-0.3);
    inner_tint_color = vec3(-0.3,0.8,0.1);
  }

  vec4 a = light(st * 1, tint_color, 3);
  vec4 b = light(st * 0.9, inner_tint_color, 3);

  color = a - b;
  color.a = max(color.a,0.0);
  color.a = dot(color.xyz,vec3(1.0));
}
