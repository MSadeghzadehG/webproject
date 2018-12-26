


#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#if wxUSE_STATLINE

#include "wx/statline.h"

#ifndef WX_PRECOMP
    #include "wx/statbox.h"
#endif

#include "wx/osx/private.h"



bool wxStaticLine::Create( wxWindow *parent,
                           wxWindowID id,
                           const wxPoint &pos,
                           const wxSize &size,
                           long style,
                           const wxString &name)
{    
    DontCreatePeer();
    
    if ( !wxStaticLineBase::Create(parent, id, pos, size,
                                   style, wxDefaultValidator, name) )
        return false;

    SetPeer(wxWidgetImpl::CreateStaticLine( this, parent, id, pos, size, style, GetExtraStyle() ));

    MacPostControlCreate(pos,size) ;

    return true;
}

#endif 