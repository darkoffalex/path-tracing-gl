/**
 * Данный шейдер используется для заключительного этапа - ростой пост-обработки и вывода изображения
 */

#version 420 core

#define GAUSSIAN_BLUR true

/*Схема входа-выхода*/

layout (location = 0) out vec4 color;        // Итоговый цвет фрагмента

/*Uniform*/

uniform sampler2D iScreenTexture;
uniform vec2 iScreenSize;

/*Вход*/

in VS_OUT
{
    vec2 uv;
} fs_in;

/*Функции*/

/**
 * Спизжено - не понимаю что происходит
 */
float normpdf(in float x, in float sigma)
{
    return 0.39894*exp(-0.5*x*x/(sigma*sigma))/sigma;
}

/**
 * Спизжено - не понимаю что происходит
 */
vec4 gaussianBlur(in vec2 fragCoord)
{
    // Declare stuff
    const int mSize = 3;
    const int kSize = (mSize-1)/2;
    float kernel[mSize];
    vec3 finalColor = vec3(0.0);

    // Create the 1-D kernel
    float sigma = 7.0;
    float Z = 0.0;
    for (int j = 0; j <= kSize; ++j)
    {
        kernel[kSize+j] = kernel[kSize-j] = normpdf(float(j), sigma);
    }

    //get the normalization factor (as the gaussian has been clamped)
    for (int j = 0; j < mSize; ++j)
    {
        Z += kernel[j];
    }

    //read out the texels
    for (int i=-kSize; i <= kSize; ++i)
    {
        for (int j=-kSize; j <= kSize; ++j)
        {
            finalColor += kernel[kSize+j]*kernel[kSize+i]*texture(iScreenTexture, (fragCoord.xy) + (vec2(float(i),float(j)) / iScreenSize)).rgb;
        }
    }

    return vec4(finalColor/(Z*Z), 1.0f);
}

/**
 * Основная функция фрагментного шейдера
 * Каждый фрамент растеризуемого полноэкранного квадрата (каждый пиксель экрана) соответствует одному или нескольким лучам
 */
void main()
{
    // Результирующий цвет
    vec4 resultColor = GAUSSIAN_BLUR ? gaussianBlur(fs_in.uv).rgba : texture(iScreenTexture, fs_in.uv).rgba;

    // Гамма коррекция
    //vec3 result = pow(clamp(resultColor.rgb,0.0,1.0),vec3(0.45));
    vec3 result = sqrt(clamp(resultColor.rgb,0.0,1.0));

    color = vec4(result, 1.0);
}