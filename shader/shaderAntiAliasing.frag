#version 330 core
in vec2 TexCoords;
out vec4 FragColor;

uniform sampler2D screenTexture;
uniform vec2 screenSize;
uniform int currentAA;
uniform sampler2D historyTexture;
uniform float blendFactor;
uniform vec2 jitter;

#define FXAA_REDUCE_MIN 1.0/512.0
#define FXAA_REDUCE_MUL 0.5
#define FXAA_SPAN_MAX 16.0
/*
NO AA = 0
FXAA = 1
SSAA = 2
MSAA = 3
TAA = 4
*/
void main() {
    switch(currentAA) {
        case 0: {
            FragColor = texture(screenTexture, TexCoords);
            break;
        }
        case 1: {
            vec3 rgbNW = texture(screenTexture, TexCoords + (vec2(-1.0, -1.0) / screenSize)).rgb;
            vec3 rgbNE = texture(screenTexture, TexCoords + (vec2(1.0, -1.0) / screenSize)).rgb;
            vec3 rgbSW = texture(screenTexture, TexCoords + (vec2(-1.0, 1.0) / screenSize)).rgb;
            vec3 rgbSE = texture(screenTexture, TexCoords + (vec2(1.0, 1.0) / screenSize)).rgb;
            vec3 rgbM  = texture(screenTexture, TexCoords).rgb;
            vec3 luma = vec3(0.299, 0.587, 0.114);
            float lumaNW = dot(rgbNW, luma);
            float lumaNE = dot(rgbNE, luma);
            float lumaSW = dot(rgbSW, luma);
            float lumaSE = dot(rgbSE, luma);
            float lumaM  = dot(rgbM, luma);
            float dirReduce = max((lumaNW + lumaNE + lumaSW + lumaSE) * (0.25 * FXAA_REDUCE_MUL), FXAA_REDUCE_MIN);
            vec2 dir;
            dir.x = -((lumaNW + lumaNE) - (lumaSW + lumaSE));
            dir.y =  ((lumaNW + lumaSW) - (lumaNE + lumaSE));
            float rcpDirMin = 1.0 / (min(abs(dir.x), abs(dir.y)) + dirReduce);
            dir = clamp(dir * rcpDirMin, vec2(-FXAA_SPAN_MAX), vec2(FXAA_SPAN_MAX)) / screenSize;
            vec3 result1 = texture(screenTexture, TexCoords + dir * (1.0/3.0 - 0.5)).rgb;
            vec3 result2 = texture(screenTexture, TexCoords + dir * (2.0/3.0 - 0.5)).rgb;
            FragColor = vec4((result1 + result2) * 0.5, 1.0);
            break;
        }
        case 2: {
            FragColor = texture(screenTexture, TexCoords);
            break;
            //OLD
           /* vec2 texelSize = 1.0 / screenSize;
            vec2 offsets[16] = vec2[](
            vec2(-0.375, -0.375), vec2(-0.125, -0.375), vec2(0.125, -0.375), vec2(0.375, -0.375),
            vec2(-0.375, -0.125), vec2(-0.125, -0.125), vec2(0.125, -0.125), vec2(0.375, -0.125),
            vec2(-0.375, 0.125), vec2(-0.125, 0.125), vec2(0.125, 0.125), vec2(0.375, 0.125),
            vec2(-0.375, 0.375), vec2(-0.125, 0.375), vec2(0.125, 0.375), vec2(0.375, 0.375)
            );
            vec3 color = vec3(0.0);
            for(int i = 0; i < 16; ++i) {
                color += texture(screenTexture, TexCoords + offsets[i] * texelSize).rgb;
            }
            color /= 16.0;
            FragColor = vec4(color, 1.0);
            break;*/
        }
        case 3: {
            vec2 texelSize = 1.0 / screenSize;
            vec2 offsets[16] = vec2[](
            vec2(-0.375, -0.375), vec2(-0.125, -0.375), vec2(0.125, -0.375), vec2(0.375, -0.375),
            vec2(-0.375, -0.125), vec2(-0.125, -0.125), vec2(0.125, -0.125), vec2(0.375, -0.125),
            vec2(-0.375, 0.125), vec2(-0.125, 0.125), vec2(0.125, 0.125), vec2(0.375, 0.125),
            vec2(-0.375, 0.375), vec2(-0.125, 0.375), vec2(0.125, 0.375), vec2(0.375, 0.375)
            );
            vec3 color = vec3(0.0);
            for(int i = 0; i < 16; ++i) {
                color += texture(screenTexture, TexCoords + offsets[i] * texelSize).rgb;
            }
            color /= 16.0;
            FragColor = vec4(color, 1.0);
            break;
        }
        case 4: {
            vec3 accumulatedColor = vec3(0.0);
            vec2 jitters[8] = vec2[](
            vec2(-0.375,-0.375), vec2(-0.125,-0.375), vec2(0.125,-0.375), vec2(0.375,-0.375),
            vec2(-0.375,-0.125), vec2(-0.125,-0.125), vec2(0.125,-0.125), vec2(0.375,-0.125)
            );
            for(int i = 0; i < 8; ++i) {
                vec2 sampleUV = TexCoords + jitter - jitters[i] / screenSize;
                accumulatedColor += pow(texture(historyTexture, sampleUV).rgb, vec3(2.2));
            }
            accumulatedColor /= 8.0;
            vec3 currentColor = pow(texture(screenTexture, TexCoords).rgb, vec3(2.2));
            vec3 finalColor = mix(currentColor, accumulatedColor, blendFactor);
            FragColor = vec4(pow(clamp(finalColor,0.0,1.0), vec3(1.0/2.2)), 1.0);
            break;
        }
    }
}
