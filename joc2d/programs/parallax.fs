#version 330 core
uniform sampler2D input;
out vec4 color;
in vec2 vUV;
uniform vec2 offset;
uniform vec2 scale;
uniform vec2 pivot;
uniform float uTime;
uniform float zval;
uniform float zspeed;
uniform float vzspeed;
uniform vec3 uViewPos;
uniform float uAR;
uniform float hdr;

void main() { 
  vec2 st = vUV - offset + pivot * sin(uTime) + vec2(uViewPos.x * (zspeed),uViewPos.y * (vzspeed));
  st.x *= uAR;
  st = fract(st);
  st.y = min(st.y,0.99);
  st.y = max(st.y,0.01);
  st.y = 1.0 - st.y;
  color = texture(input, st) * (1 + hdr);
}
