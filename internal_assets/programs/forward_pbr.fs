#version 330 core

in mat3 vTBN;
in vec2 vUV;

layout(location = 0) out vec3 albedo;
layout(location = 1) out vec3 normal;
layout(location = 2) out vec4 special;

uniform sampler2D uSpecial;
uniform sampler2D uBump;
uniform sampler2D uBaseColor;

vec3 getNormal() { 
  vec3 N = texture2D(uBump, vUV * 3.0).xyz;
  return N * 2.0 - 1.0;
}

void main() { 
  albedo = texture2D(uBaseColor, vUV).xyz;
  special = texture2D(uSpecial, vUV);
  normal = normalize(vTBN * getNormal());
}
