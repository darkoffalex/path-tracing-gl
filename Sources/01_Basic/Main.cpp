#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>

#include <Windows.h>
#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>

#include <gl/ShaderProgram.hpp>
#include <gl/GeometryBuffer.hpp>
#include <tools/Camera.hpp>
#include <tools/Timer.hpp>

/** W I N A P I  S T U F F **/

/// Дескриптор исполняемого модуля программы
HINSTANCE g_hInstance = nullptr;
/// Дескриптор осноного окна отрисовки
HWND g_hwnd = nullptr;
/// Дескриптор контекста отрисовки
HDC g_hdc = nullptr;
/// Контекст OpenGL
HGLRC g_hglrc = nullptr;
/// Наименование класса
const char* g_strClassName = "MainWindowClass";
/// Заголовок окна
const char* g_strWindowCaption = "01 - Basic example";

/// Макросы для проверки состояния кнопок
#define KEY_DOWN(vk_code) ((static_cast<uint16_t>(GetAsyncKeyState(vk_code)) & 0x8000u) ? true : false)
#define KEY_UP(vk_code) ((static_cast<uint16_t>(GetAsyncKeyState(vk_code)) & 0x8000u) ? false : true)

/**
 * \brief Обработчик оконных сообщений
 * \param hWnd Дескриптор окна
 * \param message Сообщение
 * \param wParam Параметр сообщения
 * \param lParam Параметр сообщения
 * \return Код выполнения
 */
LRESULT CALLBACK WindowProcedure(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

/**
 * \brief Создание OpenGL контекста и его активация
 * \param drawContext Контекст рисования окна Windows
 * \return Дескриптор контекста OpenGL
 */
HGLRC OpenGlCreateContext(HDC drawContext);

/**
 * \brief Получение адреса функции OpenGL
 * \param name Имя функции
 * \return Указатель на функцию
 */
void *OpenGlGetFunction(const char *name);

/**
 * \brief Получение координат курсора
 * \param hwnd Дескриптор окна
 * \return Точка с координатами
 */
inline POINT CursorPos(HWND hwnd){
    POINT p;
    if (GetCursorPos(&p)) ScreenToClient(hwnd, &p);
    return p;
}

/**
 * \brief Обработка ввода (клавиатура и мышь)
 * \param camSpeed Скорость камеры (м/с)
 * \param mouseSensitivity Чувствительность мыши
 */
void Controls(float camSpeed = 1.0f, float mouseSensitivity = 0.2f);

/** U T I L S **/

/**
 * \brief Загрузка бинарных данных из файла
 * \param path Путь к файлу
 * \return Массив байтов
 */
std::vector<unsigned char> LoadBytesFromFile(const std::string &path);

/**
 * \brief Загрузка текстовых данных из файла (строка)
 * \param path Путь к файлу
 * \return Строка
 */
std::string LoadStringFromFile(const std::string &path);

/** O P E N G L **/

/// Геометрия квадрата для отрисовки на весь экран
gl::GeometryBuffer* g_geometryQuad_;
/// Основная шейдерная программа
gl::ShaderProgram* g_shaderMain_;

/**
 * \brief Установка статуса вертикальной синхронизации
 * \param status Состояние
 */
void SetVSyncStatus(bool status);

/**
 * \brief Инициализация OpenGL и компонентов рендеринга
 * \param screenWidth Ширина экрана
 * \param screenHeight Высота жкрана
 */
void InitOpenGl(unsigned screenWidth, unsigned screenHeight);

/**
 * \brief Рендеринг полноэкранного квадрата
 * \param screenWidth Ширина экрана
 * \param screenHeight Высота жкрана
 */
void RenderQuad(unsigned screenWidth, unsigned screenHeight);

/** M A I N **/

/// Объект камеры (хранит положение и ориентацию, матрицы трансформации координат)
tools::Camera* g_camera = nullptr;
/// Объект таймера
tools::Timer* g_timer = nullptr;

/**
 * \brief Точка входа
 * \param argc Кол-во аргументов
 * \param argv Аргументы запуска
 * \return Код выполнения
 */
int main(int argc, char* argv[])
{
    (void) argc;
    (void) argv;

    try
    {
        /** WINDOW **/

        // Получение дескриптора исполняемого модуля программы
        g_hInstance = GetModuleHandle(nullptr);

        // Информация о классе
        WNDCLASSEX classInfo;
        classInfo.cbSize = sizeof(WNDCLASSEX);
        classInfo.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
        classInfo.cbClsExtra = 0;
        classInfo.cbWndExtra = 0;
        classInfo.hInstance = g_hInstance;
        classInfo.hIcon = LoadIcon(g_hInstance, IDI_APPLICATION);
        classInfo.hIconSm = LoadIcon(g_hInstance, IDI_APPLICATION);
        classInfo.hCursor = LoadCursor(nullptr, IDC_ARROW);
        classInfo.hbrBackground = CreateSolidBrush(RGB(240, 240, 240));
        classInfo.lpszMenuName = nullptr;
        classInfo.lpszClassName = g_strClassName;
        classInfo.lpfnWndProc = WindowProcedure;

        // Пытаемся зарегистрировать оконный класс
        if (!RegisterClassEx(&classInfo)) {
            throw std::runtime_error("ERROR: Can't register window class.");
        }

        // Создание окна
        g_hwnd = CreateWindow(
                g_strClassName,
                g_strWindowCaption,
                WS_OVERLAPPEDWINDOW,
                0, 0,
                800, 600,
                nullptr,
                nullptr,
                g_hInstance,
                nullptr);

        // Если не удалось создать окно
        if (!g_hwnd) {
            throw std::runtime_error("ERROR: Can't create main application window.");
        }

        // Показать окно
        ShowWindow(g_hwnd, SW_SHOWNORMAL);

        // Получение контекста отрисовки
        g_hdc = GetDC(g_hwnd);

        // Размеры клиентской области окна
        RECT clientRect;
        GetClientRect(g_hwnd, &clientRect);

        /** OPEN GL **/

        // Контекст OpenGL
        g_hglrc = OpenGlCreateContext(g_hdc);
        wglMakeCurrent(g_hdc,g_hglrc);

        // Инициализация OpenGL
        InitOpenGl(static_cast<unsigned>(clientRect.right),static_cast<unsigned>(clientRect.bottom));

        // Вертикальная синхронизация
        SetVSyncStatus(false);

        /** СЦЕНА **/

        // Установка камеры
        g_camera = new tools::Camera({0.0f,0.0f,1.0f},{0.0f,0.0f,0.0f});

        /** MAIN LOOP **/

        // Таймер основного цикла (для выяснения временной дельты и FPS)
        g_timer = new tools::Timer();

        // Оконное сообщение
        MSG msg = {};

        // Запуск цикла
        while (true)
        {
            // Обновить таймер
            g_timer->updateTimer();

            // Обработка клавиш управления
            Controls();

            // Обработка оконных сообщений
            if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
            {
                DispatchMessage(&msg);

                if (msg.message == WM_QUIT) {
                    break;
                }
            }

            // Обновление сцены
            g_camera->updatePlacement(g_timer->getDelta());

            // Рендеринг
            GetClientRect(g_hwnd, &clientRect);
            RenderQuad(static_cast<unsigned>(clientRect.right),static_cast<unsigned>(clientRect.bottom));

            // Смена буферов окна
            SwapBuffers(g_hdc);
        }
    }
    catch(std::exception& ex)
    {
        std::cout << ex.what() << std::endl;
    }

    // Уничтожение контекста OpenGL
    wglMakeCurrent(nullptr, nullptr);
    wglDeleteContext(g_hglrc);

    // Уничтожение окна
    DestroyWindow(g_hwnd);
    // Вырегистрировать класс окна
    UnregisterClass(g_strClassName, g_hInstance);

    return 1;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/** W I N A P I  S T U F F **/

/**
 * \brief Обработчик оконных сообщений
 * \param hWnd Дескриптор окна
 * \param message Сообщение
 * \param wParam Параметр сообщения
 * \param lParam Параметр сообщения
 * \return Код выполнения
 */
LRESULT CALLBACK WindowProcedure(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    if(message == WM_DESTROY){
        PostQuitMessage(0);
    }

    return DefWindowProc(hWnd, message, wParam, lParam);
}

/**
 * \brief Создание OpenGL контекста и его активация
 * \param drawContext Контекст рисования окна Windows
 * \return Дескриптор контекста OpenGL
 */
HGLRC OpenGlCreateContext(HDC drawContext)
{
    // Описываем необходимый формат пикселей
    PIXELFORMATDESCRIPTOR pfd = {};
    pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;
    pfd.cDepthBits = 24;
    pfd.cStencilBits = 8;
    pfd.iLayerType = PFD_MAIN_PLANE;

    // Номер формата пикселей
    const int pixelFormatId = ChoosePixelFormat(drawContext, &pfd);

    // Если не удалось найти подходящего формата пикселей
    if (!pixelFormatId) {
        throw std::runtime_error("Can't find suitable pixel format");
    }

    // Найти наиболее подходящее описание формата
    PIXELFORMATDESCRIPTOR bestMatchPfd;
    DescribePixelFormat(drawContext, pixelFormatId, sizeof(PIXELFORMATDESCRIPTOR), &bestMatchPfd);

    // Если не удалось найти подходящего формата пикселей
    if (bestMatchPfd.cDepthBits < pfd.cDepthBits) {
        throw std::runtime_error("OpenGl context error : Can't find suitable pixel format for depth");
    }

    // Если не удалось установить формат пикселей
    if (!SetPixelFormat(drawContext, pixelFormatId, &pfd)) {
        throw std::runtime_error("OpenGl context error : Can't set selected pixel format");
    }

    // OpenGL контекст
    HGLRC glContext = wglCreateContext(drawContext);
    if (!glContext) {
        throw std::runtime_error("OpenGl context error : Can't create OpenGL rendering context");
    }

    // Активировать OpenGL контекст
    const BOOL success = wglMakeCurrent(drawContext, glContext);
    if (!success) {
        throw std::runtime_error("OpenGl context error : Can't setup rendering context");
    }

    return glContext;
}

/**
 * \brief Получение адреса функции OpenGL
 * \param name Имя функции
 * \return Указатель на функцию
 */
void *OpenGlGetFunction(const char *name)
{
    void *p = (void *)wglGetProcAddress(name);
    if(p == nullptr ||
       (p == (void*)0x1) || (p == (void*)0x2) || (p == (void*)0x3) ||
       (p == (void*)-1) )
    {
        HMODULE module = LoadLibraryA("opengl32.dll");
        p = (void *)GetProcAddress(module, name);
    }

    return p;
}

/**
 * \brief Обработка ввода (клавиатура и мышь)
 * \param camSpeed Скорость камеры (м/с)
 * \param mouseSensitivity Чувствительность мыши
 */
void Controls(float camSpeed, float mouseSensitivity)
{
    // Координаты мыши в последнем кадре
    static POINT lastMousePos = {0, 0};

    // Вектор движения камеры
    glm::vec3 camVelocityRel = {0.0f, 0.0f, 0.0f};
    glm::vec3 camVelocityAbs = {0.0f, 0.0f, 0.0f};

    // Состояние клавиш клавиатуры
    if(KEY_DOWN(0x57u)) camVelocityRel.z = -1.0f; // W
    if(KEY_DOWN(0x41u)) camVelocityRel.x = -1.0f; // A
    if(KEY_DOWN(0x53u)) camVelocityRel.z = 1.0f;  // S
    if(KEY_DOWN(0x44u)) camVelocityRel.x = 1.0f;  // D
    if(KEY_DOWN(VK_SPACE)) camVelocityAbs.y = 1.0f;
    if(KEY_DOWN(0x43u)) camVelocityAbs.y = -1.0f; // С

    // Мышь
    auto currentMousePos = CursorPos(g_hwnd);
    if(KEY_DOWN(VK_LBUTTON))
    {
        // Разница в координатах мыши между текущим и предыдущим кадром
        POINT deltaMousePos = {lastMousePos.x - currentMousePos.x, lastMousePos.y - currentMousePos.y};

        // Изменить текущую ориентацию камеры
        auto orientation = g_camera->getOrientation();
        orientation.x += static_cast<float>(deltaMousePos.y) * mouseSensitivity;
        orientation.y += static_cast<float>(deltaMousePos.x) * mouseSensitivity;
        g_camera->setOrientation(orientation);
    }
    lastMousePos = currentMousePos;

    // Установить векторы движения камеры
    if(g_camera != nullptr){
        g_camera->setVelocityRel((glm::length2(camVelocityRel) > 0.0f ? glm::normalize(camVelocityRel) : camVelocityRel) * camSpeed);
        g_camera->setVelocity(camVelocityAbs * camSpeed);
    }
}

/** U T I L S **/

/**
 * \brief Загрузка бинарных данных из файла
 * \param path Путь к файлу
 * \return Массив байтов
 */
std::vector<unsigned char> LoadBytesFromFile(const std::string &path)
{
    // Открытие файла в режиме бинарного чтения
    std::ifstream is(path.c_str(), std::ios::binary | std::ios::in | std::ios::ate);

    if (is.is_open())
    {
        is.seekg(0,std::ios::end);
        auto size = is.tellg();
        auto pData = new char[size];
        is.seekg(0, std::ios::beg);
        is.read(pData,size);
        is.close();

        return std::vector<unsigned char>(pData, pData + size);
    }

    return {};
}

/**
 * \brief Загрузка текстовых данных из файла (строка)
 * \param path Путь к файлу
 * \return Строка
 */
std::string LoadStringFromFile(const std::string &path)
{
    // Чтение файла
    std::ifstream ifstream;

    // Открыть файл для текстового чтения
    ifstream.open(path);

    if (!ifstream.fail())
    {
        // Строковой поток
        std::stringstream sourceStringStream;
        // Считать в строковой поток из файла
        sourceStringStream << ifstream.rdbuf();
        // Закрыть файл
        ifstream.close();
        // Получить данные из строкового поток в строку
        return sourceStringStream.str();
    }

    return "";
}

/** O P E N G L **/

/**
 * \brief Установка статуса вертикальной синхронизации
 * \param status Состояние
 */
void SetVSyncStatus(bool status)
{
    // Получаем при помощи glGetString строку с доступными расширениями
    const char* extensions = reinterpret_cast<const char*>(glGetString(GL_EXTENSIONS));

    // Если есть нужное расширение
    if (strstr(extensions, "WGL_EXT_swap_control"))
    {
        // Получить указатель на функцию для управления вертикальной синхронизацией
        const auto wglSwapInterval = reinterpret_cast<BOOL(APIENTRY*)(int)>(OpenGlGetFunction("wglSwapIntervalEXT"));
        // Если удалось получить - использовать функцию, в противном случае сгенерировать исключение
        if (wglSwapInterval) wglSwapInterval(status);
    }
}


/**
 * \brief Инициализация OpenGL и компонентов рендеринга
 * \param screenWidth Ширина экрана
 * \param screenHeight Высота жкрана
 */
void InitOpenGl(unsigned int screenWidth, unsigned int screenHeight)
{
    (void) screenWidth;
    (void) screenHeight;

    /// Получение адресов OpenGL функций (GLAD)
    {
        // Установить функцию-загрузчик функций OpenGL
        if(!gladLoadGLLoader((GLADloadproc)OpenGlGetFunction)){
            throw std::runtime_error("OpenGL GLAD initialization error");
        }
        // Загрузить функции
        if(!gladLoadGL()){
            throw std::runtime_error("OpenGL GLAD loading error");
        }
    }

    /// Шейдеры
    {
        // Получить исходники шейдеров
        auto vsSource = LoadStringFromFile("../Shaders/01_Basic/path_tracing.vert");
        auto fsSource = LoadStringFromFile("../Shaders/01_Basic/path_tracing.frag");

        // Собрать шейдереную программу
        g_shaderMain_ = new gl::ShaderProgram({
            {GL_VERTEX_SHADER, vsSource.c_str()},
            {GL_FRAGMENT_SHADER, fsSource.c_str()}
        });
    }

    /// Ресурсы по умолчанию
    {
        // Геометрия квадрата на весь экран
        g_geometryQuad_ = new gl::GeometryBuffer({
            { { 1.0f,1.0f,0.0f },{ 1.0f,1.0f,1.0f },{ 1.0f,1.0f }, {0.0f,0.0f,1.0f} },
            { { 1.0f,-1.0f,0.0f },{ 1.0f,1.0f,1.0f },{ 1.0f,0.0f }, {0.0f,0.0f,1.0f} },
            { { -1.0f,-1.0f,0.0f },{ 1.0f,1.0f,1.0f },{ 0.0f,0.0f }, {0.0f,0.0f,1.0f} },
            { { -1.0f,1.0f,0.0f },{ 1.0f,1.0f,1.0f },{ 0.0f,1.0f }, {0.0f,0.0f,1.0f} },
            },{ 0,1,2, 0,2,3 });
    }

    /// Инициализация UBO-буферов
    {
        //TODO: произвести инициализацию UBO
    }

    /// Инициализация кадровых буферов
    {
        //TODO: произвести инициализацию кадровых буферов
    }

    /// Основные OpenGL настройки по умолчанию
    {
        // Передними считаются грани описанные по часовой стрелки
        glFrontFace(GL_CW);
        // Включить отсечение граней
        glEnable(GL_CULL_FACE);
        // Отсекать задние грани
        glCullFace(GL_BACK);
        // Тип рендеринга треугольников по умолчанию
        glPolygonMode(GL_FRONT, GL_FILL);
        // Функция тест глубины (Z-тест нужен лишь для отладки, для трассировки не используется)
        glDepthFunc(GL_LEQUAL);
        // Тест ножниц по умолчнию включен
        glEnable(GL_SCISSOR_TEST);
    }
}

/**
 * \brief Рендеринг полноэкранного квадрата
 * \param screenWidth Ширина экрана
 * \param screenHeight Высота жкрана
 */
void RenderQuad(unsigned screenWidth, unsigned screenHeight)
{
    // Привязываемся ко фрейм-буфферу (временно используем основной)
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Использовать шейдер
    glUseProgram(g_shaderMain_->getId());

    // Включить запись в цветовой буфер
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

    // Указать область кадра доступную для отрисовки
    glScissor(0, 0, screenWidth, screenHeight);
    glViewport(0, 0, screenWidth, screenHeight);

    // Очистка буфера
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Передать FOV
    glUniform1f(g_shaderMain_->getUniformLocations()->camFov,90.0f);
    // Передать размеры экрана
    glUniform2fv(g_shaderMain_->getUniformLocations()->screenSize, 1, glm::value_ptr(glm::vec2(static_cast<float>(screenWidth),static_cast<float>(screenHeight))));
    // Передать положение камеры
    glUniform3fv(g_shaderMain_->getUniformLocations()->camPosition, 1, glm::value_ptr(g_camera->getPosition()));
    // Передать матрицу вида
    glUniformMatrix4fv(g_shaderMain_->getUniformLocations()->viewMatrix, 1, GL_FALSE, glm::value_ptr(g_camera->getViewMatrix()));
    // Передать матрицу модели камеры
    glUniformMatrix4fv(g_shaderMain_->getUniformLocations()->camModelMatrix, 1, GL_FALSE, glm::value_ptr(g_camera->getModelMatrix()));

    // Привязать геометрию и нарисовать ее
    glBindVertexArray(g_geometryQuad_->getVaoId());
    glDrawElements(GL_TRIANGLES, g_geometryQuad_->getIndexCount(), GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);
}
