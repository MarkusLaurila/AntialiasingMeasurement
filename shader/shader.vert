#version 330 core
out vec4 FragColor;

in vec2 TexCoord;

struct Material {
    sampler2D diffuse;
    sampler2D normal;
    sampler2D roughness;
    sampler2D ao;
};

uniform Material material;
uniform vec3 lightDir;
uniform vec3 lightColor;

void main()
{
    vec3 color = texture(material.diffuse, TexCoord).rgb;


    vec3 normalMap = texture(material.normal, TexCoord).rgb;
    vec3 N = normalize(normalMap * 2.0 - 1.0);
    float diffuse = max(dot(N, -lightDir), 0.0);
    color *= diffuse * lightColor;


    float ambientOcclusion = texture(material.ao, TexCoord).r;
    color *= ambientOcclusion;

    FragColor = vec4(color, 1.0);
}
