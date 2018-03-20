


#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#if !defined(wxUSE_DATETIME) || wxUSE_DATETIME

#ifndef WX_PRECOMP
    #ifdef __WINDOWS__
        #include "wx/msw/wrapwin.h"
    #endif
    #include "wx/string.h"
    #include "wx/log.h"
    #include "wx/intl.h"
    #include "wx/stopwatch.h"               #include "wx/module.h"
    #include "wx/crt.h"
#endif 
#include "wx/thread.h"

#include <ctype.h>

#ifdef __WINDOWS__
    #include <winnls.h>
    #include <locale.h>
#endif

#include "wx/datetime.h"
#include "wx/time.h"



extern void InitTm(struct tm& tm);

extern wxString CallStrftime(const wxString& format, const tm* tm);


static const int DAYS_PER_WEEK = 7;

static const int HOURS_PER_DAY = 24;

static const int SEC_PER_MIN = 60;

static const int MIN_PER_HOUR = 60;


namespace
{


bool GetNumericToken(size_t len,
                     wxString::const_iterator& p,
                     const wxString::const_iterator& end,
                     unsigned long *number)
{
    size_t n = 1;
    wxString s;
    while ( p != end && wxIsdigit(*p) )
    {
        s += *p++;

        if ( len && ++n > len )
            break;
    }

    return !s.empty() && s.ToULong(number);
}

wxString
GetAlphaToken(wxString::const_iterator& p,
              const wxString::const_iterator& end)
{
    wxString s;
    while ( p != end && wxIsalpha(*p) )
    {
        s += *p++;
    }

    return s;
}

enum
{
    DateLang_English = 1,
    DateLang_Local   = 2
};

wxDateTime::Month
GetMonthFromName(wxString::const_iterator& p,
                 const wxString::const_iterator& end,
                 int flags,
                 int lang)
{
    const wxString::const_iterator pOrig = p;
    const wxString name = GetAlphaToken(p, end);
    if ( name.empty() )
        return wxDateTime::Inv_Month;

    wxDateTime::Month mon;
    for ( mon = wxDateTime::Jan; mon < wxDateTime::Inv_Month; wxNextMonth(mon) )
    {
                        if ( flags & wxDateTime::Name_Full )
        {
            if ( lang & DateLang_English )
            {
                if ( name.CmpNoCase(wxDateTime::GetEnglishMonthName(mon,
                        wxDateTime::Name_Full)) == 0 )
                    break;
            }

            if ( lang & DateLang_Local )
            {
                if ( name.CmpNoCase(wxDateTime::GetMonthName(mon,
                        wxDateTime::Name_Full)) == 0 )
                    break;
            }
        }

        if ( flags & wxDateTime::Name_Abbr )
        {
            if ( lang & DateLang_English )
            {
                if ( name.CmpNoCase(wxDateTime::GetEnglishMonthName(mon,
                        wxDateTime::Name_Abbr)) == 0 )
                    break;
            }

            if ( lang & DateLang_Local )
            {
                                                                wxString nameAbbr = wxDateTime::GetMonthName(mon,
                    wxDateTime::Name_Abbr);
                const bool hasPeriod = *nameAbbr.rbegin() == '.';
                if ( hasPeriod )
                    nameAbbr.erase(nameAbbr.end() - 1);

                if ( name.CmpNoCase(nameAbbr) == 0 )
                {
                    if ( hasPeriod )
                    {
                                                if ( *p == '.' )
                            ++p;
                        else                             continue;
                    }

                    break;
                }
            }
        }
    }

    if ( mon == wxDateTime::Inv_Month )
        p = pOrig;

    return mon;
}

wxDateTime::WeekDay
GetWeekDayFromName(wxString::const_iterator& p,
                   const wxString::const_iterator& end,
                   int flags, int lang)
{
    const wxString::const_iterator pOrig = p;
    const wxString name = GetAlphaToken(p, end);
    if ( name.empty() )
        return wxDateTime::Inv_WeekDay;

    wxDateTime::WeekDay wd;
    for ( wd = wxDateTime::Sun; wd < wxDateTime::Inv_WeekDay; wxNextWDay(wd) )
    {
        if ( flags & wxDateTime::Name_Full )
        {
            if ( lang & DateLang_English )
            {
                if ( name.CmpNoCase(wxDateTime::GetEnglishWeekDayName(wd,
                        wxDateTime::Name_Full)) == 0 )
                    break;
            }

            if ( lang & DateLang_Local )
            {
                if ( name.CmpNoCase(wxDateTime::GetWeekDayName(wd,
                        wxDateTime::Name_Full)) == 0 )
                    break;
            }
        }

        if ( flags & wxDateTime::Name_Abbr )
        {
            if ( lang & DateLang_English )
            {
                if ( name.CmpNoCase(wxDateTime::GetEnglishWeekDayName(wd,
                        wxDateTime::Name_Abbr)) == 0 )
                    break;
            }

            if ( lang & DateLang_Local )
            {
                if ( name.CmpNoCase(wxDateTime::GetWeekDayName(wd,
                        wxDateTime::Name_Abbr)) == 0 )
                    break;
            }
        }
    }

    if ( wd == wxDateTime::Inv_WeekDay )
        p = pOrig;

    return wd;
}

wxDateTime
ParseFormatAt(wxString::const_iterator& p,
              const wxString::const_iterator& end,
              const wxString& fmt,
              const wxString& fmtAlt = wxString())
{
    const wxString str(p, end);
    wxString::const_iterator endParse;
    wxDateTime dt;

                static const wxDateTime dtDef(1, wxDateTime::Jan, 2012);

    if ( dt.ParseFormat(str, fmt, dtDef, &endParse) ||
            (!fmtAlt.empty() && dt.ParseFormat(str, fmtAlt, dtDef, &endParse)) )
    {
        p += endParse - str.begin();
    }
    
    return dt;
}

} 

wxString wxDateTime::Format(const wxString& formatp, const TimeZone& tz) const
{
    wxCHECK_MSG( !formatp.empty(), wxEmptyString,
                 wxT("NULL format in wxDateTime::Format") );

    wxString format = formatp;
#ifdef __WXOSX__
    if ( format.Contains("%c") )
        format.Replace("%c", wxLocale::GetInfo(wxLOCALE_DATE_TIME_FMT));
    if ( format.Contains("%x") )
        format.Replace("%x", wxLocale::GetInfo(wxLOCALE_SHORT_DATE_FMT));
    if ( format.Contains("%X") )
        format.Replace("%X", wxLocale::GetInfo(wxLOCALE_TIME_FMT));
#endif
        #ifdef wxHAS_STRFTIME
    time_t time = GetTicks();

    bool canUseStrftime = time != (time_t)-1;

                for ( wxString::const_iterator p = format.begin();
          canUseStrftime && p != format.end();
          ++p )
    {
        if ( *p != '%' )
            continue;

                switch ( (*++p).GetValue() )
        {
            case 'l':
#ifdef __WINDOWS__
            case 'g':
            case 'G':
            case 'V':
            case 'z':
#endif                 canUseStrftime = false;
                break;
        }
    }

    if ( canUseStrftime )
    {
                struct tm tmstruct;
        struct tm *tm;
        if ( tz.GetOffset() == -wxGetTimeZone() )
        {
                        tm = wxLocaltime_r(&time, &tmstruct);

                        wxCHECK_MSG( tm, wxEmptyString, wxT("wxLocaltime_r() failed") );
        }
        else
        {
            time += (int)tz.GetOffset();

#if defined(__VMS__)             int time2 = (int) time;
            if ( time2 >= 0 )
#else
            if ( time >= 0 )
#endif
            {
                tm = wxGmtime_r(&time, &tmstruct);

                                wxCHECK_MSG( tm, wxEmptyString, wxT("wxGmtime_r() failed") );
            }
            else
            {
                tm = (struct tm *)NULL;
            }
        }

        if ( tm )
        {
            return CallStrftime(format, tm);
        }
    }
    #endif 
                Tm tm = GetTm(tz);

        struct tm tmTimeOnly;
    memset(&tmTimeOnly, 0, sizeof(tmTimeOnly));
    tmTimeOnly.tm_hour = tm.hour;
    tmTimeOnly.tm_min = tm.min;
    tmTimeOnly.tm_sec = tm.sec;
    tmTimeOnly.tm_mday = 1;             tmTimeOnly.tm_mon = 0;
    tmTimeOnly.tm_year = 76;
    tmTimeOnly.tm_isdst = 0;        
    wxString tmp, res, fmt;
    for ( wxString::const_iterator p = format.begin(); p != format.end(); ++p )
    {
        if ( *p != wxT('%') )
        {
                        res += *p;

            continue;
        }

                switch ( (*++p).GetValue() )
        {
            case wxT('Y'):                           case wxT('G'):                           case wxT('z'):                               fmt = wxT("%04d");
                break;

            case wxT('j'):                           case wxT('l'):                               fmt = wxT("%03d");
                break;

            case wxT('w'):                               fmt = wxT("%d");
                break;

            default:
                                                                                fmt = wxT("%02d");
        }

        bool restart = true;
        while ( restart )
        {
            restart = false;

                        switch ( (*p).GetValue() )
            {
                case wxT('a'):                       case wxT('A'):
                                        res += GetWeekDayName(tm.GetWeekDay(),
                                          *p == wxT('a') ? Name_Abbr : Name_Full);
                    break;

                case wxT('b'):                       case wxT('B'):
                    res += GetMonthName(tm.mon,
                                        *p == wxT('b') ? Name_Abbr : Name_Full);
                    break;

                case wxT('c'):                       case wxT('x'):       #ifdef wxHAS_STRFTIME
                                                                                                                                                                                                                                                                                                                                                                                                                                    {
                                                                                                                                                                                                
                        int yearReal = GetYear(tz);
                        int mod28 = yearReal % 28;

                                                                        int year;
                        if ( mod28 < 10 )
                        {
                            year = 1988 + mod28;                              }
                        else
                        {
                            year = 1970 + mod28 - 10;                         }

                        int nCentury = year / 100,
                            nCenturyReal = yearReal / 100;

                                                                                                int nLostWeekDays = (nCentury - nCenturyReal) -
                                            (nCentury / 4 - nCenturyReal / 4);

                                                                                                                        while ( (nLostWeekDays % 7) != 0 )
                        {
                            nLostWeekDays += (year++ % 4) ? 1 : 2;
                        }

                                                                                                if ( year >= 2000 )
                            year -= 28;

                        wxASSERT_MSG( year >= 1970 && year < 2000,
                                      wxT("logic error in wxDateTime::Format") );


                                                                                                                                                                                                struct tm tmAdjusted;
                        InitTm(tmAdjusted);
                        tmAdjusted.tm_hour = tm.hour;
                        tmAdjusted.tm_min = tm.min;
                        tmAdjusted.tm_sec = tm.sec;
                        tmAdjusted.tm_wday = tm.GetWeekDay();
                        tmAdjusted.tm_yday = GetDayOfYear();
                        tmAdjusted.tm_mday = tm.mday;
                        tmAdjusted.tm_mon = tm.mon;
                        tmAdjusted.tm_year = year - 1900;
                        tmAdjusted.tm_isdst = 0;                         wxString str = CallStrftime(*p == wxT('c') ? wxT("%c")
                                                                  : wxT("%x"),
                                                    &tmAdjusted);

                                                                                                                                                                                                wxString replacement("|");
                        while ( str.find(replacement) != wxString::npos )
                            replacement += '|';

                        str.Replace(wxString::Format("%d", year),
                                    replacement);
                        str.Replace(wxString::Format("%d", year % 100),
                                    wxString::Format("%d", yearReal % 100));
                        str.Replace(replacement,
                                    wxString::Format("%d", yearReal));

                        res += str;
                    }
#else                                         res += wxString::Format(wxT("%02d/%02d/%04d %02d:%02d:%02d"),
                            tm.mon+1,tm.mday, tm.year, tm.hour, tm.min, tm.sec);
#endif                     break;

                case wxT('d'):                           res += wxString::Format(fmt, tm.mday);
                    break;

                case wxT('g'):                          res += wxString::Format(fmt, GetWeekBasedYear() % 100);
                    break;

                case wxT('G'):                           res += wxString::Format(fmt, GetWeekBasedYear());
                    break;

                case wxT('H'):                           res += wxString::Format(fmt, tm.hour);
                    break;

                case wxT('I'):                           {
                                                int hour12 = tm.hour > 12 ? tm.hour - 12
                                                  : tm.hour ? tm.hour : 12;
                        res += wxString::Format(fmt, hour12);
                    }
                    break;

                case wxT('j'):                           res += wxString::Format(fmt, GetDayOfYear(tz));
                    break;

                case wxT('l'):                           res += wxString::Format(fmt, GetMillisecond(tz));
                    break;

                case wxT('m'):                           res += wxString::Format(fmt, tm.mon + 1);
                    break;

                case wxT('M'):                           res += wxString::Format(fmt, tm.min);
                    break;

                case wxT('p'):       #ifdef wxHAS_STRFTIME
                    res += CallStrftime(wxT("%p"), &tmTimeOnly);
#else                     res += (tmTimeOnly.tm_hour > 12) ? wxT("pm") : wxT("am");
#endif                     break;

                case wxT('S'):                           res += wxString::Format(fmt, tm.sec);
                    break;

                case wxT('U'):                           res += wxString::Format(fmt, GetWeekOfYear(Sunday_First, tz));
                    break;

                case wxT('V'):                       case wxT('W'):                           res += wxString::Format(fmt, GetWeekOfYear(Monday_First, tz));
                    break;

                case wxT('w'):                           res += wxString::Format(fmt, tm.GetWeekDay());
                    break;

                
                case wxT('X'):                           #ifdef wxHAS_STRFTIME
                    res += CallStrftime(wxT("%X"), &tmTimeOnly);
#else                     res += wxString::Format(wxT("%02d:%02d:%02d"),tm.hour, tm.min, tm.sec);
#endif                     break;

                case wxT('y'):                           res += wxString::Format(fmt, tm.year % 100);
                    break;

                case wxT('Y'):                           res += wxString::Format(fmt, tm.year);
                    break;

                case wxT('z'):                           {
                        int ofs = tz.GetOffset();

                                                                                                if ( ofs == -wxGetTimeZone() && IsDST() == 1 )
                        {
                                                                                                                ofs += 3600;
                        }

                        if ( ofs < 0 )
                        {
                            res += '-';
                            ofs = -ofs;
                        }
                        else
                        {
                            res += '+';
                        }

                                                res += wxString::Format(fmt,
                                                100*(ofs/3600) + (ofs/60)%60);
                    }
                    break;

                case wxT('Z'):       #ifdef wxHAS_STRFTIME
                    res += CallStrftime(wxT("%Z"), &tmTimeOnly);
#endif
                    break;

                default:
                                        for ( fmt.clear();
                          *p == wxT('-') || *p == wxT('+') ||
                            *p == wxT(' ') || wxIsdigit(*p);
                          ++p )
                    {
                        fmt += *p;
                    }

                    if ( !fmt.empty() )
                    {
                                                fmt.Prepend(wxT('%'));
                        fmt.Append(wxT('d'));

                        restart = true;

                        break;
                    }

                                        wxFAIL_MSG(wxT("unknown format specifier"));

                    wxFALLTHROUGH;

                case wxT('%'):                           res += *p;
                    break;

                case 0:                                 wxFAIL_MSG(wxT("missing format at the end of string"));

                                        res += wxT('%');
                    break;
            }
        }
    }

    return res;
}

bool
wxDateTime::ParseRfc822Date(const wxString& date, wxString::const_iterator *end)
{
    const wxString::const_iterator pEnd = date.end();
    wxString::const_iterator p = date.begin();

        const wxDateTime::WeekDay
        wd = GetWeekDayFromName(p, pEnd, Name_Abbr, DateLang_English);
    if ( wd == Inv_WeekDay )
        return false;
        
        if ( *p++ != ',' || *p++ != ' ' )
        return false;

        if ( !wxIsdigit(*p) )
        return false;

    wxDateTime_t day = (wxDateTime_t)(*p++ - '0');
    if ( wxIsdigit(*p) )
    {
        day *= 10;
        day = (wxDateTime_t)(day + (*p++ - '0'));
    }

    if ( *p++ != ' ' )
        return false;

        const Month mon = GetMonthFromName(p, pEnd, Name_Abbr, DateLang_English);
    if ( mon == Inv_Month )
        return false;

    if ( *p++ != ' ' )
        return false;

        if ( !wxIsdigit(*p) )
        return false;

    int year = *p++ - '0';
    if ( !wxIsdigit(*p) )         return false;

    year *= 10;
    year += *p++ - '0';

        if ( wxIsdigit(*p) )
    {
        year *= 10;
        year += *p++ - '0';

        if ( !wxIsdigit(*p) )
        {
                        return false;
        }

        year *= 10;
        year += *p++ - '0';
    }

    if ( *p++ != ' ' )
        return false;

        if ( !wxIsdigit(*p) )
        return false;

    wxDateTime_t hour = (wxDateTime_t)(*p++ - '0');

    if ( !wxIsdigit(*p) )
        return false;

    hour *= 10;
    hour = (wxDateTime_t)(hour + (*p++ - '0'));

    if ( *p++ != ':' )
        return false;

    if ( !wxIsdigit(*p) )
        return false;

    wxDateTime_t min = (wxDateTime_t)(*p++ - '0');

    if ( !wxIsdigit(*p) )
        return false;

    min *= 10;
    min += (wxDateTime_t)(*p++ - '0');

    wxDateTime_t sec = 0;
    if ( *p == ':' )
    {
        p++;
        if ( !wxIsdigit(*p) )
            return false;

        sec = (wxDateTime_t)(*p++ - '0');

        if ( !wxIsdigit(*p) )
            return false;

        sec *= 10;
        sec += (wxDateTime_t)(*p++ - '0');
    }

    if ( *p++ != ' ' )
        return false;

        int offset = 0;     if ( *p == '-' || *p == '+' )
    {
                bool plus = *p++ == '+';

        if ( !wxIsdigit(*p) || !wxIsdigit(*(p + 1)) )
            return false;


                offset = MIN_PER_HOUR*(10*(*p - '0') + (*(p + 1) - '0'));

        p += 2;

        if ( !wxIsdigit(*p) || !wxIsdigit(*(p + 1)) )
            return false;

                offset += 10*(*p - '0') + (*(p + 1) - '0');

        if ( !plus )
            offset = -offset;

        p += 2;
    }
    else     {
                        if ( !*(p + 1) )
        {
                        static const int offsets[26] =
            {
                                -1, -2, -3, -4, -5, -6, -7, -8, -9,   0, -10, -11, -12,
                                +1, +2, +3, +4, +5, +6, +7, +8, +9, +10, +11, +12, 0
            };

            if ( *p < wxT('A') || *p > wxT('Z') || *p == wxT('J') )
                return false;

            offset = offsets[*p++ - 'A'];
        }
        else
        {
                        const wxString tz(p, date.end());
            if ( tz == wxT("UT") || tz == wxT("UTC") || tz == wxT("GMT") )
                offset = 0;
            else if ( tz == wxT("AST") )
                offset = AST - GMT0;
            else if ( tz == wxT("ADT") )
                offset = ADT - GMT0;
            else if ( tz == wxT("EST") )
                offset = EST - GMT0;
            else if ( tz == wxT("EDT") )
                offset = EDT - GMT0;
            else if ( tz == wxT("CST") )
                offset = CST - GMT0;
            else if ( tz == wxT("CDT") )
                offset = CDT - GMT0;
            else if ( tz == wxT("MST") )
                offset = MST - GMT0;
            else if ( tz == wxT("MDT") )
                offset = MDT - GMT0;
            else if ( tz == wxT("PST") )
                offset = PST - GMT0;
            else if ( tz == wxT("PDT") )
                offset = PDT - GMT0;
            else
                return false;

            p += tz.length();
        }

                offset *= MIN_PER_HOUR;
    }


        Set(day, mon, year, hour, min, sec);

                        *this -= wxTimeSpan::Minutes(offset);
    MakeFromUTC();

    if ( end )
        *end = p;

    return true;
}

const char* wxDateTime::ParseRfc822Date(const char* date)
{
    wxString::const_iterator end;
    wxString dateStr(date);
    if ( !ParseRfc822Date(dateStr, &end) )
        return NULL;

    return date + dateStr.IterOffsetInMBStr(end);
}

const wchar_t* wxDateTime::ParseRfc822Date(const wchar_t* date)
{
    wxString::const_iterator end;
    wxString dateStr(date);
    if ( !ParseRfc822Date(dateStr, &end) )
        return NULL;

    return date + (end - dateStr.begin());
}

bool
wxDateTime::ParseFormat(const wxString& date,
                        const wxString& format,
                        const wxDateTime& dateDef,
                        wxString::const_iterator *endParse)
{
    wxCHECK_MSG( !format.empty(), false, "format can't be empty" );
    wxCHECK_MSG( endParse, false, "end iterator pointer must be specified" );

    wxString str;
    unsigned long num;

        bool haveWDay = false,
         haveYDay = false,
         haveDay = false,
         haveMon = false,
         haveYear = false,
         haveHour = false,
         haveMin = false,
         haveSec = false,
         haveMsec = false;

    bool hourIsIn12hFormat = false,          isPM = false;              
    bool haveTimeZone = false;

        wxDateTime_t msec = 0,
                 sec = 0,
                 min = 0,
                 hour = 0;
    WeekDay wday = Inv_WeekDay;
    wxDateTime_t yday = 0,
                 mday = 0;
    wxDateTime::Month mon = Inv_Month;
    int year = 0;
    long timeZone = 0;  
    wxString::const_iterator input = date.begin();
    const wxString::const_iterator end = date.end();
    for ( wxString::const_iterator fmt = format.begin(); fmt != format.end(); ++fmt )
    {
        if ( *fmt != wxT('%') )
        {
            if ( wxIsspace(*fmt) )
            {
                                                while ( input != end && wxIsspace(*input) )
                {
                    input++;
                }
            }
            else             {
                                                if ( input == end || *input++ != *fmt )
                {
                                        return false;
                }
            }

                        continue;
        }

        
                size_t width = 0;
        while ( wxIsdigit(*++fmt) )
        {
            width *= 10;
            width += *fmt - '0';
        }

                if ( !width )
        {
            switch ( (*fmt).GetValue() )
            {
                case wxT('Y'):                                   width = 4;
                    break;

                case wxT('j'):                               case wxT('l'):                                   width = 3;
                    break;

                case wxT('w'):                                   width = 1;
                    break;

                default:
                                        width = 2;
            }
        }

                switch ( (*fmt).GetValue() )
        {
            case wxT('a'):                   case wxT('A'):
                {
                    wday = GetWeekDayFromName
                           (
                            input, end,
                            *fmt == 'a' ? Name_Abbr : Name_Full,
                            DateLang_Local
                           );
                    if ( wday == Inv_WeekDay )
                    {
                                                return false;
                    }
                }
                haveWDay = true;
                break;

            case wxT('b'):                   case wxT('B'):
                {
                    mon = GetMonthFromName
                          (
                            input, end,
                            *fmt == 'b' ? Name_Abbr : Name_Full,
                            DateLang_Local
                          );
                    if ( mon == Inv_Month )
                    {
                                                return false;
                    }
                }
                haveMon = true;
                break;

            case wxT('c'):                       {
                    wxDateTime dt;

#if wxUSE_INTL
                    const wxString
                        fmtDateTime = wxLocale::GetInfo(wxLOCALE_DATE_TIME_FMT);
                    if ( !fmtDateTime.empty() )
                        dt = ParseFormatAt(input, end, fmtDateTime);
#endif                     if ( !dt.IsValid() )
                    {
                                                                        dt = ParseFormatAt(input, end, wxS("%a %b %d %H:%M:%S %Y"));
                    }

                    if ( !dt.IsValid() )
                    {
                                                dt = ParseFormatAt(input, end, wxS("%x %X"), wxS("%X %x"));
                    }

                    if ( !dt.IsValid() )
                        return false;

                    const Tm tm = dt.GetTm();

                    hour = tm.hour;
                    min = tm.min;
                    sec = tm.sec;

                    year = tm.year;
                    mon = tm.mon;
                    mday = tm.mday;

                    haveDay = haveMon = haveYear =
                    haveHour = haveMin = haveSec = true;
                }
                break;

            case wxT('d'):                   case 'e':                           if ( !GetNumericToken(width, input, end, &num) ||
                        (num > 31) || (num < 1) )
                {
                                        return false;
                }

                                                haveDay = true;
                mday = (wxDateTime_t)num;
                break;

            case wxT('H'):                       if ( !GetNumericToken(width, input, end, &num) || (num > 23) )
                {
                                        return false;
                }

                haveHour = true;
                hour = (wxDateTime_t)num;
                break;

            case wxT('I'):                       if ( !GetNumericToken(width, input, end, &num) ||
                        !num || (num > 12) )
                {
                                        return false;
                }

                haveHour = true;
                hourIsIn12hFormat = true;
                hour = (wxDateTime_t)(num % 12);                        break;

            case wxT('j'):                       if ( !GetNumericToken(width, input, end, &num) ||
                        !num || (num > 366) )
                {
                                        return false;
                }

                haveYDay = true;
                yday = (wxDateTime_t)num;
                break;

            case wxT('l'):                       if ( !GetNumericToken(width, input, end, &num) )
                    return false;

                haveMsec = true;
                msec = (wxDateTime_t)num;
                break;

            case wxT('m'):                       if ( !GetNumericToken(width, input, end, &num) ||
                        !num || (num > 12) )
                {
                                        return false;
                }

                haveMon = true;
                mon = (Month)(num - 1);
                break;

            case wxT('M'):                       if ( !GetNumericToken(width, input, end, &num) ||
                        (num > 59) )
                {
                                        return false;
                }

                haveMin = true;
                min = (wxDateTime_t)num;
                break;

            case wxT('p'):                       {
                    wxString am, pm;
                    GetAmPmStrings(&am, &pm);

                                        if ( am.empty() || pm.empty() )
                        return false;

                    const size_t pos = input - date.begin();
                    if ( date.compare(pos, pm.length(), pm) == 0 )
                    {
                        isPM = true;
                        input += pm.length();
                    }
                    else if ( date.compare(pos, am.length(), am) == 0 )
                    {
                        input += am.length();
                    }
                    else                     {
                        return false;
                    }
                }
                break;

            case wxT('r'):                       {
                    wxDateTime dt;
                    if ( !dt.ParseFormat(wxString(input, end),
                                         wxS("%I:%M:%S %p"), &input) )
                        return false;

                    haveHour = haveMin = haveSec = true;

                    const Tm tm = dt.GetTm();
                    hour = tm.hour;
                    min = tm.min;
                    sec = tm.sec;
                }
                break;

            case wxT('R'):                       {
                    const wxDateTime
                        dt = ParseFormatAt(input, end, wxS("%H:%M"));
                    if ( !dt.IsValid() )
                        return false;

                    haveHour =
                    haveMin = true;

                    const Tm tm = dt.GetTm();
                    hour = tm.hour;
                    min = tm.min;
                }
                break;

            case wxT('S'):                       if ( !GetNumericToken(width, input, end, &num) ||
                        (num > 61) )
                {
                                        return false;
                }

                haveSec = true;
                sec = (wxDateTime_t)num;
                break;

            case wxT('T'):                       {
                    const wxDateTime
                        dt = ParseFormatAt(input, end, wxS("%H:%M:%S"));
                    if ( !dt.IsValid() )
                        return false;

                    haveHour =
                    haveMin =
                    haveSec = true;

                    const Tm tm = dt.GetTm();
                    hour = tm.hour;
                    min = tm.min;
                    sec = tm.sec;
                }
                break;

            case wxT('w'):                       if ( !GetNumericToken(width, input, end, &num) ||
                        (wday > 6) )
                {
                                        return false;
                }

                haveWDay = true;
                wday = (WeekDay)num;
                break;

            case wxT('x'):                       {
#if wxUSE_INTL
                    wxString
                        fmtDate = wxLocale::GetInfo(wxLOCALE_SHORT_DATE_FMT),
                        fmtDateAlt = wxLocale::GetInfo(wxLOCALE_LONG_DATE_FMT);
#else                     wxString fmtDate, fmtDateAlt;
#endif                     if ( fmtDate.empty() )
                    {
                        if ( IsWestEuropeanCountry(GetCountry()) ||
                             GetCountry() == Russia )
                        {
                            fmtDate = wxS("%d/%m/%Y");
                            fmtDateAlt = wxS("%m/%d/%Y");
                         }
                        else                         {
                            fmtDate = wxS("%m/%d/%Y");
                            fmtDateAlt = wxS("%d/%m/%Y");
                        }
                    }

                    wxDateTime
                        dt = ParseFormatAt(input, end, fmtDate, fmtDateAlt);

                    if ( !dt.IsValid() )
                    {
                                                fmtDate.Replace("%Y","%y");
                        fmtDateAlt.Replace("%Y","%y");
                        dt = ParseFormatAt(input, end, fmtDate, fmtDateAlt);

                        if ( !dt.IsValid() )
                            return false;
                    }

                    const Tm tm = dt.GetTm();

                    haveDay =
                    haveMon =
                    haveYear = true;

                    year = tm.year;
                    mon = tm.mon;
                    mday = tm.mday;
                }

                break;

            case wxT('X'):                       {
#if wxUSE_INTL
                    wxString fmtTime = wxLocale::GetInfo(wxLOCALE_TIME_FMT),
                             fmtTimeAlt;
#else                     wxString fmtTime, fmtTimeAlt;
#endif                     if ( fmtTime.empty() )
                    {
                                                                                                fmtTime = "%T";
                        fmtTimeAlt = "%r";
                    }

                    const wxDateTime
                        dt = ParseFormatAt(input, end, fmtTime, fmtTimeAlt);
                    if ( !dt.IsValid() )
                        return false;

                    haveHour =
                    haveMin =
                    haveSec = true;

                    const Tm tm = dt.GetTm();
                    hour = tm.hour;
                    min = tm.min;
                    sec = tm.sec;
                }
                break;

            case wxT('y'):                       if ( !GetNumericToken(width, input, end, &num) ||
                        (num > 99) )
                {
                                        return false;
                }

                haveYear = true;

                                                year = (num > 30 ? 1900 : 2000) + (wxDateTime_t)num;
                break;

            case wxT('Y'):                       if ( !GetNumericToken(width, input, end, &num) )
                {
                                        return false;
                }

                haveYear = true;
                year = (wxDateTime_t)num;
                break;

            case wxT('z'):
                {
                                        if ( input == end )
                        return false;

                                        bool minusFound;
                    if ( *input == wxT('-') )
                        minusFound = true;
                    else if ( *input == wxT('+') )
                        minusFound = false;
                    else
                        return false;   
                                        ++input;
                    unsigned long tzHourMin;
                    if ( !GetNumericToken(4, input, end, &tzHourMin) )
                        return false;   
                    const unsigned hours = tzHourMin / 100;
                    const unsigned minutes = tzHourMin % 100;

                    if ( hours > 12 || minutes > 59 )
                        return false;   
                    timeZone = 3600*hours + 60*minutes;
                    if ( minusFound )
                        timeZone = -timeZone;

                    haveTimeZone = true;
                }
                break;

            case wxT('Z'):                                                       GetAlphaToken(input, end);
                break;

            case wxT('%'):                       if ( input == end || *input++ != wxT('%') )
                {
                                        return false;
                }
                break;

            case 0:                             wxFAIL_MSG(wxT("unexpected format end"));

                wxFALLTHROUGH;

            default:                            return false;
        }
    }

        Tm tmDef;
    if ( dateDef.IsValid() )
    {
                tmDef = dateDef.GetTm();
    }
    else if ( IsValid() )
    {
                tmDef = GetTm();
    }
    else
    {
                tmDef = Today().GetTm();
    }

    Tm tm = tmDef;

        if ( haveMon )
    {
        tm.mon = mon;
    }

    if ( haveYear )
    {
        tm.year = year;
    }

                if ( haveDay )
    {
        if ( mday > GetNumberOfDays(tm.mon, tm.year) )
            return false;

        tm.mday = mday;
    }
    else if ( haveYDay )
    {
        if ( yday > GetNumberOfDays(tm.year) )
            return false;

        Tm tm2 = wxDateTime(1, Jan, tm.year).SetToYearDay(yday).GetTm();

        tm.mon = tm2.mon;
        tm.mday = tm2.mday;
    }

        if ( haveHour && hourIsIn12hFormat && isPM )
    {
                hour += 12;
    }
    
        if ( haveHour )
    {
        tm.hour = hour;
    }

    if ( haveMin )
    {
        tm.min = min;
    }

    if ( haveSec )
    {
        tm.sec = sec;
    }

    if ( haveMsec )
        tm.msec = msec;

    Set(tm);

                        if ( haveTimeZone && timeZone != -wxGetTimeZone() )
        MakeFromTimezone(timeZone);

        if ( haveWDay && GetWeekDay() != wday )
        return false;

    *endParse = input;

    return true;
}

const char*
wxDateTime::ParseFormat(const char* date,
                        const wxString& format,
                        const wxDateTime& dateDef)
{
    wxString::const_iterator end;
    wxString dateStr(date);
    if ( !ParseFormat(dateStr, format, dateDef, &end) )
        return NULL;

    return date + dateStr.IterOffsetInMBStr(end);
}

const wchar_t*
wxDateTime::ParseFormat(const wchar_t* date,
                        const wxString& format,
                        const wxDateTime& dateDef)
{
    wxString::const_iterator end;
    wxString dateStr(date);
    if ( !ParseFormat(dateStr, format, dateDef, &end) )
        return NULL;

    return date + (end - dateStr.begin());
}

bool
wxDateTime::ParseDateTime(const wxString& date, wxString::const_iterator *end)
{
    wxCHECK_MSG( end, false, "end iterator pointer must be specified" );

    wxDateTime
        dtDate,
        dtTime;

    wxString::const_iterator
        endTime,
        endDate,
        endBoth;

            if ( dtDate.ParseDate(date, &endDate) )
    {
                while ( endDate != date.end() && wxIsspace(*endDate) )
            ++endDate;

        const wxString timestr(endDate, date.end());
        if ( !dtTime.ParseTime(timestr, &endTime) )
            return false;

        endBoth = endDate + (endTime - timestr.begin());
    }
    else     {
                if ( !dtTime.ParseTime(date, &endTime) )
            return false;

        while ( endTime != date.end() && wxIsspace(*endTime) )
            ++endTime;

        const wxString datestr(endTime, date.end());
        if ( !dtDate.ParseDate(datestr, &endDate) )
            return false;

        endBoth = endTime + (endDate - datestr.begin());
    }

    Set(dtDate.GetDay(), dtDate.GetMonth(), dtDate.GetYear(),
        dtTime.GetHour(), dtTime.GetMinute(), dtTime.GetSecond(),
        dtTime.GetMillisecond());

    *end = endBoth;

    return true;
}

const char* wxDateTime::ParseDateTime(const char* date)
{
    wxString::const_iterator end;
    wxString dateStr(date);
    if ( !ParseDateTime(dateStr, &end) )
        return NULL;

    return date + dateStr.IterOffsetInMBStr(end);
}

const wchar_t* wxDateTime::ParseDateTime(const wchar_t* date)
{
    wxString::const_iterator end;
    wxString dateStr(date);
    if ( !ParseDateTime(dateStr, &end) )
        return NULL;

    return date + (end - dateStr.begin());
}

bool
wxDateTime::ParseDate(const wxString& date, wxString::const_iterator *end)
{
    wxCHECK_MSG( end, false, "end iterator pointer must be specified" );

            
    const wxString::const_iterator pBegin = date.begin();
    const wxString::const_iterator pEnd = date.end();

    wxString::const_iterator p = pBegin;
    while ( p != pEnd && wxIsspace(*p) )
        p++;

        static struct
    {
        const char *str;
        int dayDiffFromToday;
    } literalDates[] =
    {
        { wxTRANSLATE("today"),             0 },
        { wxTRANSLATE("yesterday"),        -1 },
        { wxTRANSLATE("tomorrow"),          1 },
    };

    const size_t lenRest = pEnd - p;
    for ( size_t n = 0; n < WXSIZEOF(literalDates); n++ )
    {
        const wxString dateStr = wxGetTranslation(literalDates[n].str);
        size_t len = dateStr.length();

        if ( len > lenRest )
            continue;

        const wxString::const_iterator pEndStr = p + len;
        if ( wxString(p, pEndStr).CmpNoCase(dateStr) == 0 )
        {
            
            p = pEndStr;

            int dayDiffFromToday = literalDates[n].dayDiffFromToday;
            *this = Today();
            if ( dayDiffFromToday )
            {
                *this += wxDateSpan::Days(dayDiffFromToday);
            }

            *end = pEndStr;

            return true;
        }
    }

                
        bool haveDay = false,                haveWDay = false,               haveMon = false,                haveYear = false;      
    bool monWasNumeric = false; 
        WeekDay wday = Inv_WeekDay;
    wxDateTime_t day = 0;
    wxDateTime::Month mon = Inv_Month;
    int year = 0;

        while ( p != pEnd )
    {
                if ( wxStrchr(".,/-\t\r\n ", *p) )
        {
            ++p;
            continue;
        }

                        wxString::const_iterator pCopy = p;

                        unsigned long val;
        if ( GetNumericToken(10 , pCopy, pEnd, &val) )
        {
            
            bool isDay = false,
                 isMonth = false,
                 isYear = false;

            if ( !haveMon && val > 0 && val <= 12 )
            {
                                isMonth = true;
            }
            else             {
                if ( haveDay )
                {
                                        isYear = true;
                }
                else                 {
                                                            wxDateTime_t max_days = (wxDateTime_t)(
                        haveMon
                        ? GetNumberOfDays(mon, haveYear ? year : 1976)
                        : 31
                    );

                                        if ( (val == 0) || (val > (unsigned long)max_days) )
                    {
                                                isYear = true;
                    }
                    else                     {
                        isDay = true;
                    }
                }
            }

            if ( isYear )
            {
                if ( haveYear )
                    break;

                haveYear = true;

                year = (wxDateTime_t)val;
            }
            else if ( isDay )
            {
                if ( haveDay )
                    break;

                haveDay = true;

                day = (wxDateTime_t)val;
            }
            else if ( isMonth )
            {
                haveMon = true;
                monWasNumeric = true;

                mon = (Month)(val - 1);
            }
        }
        else         {
                        Month mon2 = GetMonthFromName
                         (
                            pCopy, pEnd,
                            Name_Full | Name_Abbr,
                            DateLang_Local | DateLang_English
                         );
            if ( mon2 != Inv_Month )
            {
                                if ( haveMon )
                {
                                                                                if ( haveDay || !monWasNumeric )
                        break;

                                                                                                    day = (wxDateTime_t)(mon + 1);
                    haveDay = true;
                }

                mon = mon2;

                haveMon = true;
            }
            else             {
                WeekDay wday2 = GetWeekDayFromName
                                (
                                    pCopy, pEnd,
                                    Name_Full | Name_Abbr,
                                    DateLang_Local | DateLang_English
                                );
                if ( wday2 != Inv_WeekDay )
                {
                                        if ( haveWDay )
                        break;

                    wday = wday2;

                    haveWDay = true;
                }
                else                 {
                                        static const char *const ordinals[] =
                    {
                        wxTRANSLATE("first"),
                        wxTRANSLATE("second"),
                        wxTRANSLATE("third"),
                        wxTRANSLATE("fourth"),
                        wxTRANSLATE("fifth"),
                        wxTRANSLATE("sixth"),
                        wxTRANSLATE("seventh"),
                        wxTRANSLATE("eighth"),
                        wxTRANSLATE("ninth"),
                        wxTRANSLATE("tenth"),
                        wxTRANSLATE("eleventh"),
                        wxTRANSLATE("twelfth"),
                        wxTRANSLATE("thirteenth"),
                        wxTRANSLATE("fourteenth"),
                        wxTRANSLATE("fifteenth"),
                        wxTRANSLATE("sixteenth"),
                        wxTRANSLATE("seventeenth"),
                        wxTRANSLATE("eighteenth"),
                        wxTRANSLATE("nineteenth"),
                        wxTRANSLATE("twentieth"),
                                                                    };

                    size_t n;
                    for ( n = 0; n < WXSIZEOF(ordinals); n++ )
                    {
                        const wxString ord = wxGetTranslation(ordinals[n]);
                        const size_t len = ord.length();
                        if ( date.compare(p - pBegin, len, ord) == 0 )
                        {
                            p += len;
                            break;
                        }
                    }

                    if ( n == WXSIZEOF(ordinals) )
                    {
                                                break;
                    }

                                        if ( haveDay )
                    {
                                                                                                break;
                    }

                    haveDay = true;

                    day = (wxDateTime_t)(n + 1);
                }
            }
        }

                p = pCopy;
    }

            if ( !haveDay && !haveWDay )
        return false;

    if ( haveWDay && (haveMon || haveYear || haveDay) &&
         !(haveDay && haveMon && haveYear) )
    {
                                return false;
    }

    if ( !haveWDay && haveYear && !(haveDay && haveMon) )
    {
                if ( haveDay && !haveMon )
        {
            if ( day <= 12  )
            {
                                mon = (wxDateTime::Month)(day - 1);

                                if ( (year > 0) && (year <= (int)GetNumberOfDays(mon, Inv_Year)) )
                {
                    day = (wxDateTime_t)year;

                    haveMon = true;
                    haveYear = false;
                }
                            }
        }

        if ( !haveMon )
            return false;
    }

    if ( !haveMon )
    {
        mon = GetCurrentMonth();
    }

    if ( !haveYear )
    {
        year = GetCurrentYear();
    }

    if ( haveDay )
    {
                        if ( day > GetNumberOfDays(mon, year) )
            return false;

        Set(day, mon, year);

        if ( haveWDay )
        {
                        if ( GetWeekDay() != wday )
                return false;
        }
    }
    else     {
        *this = Today();

        SetToWeekDayInSameWeek(wday);
    }

    *end = p;

    return true;
}

const char* wxDateTime::ParseDate(const char* date)
{
    wxString::const_iterator end;
    wxString dateStr(date);
    if ( !ParseDate(dateStr, &end) )
        return NULL;

    return date + dateStr.IterOffsetInMBStr(end);
}

const wchar_t* wxDateTime::ParseDate(const wchar_t* date)
{
    wxString::const_iterator end;
    wxString dateStr(date);
    if ( !ParseDate(dateStr, &end) )
        return NULL;

    return date + (end - dateStr.begin());
}

bool
wxDateTime::ParseTime(const wxString& time, wxString::const_iterator *end)
{
    wxCHECK_MSG( end, false, "end iterator pointer must be specified" );

        static const struct
    {
        const char *name;
        wxDateTime_t hour;
    } stdTimes[] =
    {
        { wxTRANSLATE("noon"),      12 },
        { wxTRANSLATE("midnight"),  00 },
            };

    for ( size_t n = 0; n < WXSIZEOF(stdTimes); n++ )
    {
        const wxString timeString = wxGetTranslation(stdTimes[n].name);
        if ( timeString.CmpNoCase(wxString(time, timeString.length())) == 0 )
        {
            Set(stdTimes[n].hour, 0, 0);

            if ( end )
                *end = time.begin() + timeString.length();

            return true;
        }
    }

            static const char *const timeFormats[] =
    {
        "%I:%M:%S %p",          "%H:%M:%S",             "%I:%M %p",             "%H:%M",                "%I %p",                "%H",                   "%X",                                   
            };

    for ( size_t nFmt = 0; nFmt < WXSIZEOF(timeFormats); nFmt++ )
    {
        if ( ParseFormat(time, timeFormats[nFmt], end) )
            return true;
    }

    return false;
}

const char* wxDateTime::ParseTime(const char* date)
{
    wxString::const_iterator end;
    wxString dateStr(date);
    if ( !ParseTime(dateStr, &end) )
        return NULL;

    return date + dateStr.IterOffsetInMBStr(end);
}

const wchar_t* wxDateTime::ParseTime(const wchar_t* date)
{
    wxString::const_iterator end;
    wxString dateStr(date);
    if ( !ParseTime(dateStr, &end) )
        return NULL;

    return date + (end - dateStr.begin());
}


bool wxDateTime::IsWorkDay(Country WXUNUSED(country)) const
{
    return !wxDateTimeHolidayAuthority::IsHoliday(*this);
}


wxDateSpan WXDLLIMPEXP_BASE operator*(int n, const wxDateSpan& ds)
{
    wxDateSpan ds1(ds);
    return ds1.Multiply(n);
}


wxTimeSpan WXDLLIMPEXP_BASE operator*(int n, const wxTimeSpan& ts)
{
    return wxTimeSpan(ts).Multiply(n);
}

enum TimeSpanPart
{
    Part_Week,
    Part_Day,
    Part_Hour,
    Part_Min,
    Part_Sec,
    Part_MSec
};

wxString wxTimeSpan::Format(const wxString& format) const
{
            if ( IsNegative() )
    {
        wxString str(Negate().Format(format));
        return "-" + str;
    }

    wxCHECK_MSG( !format.empty(), wxEmptyString,
                 wxT("NULL format in wxTimeSpan::Format") );

    wxString str;
    str.Alloc(format.length());

                                                
        TimeSpanPart partBiggest = Part_MSec;

    for ( wxString::const_iterator pch = format.begin(); pch != format.end(); ++pch )
    {
        wxChar ch = *pch;

        if ( ch == wxT('%') )
        {
                        wxString fmtPrefix(wxT('%'));

                        long n;

                        unsigned digits = 0;

            ch = *++pch;                switch ( ch )
            {
                default:
                    wxFAIL_MSG( wxT("invalid format character") );
                    wxFALLTHROUGH;

                case wxT('%'):
                    str += ch;

                                        continue;

                case wxT('D'):
                    n = GetDays();
                    if ( partBiggest < Part_Day )
                    {
                        n %= DAYS_PER_WEEK;
                    }
                    else
                    {
                        partBiggest = Part_Day;
                    }
                    break;

                case wxT('E'):
                    partBiggest = Part_Week;
                    n = GetWeeks();
                    break;

                case wxT('H'):
                    n = GetHours();
                    if ( partBiggest < Part_Hour )
                    {
                        n %= HOURS_PER_DAY;
                    }
                    else
                    {
                        partBiggest = Part_Hour;
                    }

                    digits = 2;
                    break;

                case wxT('l'):
                    n = GetMilliseconds().ToLong();
                    if ( partBiggest < Part_MSec )
                    {
                        n %= 1000;
                    }
                                        
                    digits = 3;
                    break;

                case wxT('M'):
                    n = GetMinutes();
                    if ( partBiggest < Part_Min )
                    {
                        n %= MIN_PER_HOUR;
                    }
                    else
                    {
                        partBiggest = Part_Min;
                    }

                    digits = 2;
                    break;

                case wxT('S'):
                    n = GetSeconds().ToLong();
                    if ( partBiggest < Part_Sec )
                    {
                        n %= SEC_PER_MIN;
                    }
                    else
                    {
                        partBiggest = Part_Sec;
                    }

                    digits = 2;
                    break;
            }

            if ( digits )
            {
                fmtPrefix << wxT("0") << digits;
            }

            str += wxString::Format(fmtPrefix + wxT("ld"), n);
        }
        else
        {
                        str += ch;
        }
    }

    return str;
}

#endif 