#version 330 core

/*Схема входа-выхода*/

layout (location = 0) out vec4 color;        // Итоговый цвет фрагмента

/*Uniform*/

uniform vec3 iCamPosition;
uniform float iAspectRatio;
uniform float iCamFov;
uniform mat4 iView;

/*Вход*/

in VS_OUT
{
    vec2 uv;
} fs_in;

/*Функции*/

// Основная функция фрагментного (пиксельного) шейдера
void main()
{
    // Присваиваем красный цвет (временно)
    color = vec4(1.0f,0.0f,0.0f,1.0f);
}