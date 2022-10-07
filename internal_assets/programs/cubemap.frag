#version 330 core
layout (location =0) out vec3 color;
layout (location =1) out vec3 hdr;
in vec3 pos;
uniform samplerCube uSkyBox;

void main()
{    
    color = texture(uSkyBox,pos).xyz;
    hdr = color - vec3(0.7);
    hdr = hdr * hdr * hdr * 2.0;
}
