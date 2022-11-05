#version 330 core
uniform sampler2D input1;
out vec4 color;
in vec2 vUV;
uniform vec2 offset;
uniform vec2 scale;
uniform vec2 pivot;
uniform float uTime;
uniform float zval;
uniform float zspeed;
uniform vec3 uViewPos;

void main() { 
  vec2 st = vUV * scale - offset + pivot * sin(uTime) + vec2(uViewPos.x * (zspeed),0.0);
  st += vec2(uTime * 0.004, 0.0);
  st = fract(st);
  st.y = min(st.y,0.99);
  st.y = max(st.y,0.01);
  st.y = 1.0 - st.y;
  color = texture(input1, st);
  color.a = length(color) - 1.2;
  color.a *= 0.1;
  color.x *= 0.8;
  color.y *= (0.2 + sin(uTime * 0.1 + vUV.x) * 0.2);
  color.z *= 0.6;
  color.a = -2.0;
}

