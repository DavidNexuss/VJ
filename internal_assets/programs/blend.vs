#version 330 core
layout (location = 0) in vec2 aPosition;

out vec2 uv;

void main()
{
    gl_Position = vec4(aPosition.x, aPosition.y, 0.0, 1.0); 
    uv = aPosition*0.5 + 0.5;
}  
