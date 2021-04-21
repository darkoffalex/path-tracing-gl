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
    vec3 resultColor = texture(iScreenTexture, fs_in.uv).rgb;
    color = vec4(resultColor, 1.0);
}