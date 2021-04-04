#version 330 core

#define SAMPLES 1

/*Схема входа-выхода*/

layout (location = 0) out vec4 color;        // Итоговый цвет фрагмента

/*Вспомогательные типы*/

// Луч
struct Ray
{
    vec3 origin;
    vec3 direction;
    float weight;
};

/*Uniform*/

uniform vec2 iScreenSize;
uniform vec3 iCamPosition;
uniform float iCamFov;
uniform mat4 iView;
uniform mat4 iCamModel;

/*Вход*/

in VS_OUT
{
    vec2 uv;
} fs_in;

/*Функции*/

/**
 * Пересечение луча со сферой
 * \param ray Луч
 * \param position Позиция центра сферы
 * \param radius Радиус сферы
 * \param tMin Минимальное расстояние до точки пересечения
 * \param tMax Максимальное расстояние до точки пересечения
 * \param tOut Расстояние от начала, до точки пересечения
 * \return Было ли пересечение с объектом
 */
bool raySphereIntersection(Ray ray, vec3 position, float radius, float tMin, float tMax, inout float tOut)
{
    // Уравнение сферы : (x−x0)^2+(y−y0)^2+(z−z0)^2 = R^2.
    // Уравнение сферы в векторном виде : |P−C|^2 = R^2 что эквивалентно dot((P−C),(P−C))=r2 (P - точка сфере, C - центр)
    // Параметрическое уравнение прямой (в векторном виде) : A+tB (А - начальная точка, B - направляющий вектор, t - параметр)
    // Подставив получим : dot((A+tB−C),(A+tB−C))=R^2
    // Преобразовав получим : t^2 * dot(B,B) + 2*dot(B,A−C) + dot(A−C,A−C) − r^2=0
    // Разделим на 3 части при коэффициенте t : a=dot(B,B) , b=2*dot(B,A−C), c=dot(A−C,A−C)−r2
    // Теперь можно решать квадратное уравнение относительно коэффициента t : t = (−b ± sqrt(b^2−4ac))/2a, где b2−4ac - дискриминант

    // Вектор к началу луча от центра сферы (для вычисления коэффициентов квадратного уравнения)
    vec3 oc = ray.origin - position;

    // Коэффициенты квадратного уравнения
    float a = dot(ray.direction,ray.direction);
    float b = 2.0f * dot(ray.direction,oc);
    float c = dot(oc,oc) - (radius * radius);

    // Дискриминант
    float discriminant = b*b - 4.0f * a * c;

    // Если дискриминант отрицательный - пересечения не было
    if(discriminant < 0) return false;

    // 2 решения для параметра t
    float t1 = (-b - sqrt(discriminant))/(2.0f * a);
    float t2 = (-b + sqrt(discriminant))/(2.0f * a);

    // Нужна ближайшая точка пересечения, соотвественно t должен быть наименьший НО не отрицательный
    // Если оба значения t отрицательны, значит сфера сзади начальной точки луча, соответственно пересечния не было
    if(t1 < tMin && t2 < tMin) return false;

    // Если точка пересечения сзади, либо слишком близка, делаем t очень большим, чтобы при выборе наименьшего значения он не был выбрн
    if(t1 < tMin) t1 = tMax;
    if(t2 < tMin) t2 = tMax;
    float tResult = min(t1,t2);

    if(tResult <= tMax){
        tOut = tResult;
        return true;
    }

    return false;
}

/**
 * Получение направления луча в мировом пространстве
 * \param fov Угол обзора
 * \param aspectRatio Соотношение сторон
 * \param fragCoord Текстурные координаты фрамента
 * \param pixelBias Суб-пиксельное смещение для мульти-семплинга
 * \return Вектор направления
 */
vec3 getRayDirection(float fov, float aspectRatio, vec2 fragCoord, vec2 pixelBias)
{
    // Преобразовать текстурные координаты (0;1) в клип-координаты экрана (-1;1)
    // К координатам также добавляется суб-пиксельное смещение (используется при мультисемплинге)
    vec2 clipCoords = (fragCoord * 2.0) - vec2(1.0) + pixelBias;

    // Вектор направления с учетом угла обзора (fov) и пропорций экрана (aspectRatio)
    vec3 direction = vec3(
        clipCoords.x * tan(radians(fov) / 2.0) * aspectRatio,
        clipCoords.y * tan(radians(fov) / 2.0),
        -1.0);

    // Трансформировать вектор в мировое пространство
    vec3 directionWorld = (iCamModel * vec4(direction,0.0f)).xyz;

    // Вернуть направление
    return normalize(directionWorld);
}

/**
 * Основная функция фрагментного шейдера
 * Каждый фрамент растеризуемого полноэкранного квадрата (каждый пиксель экрана) соответствует одному или нескольким лучам
 */
void main()
{
    // Тестовая сфера
    vec4 sphere = vec4(0.0f,0.0f,-2.0f,1.0f);

    // Итоговый цвет
    vec3 resultColor = vec3(0.0f);

    // Для каждого семпла
    for(int i = 0; i < SAMPLES; i++)
    {
        // Отклонение от центра пикселя
        // TODO: в случае с кол-вом семплов большим чем единица отклонение должно быть случайным
        vec2 pixelBias = vec2(1.0f / iScreenSize.x, 1.0f / iScreenSize.y);

        // Генерация луча
        Ray ray = Ray(
            (iCamModel * vec4(0.0f,0.0f,0.0f,1.0f)).xyz,
            getRayDirection(iCamFov, iScreenSize.x/iScreenSize.y, fs_in.uv, pixelBias),
            1.0f
        );

        // Расстояние до пересечения
        float t = 0.0f;
        // Если было пересечение
        if(raySphereIntersection(ray, sphere.xyz, sphere.w, 0.01f, 1000.0f, t)){
            resultColor += vec3(1.0f,0.0f,0.0f);
        }
    }

    // Усреднить по кол-во семплов на пиксель
    resultColor /= SAMPLES;

    // Присваиваем красный цвет (временно)
    color = vec4(resultColor,1.0f);
}