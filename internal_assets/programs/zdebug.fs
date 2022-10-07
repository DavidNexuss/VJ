#version 330 core

in mat3 vTBN;
in vec2 vUV;
in vec3 vWorldPos;

uniform vec3 uViewPos;
out vec3 color;

void main() { 
  vec3 dir = normalize(vWorldPos - uViewPos);
  vec3 n = vTBN[2];
  color = vec3(dot(dir,-n));
}
