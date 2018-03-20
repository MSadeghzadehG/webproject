


#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#include "wx/statline.h"

#if wxUSE_STATLINE

#ifndef WX_PRECOMP
    #include "wx/msw/private.h"
    #include "wx/msw/missing.h"
#endif



bool wxStaticLine::Create(wxWindow *parent,
                          wxWindowID id,
                          const wxPoint& pos,
                          const wxSize& sizeOrig,
                          long style,
                          const wxString &name)
{
    wxSize size = AdjustSize(sizeOrig);

    if ( !CreateControl(parent, id, pos, size, style, wxDefaultValidator, name) )
        return false;

    return MSWCreateControl(wxT("STATIC"), wxEmptyString, pos, size);
}

WXDWORD wxStaticLine::MSWGetStyle(long style, WXDWORD *exstyle) const
{
        style &= ~wxBORDER_MASK;
    style |= wxBORDER_NONE;

    WXDWORD msStyle = wxControl::MSWGetStyle(style, exstyle);

        msStyle |= SS_SUNKEN | SS_NOTIFY | WS_CLIPSIBLINGS;
    msStyle |= SS_GRAYRECT ;

    return msStyle ;
}

#endif 