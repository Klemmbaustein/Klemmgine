#version 330 
out vec4 FragColor;
  
in vec2 v_texcoords;

uniform sampler2D image;
  
uniform bool horizontal = false;
uniform float weight[5] = float[] (0.257027, 0.2245946, 0.1516216, 0.084054, 0.016216);

void main()
{
    vec2 tex_offset = 1.f / textureSize(image, 0); // gets size of single texel
    vec3 result = texture(image, v_texcoords).rgb * weight[0]; // current fragment's contribution
    if(horizontal)
    {
        for(int i = 1; i < 3; ++i)
        {
            result += texture(image, v_texcoords + vec2(tex_offset.x * i, 0.0)).rgb * weight[i];
            result += texture(image, v_texcoords - vec2(tex_offset.x * i, 0.0)).rgb * weight[i];
        }
    }
    else
    {
        for(int i = 1; i < 3; ++i)
        {
            result += texture(image, v_texcoords + vec2(0.0, tex_offset.y * i)).rgb * weight[i];
            result += texture(image, v_texcoords - vec2(0.0, tex_offset.y * i)).rgb * weight[i];
        }
    }
    FragColor = vec4(result, 1.0);
}