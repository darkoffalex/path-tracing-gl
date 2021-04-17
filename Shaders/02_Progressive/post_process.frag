/**
 * Данный шейдер используется для заключительного этапа - ростой пост-обработки и вывода изображения
 */

#version 420 core

/*Схема входа-выхода*/

layout (location = 0) out vec4 color;        // Итоговый цвет фрагмента

/*Uniform*/

uniform sampler2D iScreenTexture;

/*Вход*/

in VS_OUT
{
    vec2 uv;
} fs_in;

/*Функции*/

/**
 * Основная функция фрагментного шейдера
 * Каждый фрамент растеризуемого полноэкранного квадрата (каждый пиксель экрана) соответствует одному или нескольким лучам
 */
void main()
{
    // Результирующий цвет
    vec4 resultColor = texture(iScreenTexture, fs_in.uv).rgba;

    // Поделить аккумулированный результат на кол-во кадров
    vec3 result = resultColor.rgb / resultColor.a;

    // Гамма коррекция
    result = sqrt(clamp(result,0.0,1.0));

    color = vec4(result, 1.0);
}