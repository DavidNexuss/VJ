#version 330 core
out vec3 color;
out vec3 hdr;
out vec3 position;
in vec3 globalPosition;
in vec3 normalColor;
void main()
{
  color = smoothstep(0.0,0.3,sin(globalPosition * 10.0));
  hdr = color * 0.4;
  position = globalPosition;
}
