#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

namespace tools
{
    class Camera
    {
    public:
        /**
         * Оси координат
         * Используется при указании порядка во время поворотом на углы Эйлера
         */
        enum Axis { eAxisX, eAxisY, eAxisZ };

    private:
        /// Матрица модели (локальные -> глобальные)
        glm::mat4 modelMatrix_ = glm::mat4(1);
        /// Матрица вида (глобальные -> локальные)
        glm::mat4 viewMatrix_ = glm::mat4(1);

        /// Абсолютное положение в пространстве
        glm::vec3 position_;
        /// Ориентация в пространстве
        glm::vec3 orientation_;

        /// Вектор скорости в локальном пространстве
        glm::vec3 velocityRel_;
        /// Вектор скорости в глобальном пространстве
        glm::vec3 velocity_;

        /**
         * Построить матрицу поворота (углы Эйлера)
         * \param r0 Порядок вращения - первая ось
         * \param r1 Порядок вращения - вторая ось
         * \param r2 Порядок вращения - третья ось
         * \return Матрица 4*4
         */
        glm::mat4 makeRotationMatrix(const Axis& r0, const Axis& r1, const Axis& r2) const
        {
            glm::float32 angles[3] = { glm::radians(this->orientation_.x), glm::radians(this->orientation_.y), glm::radians(this->orientation_.z) };
            glm::vec3 vectors[3] = { glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f) };

            glm::mat4 rot(1);
            rot = glm::rotate(rot, angles[r0], vectors[r0]);
            rot = glm::rotate(rot, angles[r1], vectors[r1]);
            rot = glm::rotate(rot, angles[r2], vectors[r2]);

            return rot;
        }

        /**
         * \brief Построить матрицу поворота (кватернионы)
         * \return Матрица 4*4
         */
        glm::mat4 makeRotationMatrixQuaternion() const
        {
            const glm::quat rot = glm::quat(glm::vec3(
                    glm::radians(this->orientation_.x),
                    glm::radians(this->orientation_.y),
                    glm::radians(this->orientation_.z)));

            return glm::toMat4(rot);
        }

        /**
         * \brief Построить матрицу поворота 3x3  (кватернионы)
         * \return Матрица 3*3
         */
        glm::mat3 makeRotationMatrixQuaternion3x3() const
        {
            const glm::quat rot = glm::quat(glm::vec3(
                    glm::radians(this->orientation_.x),
                    glm::radians(this->orientation_.y),
                    glm::radians(this->orientation_.z)));

            return glm::toMat3(rot);
        }

        /**
         * Обновление матрицы модели
         */
        void updateModelMatrix()
        {
            this->modelMatrix_ =
                    // 2 - сдвигаем
                    glm::translate(glm::mat4(1.0f), this->position_) *
                    // 1 - вращаем (вокруг начала координат)
                    this->makeRotationMatrixQuaternion();
        }

        /**
         * Обновление матрицы вида
         */
        void updateViewMatrix()
        {
            auto model =
                    // 2 - сдвигаем
                    glm::translate(glm::mat4(1.0f), this->position_) *
                    // 1 - вращаем (вокруг начала координат)
                    this->makeRotationMatrixQuaternion();

            this->viewMatrix_ = glm::inverse(model);
        }

    public:
        /**
         * \brief Конструктор по умолчанию
         */
        Camera():
                position_({0.0f,0.0f,0.0f}),
                orientation_({0.0f,0.0f,0.0f}),
                velocityRel_({0.0f,0.0f,0.0f}),
                velocity_({0.0f,0.0f,0.0f})
        {
            this->updateModelMatrix();
            this->updateViewMatrix();
        }

        /**
         * \brief Основной конструктор
         * \param position Положение
         * \param orientation Ориентация
         */
        explicit Camera(const glm::vec3& position, const glm::vec3& orientation = { 0.0f, 0.0f, 0.0f }):
                position_(position),
                orientation_(orientation),
                velocityRel_({0.0f,0.0f,0.0f}),
                velocity_({0.0f,0.0f,0.0f})
        {
            this->updateModelMatrix();
            this->updateViewMatrix();
        }

        /**
         * \brief Деструктор
         */
        virtual ~Camera() = default;

        /**
         * \brief Получить ссылку на матрицу модели
         * \return Константная ссылка на матрицу 4*4
         */
        [[nodiscard]] const glm::mat4& getModelMatrix() const
        {
            return modelMatrix_;
        }

        /**
         * \brief Получить ссылку на матрицу вида
         * \return Константная ссылка на матрицу 4*4
         */
        [[nodiscard]] const glm::mat4& getViewMatrix() const
        {
            return viewMatrix_;
        }

        /**
         * \brief Установить положение
         * \param position Тчока в пространстве
         * \param updateMatrices Обновление матриц
         */
        void setPosition(const glm::vec3& position, bool updateMatrices = true)
        {
            this->position_ = position;

            if(updateMatrices){
                this->updateViewMatrix();
                this->updateModelMatrix();
            }
        }

        /**
         * \brief Получить положение
         * \return Тчока в пространстве
         */
        [[nodiscard]] const glm::vec3& getPosition() const
        {
            return position_;
        }

        /**
         * \brief Установить ориентацию
         * \param orientation Углы поворота вокруг осей
         * \param updateMatrices Обновление матриц
         */
        void setOrientation(const glm::vec3& orientation, bool updateMatrices = true)
        {
            this->orientation_ = orientation;

            if(updateMatrices){
                this->updateViewMatrix();
                this->updateModelMatrix();
            }
        }

        /**
         * Получить ориентацию
         * \return Углы поворота вокруг осей
         */
        [[nodiscard]] const glm::vec3& getOrientation() const
        {
            return orientation_;
        }

        /**
         * \brief Установить вектор скорости в локальном пространстве
         * \param velocity Вектор скорости
         */
        void setVelocityRel(const glm::vec3& velocity)
        {
            this->velocityRel_ = velocity;
        }

        /**
         * \brief Получить вектор скорости в локальном пространстве
         * \return Вектор скорости
         */
        [[nodiscard]] const glm::vec3& getVelocityRel()
        {
            return velocityRel_;
        }

        /**
         * \brief Установить вектор скорости в глобальном пространстве
         * \details Вектора velocity и velocityRel не связаны, но внояст вклад в итоговый вектор движения
         * \param velocity Вектор скорости
         */
        void setVelocity(const glm::vec3& velocity)
        {
            this->velocity_ = velocity;
        }

        /**
         * \brief Получить вектор скорости в глобальном пространстве
         * \details Вектора velocity и velocityRel не связаны, но внояст вклад в итоговый вектор движения
         * \return Вектор скорости
         */
        [[nodiscard]] const glm::vec3& getVelocity()
        {
            return velocityRel_;
        }

        /**
         * \brief Движение - прирастить положение и ориентацию с учетом пройденного времени
         * \param deltaTime Время кадра
         */
        void updatePlacement(const float& deltaTime)
        {
            // Прирастить абсолютную скорость
            this->position_ += (this->velocity_ * deltaTime);

            // Приращение вектора относительной скорости
            this->position_ += (this->makeRotationMatrixQuaternion3x3() * this->velocityRel_ * deltaTime);

            // Обновить матрицы
            this->updateModelMatrix();
            this->updateViewMatrix();
        }
    };
}
