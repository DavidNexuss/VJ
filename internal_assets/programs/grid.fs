#version 330 core

in vec3 vWorldPos;
out vec4 color;
#define PI 3.1415926538

float fun(float p) { 
    float diff = 0.3;
    p = cos(p * PI * 2);
    p = pow(p,6.0);
    p = smoothstep(1.0 - diff,1.0,p);
    return p;
}
float grid(vec2 p) { 
    return max(fun(p.x), fun(p.y));
}
void main() { 
    color = vec4(grid(vWorldPos.xy));
    color += vec4(0.0,0.4,1.0,0.02) * 0.3;
    color *= 0.6;
}
