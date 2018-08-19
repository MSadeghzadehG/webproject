


#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#ifdef wxNEEDS_STRICT_ANSI_WORKAROUNDS
    #undef __STRICT_ANSI__
    #include <time.h>
    #define __STRICT_ANSI__
#endif

#include "wx/time.h"

#ifndef WX_PRECOMP
    #ifdef __WINDOWS__
        #include "wx/msw/wrapwin.h"
    #endif
    #include "wx/intl.h"
    #include "wx/log.h"
#endif

#ifndef WX_GMTOFF_IN_TM
            #if defined(__DARWIN__)
        #define WX_GMTOFF_IN_TM
    #endif
#endif

#include <time.h>


#if !defined(__WXMAC__)
    #include <sys/types.h>      #endif

#if defined(HAVE_GETTIMEOFDAY)
    #include <sys/time.h>
    #include <unistd.h>
#elif defined(HAVE_FTIME)
    #include <sys/timeb.h>
#endif

#if defined(__WINE__)
    #include <sys/timeb.h>
    #include <values.h>
#endif

namespace
{

const int MILLISECONDS_PER_SECOND = 1000;
const int MICROSECONDS_PER_MILLISECOND = 1000;
const int MICROSECONDS_PER_SECOND = 1000*1000;

} 

#if (!defined(HAVE_LOCALTIME_R) || !defined(HAVE_GMTIME_R)) && wxUSE_THREADS && !defined(__WINDOWS__)
static wxMutex timeLock;
#endif

#ifndef HAVE_LOCALTIME_R
struct tm *wxLocaltime_r(const time_t* ticks, struct tm* temp)
{
#if wxUSE_THREADS && !defined(__WINDOWS__)
      wxMutexLocker locker(timeLock);
#endif

  #ifdef __BORLANDC__
  if ( !*ticks )
      return NULL;
#endif

  const tm * const t = localtime(ticks);
  if ( !t )
      return NULL;

  memcpy(temp, t, sizeof(struct tm));
  return temp;
}
#endif 
#ifndef HAVE_GMTIME_R
struct tm *wxGmtime_r(const time_t* ticks, struct tm* temp)
{
#if wxUSE_THREADS && !defined(__WINDOWS__)
      wxMutexLocker locker(timeLock);
#endif

#ifdef __BORLANDC__
  if ( !*ticks )
      return NULL;
#endif

  const tm * const t = gmtime(ticks);
  if ( !t )
      return NULL;

  memcpy(temp, gmtime(ticks), sizeof(struct tm));
  return temp;
}
#endif 
int wxGetTimeZone()
{
#ifdef WX_GMTOFF_IN_TM
        static bool s_timezoneSet = false;
    static long gmtoffset = LONG_MAX; 
        if ( !s_timezoneSet )
    {
                        time_t t = time(NULL);
        struct tm tm;

        wxLocaltime_r(&t, &tm);
        s_timezoneSet = true;

                                gmtoffset = -tm.tm_gmtoff;

                                if ( tm.tm_isdst )
            gmtoffset += 3600;
    }
    return (int)gmtoffset;
#elif defined(__WINE__)
    struct timeb tb;
    ftime(&tb);
    return tb.timezone*60;
#elif defined(__VISUALC__)
            _tzset();

                #if wxCHECK_VISUALC_VERSION(8)
        long t;
        _get_timezone(&t);
        return t;
    #else         return timezone;
    #endif
#else         tzset();

    #if defined(WX_TIMEZONE)         return WX_TIMEZONE;
    #elif defined(__BORLANDC__) || defined(__MINGW32__)
        return _timezone;
    #else         return timezone;
    #endif #endif }

long wxGetLocalTime()
{
    struct tm tm;
    time_t t0, t1;

            memset(&tm, 0, sizeof(tm));
    tm.tm_year  = 70;
    tm.tm_mon   = 0;
    tm.tm_mday  = 5;            tm.tm_hour  = 0;
    tm.tm_min   = 0;
    tm.tm_sec   = 0;
    tm.tm_isdst = -1;       
            t1 = time(&t1);             t0 = mktime(&tm);       
            if (( t0 != (time_t)-1 ) && ( t1 != (time_t)-1 ))
        return (long)difftime(t1, t0) + (60 * 60 * 24 * 4);

    wxLogSysError(_("Failed to get the local system time"));
    return -1;
}

long wxGetUTCTime()
{
    return (long)time(NULL);
}

#if wxUSE_LONGLONG

wxLongLong wxGetUTCTimeUSec()
{
#if defined(__WINDOWS__)
    FILETIME ft;
    ::GetSystemTimeAsFileTime(&ft);

            wxLongLong t(ft.dwHighDateTime, ft.dwLowDateTime);
    t /= 10;
    t -= wxLL(11644473600000000);     return t;
#else 
#ifdef HAVE_GETTIMEOFDAY
    timeval tv;
    if ( wxGetTimeOfDay(&tv) != -1 )
    {
        wxLongLong val(tv.tv_sec);
        val *= MICROSECONDS_PER_SECOND;
        val += tv.tv_usec;
        return val;
    }
#endif 
        return wxGetUTCTimeMillis()*MICROSECONDS_PER_MILLISECOND;
#endif }

wxLongLong wxGetUTCTimeMillis()
{
        #if defined(__WINDOWS__)
    FILETIME ft;
    ::GetSystemTimeAsFileTime(&ft);

            wxLongLong t(ft.dwHighDateTime, ft.dwLowDateTime);
    t /= 10000;
    t -= wxLL(11644473600000);     return t;
#else     wxLongLong val = MILLISECONDS_PER_SECOND;

#if defined(HAVE_GETTIMEOFDAY)
    struct timeval tp;
    if ( wxGetTimeOfDay(&tp) != -1 )
    {
        val *= tp.tv_sec;
        return (val + (tp.tv_usec / MICROSECONDS_PER_MILLISECOND));
    }
    else
    {
        wxLogError(_("wxGetTimeOfDay failed."));
        return 0;
    }
#elif defined(HAVE_FTIME)
    struct timeb tp;

            (void)::ftime(&tp);
    val *= tp.time;
    return (val + tp.millitm);
#else             
    #if defined(__VISUALC__)
        #pragma message("wxStopWatch will be up to second resolution!")
    #elif defined(__BORLANDC__)
        #pragma message "wxStopWatch will be up to second resolution!"
    #else
        #warning "wxStopWatch will be up to second resolution!"
    #endif 
    val *= wxGetUTCTime();
    return val;
#endif 
#endif }

wxLongLong wxGetLocalTimeMillis()
{
    return wxGetUTCTimeMillis() - wxGetTimeZone()*MILLISECONDS_PER_SECOND;
}

#else 
double wxGetLocalTimeMillis(void)
{
    return (double(clock()) / double(CLOCKS_PER_SEC)) * 1000.0;
}

#endif 