


#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#if wxUSE_RADIOBTN

#include "wx/radiobut.h"

#ifndef WX_PRECOMP
    #include "wx/settings.h"
    #include "wx/dcscreen.h"
#endif

extern WXDLLEXPORT_DATA(const char) wxRadioButtonNameStr[] = "radioButton";
extern WXDLLEXPORT_DATA(const char) wxBitmapRadioButtonNameStr[] = "radioButton";


wxDEFINE_FLAGS( wxRadioButtonStyle )
wxBEGIN_FLAGS( wxRadioButtonStyle )
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
    wxFLAGS_MEMBER(wxALWAYS_SHOW_SB )
    wxFLAGS_MEMBER(wxVSCROLL)
    wxFLAGS_MEMBER(wxHSCROLL)

    wxFLAGS_MEMBER(wxRB_GROUP)
wxEND_FLAGS( wxRadioButtonStyle )

wxIMPLEMENT_DYNAMIC_CLASS_XTI(wxRadioButton, wxControl, "wx/radiobut.h");

wxBEGIN_PROPERTIES_TABLE(wxRadioButton)
    wxEVENT_PROPERTY( Click, wxEVT_RADIOBUTTON, wxCommandEvent )
    wxPROPERTY( Font, wxFont, SetFont, GetFont , wxEMPTY_PARAMETER_VALUE, 0 , \
                wxT("Helpstring"), wxT("group"))
    wxPROPERTY( Label,wxString, SetLabel, GetLabel, wxString(), 0 , \
                wxT("Helpstring"), wxT("group") )
    wxPROPERTY( Value,bool, SetValue, GetValue, wxEMPTY_PARAMETER_VALUE, 0 , \
                wxT("Helpstring"), wxT("group") )
    wxPROPERTY_FLAGS( WindowStyle, wxRadioButtonStyle, long, SetWindowStyleFlag, \
                      GetWindowStyleFlag, wxEMPTY_PARAMETER_VALUE, 0 , \
                      wxT("Helpstring"), wxT("group")) wxEND_PROPERTIES_TABLE()

wxEMPTY_HANDLERS_TABLE(wxRadioButton)

wxCONSTRUCTOR_6( wxRadioButton, wxWindow*, Parent, wxWindowID, Id, \
                 wxString, Label, wxPoint, Position, wxSize, Size, long, WindowStyle )


#endif 