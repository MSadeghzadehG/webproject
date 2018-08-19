


#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#ifndef WX_PRECOMP
    #include "wx/app.h"
#endif

#if wxUSE_SPINBTN

#include "wx/spinbutt.h"


wxDEFINE_FLAGS( wxSpinButtonStyle )
wxBEGIN_FLAGS( wxSpinButtonStyle )
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

    wxFLAGS_MEMBER(wxSP_HORIZONTAL)
    wxFLAGS_MEMBER(wxSP_VERTICAL)
    wxFLAGS_MEMBER(wxSP_ARROW_KEYS)
    wxFLAGS_MEMBER(wxSP_WRAP)
wxEND_FLAGS( wxSpinButtonStyle )

wxIMPLEMENT_DYNAMIC_CLASS_XTI(wxSpinButton, wxControl, "wx/spinbutt.h");

wxBEGIN_PROPERTIES_TABLE(wxSpinButton)
    wxEVENT_RANGE_PROPERTY( Spin, wxEVT_SCROLL_TOP, wxEVT_SCROLL_CHANGED, wxSpinEvent )

    wxPROPERTY( Value, int, SetValue, GetValue, 0, 0 , \
                wxT("Helpstring"), wxT("group"))
    wxPROPERTY( Min, int, SetMin, GetMin, 0, 0 , \
                wxT("Helpstring"), wxT("group"))
    wxPROPERTY( Max, int, SetMax, GetMax, 0, 0 , \
                wxT("Helpstring"), wxT("group"))

    wxPROPERTY_FLAGS( WindowStyle, wxSpinButtonStyle, long, SetWindowStyleFlag, \
                      GetWindowStyleFlag, wxEMPTY_PARAMETER_VALUE, 0 , \
                      wxT("Helpstring"), wxT("group")) wxEND_PROPERTIES_TABLE()

wxEMPTY_HANDLERS_TABLE(wxSpinButton)

wxCONSTRUCTOR_5( wxSpinButton, wxWindow*, Parent, wxWindowID, Id, \
                 wxPoint, Position, wxSize, Size, long, WindowStyle )

wxIMPLEMENT_DYNAMIC_CLASS(wxSpinEvent, wxNotifyEvent);


#endif 