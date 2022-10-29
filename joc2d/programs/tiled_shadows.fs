#version 330 core
in vec2 vUV;
out vec4 color;
uniform sampler2D input;
uniform sampler2D shadow;

void main() { 
  vec2 st = vUV;
  st.y = 1.0 - st.y;
  float shadow_val = 0.0;

  for(int i = -2; i < 2; i++) { 
    for(int j = -2; j < 2; j++) { 
      shadow_val += texture(shadow, st + vec2(i,j) * 0.002).x;
    }
  }
  shadow_val = shadow_val / 16.0;
  color = texture(input, st) - shadow_val * 0.25;
  color.a += shadow_val * 2.0;
  color.a = clamp(color.a,0.0,1.0);
}




