#version 330 core
out vec4 FragColor;

in vec2 TexCoord;
in vec3 FragPos;
in vec3 Normal;
in vec3 Tangent;
in vec3 Bitangent;

struct Material {
    sampler2D diffuse;
    sampler2D normal;
    sampler2D roughness;
    sampler2D ao;
    sampler2D mask;
    sampler2D metallic;

    bool hasDiffuse;
    bool hasNormal;
    bool hasRoughness;
    bool hasAO;
    bool hasMask;
    bool hasMetallic;
};

uniform Material material;

uniform vec3 cameraPos;
uniform vec3 lightDir;
uniform vec3 lightColor;

void main()
{

    vec3 albedo = material.hasDiffuse
    ? texture(material.diffuse, TexCoord).rgb
    : vec3(1.0);


    vec3 N;

    if (material.hasNormal)
    {
        vec3 normalMap = texture(material.normal, TexCoord).rgb * 2.0 - 1.0;

        vec3 T = normalize(Tangent);
        vec3 B = normalize(Bitangent);
        vec3 Ng = normalize(Normal);

        mat3 TBN = mat3(T, B, Ng);
        N = normalize(TBN * normalMap);
    }
    else
    {
        N = normalize(Normal);
    }

    vec3 L = normalize(-lightDir);
    vec3 V = normalize(cameraPos - FragPos);
    vec3 H = normalize(L + V);

    float ao = material.hasAO
    ? max(texture(material.ao, TexCoord).r, 0.1)
    : 1.0;

    float roughness = material.hasRoughness
    ? clamp(texture(material.roughness, TexCoord).r, 0.05, 1.0)
    : 0.5;

    float metallic = material.hasMetallic
    ? texture(material.metallic, TexCoord).r
    : 0.0;

    float mask = material.hasMask
    ? texture(material.mask, TexCoord).r
    : 1.0;

    float NdotL = max(dot(N, L), 0.0);


    float shininess = mix(32.0, 2.0, roughness);

    float specAngle = max(dot(N, H), 0.0);
    float spec = pow(specAngle, shininess);

    vec3 F0 = mix(vec3(0.04), albedo, metallic);

    vec3 specular = spec * F0 * lightColor;
    vec3 diffuse  = albedo * NdotL * (1.0 - metallic) * lightColor;

    diffuse  *= mask;
    specular *= mask;

    vec3 ambient = albedo * 0.15 * ao;

    vec3 color = ambient + diffuse + specular;
    color = pow(color, vec3(1.0/2.2));

    FragColor = vec4(color, 1.0);
}
