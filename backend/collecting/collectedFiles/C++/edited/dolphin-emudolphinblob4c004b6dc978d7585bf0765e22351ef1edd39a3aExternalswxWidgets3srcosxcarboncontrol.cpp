
#include "wx/wxprec.h"

#include "wx/control.h"

#ifndef WX_PRECOMP
    #include "wx/app.h"
    #include "wx/panel.h"
    #include "wx/dc.h"
    #include "wx/dcclient.h"
    #include "wx/button.h"
    #include "wx/dialog.h"
    #include "wx/scrolbar.h"
    #include "wx/stattext.h"
    #include "wx/statbox.h"
    #include "wx/radiobox.h"
    #include "wx/sizer.h"
#endif 
#include "wx/osx/private.h"

wxIMPLEMENT_ABSTRACT_CLASS(wxControl, wxWindow);


wxControl::wxControl()
{
}

bool wxControl::Create( wxWindow *parent,
       wxWindowID id,
       const wxPoint& pos,
       const wxSize& size,
       long style,
       const wxValidator& validator,
       const wxString& name )
{
    bool rval = wxWindow::Create( parent, id, pos, size, style, name );

#if 0
        if ( parent )
    {
        m_backgroundColour = parent->GetBackgroundColour();
        m_foregroundColour = parent->GetForegroundColour();
    }
#endif

#if wxUSE_VALIDATORS
    if (rval)
        SetValidator( validator );
#endif

    return rval;
}

bool wxControl::ProcessCommand( wxCommandEvent &event )
{
                return HandleWindowEvent( event );
}

void  wxControl::OnKeyDown( wxKeyEvent &WXUNUSED(event) )
{
    if ( GetPeer() == NULL || !GetPeer()->IsOk() )
        return;

    }
