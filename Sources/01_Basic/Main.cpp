#include <iostream>
#include <string>
#include <vector>
#include <fstream>

#include <Windows.h>
#include <glad/glad.h>

/// Дескриптор исполняемого модуля программы
HINSTANCE g_hInstance = nullptr;
/// Дескриптор осноного окна отрисовки
HWND g_hwnd = nullptr;
/// Дескриптор контекста отрисовки
HDC g_hdc = nullptr;
/// Наименование класса
const char* g_strClassName = "MainWindowClass";
/// Заголовок окна
const char* g_strWindowCaption = "01 - Basic example";

/** W I N A P I  S T U F F **/

/**
 * \brief Обработчик оконных сообщений
 * \param hWnd Дескриптор окна
 * \param message Сообщение
 * \param wParam Параметр сообщения
 * \param lParam Параметр сообщения
 * \return Код выполнения
 */
LRESULT CALLBACK WindowProcedure(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

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

/** M A I N **/

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

        /** MAIN LOOP **/

        // Оконное сообщение
        MSG msg = {};

        // Запуск цикла
        while (true)
        {
            // Обработка оконных сообщений
            if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
            {
                DispatchMessage(&msg);

                if (msg.message == WM_QUIT) {
                    break;
                }
            }
        }
    }
    catch(std::exception& ex)
    {
        std::cout << ex.what() << std::endl;
    }

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
    // Открытие файла в режиме текстового чтения
    std::ifstream is(path.c_str());

    if(is.is_open())
    {
        is.seekg(0,std::ios::end);
        auto size = static_cast<unsigned >(is.tellg()) + 1;
        auto pData = new char[size];
        is.seekg(0,std::ios::beg);
        is.read(pData,size);
        is.close();

        // Добавляем null character в конец, для валидной работы С-строк
        pData[size-1] = '\0';

        return std::string(pData);
    }

    return {};
}
