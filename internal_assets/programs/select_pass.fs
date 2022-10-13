#version 330 core
uniform int uModelID;
out vec3 color;
void main() { 
    color = vec3(float(uModelID) / 255.0f);
}
