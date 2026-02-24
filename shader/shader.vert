#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;
layout(location = 3) in vec3 aTangent;
layout(location = 4) in vec3 aBitangent;

out vec2 TexCoord;
out vec3 FragPos;
out vec3 Normal;
out vec3 Tangent;
out vec3 Bitangent;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform bool hasDisplacement;
uniform sampler2D displacement;
uniform float displacementScale;

void main()
{
    mat3 normalMatrix = mat3(transpose(inverse(model)));

    vec3 N = normalize(normalMatrix * aNormal);
    vec3 T = normalize(normalMatrix * aTangent);
    vec3 B = normalize(normalMatrix * aBitangent);
    vec3 displacedPos = aPos;
    if (hasDisplacement)
    {
        float height = texture(displacement, aTexCoord).r;
        displacedPos += aNormal * (height * displacementScale);
    }

    vec4 worldPos = model * vec4(displacedPos, 1.0);

    FragPos  = worldPos.xyz;
    Normal   = N;
    Tangent  = T;
    Bitangent= B;
    TexCoord = aTexCoord;

    gl_Position = projection * view * worldPos;
}
