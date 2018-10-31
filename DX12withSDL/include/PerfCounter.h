#pragma once

#include "LogUtils.h"

using namespace std;

namespace GAL {

    class PerfCounter
    {
    public:

        PerfCounter()
        {
            LARGE_INTEGER frequency;
            QueryPerformanceFrequency(&frequency);
            m_periodSeconds = 1.0 / frequency.QuadPart;
            m_periodMillis = 1000.0 / frequency.QuadPart;
            m_periodMicros = 1000000.0 / frequency.QuadPart;
            m_periodNanos = 1000000000.0 / frequency.QuadPart;
        }

        ~PerfCounter()
        {
        }

        void start()
        {
            QueryPerformanceCounter(&mStartingTime);
        }

        void stop()
        {
            QueryPerformanceCounter(&mEndingTime);
            calc();
        }

        double getElapsedSeconds() const { return mElapsedSeconds; }
        double getElapsedMS() const { return mElapsedMS; }
        double getElapsedUS() const { return mElapsedUS; }
        double getElapsedNS() const { return mElapsedNS; }

        void dump(int loopCnt, ostream& os) const {
            os << "Frequency:" << 1.0/ m_periodSeconds << "Period:" << m_periodSeconds << ", start:" << mStartingTime.QuadPart << ", end:" << mEndingTime.QuadPart << endl;
            LARGE_INTEGER tickCount;
            tickCount.QuadPart = mEndingTime.QuadPart - mStartingTime.QuadPart;

            double elapsedSeconds, elapsedMS, elapsedUS, elapsedNS;
            elapsedSeconds = m_periodSeconds * tickCount.QuadPart;
            elapsedMS = m_periodMillis * tickCount.QuadPart;
            elapsedUS = m_periodMicros * tickCount.QuadPart;
            elapsedNS = m_periodNanos * tickCount.QuadPart;

            os << "tick count:" << tickCount.QuadPart << ", secs:" << elapsedSeconds << ", ms:" << elapsedMS << ", us:" << elapsedUS << ", ns:" << elapsedNS << endl;

            double secs_per_loop = elapsedSeconds / loopCnt;
            double ms_per_loop = elapsedMS / loopCnt;
            double us_per_loop = elapsedUS / loopCnt;
            double ns_per_loop = elapsedNS / loopCnt;


            double ticks_per_loop = (double)tickCount.QuadPart;
            ticks_per_loop /= loopCnt;

            os << "ticks/loop:" << ticks_per_loop << ", secs/loop:" << secs_per_loop << ", ms/loop:" << ms_per_loop << ", us/loop:" << us_per_loop << ", ns/loop:" << ns_per_loop << endl;
        }

        void dump(int loopCnt, FILE* fp) const {
            ostringstream strstream;
            dump(loopCnt, strstream);
            string str = strstream.str();
            fwrite(str.c_str(), 1, str.length(), fp);
        }

        void dump(int loopCnt) const {
            ostringstream oss;
            dump(loopCnt, oss);
            odprintf("%s", oss.str().c_str());
        }

    private:
        double m_periodSeconds;
        double m_periodMillis;
        double m_periodMicros;
        double m_periodNanos;
        LARGE_INTEGER mStartingTime, mEndingTime;
        double mElapsedSeconds;
        double mElapsedMS;
        double mElapsedUS;
        double mElapsedNS;
        double mTickCnt;

        void calc() {
            LARGE_INTEGER tickCount;
            tickCount.QuadPart = mEndingTime.QuadPart - mStartingTime.QuadPart;
            mElapsedSeconds = m_periodSeconds * tickCount.QuadPart;
            mElapsedMS = m_periodMillis * tickCount.QuadPart;
            mElapsedUS = m_periodMicros * tickCount.QuadPart;
            mElapsedNS = m_periodNanos * tickCount.QuadPart;
            mTickCnt = (double)tickCount.QuadPart;
        }
    };

}
