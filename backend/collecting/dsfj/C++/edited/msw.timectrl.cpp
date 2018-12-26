


#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#if wxUSE_DATEPICKCTRL

#ifndef WX_PRECOMP
    #include "wx/msw/wrapcctl.h"
#endif

#include "wx/timectrl.h"
#include "wx/dateevt.h"

wxIMPLEMENT_DYNAMIC_CLASS(wxTimePickerCtrl, wxControl);


WXDWORD wxTimePickerCtrl::MSWGetStyle(long style, WXDWORD *exstyle) const
{
    WXDWORD styleMSW = wxTimePickerCtrlBase::MSWGetStyle(style, exstyle);

    styleMSW |= DTS_TIMEFORMAT;

    return styleMSW;
}

#if wxUSE_INTL

wxLocaleInfo wxTimePickerCtrl::MSWGetFormat() const
{
    return wxLOCALE_TIME_FMT;
}

#endif 
bool wxTimePickerCtrl::MSWOnDateTimeChange(const NMDATETIMECHANGE& dtch)
{
    m_date.SetFromMSWSysTime(dtch.st);

    wxDateEvent event(this, m_date, wxEVT_TIME_CHANGED);
    return HandleWindowEvent(event);
}
#endif 