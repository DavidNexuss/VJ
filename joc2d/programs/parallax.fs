#version 330 core
uniform sampler2D base;
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
  
  st.y = 1.0 - st.y;
  st.x = fract(st.x);
  st.y = max(st.y,0.01);
  if(st.y > 0.9) { 
  	st.y = 0.83 + fract(st.y / 0.02) * 0.02;
  }
  color = texture(base, st) * (1 + hdr);
}


