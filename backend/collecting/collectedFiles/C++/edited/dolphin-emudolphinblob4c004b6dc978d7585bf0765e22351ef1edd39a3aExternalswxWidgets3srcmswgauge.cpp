


#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#if wxUSE_GAUGE

#include "wx/gauge.h"

#ifndef WX_PRECOMP
    #include "wx/app.h"

    #include "wx/msw/wrapcctl.h" #endif

#include "wx/appprogress.h"
#include "wx/msw/private.h"


#ifndef PBS_SMOOTH
    #define PBS_SMOOTH 0x01
#endif

#ifndef PBS_VERTICAL
    #define PBS_VERTICAL 0x04
#endif

#ifndef PBM_SETBARCOLOR
    #define PBM_SETBARCOLOR         (WM_USER+9)
#endif

#ifndef PBM_SETBKCOLOR
    #define PBM_SETBKCOLOR          0x2001
#endif

#ifndef PBS_MARQUEE
    #define PBS_MARQUEE             0x08
#endif

#ifndef PBM_SETMARQUEE
    #define PBM_SETMARQUEE          (WM_USER+10)
#endif




bool wxGauge::Create(wxWindow *parent,
                     wxWindowID id,
                     int range,
                     const wxPoint& pos,
                     const wxSize& size,
                     long style,
                     const wxValidator& validator,
                     const wxString& name)
{
    if ( !CreateControl(parent, id, pos, size, style, validator, name) )
        return false;

    if ( !MSWCreateControl(PROGRESS_CLASS, wxEmptyString, pos, size) )
        return false;

        m_nDirection = wxRIGHT;

    SetRange(range);

    InitProgressIndicatorIfNeeded();

    return true;
}

wxGauge::~wxGauge()
{
}

WXDWORD wxGauge::MSWGetStyle(long style, WXDWORD *exstyle) const
{
    WXDWORD msStyle = wxControl::MSWGetStyle(style, exstyle);

    if ( style & wxGA_VERTICAL )
        msStyle |= PBS_VERTICAL;

    if ( style & wxGA_SMOOTH )
        msStyle |= PBS_SMOOTH;

    return msStyle;
}


wxSize wxGauge::DoGetBestSize() const
{
            
    if (HasFlag(wxGA_VERTICAL))
        return ConvertDialogToPixels(wxSize(8, 107));
    else
        return ConvertDialogToPixels(wxSize(107, 8));
}


void wxGauge::SetRange(int r)
{
        if ( IsInIndeterminateMode() )
        SetDeterminateMode();

    wxGaugeBase::SetRange(r);

#ifdef PBM_SETRANGE32
    ::SendMessage(GetHwnd(), PBM_SETRANGE32, 0, r);
#else         ::SendMessage(GetHwnd(), PBM_SETRANGE, 0, MAKELPARAM(0, r));
#endif }

void wxGauge::SetValue(int pos)
{
        if ( IsInIndeterminateMode() )
        SetDeterminateMode();

    if ( GetValue() != pos )
    {
        wxGaugeBase::SetValue(pos);

        ::SendMessage(GetHwnd(), PBM_SETPOS, pos, 0);
    }
}

bool wxGauge::SetForegroundColour(const wxColour& col)
{
    if ( !wxControl::SetForegroundColour(col) )
        return false;

    ::SendMessage(GetHwnd(), PBM_SETBARCOLOR, 0, (LPARAM)wxColourToRGB(col));

    return true;
}

bool wxGauge::SetBackgroundColour(const wxColour& col)
{
    if ( !wxControl::SetBackgroundColour(col) )
        return false;

    ::SendMessage(GetHwnd(), PBM_SETBKCOLOR, 0, (LPARAM)wxColourToRGB(col));

    return true;
}

bool wxGauge::IsInIndeterminateMode() const
{
    return (::GetWindowLong(GetHwnd(), GWL_STYLE) & PBS_MARQUEE) != 0;
}

void wxGauge::SetIndeterminateMode()
{
        if ( !IsInIndeterminateMode() )
    {
        const long style = ::GetWindowLong(GetHwnd(), GWL_STYLE);
        ::SetWindowLong(GetHwnd(), GWL_STYLE, style | PBS_MARQUEE);
        ::SendMessage(GetHwnd(), PBM_SETMARQUEE, TRUE, 0);
    }
}

void wxGauge::SetDeterminateMode()
{
    if ( IsInIndeterminateMode() )
    {
        const long style = ::GetWindowLong(GetHwnd(), GWL_STYLE);
        ::SendMessage(GetHwnd(), PBM_SETMARQUEE, FALSE, 0);
        ::SetWindowLong(GetHwnd(), GWL_STYLE, style & ~PBS_MARQUEE);
    }
}

void wxGauge::Pulse()
{
    if (wxApp::GetComCtl32Version() >= 600)
    {
                SetIndeterminateMode();

        SendMessage(GetHwnd(), PBM_STEPIT, 0, 0);

        if ( m_appProgressIndicator )
            m_appProgressIndicator->Pulse();
    }
    else
    {
                wxGaugeBase::Pulse();
    }
}

#endif 