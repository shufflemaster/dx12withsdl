#pragma once

#include <Windows.h>

namespace GAL {

    class TimeCounter
    {
    public:

        TimeCounter()
        {
            QueryPerformanceFrequency(&m_frequency);
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
                LARGE_INTEGER elapsedUS;
                elapsedUS.QuadPart = tickCount.QuadPart * 1000000;
                elapsedUS.QuadPart /= m_frequency.QuadPart;
                *timeMicros = (double)elapsedUS.QuadPart;
            }

            if (timeNanos)
            {
                LARGE_INTEGER elapsedNS;
                elapsedNS.QuadPart = tickCount.QuadPart * 1000000000;
                elapsedNS.QuadPart /= m_frequency.QuadPart;
                *timeNanos = (double)elapsedNS.QuadPart;
            }

            LARGE_INTEGER elapsedMS;
            elapsedMS.QuadPart = tickCount.QuadPart * 1000;
            elapsedMS.QuadPart /= m_frequency.QuadPart;
            
            return (double)elapsedMS.QuadPart;
        }

        //Read delta time since last call to readDeltaTimeMillis() was called.
        double readDeltaTimeMillis(double* deltaTimeMicros = nullptr, double* deltaTimeNanos = nullptr)
        {
            LARGE_INTEGER currentTime;
            QueryPerformanceCounter(&currentTime);

            LARGE_INTEGER tickCount;
            tickCount.QuadPart = currentTime.QuadPart - m_previousTime.QuadPart;
            m_previousTime = currentTime;

            if (deltaTimeMicros)
            {
                LARGE_INTEGER elapsedUS;
                elapsedUS.QuadPart = tickCount.QuadPart * 1000000;
                elapsedUS.QuadPart /= m_frequency.QuadPart;
                *deltaTimeMicros = (double)elapsedUS.QuadPart;
            }

            if (deltaTimeNanos)
            {
                LARGE_INTEGER elapsedNS;
                elapsedNS.QuadPart = tickCount.QuadPart * 1000000000;
                elapsedNS.QuadPart /= m_frequency.QuadPart;
                *deltaTimeNanos = (double)elapsedNS.QuadPart;
            }

            LARGE_INTEGER elapsedMS;
            elapsedMS.QuadPart = tickCount.QuadPart * 1000;
            elapsedMS.QuadPart /= m_frequency.QuadPart;

            return (double)elapsedMS.QuadPart;
        }

    private:
        LARGE_INTEGER m_frequency;
        LARGE_INTEGER m_startingTime, m_previousTime;
    };

}
