


#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#if wxUSE_CLIPBOARD

#include "wx/clipbrd.h"

#ifndef WX_PRECOMP
    #include "wx/dataobj.h"
    #include "wx/module.h"
#endif


wxIMPLEMENT_DYNAMIC_CLASS(wxClipboardEvent,wxEvent);

wxDEFINE_EVENT( wxEVT_CLIPBOARD_CHANGED, wxClipboardEvent );

bool wxClipboardEvent::SupportsFormat( const wxDataFormat &format ) const
{
#ifdef __WXGTK20__
    for (wxVector<wxDataFormat>::size_type n = 0; n < m_formats.size(); n++)
    {
        if (m_formats[n] == format)
            return true;
    }

    return false;
#else
            wxClipboard* clipboard = (wxClipboard*) GetEventObject();
    return clipboard->IsSupported( format );
#endif
}

void wxClipboardEvent::AddFormat(const wxDataFormat& format)
{
    m_formats.push_back( format );
}


static wxClipboard *gs_clipboard = NULL;

 wxClipboard *wxClipboardBase::Get()
{
    if ( !gs_clipboard )
    {
        gs_clipboard = new wxClipboard;
    }
    return gs_clipboard;
}

bool wxClipboardBase::IsSupportedAsync( wxEvtHandler *sink )
{
            wxClipboardEvent *event = new wxClipboardEvent(wxEVT_CLIPBOARD_CHANGED);
    event->SetEventObject( this );

    sink->QueueEvent( event );

    return true;
}



class wxClipboardModule : public wxModule
{
public:
    bool OnInit() wxOVERRIDE { return true; }
    void OnExit() wxOVERRIDE { wxDELETE(gs_clipboard); }

private:
    wxDECLARE_DYNAMIC_CLASS(wxClipboardModule);
};

wxIMPLEMENT_DYNAMIC_CLASS(wxClipboardModule, wxModule);

#endif 