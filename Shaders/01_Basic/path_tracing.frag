#version 330 core

#define M_PI 3.141592
#define M_RAD 57.2958

#define SAMPLES 8
#define PATH_LENGHT 3

/*Схема входа-выхода*/

layout (location = 0) out vec4 color;        // Итоговый цвет фрагмента

/*Вспомогательные типы*/

/**
 * Луч
 */
struct Ray
{
    // Точка начала луча
    vec3 origin;
    // Направление
    vec3 direction;
};

/**
 * Информация о пересечении луча со сценой
 */
struct RayHitInfo
{
    // Точка пересечения
    vec3 point;
    // Нормаль в точке пересечения
    vec3 normal;
    // Расстояние
    float t;
    // Является ли поверность лицевой
    bool isFrontFace;
    // Индекс материала (для дальнейшего)
    int materialIndex;
};

/*Uniform*/

uniform vec2 iScreenSize;
uniform vec3 iCamPosition;
uniform float iCamFov;
uniform mat4 iView;
uniform mat4 iCamModel;
uniform float iTime;

/*Вход*/

in VS_OUT
{
    vec2 uv;
} fs_in;

/*Функции*/

/**
 * Используется для генерции псевдослучайного значения от 0 до 1
 * \param seed Семя рандомизации (значение изменябщееся для каждого пикселя и,если необходимо, для каждого кадра)
 * \return Значение от 0 до 1
 */
float hash1(inout float seed) {
    return fract(sin(seed += 0.1)*43758.5453123);
}

/**
 * Используется для генерции пары псевдослучайных значений от 0 до 1
 * \param seed Семя рандомизации (значение изменябщееся для каждого пикселя и,если необходимо, для каждого кадра)
 * \return 2D-вектор со значениями компонентов от 0 до 1
 */
vec2 hash2(inout float seed) {
    return fract(sin(vec2(seed+=0.1,seed+=0.1))*vec2(43758.5453123,22578.1459123));
}

/**
 * Используется для генерции тройки псевдослучайных значений от 0 до 1
 * \param seed Семя рандомизации (значение изменябщееся для каждого пикселя и,если необходимо, для каждого кадра)
 * \return 3D-вектор со значениями компонентов от 0 до 1
 */
vec3 hash3(inout float seed) {
    return fract(sin(vec3(seed+=0.1,seed+=0.1,seed+=0.1))*vec3(43758.5453123,22578.1459123,19642.3490423));
}


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
 * Пересечение луча со сферой
 * \param ray Луч
 * \param tMin Минимальное расстояние до точки пересечения
 * \param tMax Максимальное расстояние до точки пересечения
 * \param hitInfo Информация о пересечении
 * \return Было ли пересечение с объектом
 */
bool raySceneIntersection(Ray ray, float tMin, float tMax, inout RayHitInfo hitInfo)
{
    // Было ли пересечение
    bool hit = false;
    // Ближайшее пересечение
    float tClosest = tMax;
    // Расстояние до точки пересечения
    float t = 0;

    // Набор примитивов сцены
    int primitiveCount = 2;
    // TODO: предусмотреть другие виды примитивов
    vec4[] spheres = vec4[](
        vec4(0.0f,0.0f,-2.0f,1.0f),
        vec4(0.0f,-6.0f,-2.0f,5.0f)
    );

    // Пройтись по примитивам
    for(int i = 0; i < primitiveCount; i++)
    {
        if(raySphereIntersection(ray,spheres[i].xyz,spheres[i].w,tMin,tMax,t) && t < tClosest)
        {
            // Пересечние засчитано
            hit = true;
            // Обновить расстояние ближ. пересечения
            tClosest = t;

            // Информация о пересечении
            hitInfo.t = t;
            hitInfo.point = ray.origin + ray.direction * t;
            hitInfo.normal = normalize(hitInfo.point - spheres[i].xyz);
            hitInfo.materialIndex = 0;
            hitInfo.isFrontFace = true;

            // Если нормаль не направлена против луча, считать что это обратная сторона (и инвертировать нормаль)
            if(dot(-(ray.direction),hitInfo.normal) < 0.0f){
                hitInfo.normal *= -1;
                hitInfo.isFrontFace = false;
            }
        }
    }

    return hit;
}

/**
 * Получение направления луча в мировом пространстве
 * \param fov Угол обзора
 * \param aspectRatio Соотношение сторон
 * \param fragCoord Текстурные координаты фрамента
 * \param pixelBias Суб-пиксельное смещение для мульти-семплинга в текстурных координатах
 * \return Вектор направления
 */
vec3 getCamRayDirection(float fov, float aspectRatio, vec2 fragCoord, vec2 pixelBias)
{
    // Преобразовать текстурные координаты (0;1) в клип-координаты экрана (-1;1)
    // К координатам также добавляется суб-пиксельное смещение (используется при мультисемплинге)
    vec2 clipCoords = ((fragCoord + pixelBias) * 2.0) - vec2(1.0);

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
 * Инициализация "семени" для случайных чисел
 * \param useTime Использовать время для рандомизации
 * \return Значение "семени"
 */
float initRndSeed(bool useTime)
{
    // Положение пикселя в clip-пространстве
    vec2 p = -1.0 + 2.0 * (fs_in.uv);
    p.x *= (iScreenSize.x/iScreenSize.y);

    // Использовать для seed положение пикселя (псевдослучайные значения будут отличаться для каждого пикселя)
    float seed = p.x + fract(p.y * 18753.43121412313);

    // Если нужно использовать время (псевдослучайные значения будут отличаться для каждого кадра)
    if(useTime) seed += fract(12.12345314312*iTime);

    return seed;
}

/**
 * Получить случайное направление в пределах ориентированной полусферы
 * \param dir Направление ориентированности полсферы
 * \param seed "Семя" рандомизации
 * \return Вектор
 */
vec3 getHemisphereRndDir(vec3 dir, inout float seed)
{
    // Вектор перпендикулярный вектору направления
    vec3 b = normalize(cross(dir, dir + vec3(0.01f)));
    // Второй перпендикулярный вектор
    vec3 c = normalize(cross(dir, b));

    // Случайные углы
    // Для более равномерного распределения используем не случайный угол, но случайную точку на высоте полсферы
    // При помощи данной выосты получаем случайный угол theta

    // Fi - угол вектора вращяющегося вокруг направления dir [0:360]
    // Theta - угол между итоговым вектором и направлением dir (полярный угол) [0:90]
    float fi = hash1(seed) * 360.0f / M_RAD;
    float theta = acos(hash1(seed));

    // Вектор описывающий круг вокруг направления dir
    vec3 d = b * cos(fi) + c * sin(fi);
    // Вектор отклоненный от dir на случайный угол theta
    return dir * cos(theta) + d * sin(theta);
}

/**
 * Получить случайное направление в пределах ориентированной полусферы (взавшанное по косинусу)
 * В отличии от равномерного распределения, данный метод разбрасывает случайные направления гуще к полюсу полусферы (по закону косинуса)
 * Это дает возможность выбирать только наиболее значимые направления, и отбросить часть затеняемых косинусом лучей.
 * При использовании этого метода затенение по закону косинуса следует отключить
 * \param dir Направление ориентированности полсферы
 * \param seed "Семя" рандомизации
 * \return Вектор
 */
vec3 getHemisphereRndDirCosWeight(vec3 dir, inout float seed)
{
    vec2 u = hash2(seed);

    float r = sqrt(u.x);
    float theta = 2.0 * M_PI * u.y;

    vec3 B = normalize(cross(dir, vec3(0.0,1.0,1.0)));
    vec3 T = cross(B, dir);

    return normalize(r * sin(theta) * B + sqrt(1.0 - u.x) * dir + r * cos(theta) * T);
}


/**
 * Трассировка пути луча из камеры
 * \param ray Исходный луч
 * \param seed "Семя" рандомизации
 * \return Итоговый цвет
 */
vec3 traceEyePath(in Ray ray, inout float seed)
{
    // Итоговый цвет
    vec3 resultColor = vec3(0.0f);
    // Кол-во света, изменяющиеся в процессе пути (в результате отскоков)
    // Оно будет назначено итоговому цвету если в конце луч достигнет источника/неба
    vec3 light = vec3(1.0f);

    // Простроить путь луча
    for(int i = 0; i < PATH_LENGHT; i++)
    {
        // Информация о пересечении
        RayHitInfo hitInfo;

        // Если пересечение НЕ состоялось (считать это попаданием луча в источника света/небо)
        if(!raySceneIntersection(ray, 0.01f, 1000.0f, hitInfo)){
            // Установить цвет
            resultColor = light;
            // Оборвать цикл
            break;
        }

        // Исходим из того что цикл не был оборван
        // Нужно обновить начало и направление луча
        ray.origin = hitInfo.point;
        ray.direction = getHemisphereRndDir(hitInfo.normal, seed);

        // Свет тускнеет по закону косинуса (только для равного распределения по полусфере)
        light *= 2.0 * dot(ray.direction, hitInfo.normal);
    }

    // Вернуть итоговый цвет
    return resultColor;
}

/**
 * Основная функция фрагментного шейдера
 * Каждый фрамент растеризуемого полноэкранного квадрата (каждый пиксель экрана) соответствует одному или нескольким лучам
 */
void main()
{
    // Инициализировать "семя" для случайных чисел
    float seed = initRndSeed(true);

    // Итоговый цвет
    vec3 resultColor = vec3(0.0f);

    // Для каждого семпла
    for(int i = 0; i < SAMPLES; i++)
    {
        // Случайное суб-пиксельное отклонение начального луча
        vec2 pixelBias = vec2(1.0f / iScreenSize.x, 1.0f / iScreenSize.y) * (SAMPLES > 1 ? hash2(seed) : vec2(0.5f));
        //vec2 pixelBias = vec2(0.0f);

        // Генерация луча из камеры
        Ray camRay = Ray(
            (iCamModel * vec4(0.0f,0.0f,0.0f,1.0f)).xyz,
            getCamRayDirection(iCamFov, iScreenSize.x/iScreenSize.y, fs_in.uv, pixelBias)
        );

        // Построить путь луча и получить итоговый цвет
        resultColor += traceEyePath(camRay, seed);
    }

    // Усреднить по кол-ву семплов на пиксель
    resultColor /= SAMPLES;

    //resultColor = pow(clamp(resultColor,0.0,1.0),vec3(0.45));

    // Присваиваем красный цвет (временно)
    color = vec4(resultColor,1.0f);
}