#version 330 core
uniform sampler2D input;
out vec4 color;
in vec2 uv;
uniform vec2 offset;
uniform vec2 scale;
uniform vec2 pivot;
uniform float uTime;
uniform float zval;
uniform float zspeed;

void main() { 
  vec2 st = uv * scale - offset + pivot * sin(uTime);
  st.y = min(st.y,0.99);
  st.y = max(st.y,0.01);
  st.y = 1.0 - st.y;
  color = texture(input, st);
}
