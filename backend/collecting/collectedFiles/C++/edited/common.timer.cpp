
#include <time.h>
#ifdef _WIN32
#include <windows.h>
#include <mmsystem.h>
#include <sys/timeb.h>
#else
#include <sys/time.h>
#endif
#include "common/common_types.h"
#include "common/string_util.h"
#include "common/timer.h"

namespace Common {

u32 Timer::GetTimeMs() {
#ifdef _WIN32
    return timeGetTime();
#else
    struct timeval t;
    (void)gettimeofday(&t, nullptr);
    return ((u32)(t.tv_sec * 1000 + t.tv_usec / 1000));
#endif
}


Timer::Timer() : m_LastTime(0), m_StartTime(0), m_Running(false) {
    Update();
}

void Timer::Start() {
    m_StartTime = GetTimeMs();
    m_Running = true;
}

void Timer::Stop() {
        m_LastTime = GetTimeMs();
    m_Running = false;
}

void Timer::Update() {
    m_LastTime = GetTimeMs();
    }


u64 Timer::GetTimeDifference() {
    return GetTimeMs() - m_LastTime;
}

void Timer::AddTimeDifference() {
    m_StartTime += GetTimeDifference();
}

u64 Timer::GetTimeElapsed() {
            if (m_StartTime == 0)
        return 1;

        if (!m_Running)
        return (m_LastTime - m_StartTime);

    return (GetTimeMs() - m_StartTime);
}

std::string Timer::GetTimeElapsedFormatted() const {
        if (m_StartTime == 0)
        return "00:00:00:000";

            u64 Milliseconds;
    if (m_Running)
        Milliseconds = GetTimeMs() - m_StartTime;
    else
        Milliseconds = m_LastTime - m_StartTime;
        u32 Seconds = (u32)(Milliseconds / 1000);
        u32 Minutes = Seconds / 60;
        u32 Hours = Minutes / 60;

    std::string TmpStr = StringFromFormat("%02i:%02i:%02i:%03i", Hours, Minutes % 60, Seconds % 60,
                                          Milliseconds % 1000);
    return TmpStr;
}

void Timer::IncreaseResolution() {
#ifdef _WIN32
    timeBeginPeriod(1);
#endif
}

void Timer::RestoreResolution() {
#ifdef _WIN32
    timeEndPeriod(1);
#endif
}

u64 Timer::GetTimeSinceJan1970() {
    time_t ltime;
    time(&ltime);
    return ((u64)ltime);
}

u64 Timer::GetLocalTimeSinceJan1970() {
    time_t sysTime, tzDiff, tzDST;
    struct tm* gmTime;

    time(&sysTime);

        gmTime = localtime(&sysTime);
    if (gmTime->tm_isdst == 1)
        tzDST = 3600;
    else
        tzDST = 0;

        gmTime = gmtime(&sysTime);
    tzDiff = sysTime - mktime(gmTime);

    return (u64)(sysTime + tzDiff + tzDST);
}

std::string Timer::GetTimeFormatted() {
    time_t sysTime;
    struct tm* gmTime;
    char tmp[13];

    time(&sysTime);
    gmTime = localtime(&sysTime);

    strftime(tmp, 6, "%M:%S", gmTime);

#ifdef _WIN32
    struct timeb tp;
    (void)::ftime(&tp);
    return StringFromFormat("%s:%03i", tmp, tp.millitm);
#else
    struct timeval t;
    (void)gettimeofday(&t, nullptr);
    return StringFromFormat("%s:%03d", tmp, (int)(t.tv_usec / 1000));
#endif
}

double Timer::GetDoubleTime() {
#ifdef _WIN32
    struct timeb tp;
    (void)::ftime(&tp);
#else
    struct timeval t;
    (void)gettimeofday(&t, nullptr);
#endif
        u64 TmpSeconds = Common::Timer::GetTimeSinceJan1970();

                    TmpSeconds = TmpSeconds - (38 * 365 * 24 * 60 * 60);

        u32 Seconds = (u32)TmpSeconds;
#ifdef _WIN32
    double ms = tp.millitm / 1000.0 / 1000.0;
#else
    double ms = t.tv_usec / 1000000.0;
#endif
    double TmpTime = Seconds + ms;

    return TmpTime;
}

} 