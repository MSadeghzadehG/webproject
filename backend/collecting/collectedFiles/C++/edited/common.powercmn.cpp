


#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#ifndef WX_PRECOMP
#endif 
#include "wx/power.h"


#ifdef wxHAS_POWER_EVENTS
    wxDEFINE_EVENT( wxEVT_POWER_SUSPENDING, wxPowerEvent );
    wxDEFINE_EVENT( wxEVT_POWER_SUSPENDED, wxPowerEvent );
    wxDEFINE_EVENT( wxEVT_POWER_SUSPEND_CANCEL, wxPowerEvent );
    wxDEFINE_EVENT( wxEVT_POWER_RESUME, wxPowerEvent );

    wxIMPLEMENT_DYNAMIC_CLASS(wxPowerEvent, wxEvent);
#endif

#if !defined(__WINDOWS__) && !defined(__APPLE__)

bool
wxPowerResource::Acquire(wxPowerResourceKind WXUNUSED(kind),
                         const wxString& WXUNUSED(reason))
{
    return false;
}

void wxPowerResource::Release(wxPowerResourceKind WXUNUSED(kind))
{
}

#endif 
#if !defined(__WINDOWS__)

wxPowerType wxGetPowerType()
{
    return wxPOWER_UNKNOWN;
}

wxBatteryState wxGetBatteryState()
{
    return wxBATTERY_UNKNOWN_STATE;
}

#endif 
