#version 330 core
uniform sampler2D input1;
out vec4 color;
in vec2 vUV;
uniform float shadowLevel;

void main() { 
  vec2 st = vUV;
  st.y = 1.0 - st.y;
  color = texture(input1, st);
  color = vec4(vec3(shadowLevel / 300.0) * color.a,color.a);
}
