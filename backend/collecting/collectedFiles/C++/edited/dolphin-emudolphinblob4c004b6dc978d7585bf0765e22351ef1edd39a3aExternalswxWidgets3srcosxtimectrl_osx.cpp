


#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#if wxUSE_TIMEPICKCTRL && wxOSX_USE_COCOA

#include "wx/timectrl.h"
#include "wx/dateevt.h"

#include "wx/osx/core/private/datetimectrl.h"


wxIMPLEMENT_DYNAMIC_CLASS(wxTimePickerCtrl, wxControl);

bool
wxTimePickerCtrl::Create(wxWindow *parent,
                         wxWindowID id,
                         const wxDateTime& dt,
                         const wxPoint& pos,
                         const wxSize& size,
                         long style,
                         const wxValidator& validator,
                         const wxString& name)
{
    DontCreatePeer();

    if ( !wxTimePickerCtrlBase::Create(parent, id, pos, size,
                                       style, validator, name) )
        return false;

    wxOSXWidgetImpl* const peer = wxDateTimeWidgetImpl::CreateDateTimePicker
                                  (
                                    this,
                                    dt,
                                    pos,
                                    size,
                                    style,
                                    wxDateTimeWidget_HourMinuteSecond
                                  );
    if ( !peer )
        return false;

    SetPeer(peer);

    MacPostControlCreate(pos, size);

    return true;
}

void wxTimePickerCtrl::OSXGenerateEvent(const wxDateTime& dt)
{
    wxDateEvent event(this, dt, wxEVT_TIME_CHANGED);
    HandleWindowEvent(event);
}

#endif 