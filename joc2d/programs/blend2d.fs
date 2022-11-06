#version 330 core
uniform sampler2D scene;

in vec2 vUV;
out vec3 color;
uniform float uTime;
uniform float delta = 0.002;

vec3 getBloom(float size) {
int n = 3;
	vec3 col = vec3(0.0);
	for(int i = -n; i < n; i++) {
		for(int j = -n; j < n; j++) {
			vec2 offset = vec2(i,j);
			float l = dot(offset,offset) + 1;
			l = pow(l,0.2);
			vec2 st = vUV + offset * size;
			vec3 aux =texture2D(scene, st).xyz - vec3(1.0);
			aux = max(vec3(0.0), aux); 
			col += (aux) / (l);
		}
	}
  return col / (n * n * 4);
}

vec3 gammaCorrection(vec3 color, float gamma, float exposure) { 
	vec3 mapped = vec3(1.0) - exp(-color * exposure);
    mapped = pow(mapped, vec3(1.0 / gamma));
    return mapped;
}
float w(float x) { 
	return sin(x) * 0.5+ 0.5;
}
void main() {
	
  vec3 baseColor = texture2D(scene, vUV).xyz * 0.5;
  vec3 bloomColor = getBloom(delta) * 0.7;
  
  float h = w(uTime * 2.0 + vUV.x * 10.0)*0.2;
  bloomColor *= 1.4 - h;
  color = bloomColor + baseColor * (0.8 + h * 0.5);
  color = gammaCorrection(color,1.5,0.8);
}








