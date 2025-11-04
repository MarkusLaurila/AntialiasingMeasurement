#version 330 core

in vec2 TexCoords;
out vec4 FragColor;


uniform sampler2D screenTexture;
uniform vec2 screenSize;
uniform int currentAA;
uniform int sampleCount;
uniform sampler2D historyTexture;
uniform float blendFactor;
uniform vec2 jitter;
#define FXAA_REDUCE_MIN   (1.0/128.0)
#define FXAA_REDUCE_MUL   (1.0/8.0)
#define FXAA_SPAN_MAX     8.0

void main()
{
    switch(currentAA){
        case 0:{
            FragColor = texture(screenTexture, TexCoords);
            break;

        }
        //FXAA
        case 1:{
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
            float lumaM  = dot(rgbM,  luma);

            float lumaMin = min(lumaM, min(min(lumaNW, lumaNE), min(lumaSW, lumaSE)));
            float lumaMax = max(lumaM, max(max(lumaNW, lumaNE), max(lumaSW, lumaSE)));

            vec2 dir;
            dir.x = -((lumaNW + lumaNE) - (lumaSW + lumaSE));
            dir.y =  ((lumaNW + lumaSW) - (lumaNE + lumaSE));

            float dirReduce = max((lumaNW + lumaNE + lumaSW + lumaSE) *
            (0.25 * FXAA_REDUCE_MUL), FXAA_REDUCE_MIN);

            float rcpDirMin = 1.0 / (min(abs(dir.x), abs(dir.y)) + dirReduce);
            dir = clamp(dir * rcpDirMin, vec2(-FXAA_SPAN_MAX), vec2(FXAA_SPAN_MAX)) / screenSize;

            vec3 result1 = texture(screenTexture, TexCoords + dir * (1.0 / 3.0 - 0.5)).rgb;
            vec3 result2 = texture(screenTexture, TexCoords + dir * (2.0 / 3.0 - 0.5)).rgb;

            FragColor = vec4((result1 + result2) * 0.5, 1.0);
            break;
        }
        //SSAA
        case 2:
        {

            vec2 texelSize = 1.0 / screenSize;

            vec3 color = vec3(0.0);
            color += texture(screenTexture, TexCoords + vec2(-0.25, -0.25) * texelSize).rgb;
            color += texture(screenTexture, TexCoords + vec2( 0.25, -0.25) * texelSize).rgb;
            color += texture(screenTexture, TexCoords + vec2(-0.25,  0.25) * texelSize).rgb;
            color += texture(screenTexture, TexCoords + vec2( 0.25,  0.25) * texelSize).rgb;

            color *= 0.25;

            FragColor = vec4(color, 1.0);
        break;
        }
        //Software Msaa because OPENGL wont allow you to access pipeline unless you write a rasterisation yourself
        case 3:
        {
            const int sampleCount = 4;
            vec2 texelSize = 1.0 / screenSize;
            vec2 offsets[4] = vec2[](
            vec2(-0.25, -0.25),
            vec2( 0.25, -0.25),
            vec2(-0.25,  0.25),
            vec2( 0.25,  0.25)
            );

            vec3 color = vec3(0.0);

            for (int i = 0; i < sampleCount; ++i)
            {
                vec2 sampleUV = TexCoords + offsets[i] * texelSize;
                color += texture(screenTexture, sampleUV).rgb;
            }

            color /= float(sampleCount);
            FragColor = vec4(color, 1.0);
            break;
        }
        //TAA
        case 4:
        {
            vec3 currentColor = pow(texture(screenTexture, TexCoords).rgb, vec3(2.2));
            vec2 jitteredUV = TexCoords - jitter;
            vec3 historyColor = pow(texture(historyTexture, jitteredUV).rgb, vec3(2.2));

            vec3 finalColor = mix(currentColor, historyColor, blendFactor);
            finalColor = clamp(finalColor, 0.0, 1.0);
            FragColor = vec4(pow(finalColor, vec3(1.0/2.2)), 1.0);
            break;
        }
        default:{
            FragColor = texture(screenTexture, TexCoords);
            break;

        }
    }


return;


}