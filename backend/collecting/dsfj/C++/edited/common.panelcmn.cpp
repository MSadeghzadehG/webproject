


#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#ifndef WX_PRECOMP
    #include "wx/object.h"
    #include "wx/font.h"
    #include "wx/colour.h"
    #include "wx/settings.h"
    #include "wx/log.h"
    #include "wx/panel.h"
    #include "wx/containr.h"
#endif


wxDEFINE_FLAGS( wxPanelStyle )
wxBEGIN_FLAGS( wxPanelStyle )
            wxFLAGS_MEMBER(wxBORDER_SIMPLE)
    wxFLAGS_MEMBER(wxBORDER_SUNKEN)
    wxFLAGS_MEMBER(wxBORDER_DOUBLE)
    wxFLAGS_MEMBER(wxBORDER_RAISED)
    wxFLAGS_MEMBER(wxBORDER_STATIC)
    wxFLAGS_MEMBER(wxBORDER_NONE)

        wxFLAGS_MEMBER(wxSIMPLE_BORDER)
    wxFLAGS_MEMBER(wxSUNKEN_BORDER)
    wxFLAGS_MEMBER(wxDOUBLE_BORDER)
    wxFLAGS_MEMBER(wxRAISED_BORDER)
    wxFLAGS_MEMBER(wxSTATIC_BORDER)
    wxFLAGS_MEMBER(wxBORDER)

        wxFLAGS_MEMBER(wxTAB_TRAVERSAL)
    wxFLAGS_MEMBER(wxCLIP_CHILDREN)
    wxFLAGS_MEMBER(wxTRANSPARENT_WINDOW)
    wxFLAGS_MEMBER(wxWANTS_CHARS)
    wxFLAGS_MEMBER(wxFULL_REPAINT_ON_RESIZE)
    wxFLAGS_MEMBER(wxALWAYS_SHOW_SB)
    wxFLAGS_MEMBER(wxVSCROLL)
    wxFLAGS_MEMBER(wxHSCROLL)
wxEND_FLAGS( wxPanelStyle )

wxIMPLEMENT_DYNAMIC_CLASS_XTI(wxPanel, wxWindow, "wx/panel.h");

wxBEGIN_PROPERTIES_TABLE(wxPanel)
    wxPROPERTY_FLAGS( WindowStyle, wxPanelStyle, long, \
                      SetWindowStyleFlag, GetWindowStyleFlag, \
                      wxEMPTY_PARAMETER_VALUE, 0 , \
                      wxT("Helpstring"), wxT("group"))     wxEND_PROPERTIES_TABLE()

wxEMPTY_HANDLERS_TABLE(wxPanel)

wxCONSTRUCTOR_6( wxPanel, wxWindow*, Parent, wxWindowID, Id, \
                 wxPoint, Position, wxSize, Size, long, WindowStyle, \
                 wxString, Name)




bool wxPanelBase::Create(wxWindow *parent, wxWindowID id,
                         const wxPoint& pos,
                         const wxSize& size,
                         long style,
                         const wxString& name)
{
    if ( !wxWindow::Create(parent, id, pos, size, style, name) )
        return false;

        SetThemeEnabled(true);

    return true;
}

void wxPanelBase::InitDialog()
{
    wxInitDialogEvent event(GetId());
    event.SetEventObject(this);
    GetEventHandler()->ProcessEvent(event);
}
