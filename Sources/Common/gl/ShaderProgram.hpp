#pragma once

#include <unordered_map>
#include <glad/glad.h>

namespace gl
{
    template <class T>
    class ShaderProgram final
    {
    private:
        /// Хендл ресурса OpenGL
        GLuint id_;
        /// Идентификаторы положений uniform-переменных
        T uniformLocations_;

        /**
         * Компиляция исходного кода шейдера
         * \param shaderSource Исходный код шейдера
         * \param type Тип шейдера
         * \return Идентификатор ресурса шейдера
         */
        static GLuint compileShader(const GLchar* shaderSource, GLuint type)
        {
            // Зарегистрировать шейдер нужного типа
            const GLuint id = glCreateShader(type);

            // Связать исходный код и шейдер
            glShaderSource(id, 1, &shaderSource, nullptr);

            // Компиляция шейдера
            glCompileShader(id);

            // Успешна ли компиляция
            GLint success;
            glGetShaderiv(id, GL_COMPILE_STATUS, &success);

            // Если не скомпилирован - генерация исключения
            if (!success) {
                GLsizei messageLength = 0;
                GLchar message[1024];
                glGetShaderInfoLog(id, 1024, &messageLength, message);
                throw std::runtime_error(
                        std::string("Shader compilation error : ").append(message).c_str());
            }

            // Вернуть дескриптор шейдера
            return id;
        }

        /**
         * \brief Получить идентификаторы положений uniform-переменных
         * \details Данный метод вызывается только после инициализации шейдерной программы
         */
        void initUniformLocations()
        {
            // Если не проинициализирована шейдерная программа
            if(this->id_ == 0) return;

            // Считаем что в структуре T задано поле bindings представляющее из себя ассоциативный массив
            // Ассоциативный массив должен быть вида "указатель на поле структуры -> название переменной в шейдере"
            for(const auto& entry : this->uniformLocations_.bindings){
                *entry.first = glGetUniformLocation(id_, entry.second.c_str());
            }
        }

    public:
        /**
         * Запрет копирования через инициализацию
         * \param other Ссылка на копируемый объект
         */
        ShaderProgram(const ShaderProgram& other) = delete;

        /**
         * Запрет копирования через присваивание
         * \param other Ссылка на копируемый объект
         * \return Ссылка на текущий объект
         */
        ShaderProgram& operator=(const ShaderProgram& other) = delete;

        /**
         * Конструктор перемещения
         * \param other R-value ссылка на другой объект
         * \details Нельзя копировать объект, но можно обменяться с ним ресурсом
         */
        ShaderProgram(ShaderProgram&& other) noexcept : id_(other.id_), uniformLocations_(other.uniformLocations_)
        {
            other.id_ = 0;
            other.uniformLocations_ = {};
        }

        /**
         * Перемещение через присваивание
         * \param other R-value ссылка на другой объект
         * \return Нельзя копировать объект, но можно обменяться с ним ресурсом
         */
        ShaderProgram& operator=(ShaderProgram&& other) noexcept
        {
            // Если присваивание самому себе - просто вернуть ссылку на этот объект
            if (&other == this) return *this;

            // Удалить ресурс которым владеет объект, обнулить дескриптор
            if (id_) glDeleteProgram(id_);
            id_ = 0;
            uniformLocations_ = {};

            // Обменять ресурсы объектов
            std::swap(this->id_, other.id_);
            std::swap(this->uniformLocations_, other.uniformLocations_);

            // Вернуть ссылку на этот объект
            return *this;
        }

        /**
         * Конструктор ресурса
         * \param shaderSources Ассоциативный массив (тип => исходный код шейдера)
         */
        explicit ShaderProgram(const std::unordered_map<GLuint, std::string>& shaderSources):id_(0)
        {
            // Зарегистрировать шейдерную программу
            this->id_ = glCreateProgram();

            // Идентификаторы созданных шейдеров (чтобы освободить память после сборки программы)
            std::vector<GLuint> shaderIds;

            // Пройтись по всем элементам ассоциативного массива
            for (auto & shaderSource : shaderSources)
            {
                // Если код шейдера не пуст
                if (!shaderSource.second.empty())
                {
                    // Скомпилировать шейдер
                    GLuint shaderId = ShaderProgram::compileShader(shaderSource.second.c_str(), shaderSource.first);
                    // Добавить шейдер к программе
                    glAttachShader(this->id_, shaderId);
                    // Добавить в список идентификаторов
                    shaderIds.push_back(shaderId);
                }
            }

            // Собрать шейдерную программу
            glLinkProgram(this->id_);

            // Удалить шейдеры (после сборки шейдерной программы они уже не нужны в памяти)
            for (const GLuint& shaderId : shaderIds) {
                glDeleteShader(shaderId);
            }

            // Проверка ошибок сборки шейдерной программы
            GLint success;
            glGetProgramiv(this->id_, GL_LINK_STATUS, &success);

            // Если не удалось собрать программу
            if (!success) {
                GLsizei messageLength = 0;
                GLchar message[1024];
                glGetProgramInfoLog(this->id_, 1024, &messageLength, message);

                throw std::runtime_error(std::string("Shader program linking error: ").append(message));
            }

            // Получить локации uniform-переменных
            this->initUniformLocations();
        }

        /**
         * \brief Деструтор
         */
        ~ShaderProgram()
        {
            // Удалить ресурс которым владеет объект, обнулить дескриптор
            if (id_) glDeleteProgram(id_);
            id_ = 0;
            uniformLocations_ = {};
        }

        /**
         * Получить дескриптор шейдерной программы
         * @return OpenGL дескриптор
         */
        [[nodiscard]] GLuint getId() const
        {
            return id_;
        }

        /**
         * Получить набор локаций uniform-переменных
         * @return Указатель на структуру с набором локаций
         */
        [[nodiscard]] const T* getUniformLocations() const
        {
            return &uniformLocations_;
        }
    };
}
