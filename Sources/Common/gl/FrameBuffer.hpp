#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>

namespace gl
{
    class FrameBuffer final
    {
    private:
        /// Идентификатор кадрового буфера
        GLuint id_;
        /// Размеры буфера
        struct { GLsizei width; GLsizei height; } sizes_;
        /// Текстурные вложения (как правило это цветовые вложения из которых затем можно делать выборку в шейдере)
        std::vector<GLuint> textureAttachments_;
        /// Идентификаторы вложений для каждого текстурного вложения
        std::vector<GLuint> textureAttachmentBindings_;
        /// Буферные вложения (рендер-буферы аналогичны текстурам, но не пригодны для выборки в шейдерах)
        std::vector<GLuint> renderBufferAttachments_;
        /// Идентификаторы вложений для каждого рендер-буфера
        std::vector<GLuint> renderBufferAttachmentBindings_;

    public:
        /**
         * Запрет копирования через инициализацию
         * \param other Ссылка на копируемый объекта
         */
        FrameBuffer(const FrameBuffer& other) = delete;

        /**
         * Запрет копирования через присваивание
         * \param other Ссылка на копируемый объекта
         * \return Ссылка на текущий объект
         */
        FrameBuffer& operator=(const FrameBuffer& other) = delete;

        /**
         * Конструктор перемещения
         * \param other R-value ссылка на другой объект
         * \details Нельзя копировать объект, но можно обменяться с ним ресурсом
         */
        FrameBuffer(FrameBuffer&& other) noexcept:
                id_(other.id_),
                sizes_(other.sizes_),
                textureAttachments_({}),
                textureAttachmentBindings_({}),
                renderBufferAttachments_({}),
                renderBufferAttachmentBindings_({})
        {
            other.id_ = 0;
            other.sizes_ = {};
            std::swap(this->textureAttachments_, other.textureAttachments_);
            std::swap(this->textureAttachmentBindings_, other.textureAttachmentBindings_);
            std::swap(this->renderBufferAttachments_, other.renderBufferAttachments_);
            std::swap(this->renderBufferAttachmentBindings_, other.renderBufferAttachmentBindings_);
        }

        /**
         * Перемещение через присваивание
         * \param other R-value ссылка на другой объект
         * \details Нельзя копировать объект, но можно обменяться с ним ресурсом
         * \return Ссылка на текущий объект
         */
        FrameBuffer& operator=(FrameBuffer&& other) noexcept
        {
            // Если присваивание самому себе - просто вернуть ссылку на этот объект
            if (&other == this) return *this;

            // Удалить ресурсы которыми владеет текущий объект
            if (!textureAttachments_.empty()) glDeleteTextures(static_cast<GLsizei>(textureAttachments_.size()), textureAttachments_.data());
            if (!renderBufferAttachments_.empty()) glDeleteRenderbuffers(static_cast<GLsizei>(renderBufferAttachments_.size()), renderBufferAttachments_.data());
            if (id_ != 0) glDeleteFramebuffers(1, &id_);

            // Очистить дескрипторы
            textureAttachments_.clear(); textureAttachments_.shrink_to_fit();
            textureAttachmentBindings_.clear(); textureAttachmentBindings_.shrink_to_fit();
            renderBufferAttachments_.clear(); renderBufferAttachments_.shrink_to_fit();
            renderBufferAttachmentBindings_.clear(); renderBufferAttachmentBindings_.shrink_to_fit();
            id_ = 0;

            // Обмен
            std::swap(id_, other.id_);
            std::swap(sizes_, other.sizes_);
            std::swap(this->textureAttachments_, other.textureAttachments_);
            std::swap(this->textureAttachmentBindings_, other.textureAttachmentBindings_);
            std::swap(this->renderBufferAttachments_, other.renderBufferAttachments_);
            std::swap(this->renderBufferAttachmentBindings_, other.renderBufferAttachmentBindings_);

            // Вернуть ссылку на этот объект
            return *this;
        }

        /**
         * Конструктор буфера
         * \param width Ширина
         * \param height Высота
         */
        FrameBuffer(GLsizei width, GLsizei height) :
                id_(0),
                sizes_({width,height})
        {
            glGenFramebuffers(1, &id_);
        }

        /**
         * \brief Очистка памяти, удаление текстур
         */
        ~FrameBuffer()
        {
            // Удалить ресурсы которыми владеет текущий объект
            if (!textureAttachments_.empty()) glDeleteTextures(static_cast<GLsizei>(textureAttachments_.size()), textureAttachments_.data());
            if (!renderBufferAttachments_.empty()) glDeleteRenderbuffers(static_cast<GLsizei>(renderBufferAttachments_.size()), renderBufferAttachments_.data());
            if (id_ != 0) glDeleteFramebuffers(1, &id_);
        }

        /**
         * Получить дескриптор ресурса
         * \return Дескриптор
         */
        [[nodiscard]] GLuint getId() const
        {
            return id_;
        }

        /**
         * Получить массив дескрипторов текстурных вложений
         * \return Ссылка на массив
         */
        [[nodiscard]] const std::vector<GLuint>& getTextureAttachments() const
        {
            return this->textureAttachments_;
        }

        /**
         * Получить массив дескрипторов рендер-буферных вложений
         * \return Ссылка на массив
         */
        [[nodiscard]] const std::vector<GLuint>& getRenderBufferAttachments() const
        {
            return this->renderBufferAttachments_;
        }

        /**
         * Добавить текстурное вложение
         * \param internalFormat Внутренний формат (формат хранения)
         * \param format Внешний формат (формат при выборке/записи)
         * \param attachmentBindingId Идентификатор вложения у фрейм-буфера
         * \param mip Использовать мип-уровни
         */
        void addTextureAttachment(
                GLuint internalFormat = GL_RGBA,
                GLuint format = GL_RGBA,
                GLuint attachmentBindingId = GL_COLOR_ATTACHMENT0,
                bool mip = false)
        {
            GLuint id;

            glGenTextures(1, &id);
            glBindTexture(GL_TEXTURE_2D, id);
            glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, sizes_.width, sizes_.height, 0, format, GL_FLOAT, nullptr);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, mip ? GL_NEAREST_MIPMAP_NEAREST : GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mip ? GL_NEAREST_MIPMAP_NEAREST : GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
            if (mip) glGenerateMipmap(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, 0);

            this->textureAttachments_.push_back(id);
            this->textureAttachmentBindings_.push_back(attachmentBindingId);
        }

        /**
         * Добавить рендер-буферное вложение
         * \param internalFormat Внутренний формат (формат хранения)
         * \param attachmentBindingId Идентификатор вложения у фрейм-буфера
         */
        void addRenderBufferAttachment(
                GLuint internalFormat = GL_DEPTH24_STENCIL8,
                GLuint attachmentBindingId = GL_DEPTH_STENCIL_ATTACHMENT)
        {
            GLuint id;
            glGenRenderbuffers(1, &id);
            glBindRenderbuffer(GL_RENDERBUFFER, id);
            glRenderbufferStorage(GL_RENDERBUFFER, internalFormat, sizes_.width, sizes_.height);
            glBindRenderbuffer(GL_RENDERBUFFER, 0);

            this->renderBufferAttachments_.push_back(id);
            this->renderBufferAttachmentBindings_.push_back(attachmentBindingId);
        }

        /**
         * Собрать кадровый буфер используя добавленные ранее компоненты
         * \param drawToAttachments Список номеров вложений доступных для рендеринга
         * \return Состояния сборки
         */
        bool prepareBuffer(const std::vector<GLuint>& drawToAttachments)
        {
            // Если пусты массивы дескрипторов вложений
            if (this->textureAttachments_.empty() && this->renderBufferAttachments_.empty()) return false;

            // Работает с кадровым буфером
            glBindFramebuffer(GL_FRAMEBUFFER, id_);

            // Добавить текстурные вложения
            for (unsigned i = 0; i < this->textureAttachments_.size(); i++)
            {
                glFramebufferTexture2D(
                        GL_FRAMEBUFFER,
                        this->textureAttachmentBindings_[i],
                        GL_TEXTURE_2D, this->textureAttachments_[i],
                        0);
            }

            // Добавить рендер-буферные вложения
            for (unsigned i = 0; i < this->renderBufferAttachments_.size(); i++)
            {
                glFramebufferRenderbuffer(
                        GL_FRAMEBUFFER,
                        this->renderBufferAttachmentBindings_[i],
                        GL_RENDERBUFFER,
                        this->renderBufferAttachments_[i]);
            }

            // Указать какие вложения будут использованы для рендеринга
            if(!drawToAttachments.empty()) glDrawBuffers(static_cast<GLsizei>(drawToAttachments.size()), drawToAttachments.data());

            // Если фрейм-буфер не готов
            if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
                return false;
            }

            // Прекращаем работу с фрейм-буфером
            glBindFramebuffer(GL_FRAMEBUFFER, 0);

            // Буфер готов
            return true;
        }

        /**
         * Получить ширину кадра в пикселях
         * \return Ширина
         */
        [[nodiscard]] const GLsizei& getWidth() const
        {
            return this->sizes_.width;
        }

        /**
         * Получить высоту кадра в пикселях
         * \return Высота
         */
        [[nodiscard]] const GLsizei& getHeight() const
        {
            return this->sizes_.height;
        }
    };
}