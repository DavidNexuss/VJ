#version 450 core
out vec4 color;
in vec2 uv;
uniform sampler2D scene;
uniform sampler2D bloom;
uniform float exposure = 1.0;

void gammaCorrection() {

    const float gamma = 1.2;
    vec3 hdrColor = texture(scene, uv).rgb;
    vec3 bloomColor = texture(bloom, uv).rgb;
    hdrColor += bloomColor; // additive blending
    // tone mapping
    vec3 result = vec3(1.0) - exp(-hdrColor * exposure);
    // also gamma correct while we're at it       
    result = pow(result, vec3(1.0 / gamma));
    color = vec4(result, 1.0);

    //color = vec4(vec3(shadow),1.0);
}
void additiveBlending() { 
  color = vec4(texture(scene,uv).rgb + texture(bloom, uv).rgb,1.0);
}
void main()
{             
  //gammaCorrection();
  additiveBlending();
}  
