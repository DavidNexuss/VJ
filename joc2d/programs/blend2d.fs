#version 330 core
uniform sampler2D scene;

in vec2 vUV;
out vec3 color;

void main() { 
  color = texture2D(scene,vUV).xyz;
}



