#version 450 core

out vec4 color;
in vec2 uv;
uniform sampler2D image;
  
int samplingSize = 8;
const int samplingCount = 1;
const float factor = 1.0;

float weight(float index) { 
  return 1 - index * 0.0001;
}
void main()
{             
    vec2 tex_offset = 1.0 / textureSize(image, 0); // gets size of single texel
    vec3 result = texture(image, uv).rgb * weight(0);
    for(int j = 0; j < samplingCount; j++) {

        for(int x = 1; x < samplingSize; ++x) {

          float offx = pow(x,factor);
          for(int y = 1; y < samplingSize; ++y) {
            
            int l = x*x + y*y;
            float offy = pow(y,factor);
            result += texture(image, uv + vec2(tex_offset.x * offx, tex_offset.y * offy )).rgb * weight(l) * 0.01;
            result += texture(image, uv - vec2(tex_offset.x * offx, tex_offset.y * offy )).rgb * weight(l) * 0.01;
          }
        }
    }

    color = min(vec4(0.8), vec4(result, 1.0) * 2);
}

/*
const int samples = 35,
          LOD = 2,         // gaussian done on MIPmap at scale LOD
          sLOD = 1 << LOD; // tile size = 2^LOD
const float sigma = float(samples) * .25;

float gaussian(vec2 i) {
    return exp( -.5* dot(i/=sigma,i) ) / ( 6.28 * sigma*sigma );
}

vec4 blur(sampler2D sp, vec2 U, vec2 scale) {
    vec4 O = vec4(0);  
    int s = samples/sLOD;
    
    for ( int i = 0; i < s*s; i++ ) {
        vec2 d = vec2(i%s, i/s)*float(sLOD) - float(samples)/2.;
        O += gaussian(d) * textureLod( sp, U + scale * d , float(LOD) );
    }
    
    return O / O.a;
}

void main() {
    color = blur( image , uv ,1.0 / textureSize(image, 0));
} */
