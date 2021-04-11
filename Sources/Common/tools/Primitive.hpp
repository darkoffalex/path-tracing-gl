#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#define PRIMITIVE_SIZE 128

namespace tools
{
    class Primitive
    {
    public:
        /**
         * \brief Типы примитивов
         */
        enum Type : GLint {
            eSphere = 1,
            ePlane = 2,
            eRectangle = 3,
            eBox = 4
        };

        /**
         * \brief Типы материалов
         */
        enum MaterialType : GLint  {
            eLightEmitter = 0,
            eLambert = 1,
            eMetal = 2,
            eDielectric = 3
        };

        /**
         * \brief Основные параметры материала
         */
        struct MaterialInfo
        {
            // Тип материала
            MaterialType type = MaterialType::eLambert;
            // Собственный цвет
            glm::vec3 albedo = {1.0f,1.0f,1.0f};
            // Шероховатость
            float roughness = 1.0f;
            // Индекс преломления (стекло - 1.5)
            float refraction = 1.5f;
        };

    protected:
        /// Положение
        glm::vec3 position_ = {0.0f,0.0f,0.0f};
        /// Ориентация
        glm::vec3 orientation_ = {0.0f, 0.0f, 0.0f};
        /// Параметры материала
        MaterialInfo materialInfo_ = {};

        /**
         * \brief Обновление участка UBO буфера
         * \param type Тип примитива
         * \param position Положение
         * \param orientation Ориентация
         * \param sphereRadius Радицс сферы (для примитива "сфера")
         * \param planeNormal Нормаль поверхности (для примитива "плоаскость")
         * \param rectSizes Размеры прямоугольника (для примитива "прямоугольник")
         * \param boxSizes Размеры коробки (для примитива "коробка")
         * \param materialInfo Информация о материале
         * \param bufferId Идентификатор буфера
         * \param offset Сдвиг в буфере
         */
        static void updateUniformBufferStd140(
                GLint type,
                const glm::vec3& position,
                const glm::vec3& orientation,
                GLfloat sphereRadius,
                const glm::vec3& planeNormal,
                const glm::vec2& rectSizes,
                const glm::vec3& boxSizes,
                const MaterialInfo& materialInfo,
                GLuint bufferId, GLsizei offset)
        {
            // Запись в UBO
            glBindBuffer(GL_UNIFORM_BUFFER, bufferId);
            glBufferSubData(GL_UNIFORM_BUFFER, offset, 4, &type); offset += 16;
            glBufferSubData(GL_UNIFORM_BUFFER, offset, 12, glm::value_ptr(position)); offset += 16;
            glBufferSubData(GL_UNIFORM_BUFFER, offset, 12, glm::value_ptr(orientation)); offset+=12;
            glBufferSubData(GL_UNIFORM_BUFFER, offset, 4, &sphereRadius); offset+=4;
            glBufferSubData(GL_UNIFORM_BUFFER, offset, 12, glm::value_ptr(planeNormal)); offset+=16;
            glBufferSubData(GL_UNIFORM_BUFFER, offset, 8, glm::value_ptr(rectSizes)); offset+=16;
            glBufferSubData(GL_UNIFORM_BUFFER, offset, 12, glm::value_ptr(boxSizes)); offset+=12;
            glBufferSubData(GL_UNIFORM_BUFFER, offset, 4, &(materialInfo.type)); offset+=4;
            glBufferSubData(GL_UNIFORM_BUFFER, offset, 12, glm::value_ptr(materialInfo.albedo)); offset+=12;
            glBufferSubData(GL_UNIFORM_BUFFER, offset, 4, &(materialInfo.roughness)); offset+=4;
            glBufferSubData(GL_UNIFORM_BUFFER, offset, 4, &(materialInfo.refraction));
            glBindBuffer(GL_UNIFORM_BUFFER, 0);
        }

    public:
        /**
         * \brief Конструктор по умолчанию
         */
        Primitive() = default;

        /**
         * \brief Основной конструктор
         * \param materialInfo Информация о материале
         */
        explicit Primitive(
                const glm::vec3& position,
                const glm::vec3& orientation = {0.0f,0.0f,0.0f},
                const MaterialInfo& materialInfo = {MaterialType::eLambert,glm::vec3(1.0f),1.0f,1.5f}):
                position_(position),
                orientation_(orientation),
                materialInfo_(materialInfo){}

        /**
         * \brief Деструктор по умолчанию
         */
        virtual ~Primitive() = default;

        /**
         * \brief Запись в Uniform Buffer с учетом выравнивания std140
         * \param bufferId Идентификатор UBO OpenGL
         * \param index Смещение в буфере
         */
        virtual void writeToUniformBuffer(GLuint bufferId, GLsizei index) = 0;
    };

    /**
     * \brief Класс примитива типа "сфера"
     */
    class PrimitiveSphere : public Primitive
    {
    private:
        /// Радиус сферы
        GLfloat radius_ = 1.0f;

    public:
        /**
         * \brief Конструктор по умолчанию
         */
        PrimitiveSphere():Primitive(){}

        /**
         * \brief Основной конструктор
         * \param position Центр сферы
         * \param radius Радиус сферы
         * \param materialInfo Праметры материала
         */
        explicit PrimitiveSphere(
                const glm::vec3& position = {0.0f,0.0f,0.0f},
                GLfloat radius = 1.0f,
                const MaterialInfo& materialInfo = {})
        :Primitive(position,glm::vec3(0.0f),materialInfo),radius_(radius){}

        /**
         * \brief Запись в Uniform Buffer с учетом выравнивания std140
         * \param bufferId Идентификатор UBO OpenGL
         * \param index Индекс примитива в массиве
         */
        void writeToUniformBuffer(GLuint bufferId, GLsizei index) override
        {
            updateUniformBufferStd140(
                    Type::eSphere,
                    position_,
                    orientation_,
                    radius_,
                    {0.0f,0.0f,0.0f},
                    {0.0f,0.0f},
                    {0.0f,0.0f,0.0f},
                    materialInfo_,
                    bufferId,
                    index * PRIMITIVE_SIZE);
        }
    };

    /**
     * \brief Класс примитива типа "плоскость"
     */
    class PrimitivePlane : public Primitive
    {
    private:
        /// Нормаль поверхности плоскости
        glm::vec3 normal_ = {0.0f,1.0f,0.0f};

    public:
        /**
         * \brief Конструктор по умолчанию
         */
        PrimitivePlane():Primitive(){};

        /**
         * \brief Основной конструктор
         * \param position Точка плоскости
         * \param normal Нормаль в точке плоскости
         * \param materialInfo Праметры материала
         */
        explicit PrimitivePlane(
                const glm::vec3& position = {0.0f,0.0f,0.0f},
                const glm::vec3& normal = {0.0f,0.0f,0.0f},
                const MaterialInfo& materialInfo = {})
        :Primitive(position,glm::vec3(0.0f),materialInfo),normal_(normal){}

        /**
         * \brief Запись в Uniform Buffer с учетом выравнивания std140
         * \param bufferId Идентификатор UBO OpenGL
         * \param index Индекс примитива в массиве
         */
        void writeToUniformBuffer(GLuint bufferId, GLsizei index) override
        {
            updateUniformBufferStd140(
                    Type::ePlane,
                    position_,
                    orientation_,
                    0.0f,
                    normal_,
                    {0.0f,0.0f},
                    {0.0f,0.0f,0.0f},
                    materialInfo_,
                    bufferId,
                    index * PRIMITIVE_SIZE);
        }
    };

    /**
     * \brief Класс примитива типа "прямоугольник"
     */
    class PrimitiveRectangle : public Primitive
    {
    private:
        /// Размеры прямоугольника
        glm::vec2 sizes_ = {1.0f,1.0f};

    public:
        /**
         * \brief Конструктор по умолчанию
         */
        PrimitiveRectangle():Primitive(){};

        /**
         * \brief Осн
         * \param position
         * \param orientation
         * \param sizes
         * \param materialInfo
         */
        explicit PrimitiveRectangle(
                const glm::vec3& position = {0.0f,0.0f,0.0f},
                const glm::vec3& orientation = {0.0f,0.0f,0.0f},
                const glm::vec2& sizes = {1.0f,1.0f},
                const MaterialInfo& materialInfo = {})
        :Primitive(position,orientation,materialInfo),sizes_(sizes){}

        /**
         * \brief Запись в Uniform Buffer с учетом выравнивания std140
         * \param bufferId Идентификатор UBO OpenGL
         * \param index Индекс примитива в массиве
         */
        void writeToUniformBuffer(GLuint bufferId, GLsizei index) override
        {
            updateUniformBufferStd140(
                    Type::eRectangle,
                    position_,
                    orientation_,
                    0.0f,
                    {0.0f,0.0f,0.0f},
                    sizes_,
                    {0.0f,0.0f,0.0f},
                    materialInfo_,
                    bufferId,
                    index * PRIMITIVE_SIZE);
        }
    };
}