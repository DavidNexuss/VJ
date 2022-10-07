#version 440 core

in mat3 vTBN;
in vec2 vUV;
in vec3 vWorldPos;

uniform sampler2D uSpecial;
uniform sampler2D uBump;
uniform sampler2D uBaseColor;
uniform samplerCube uSkyBox;

uniform vec3 uViewPos;

layout(location = 0) out vec3 color;
layout(location = 1) out vec3 glow;

vec3 getNormal() { 
  vec3 N = texture2D(uBump, vUV).xyz;
  return N * 2.0 - 1.0;
}

uniform float metallicDrag = 0.4;
uniform float reflectEnv = 0.2;
uniform float specularTint = 10.0;
uniform float glowMaximizer = 10.0;
uniform float kd = 1.2;

const float factor = 3.0;
void main() { 
  vec3 albedo = texture2D(uBaseColor, vUV).xyz;
  vec4 special = texture2D(uSpecial, vUV);
  vec3 normal = normalize(vTBN * getNormal());
  vec3 cameraDir = normalize(vWorldPos - uViewPos);

  vec3 D = normalize(vec3(1.0,1.0,0.3));
  vec3 R = reflect(cameraDir,normal);

  float ao = special.x;
  float roughness = special.y;
  float metallic = special.z;

  float specular = max(0.0,dot(R,D));
  float fresnel = 1.0 - max(0.0,dot(-cameraDir, normal));

  specular = pow(specular, specularTint);
  specular *= max(0.0,(1.0 - roughness + fresnel));
  vec3 ambientReflection = texture(uSkyBox, R).xyz;

  float diffuse = max(0.0,dot(normal,D));
  diffuse = max(0.0,diffuse - metallic * metallicDrag);
  diffuse *= kd;

  color = albedo * diffuse * ao + vec3(specular) + ambientReflection * max(0.0,(metallic - roughness + fresnel)) * reflectEnv;
  glow = color - 1.0;
  glow = glow * metallic * glowMaximizer * specular * (1.0 - roughness);
}
