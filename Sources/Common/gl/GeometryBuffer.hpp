#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>

namespace gl
{
    class GeometryBuffer final
    {
    public:
        /// Описание вершины
        struct Vertex
        {
            glm::vec3 position;
            glm::vec3 color;
            glm::vec2 uv;
            glm::vec3 normal;
        };

    private:
        /// OpenGL дескриптор вершинного буфера
        GLuint vboId_;
        /// OpenGL дескриптор индексного буфера
        GLuint eboId_;
        /// OpenGL дескриптор VAO объекта
        GLuint vaoId_;

        /// Кол-во вершин
        GLsizei vertexCount_;
        /// Кол-во индексов основной геометрии
        GLsizei indexCount_;

        /**
         * Конфигурация вершинных атрибутов
         * Пояснения шейдеру как понимать данные из активного VBO (буфера вершин) в контексте активного VAO
         */
        static void setupVertexAttributes()
        {
            // Атрибут "положение"
            glVertexAttribPointer(
                    0,                           // Номер положения (location у шейдера)
                    3,                           // Размер (сколько значений конкретного типа приходится на один атрибут)
                    GL_FLOAT,                    // Конкретный тип одного значения
                    GL_FALSE,                    // Не нормализовать
                    sizeof(Vertex),     // Размер шага (размер одной вершины)
                    reinterpret_cast<GLvoid*>(offsetof(Vertex, position)) // Сдвиг (с какого места в блоке данных начинается нужная часть)
            );

            // Атрибут "цвет"
            glVertexAttribPointer(
                    1,                           // Номер положения (location у шейдера)
                    3,                           // Размер (сколько значений конкретного типа приходится на один атрибут)
                    GL_FLOAT,                    // Конкретный тип одного значения
                    GL_FALSE,                    // Не нормализовать
                    sizeof(Vertex),     // Размер шага (размер одной вершины)
                    reinterpret_cast<GLvoid*>(offsetof(Vertex, color)) // Сдвиг (с какого места в блоке данных начинается нужная часть)
            );

            // Атрибут "текстурные координаты"
            glVertexAttribPointer(
                    2,                           // Номер положения (location у шейдера)
                    2,                           // Размер (сколько значений конкретного типа приходится на один атрибут)
                    GL_FLOAT,                    // Конкретный тип одного значения
                    GL_FALSE,                    // Не нормализовать
                    sizeof(Vertex),     // Размер шага (размер одной вершины)
                    reinterpret_cast<GLvoid*>(offsetof(Vertex, uv)) // Сдвиг (с какого места в блоке данных начинается нужная часть)
            );

            // Атрибут "нормаль"
            glVertexAttribPointer(
                    3,                           // Номер положения (location у шейдера)
                    3,                           // Размер (сколько значений конкретного типа приходится на один атрибут)
                    GL_FLOAT,                    // Конкретный тип одного значения
                    GL_FALSE,                    // Не нормализовать
                    sizeof(Vertex),     // Размер шага (размер одной вершины)
                    reinterpret_cast<GLvoid*>(offsetof(Vertex, normal)) // Сдвиг (с какого места в блоке данных начинается нужная часть)
            );

            // Включить атрибуты
            glEnableVertexAttribArray(0);
            glEnableVertexAttribArray(1);
            glEnableVertexAttribArray(2);
            glEnableVertexAttribArray(3);
        }

    public:
        /**
         * Запрет копирования через инициализацию
         * \param other Ссылка на копируемый объекта
         */
        GeometryBuffer(const GeometryBuffer& other) = delete;

        /**
         * Запрет копирования через присваивание
         * \param other Ссылка на копируемый объекта
         * \return Ссылка на текущий объект
         */
        GeometryBuffer& operator=(const GeometryBuffer& other) = delete;

        /**
         * Конструктор перемещения
         * \param other R-value ссылка на другой объект
         * \details Нельзя копировать объект, но можно обменяться с ним ресурсом
         */
        GeometryBuffer(GeometryBuffer&& other) noexcept :
                vboId_(other.vboId_),
                eboId_(other.eboId_),
                vaoId_(other.vaoId_),
                vertexCount_(other.vertexCount_),
                indexCount_(other.indexCount_)
        {
            // Поскольку подразумевается что обмен происходит с новым объектом
            // то объект, что отдал свой ресурс, получает в замен пустой дескриптор ресурса
            other.vboId_ = 0;
            other.eboId_ = 0;
            other.vaoId_ = 0;
            other.vertexCount_ = 0;
            other.indexCount_ = 0;
        }

        /**
         * Перемещение через присваивание
         * \param other other R-value ссылка на другой объект
         * \return Нельзя копировать объект, но можно обменяться с ним ресурсом
         */
        GeometryBuffer& operator=(GeometryBuffer&& other) noexcept
        {
            // Если присваивание самому себе - просто вернуть ссылку на этот объект
            if (&other == this) return *this;

            // Удалить ресурс которым владеет объект, обнулить дескриптор
            if (this->vboId_) glDeleteBuffers(1, &vboId_);
            if (this->eboId_) glDeleteBuffers(1, &eboId_);
            if (this->vaoId_) glDeleteVertexArrays(1, &vaoId_);
            this->vboId_ = 0;
            this->eboId_ = 0;
            this->vaoId_ = 0;

            // Обменять ресурсы объектов
            std::swap(this->vboId_, other.vboId_);
            std::swap(this->eboId_, other.eboId_);
            std::swap(this->vaoId_, other.vaoId_);
            std::swap(this->vertexCount_, other.vertexCount_);
            std::swap(this->indexCount_, other.indexCount_);

            // Вернуть ссылку на этот объект
            return *this;
        }

        /**
         * Конструктор ресурса
         * \param vertices Массив вершин
         * \param indices Массив индексов
         */
        GeometryBuffer(const std::vector<Vertex>& vertices, const std::vector<GLuint>& indices):
                vboId_(0),
                eboId_(0),
                vaoId_(0),
                vertexCount_(0),
                indexCount_(0)
        {
            // Сохранить кол-во вершин и индексов (пригодится при рисовании)
            this->vertexCount_ = static_cast<GLsizei>(vertices.size());
            this->indexCount_ = static_cast<GLsizei>(indices.size());

            // Если не переданы вершины или индексы
            if (this->vertexCount_ == 0) throw std::runtime_error("Geometry buffer error: vertex array is empty");
            if (this->indexCount_ == 0) throw std::runtime_error("Geometry buffer error: index array is empty");

            // Регистрация необходимых буферов
            glGenBuffers(1, &vboId_);
            glGenBuffers(1, &eboId_);

            // Регистрация VAO
            glGenVertexArrays(1, &vaoId_);

            // Работаем с основным VAO
            glBindVertexArray(vaoId_);

            // Помещаем данные в буфер вершин
            glBindBuffer(GL_ARRAY_BUFFER, vboId_);
            glBufferData(GL_ARRAY_BUFFER, vertexCount_ * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

            // Помещаем данные в основной буфер индексов
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eboId_);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexCount_ * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

            // Пояснения шейдеру как понимать данные из активного VBO (буфера вершин) в контексте активного VAO
            setupVertexAttributes();

            // Завершаем работу с VAO
            glBindVertexArray(0);
        }

        /**
         * Очистка ресурса
         */
        ~GeometryBuffer()
        {
            // Удаление буферов
            if (this->vboId_) glDeleteBuffers(1, &vboId_);
            if (this->eboId_) glDeleteBuffers(1, &eboId_);

            // Удаление VAO
            if (this->vaoId_) glDeleteVertexArrays(1, &vaoId_);
        }

        /**
         * Получить кол-во вершин
         * \return Число вершин
         */
        [[nodiscard]] GLuint getVertexCount() const
        {
            return this->vertexCount_;
        }

        /**
         * Получить кол-во индексов
         * \return Число индексов
         */
        [[nodiscard]] GLuint getIndexCount() const
        {
            return this->indexCount_;
        }

        /**
         * Получить ID OpenGL объекта VAO (Vertex Array Object)
         * \return Дескриптор OpenGL объекта
         */
        [[nodiscard]] GLuint getVaoId() const
        {
            return this->vaoId_;
        }
    };
}


