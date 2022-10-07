#version 330 core
uniform vec3 uViewPos;
in vec3 vWorldPos;
out vec4 color;
uniform vec4 uColor = vec4(1.0);
void main() { 
  
    color = uColor;
}
