
#include "wx/wxprec.h"

#if wxUSE_STATTEXT

#include "wx/stattext.h"

#ifndef WX_PRECOMP
    #include "wx/app.h"
    #include "wx/utils.h"
    #include "wx/dc.h"
    #include "wx/dcclient.h"
    #include "wx/settings.h"
#endif 
#include "wx/osx/private.h"

#include <stdio.h>


bool wxStaticText::Create( wxWindow *parent,
    wxWindowID id,
    const wxString& label,
    const wxPoint& pos,
    const wxSize& size,
    long style,
    const wxString& name )
{    
    DontCreatePeer();
    
    if ( !wxControl::Create( parent, id, pos, size, style, wxDefaultValidator, name ) )
        return false;

    SetPeer(wxWidgetImpl::CreateStaticText( this, parent, id, label, pos, size, style, GetExtraStyle() ));

    MacPostControlCreate( pos, size );

    SetLabel(label);
    if ( HasFlag(wxST_NO_AUTORESIZE) )
    {
                                SetInitialSize(size);
    }

    return true;
}

void wxStaticText::SetLabel(const wxString& label)
{
    m_labelOrig = label;

        if ( HasFlag(wxST_ELLIPSIZE_END) || HasFlag(wxST_ELLIPSIZE_MIDDLE)
#if wxOSX_USE_COCOA          || HasFlag(wxST_ELLIPSIZE_START)
#endif
    )
    {
                DoSetLabel(GetLabel());
    }
    else     {
        DoSetLabel(GetEllipsizedLabel());
    }

    InvalidateBestSize();

    if ( !(GetWindowStyle() & wxST_NO_AUTORESIZE) &&
         !IsEllipsized() )      {
        SetSize( GetBestSize() );
    }

    Refresh();

        }

bool wxStaticText::SetFont(const wxFont& font)
{
    bool ret = wxControl::SetFont( font );

    if ( ret )
    {
        if ( !(GetWindowStyle() & wxST_NO_AUTORESIZE) )
        {
            InvalidateBestSize();
            SetSize( GetBestSize() );
        }
    }

    return ret;
}

void wxStaticText::DoSetLabel(const wxString& label)
{
    m_label = RemoveMnemonics(label);
    GetPeer()->SetLabel(m_label , GetFont().GetEncoding() );
}

#if wxUSE_MARKUP && wxOSX_USE_COCOA

bool wxStaticText::DoSetLabelMarkup(const wxString& markup)
{
    if ( !wxStaticTextBase::DoSetLabelMarkup(markup) )
        return false;

    GetPeer()->SetLabelMarkup(markup);

    return true;
}

#endif 
wxString wxStaticText::DoGetLabel() const
{
    return m_label;
}



#endif 