


#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#include "wx/datetimectrl.h"

#ifdef wxNEEDS_DATETIMEPICKCTRL

#ifndef WX_PRECOMP
    #include "wx/msw/wrapwin.h"
    #include "wx/msw/wrapcctl.h"     #include "wx/msw/private.h"
    #include "wx/dcclient.h"
#endif 
#include "wx/msw/private/datecontrols.h"

#ifndef DateTime_GetSystemtime
    #define DateTime_GetSystemtime DateTime_GetSystemTime
#endif

#ifndef DateTime_SetSystemtime
    #define DateTime_SetSystemtime DateTime_SetSystemTime
#endif

#ifndef DTM_GETIDEALSIZE
    #define DTM_GETIDEALSIZE 0x100f
#endif


bool
wxDateTimePickerCtrl::MSWCreateDateTimePicker(wxWindow *parent,
                                              wxWindowID id,
                                              const wxDateTime& dt,
                                              const wxPoint& pos,
                                              const wxSize& size,
                                              long style,
                                              const wxValidator& validator,
                                              const wxString& name)
{
    if ( !wxMSWDateControls::CheckInitialization() )
        return false;

        if ( !CreateControl(parent, id, pos, size, style, validator, name) )
        return false;

        if ( !MSWCreateControl(DATETIMEPICK_CLASS, wxString(), pos, size) )
        return false;

    if ( dt.IsValid() || MSWAllowsNone() )
        SetValue(dt);
    else
        SetValue(wxDateTime::Now());

    return true;
}

void wxDateTimePickerCtrl::SetValue(const wxDateTime& dt)
{
    wxCHECK_RET( dt.IsValid() || MSWAllowsNone(),
                    wxT("this control requires a valid date") );

    SYSTEMTIME st;
    if ( dt.IsValid() )
        dt.GetAsMSWSysTime(&st);

    if ( !DateTime_SetSystemtime(GetHwnd(),
                                 dt.IsValid() ? GDT_VALID : GDT_NONE,
                                 &st) )
    {
                        wxFAIL_MSG( wxT("Setting the calendar date unexpectedly failed.") );

                return;
    }

    m_date = dt;
}

wxDateTime wxDateTimePickerCtrl::GetValue() const
{
    return m_date;
}

wxSize wxDateTimePickerCtrl::DoGetBestSize() const
{
        wxSize size;
    if ( wxGetWinVersion() >= wxWinVersion_Vista )
    {
        SIZE idealSize;
        ::SendMessage(m_hWnd, DTM_GETIDEALSIZE, 0, (LPARAM)&idealSize);

        size = wxSize(idealSize.cx, idealSize.cy);
    }
    else     {
        wxClientDC dc(const_cast<wxDateTimePickerCtrl *>(this));

        #if wxUSE_INTL
        wxString s = wxDateTime::Now().Format(wxLocale::GetOSInfo(MSWGetFormat()));
#else         wxString s("XXX-YYY-ZZZZ");
#endif 
                                        s += wxS("W");

        size = dc.GetTextExtent(s);

                size.x += wxSystemSettings::GetMetric(wxSYS_HSCROLL_ARROW_X);
    }

                if ( MSWAllowsNone() )
        size.x += 3*GetCharWidth();

        size.y = EDIT_HEIGHT_FROM_CHAR_HEIGHT(size.y);

    return size;
}

bool
wxDateTimePickerCtrl::MSWOnNotify(int idCtrl, WXLPARAM lParam, WXLPARAM *result)
{
    NMHDR* hdr = (NMHDR *)lParam;
    switch ( hdr->code )
    {
        case DTN_DATETIMECHANGE:
            if ( MSWOnDateTimeChange(*(NMDATETIMECHANGE*)(hdr)) )
            {
                *result = 0;
                return true;
            }
            break;
    }

    return wxDateTimePickerCtrlBase::MSWOnNotify(idCtrl, lParam, result);
}

#endif 