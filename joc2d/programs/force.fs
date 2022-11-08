#version 330 core
out vec4 color;
in float vEmission;
uniform float uTime;

void main() { 
  float t = 1.0 / max(1.0, uTime - vEmission);
  t = t * t * t  *t;
color = vec4(vec3(t * 17.0) * vec3(0.1,0.3,0.8),t);
}






















