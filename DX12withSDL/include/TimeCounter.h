#pragma once

namespace GAL {

    class TimeCounter
    {
    public:

        TimeCounter()
        {
            LARGE_INTEGER frequency;
            QueryPerformanceFrequency(&frequency);
            m_periodMillis = 1000.0 / frequency.QuadPart;
            m_periodMicros = 1000000.0 / frequency.QuadPart;
            m_periodNanos = 1000000000.0 / frequency.QuadPart;
        }

        ~TimeCounter()
        {
        }

        void start()
        {
            QueryPerformanceCounter(&m_startingTime);
            QueryPerformanceCounter(&m_previousTime);
        }

        //Read absolute time since start() was called.
        double readTimeMillis(double* timeMicros = nullptr, double* timeNanos = nullptr)
        {
            LARGE_INTEGER currentTime;
            QueryPerformanceCounter(&currentTime);

            LARGE_INTEGER tickCount;
            tickCount.QuadPart = currentTime.QuadPart - m_startingTime.QuadPart;
            m_previousTime = currentTime;

            if (timeMicros)
            {
                *timeMicros = m_periodMicros * tickCount.QuadPart;
            }

            if (timeNanos)
            {
                *timeNanos = m_periodNanos * tickCount.QuadPart;
            }

            double retValMillis = m_periodMillis * tickCount.QuadPart;
            return retValMillis;
        }

        //Read delta time since last call to readDeltaTimeMillis() was called.
        double readDeltaTimeMillis(double* deltaTimeMicros = nullptr, double* deltaTimeNanos = nullptr)
        {
            LARGE_INTEGER currentTime;
            QueryPerformanceCounter(&currentTime);

            LARGE_INTEGER tickCount;
            tickCount.QuadPart = currentTime.QuadPart - m_previousTime.QuadPart;
            m_previousTime.QuadPart = currentTime.QuadPart;

            if (deltaTimeMicros)
            {
                *deltaTimeMicros = m_periodMicros * tickCount.QuadPart;
            }

            if (deltaTimeNanos)
            {
                *deltaTimeNanos = m_periodNanos * tickCount.QuadPart;
            }

            double retValMillis = m_periodMillis * tickCount.QuadPart;
            return retValMillis;
        }

    private:
        double m_periodMillis;
        double m_periodMicros;
        double m_periodNanos;
        LARGE_INTEGER m_startingTime, m_previousTime;
    };

};
