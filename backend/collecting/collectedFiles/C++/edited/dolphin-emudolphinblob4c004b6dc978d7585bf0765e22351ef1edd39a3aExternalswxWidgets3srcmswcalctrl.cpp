


#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#if wxUSE_CALENDARCTRL

#ifndef WX_PRECOMP
    #include "wx/msw/wrapwin.h"
    #include "wx/msw/wrapcctl.h"     #include "wx/msw/private.h"
#endif

#include "wx/calctrl.h"

#include "wx/msw/private/datecontrols.h"


namespace
{

enum
{
    MonthCal_Monday,
    MonthCal_Tuesday,
    MonthCal_Wednesday,
    MonthCal_Thursday,
    MonthCal_Friday,
    MonthCal_Saturday,
    MonthCal_Sunday
};

} 


void wxCalendarCtrl::Init()
{
    m_marks =
    m_holidays = 0;
}

bool
wxCalendarCtrl::Create(wxWindow *parent,
                       wxWindowID id,
                       const wxDateTime& dt,
                       const wxPoint& pos,
                       const wxSize& size,
                       long style,
                       const wxString& name)
{
    if ( !wxMSWDateControls::CheckInitialization() )
        return false;

        style |= wxWANTS_CHARS;

        if ( !CreateControl(parent, id, pos, size, style, wxDefaultValidator, name) )
        return false;

                static ClassRegistrar s_clsMonthCal;
    if ( !s_clsMonthCal.IsInitialized() )
    {
                WNDCLASS wc;
        if ( ::GetClassInfo(NULL, MONTHCAL_CLASS, &wc) )
        {
            wc.lpszClassName = wxT("_wx_SysMonthCtl32");
            wc.style |= CS_DBLCLKS;
            s_clsMonthCal.Register(wc);
        }
        else
        {
            wxLogLastError(wxT("GetClassInfoEx(SysMonthCal32)"));
        }
    }

    const wxChar * const clsname = s_clsMonthCal.IsRegistered()
        ? static_cast<const wxChar*>(s_clsMonthCal.GetName().t_str())
        : MONTHCAL_CLASS;

    if ( !MSWCreateControl(clsname, wxEmptyString, pos, size) )
        return false;

        UpdateFirstDayOfWeek();

    SetDate(dt.IsValid() ? dt : wxDateTime::Today());

    SetHolidayAttrs();
    UpdateMarks();

    Connect(wxEVT_LEFT_DOWN,
            wxMouseEventHandler(wxCalendarCtrl::MSWOnClick));
    Connect(wxEVT_LEFT_DCLICK,
            wxMouseEventHandler(wxCalendarCtrl::MSWOnDoubleClick));

    return true;
}

WXDWORD wxCalendarCtrl::MSWGetStyle(long style, WXDWORD *exstyle) const
{
    WXDWORD styleMSW = wxCalendarCtrlBase::MSWGetStyle(style, exstyle);

        
            styleMSW |= MCS_NOTODAY;

        styleMSW |= MCS_DAYSTATE;

    if ( style & wxCAL_SHOW_WEEK_NUMBERS )
       styleMSW |= MCS_WEEKNUMBERS;

    return styleMSW;
}

void wxCalendarCtrl::SetWindowStyleFlag(long style)
{
    const bool hadMondayFirst = HasFlag(wxCAL_MONDAY_FIRST);

    wxCalendarCtrlBase::SetWindowStyleFlag(style);

    if ( HasFlag(wxCAL_MONDAY_FIRST) != hadMondayFirst )
        UpdateFirstDayOfWeek();
}


wxSize wxCalendarCtrl::DoGetBestSize() const
{
    RECT rc;
    if ( !GetHwnd() || !MonthCal_GetMinReqRect(GetHwnd(), &rc) )
    {
        return wxCalendarCtrlBase::DoGetBestSize();
    }

    const wxSize best = wxRectFromRECT(rc).GetSize() + GetWindowBorderSize();
    CacheBestSize(best);
    return best;
}

wxCalendarHitTestResult
wxCalendarCtrl::HitTest(const wxPoint& pos,
                        wxDateTime *date,
                        wxDateTime::WeekDay *wd)
{
    WinStruct<MCHITTESTINFO> hti;

                #ifdef MCHITTESTINFO_V1_SIZE
    hti.cbSize = MCHITTESTINFO_V1_SIZE;
#endif

    hti.pt.x = pos.x;
    hti.pt.y = pos.y;
    switch ( MonthCal_HitTest(GetHwnd(), &hti) )
    {
        default:
        case MCHT_CALENDARWEEKNUM:
            wxFAIL_MSG( "unexpected" );
            
        case MCHT_NOWHERE:
        case MCHT_CALENDARBK:
        case MCHT_TITLEBK:
        case MCHT_TITLEMONTH:
        case MCHT_TITLEYEAR:
            return wxCAL_HITTEST_NOWHERE;

        case MCHT_CALENDARDATE:
            if ( date )
                date->SetFromMSWSysDate(hti.st);
            return wxCAL_HITTEST_DAY;

        case MCHT_CALENDARDAY:
            if ( wd )
            {
                int day = hti.st.wDayOfWeek;

                                                                const int first = LOWORD(MonthCal_GetFirstDayOfWeek(GetHwnd()));
                if ( first == MonthCal_Monday )
                {
                                                                                day++;
                    day %= 7;
                }
                                                                
                *wd = static_cast<wxDateTime::WeekDay>(day);
            }
            return wxCAL_HITTEST_HEADER;

        case MCHT_TITLEBTNNEXT:
            return wxCAL_HITTEST_INCMONTH;

        case MCHT_TITLEBTNPREV:
            return wxCAL_HITTEST_DECMONTH;

        case MCHT_CALENDARDATENEXT:
        case MCHT_CALENDARDATEPREV:
            return wxCAL_HITTEST_SURROUNDING_WEEK;
    }
}


bool wxCalendarCtrl::SetDate(const wxDateTime& dt)
{
    wxCHECK_MSG( dt.IsValid(), false, "invalid date" );

    SYSTEMTIME st;
    dt.GetAsMSWSysDate(&st);
    if ( !MonthCal_SetCurSel(GetHwnd(), &st) )
    {
        wxLogDebug(wxT("DateTime_SetSystemtime() failed"));

        return false;
    }

    m_date = dt.GetDateOnly();

    return true;
}

wxDateTime wxCalendarCtrl::GetDate() const
{
#if wxDEBUG_LEVEL
    SYSTEMTIME st;

    if ( !MonthCal_GetCurSel(GetHwnd(), &st) )
    {
        wxASSERT_MSG( !m_date.IsValid(), "mismatch between data and control" );

        return wxDefaultDateTime;
    }

    wxDateTime dt;
    dt.SetFromMSWSysDate(st);

                    wxASSERT_MSG( dt.IsSameDate(m_date), "mismatch between data and control" );
#endif 
    return m_date;
}

bool wxCalendarCtrl::SetDateRange(const wxDateTime& dt1, const wxDateTime& dt2)
{
    SYSTEMTIME st[2];

    DWORD flags = 0;
    if ( dt1.IsValid() )
    {
        dt1.GetAsMSWSysTime(st + 0);
        flags |= GDTR_MIN;
    }

    if ( dt2.IsValid() )
    {
        dt2.GetAsMSWSysTime(st + 1);
        flags |= GDTR_MAX;
    }

    if ( !MonthCal_SetRange(GetHwnd(), flags, st) )
    {
        wxLogDebug(wxT("MonthCal_SetRange() failed"));
    }

    return flags != 0;
}

bool wxCalendarCtrl::GetDateRange(wxDateTime *dt1, wxDateTime *dt2) const
{
    SYSTEMTIME st[2];

    DWORD flags = MonthCal_GetRange(GetHwnd(), st);
    if ( dt1 )
    {
        if ( flags & GDTR_MIN )
            dt1->SetFromMSWSysDate(st[0]);
        else
            *dt1 = wxDefaultDateTime;
    }

    if ( dt2 )
    {
        if ( flags & GDTR_MAX )
            dt2->SetFromMSWSysDate(st[1]);
        else
            *dt2 = wxDefaultDateTime;
    }

    return flags != 0;
}


bool wxCalendarCtrl::EnableMonthChange(bool enable)
{
    if ( !wxCalendarCtrlBase::EnableMonthChange(enable) )
        return false;

    wxDateTime dtStart, dtEnd;
    if ( !enable )
    {
        dtStart = GetDate();
        dtStart.SetDay(1);

        dtEnd = dtStart.GetLastMonthDay();
    }
    
    SetDateRange(dtStart, dtEnd);

    return true;
}

void wxCalendarCtrl::Mark(size_t day, bool mark)
{
    wxCHECK_RET( day > 0 && day < 32, "invalid day" );

    int mask = 1 << (day - 1);
    if ( mark )
        m_marks |= mask;
    else
        m_marks &= ~mask;

        UpdateMarks();
}

void wxCalendarCtrl::SetHoliday(size_t day)
{
    wxCHECK_RET( day > 0 && day < 32, "invalid day" );

    m_holidays |= 1 << (day - 1);
}

void wxCalendarCtrl::UpdateMarks()
{
                        MONTHDAYSTATE states[14] = { 0 };
    const DWORD nMonths = MonthCal_GetMonthRange(GetHwnd(), GMR_DAYSTATE, NULL);

                                            if ( nMonths >= 2 && nMonths <= WXSIZEOF(states) )
    {
                states[1] = m_marks | m_holidays;

        if ( !MonthCal_SetDayState(GetHwnd(), nMonths, states) )
        {
            wxLogLastError(wxT("MonthCal_SetDayState"));
        }
    }
    }

void wxCalendarCtrl::UpdateFirstDayOfWeek()
{
    MonthCal_SetFirstDayOfWeek(GetHwnd(),
                               HasFlag(wxCAL_MONDAY_FIRST) ? MonthCal_Monday
                                                           : MonthCal_Sunday);
}


bool wxCalendarCtrl::MSWOnNotify(int idCtrl, WXLPARAM lParam, WXLPARAM *result)
{
    NMHDR* hdr = (NMHDR *)lParam;
    switch ( hdr->code )
    {
        case MCN_SELCHANGE:
            {
                                                const wxDateTime dateOld = m_date;
                const NMSELCHANGE * const sch = (NMSELCHANGE *)lParam;
                m_date.SetFromMSWSysDate(sch->stSelStart);

                                                                if ( m_date != dateOld )
                {
                    if ( GenerateAllChangeEvents(dateOld) )
                    {
                                                                        SetHolidayAttrs();
                        UpdateMarks();
                    }
                }
            }
            break;

        case MCN_GETDAYSTATE:
            {
                const NMDAYSTATE * const ds = (NMDAYSTATE *)lParam;

                wxDateTime startDate;
                startDate.SetFromMSWSysDate(ds->stStart);

                                wxDateTime currentDate = m_date.IsValid() ? m_date : startDate;

                                                currentDate.SetDay(1);

                for ( int i = 0; i < ds->cDayState; i++ )
                {
                                        if ( startDate == currentDate )
                        ds->prgDayState[i] = m_marks | m_holidays;
                    else
                        ds->prgDayState[i] = 0;

                    startDate += wxDateSpan::Month();
                }
            }
            break;

        default:
            return wxCalendarCtrlBase::MSWOnNotify(idCtrl, lParam, result);
    }

    *result = 0;
    return true;
}

void wxCalendarCtrl::MSWOnDoubleClick(wxMouseEvent& event)
{
    if ( HitTest(event.GetPosition()) == wxCAL_HITTEST_DAY )
    {
        if ( GenerateEvent(wxEVT_CALENDAR_DOUBLECLICKED) )
            return;     }

    event.Skip();
}

void wxCalendarCtrl::MSWOnClick(wxMouseEvent& event)
{
            SetFocus();

    event.Skip();
}

#endif 