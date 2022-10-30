#version 330 core
layout (location = 0) in vec2 aPosition;
out vec2 vUV;
void main() { 
  gl_Position = vec4(aPosition.x* 2.0-1.0, aPosition.y *2.0 - 1.0, 0.0, 1.0);
  vUV = vec2(aPosition.x, aPosition.y);
}
