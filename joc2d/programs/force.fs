#version 330 core
out vec4 color;
in float vEmission;
uniform float uTime;

void main() { 
  float t = 1.0 / max(1.0, uTime - vEmission);
  t = t * t;
color = vec4(vec3(t * 5.0) * vec3(0.5,1.2,0.5),t);
}

