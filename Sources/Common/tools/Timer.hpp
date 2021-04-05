#pragma once

#include <chrono>

using namespace std::chrono;

namespace tools
{
    class Timer
    {
    private:
        /// Время текущего кадра
        time_point<high_resolution_clock> currentFrameTick_;
        /// Время предыдущего кадра
        time_point<high_resolution_clock> previousFrameTick_;
        /// Время последнего обновления FPS счетчика
        time_point<high_resolution_clock> lastFpsCounterUpdatedTime_;
        /// Время первой инициализации объекта
        time_point<high_resolution_clock> initializationTime_;
        /// Кол-во кадров (для нужд FPS счетчика)
        unsigned framesCount_;
        /// FPS счетчик (обновляется каждую секунду)
        unsigned fps_;
        /// FPS счтчик готов к показу (каждую секунду, после обновления счетчика, становится true на один тик)
        bool fpsCounterReady_;
        /// Время между кадрами
        float delta_;


    public:

        /**
         * При создании таймера currentTick_ устанавливается в текущее время
         * \details Создавать таймер следует до цикла
         */
        Timer():
                currentFrameTick_(std::chrono::high_resolution_clock::now()),
                lastFpsCounterUpdatedTime_(std::chrono::high_resolution_clock::now()),
                framesCount_(0),
                fps_(0),
                fpsCounterReady_(false),
                delta_(0.0f)
        {
            // Получить время инициализации траймера
            initializationTime_ = std::chrono::high_resolution_clock::now();
        }

        /**
         * Получить разницу во времени между текущим и предыдущим кадром
         * \return Значение разницы в секундах
         */
        [[nodiscard]] float getDelta() const
        {
            return delta_;
        }

        /**
         * Получить время прошедшее с инициализации таймера (в секундах)
         * \return Значение времени в секунддах
         */
        [[nodiscard]] float getCurrentTime()
        {
            // Сколько времени прошло с создания таймера
            const int64_t delta = std::chrono::duration_cast<std::chrono::microseconds>(currentFrameTick_ - initializationTime_).count();
            // Вернуть время в секундах
            return static_cast<float>(delta) / 1000000.0f;
        }

        /**
         * Обновить таймер
         * \details Время предыдущего кадра - текущее время ПРЕДЫДУЩЕГО кадра, время текущего кадра - НАСТОЯЩЕЕ время
         */
        void updateTimer()
        {
            // Время предыдущего кадра это текущее время предыдущего кадра (до обновления таймера)
            previousFrameTick_ = currentFrameTick_;
            // Время текущего кадра это НАСТОЯЩЕЕ время
            currentFrameTick_ = std::chrono::high_resolution_clock::now();
            // Считать счетчик FPS не готовым к показу
            fpsCounterReady_ = false;

            // Сколько времени прошло с прошлого кадра
            const int64_t delta = std::chrono::duration_cast<std::chrono::microseconds>(currentFrameTick_ - previousFrameTick_).count();
            delta_ = static_cast<float>(delta) / 1000000.0f;

            // Подсчет FPS
            // Если прошла секунда с прошлого обновления счетчика FPS
            if (std::chrono::duration_cast<std::chrono::milliseconds>(currentFrameTick_ - lastFpsCounterUpdatedTime_).count() > 1000)
            {
                // Счетчик FPS равен кол-ву кадров набранных за секунду с прошлого обновления счетчика
                fps_ = framesCount_;
                // Обнулить кол-во кадров
                framesCount_ = 0;
                // Последнее обновление счетчика произошло сейчас
                lastFpsCounterUpdatedTime_ = currentFrameTick_;
                // FPS готов показу (пока таймер не обновлен)
                fpsCounterReady_ = true;
            }

            // Увеличить счетчик кадров
            framesCount_++;
        }

        /**
         * Получить FPS
         * \details Для корректного значения таймер должен обновляться в каждом кадре
         * \return Кол-во кадров за секунду с прошлого обновления счетчика
         */
        [[nodiscard]] unsigned getFps() const
        {
            return fps_;
        }

        /**
         * Готов ли счетчик FPS к показу
         * \details Если показ FPS занимает время, стоит показывать его только тогда когда счетчик обновлен
         * \return Да или нет
         */
        [[nodiscard]] bool isFpsCounterReady() const
        {
            return fpsCounterReady_;
        }
    };
}

