#version 330 core

layout (location = 0) in vec2 aPos;    // vertex position in NDC (clip space)
layout (location = 1) in vec2 aTexCoords; // texture coordinates

out vec2 TexCoords;

void main()
{
    TexCoords = aTexCoords;
    gl_Position = vec4(aPos.xy, 0.0, 1.0);
}