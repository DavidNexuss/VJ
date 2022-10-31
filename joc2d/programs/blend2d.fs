#version 330 core
uniform sampler2D scene;

in vec2 vUV;
out vec3 color;

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
void main() {
	
  vec3 baseColor = texture2D(scene, vUV).xyz * 0.5;
  vec3 bloomColor = getBloom(0.006) * 0.7;
  color = bloomColor + baseColor;
  color = gammaCorrection(color,1.5,0.8);
}







