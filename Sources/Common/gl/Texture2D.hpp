#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>

namespace gl
{
    class Texture2D final
    {
    public:
        enum ColorSpace { eGrayscale, eGrayscaleAlpha, eRgb, eRgbAlpha, eSrgb, eSrgbAlpha };
        enum FiltrationType { eNone, eBilinear, eTrilinear };

    private:
        GLuint id_;                    // OpenGL идентификатор текстуры
        GLuint width_;                 // Ширина
        GLuint height_;                // Высота
        ColorSpace colors_;            // Цветовые каналы (GRAY, RGB, RGBA)
        bool mip_;                     // Используется ли авто-генерация mip-уровней

        /**
         * \brief Получить OpenGL-тип фильтрации текстуры с учетом запрашиваемой и статусом опции генерации мип-уровней
         * \param filtering Запрашиваемая фильтрация
         * \param mip Включена ли генерация мип-уровней
         * \return OpenGL идентификатор фильтрации
         */
        static GLuint getGlFilteringType(FiltrationType filtering, bool mip)
        {
            switch (filtering)
            {
                case FiltrationType::eNone:
                default:
                    return GL_NEAREST;
                case FiltrationType::eBilinear:
                    return GL_LINEAR;
                case FiltrationType::eTrilinear:
                    return mip ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR;
            }
        }

    public:

        /**
         * \brief Запрет копирования через инициализацию
         * \param other Ссылка на копируемый объекта
         */
        Texture2D(const Texture2D& other) = delete;

        /**
         * \brief Запрет копирования через присваивание
         * \param other Ссылка на копируемый объекта
         */
        Texture2D& operator=(const Texture2D& other) = delete;

        /**
         * \brief Конструктор перемещения
         * \param other R-value ссылка на другой объект
         * \details Нельзя копировать объект, но можно обменяться с ним ресурсом
         */
        Texture2D(Texture2D&& other) noexcept:
                id_(other.id_),
                width_(other.width_),
                height_(other.height_),
                colors_(other.colors_),
                mip_(other.mip_)
        {
            // Поскольку подразумевается что обмен происходит с новым объектом
            // то объект, что отдал свой ресурс, получает в замен пустой дескриптор ресурса
            other.id_ = 0;
            other.width_ = 0;
            other.height_ = 0;
            other.colors_ = ColorSpace::eRgb;
            other.mip_ = false;
        }

        /**
         * \brief Перемещение через присваивание
         * \param other R-value ссылка на другой объект
         * \return Нельзя копировать объект, но можно обменяться с ним ресурсом
         */
        Texture2D& operator=(Texture2D&& other) noexcept
        {
            // Если присваивание самому себе - просто вернуть ссылку на этот объект
            if (&other == this) return *this;

            // Удалить ресурс которым владеет объект, обнулить дескриптор
            if (id_) glDeleteTextures(1, &id_);
            id_ = 0;
            width_ = 0;
            height_ = 0;
            colors_ = ColorSpace::eRgb;
            mip_ = false;

            // Обменять ресурсы объектов
            std::swap(this->id_, other.id_);
            std::swap(width_, other.width_);
            std::swap(height_, other.height_);
            std::swap(colors_, other.colors_);
            std::swap(mip_, other.mip_);

            // Вернуть ссылку на этот объект
            return *this;
        }

        /**
         * \brief Конструктор ресурса
         * \param textureData Указатель на массив пикселей текстуры
         * \param width Ширина
         * \param height Высота
         * \param colorSpace Цветовое пространство
         * \param filtering Фильтрация
         * \param mip Авто-генерация мип-уровней
         * \param type Тип данных (каким типом представлены компоненты цвета пикселя)
         */
        Texture2D(void* textureData, GLuint width, GLuint height, ColorSpace colorSpace, FiltrationType filtering, bool mip, GLuint type = GL_UNSIGNED_BYTE):
                id_(0),
                width_(width),
                height_(height),
                colors_(colorSpace),
                mip_(mip)
        {
            // Генерация идентификатора текстуры
            glGenTextures(1, &id_);
            // Привязываемся к текстуре по идентификатору (работаем с текстурой)
            glBindTexture(GL_TEXTURE_2D, this->id_);
            // Фильтр при уменьшении (когда один пиксель вмещает несколько текселей текстуры, то есть маленькие объекты)
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, getGlFilteringType(filtering, mip_));
            // Когда на один тексель текстуры приходится несколько пикселей (объекты рассматриваются вблизи)
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, getGlFilteringType(filtering, mip_));

            // Определить подходящий формат
            GLuint format;
            GLuint internalFormat;
            switch (colors_)
            {
                case eRgb:
                default:
                    internalFormat = GL_RGB;
                    format = GL_RGB;
                    break;
                case eRgbAlpha:
                    internalFormat = GL_RGBA;
                    format = GL_RGBA;
                    break;
                case eGrayscaleAlpha:
                    internalFormat = GL_RG;
                    format = GL_RG;
                    break;
                case eGrayscale:
                    internalFormat = GL_RED;
                    format = GL_RED;
                    break;
                case eSrgb:
                    internalFormat = GL_SRGB;
                    format = GL_RGB;
                    break;
                case eSrgbAlpha:
                    internalFormat = GL_SRGB_ALPHA;
                    format = GL_RGBA;
                    break;
            }


            // Устанавливаем данные текстуры (загрузка в текстурную память)
            glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, this->width_, this->height_, 0, format, type, textureData);

            // Генерация мип-уровней (если нужно)
            if (this->mip_) {
                glGenerateMipmap(GL_TEXTURE_2D);
            }

            // Отвязка от текстуры (настроили и прекращаем работать с текстурой)
            glBindTexture(GL_TEXTURE_2D, 0);
        }

        /**
         * \brief Очистка ресурса
         */
        ~Texture2D()
        {
            if (id_) glDeleteTextures(1, &id_);
        }

        /**
         * \brief Получить дескриптор текстурного объекта
         * \return Дескриптор OpenGL
         */
        GLuint getId() const
        {
            return id_;
        }

        /**
         * \brief Получить ширину
         * \return Ширина
         */
        GLuint getWidth() const
        {
            return width_;
        }

        /**
         * \brief Получить высоту
         * \return Высота
         */
        GLuint getHeight() const
        {
            return height_;
        }

        /**
         * \brief Получить цветовое пространство текстуры
         * \return Цветовое пространство
         */
        ColorSpace getColorSpace() const
        {
            return colors_;
        }
    };
}