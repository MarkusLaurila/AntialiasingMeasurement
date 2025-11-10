#version 330 core

in vec2 TexCoord;
out vec4 color;

uniform sampler2D material_baseColor;
uniform sampler2D material_normal;
uniform sampler2D material_roughness;
uniform sampler2D material_ao;

uniform vec3 lightDir;
uniform vec3 lightColor;

void main()
{

    vec3 baseColor = texture(material_baseColor, TexCoord).rgb;


    vec3 normalMap = texture(material_normal, TexCoord).rgb;
    vec3 N = normalize(normalMap * 2.0 - 1.0);


    float diffuse = max(dot(N, -lightDir), 0.0);
    vec3 lighting = baseColor * diffuse * lightColor;


    float ao = texture(material_ao, TexCoord).r;
    lighting *= ao;

    color = vec4(lighting, 1.0);
}
