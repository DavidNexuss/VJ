#version 330 core
in vec2 uv;

uniform sampler2D uSpecial;
uniform sampler2D uBump;
uniform sampler2D uBaseColor;
uniform sampler2D uDepth;

out vec3 color;
void main() { 
  color = (texture2D(uBaseColor, uv)).xyz + vec3(texture2D(uDepth,uv).r * 0.5);
}

