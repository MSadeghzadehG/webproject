


#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#if wxUSE_STATBMP

#include "wx/statbmp.h"

extern WXDLLEXPORT_DATA(const char) wxStaticBitmapNameStr[] = "staticBitmap";


wxDEFINE_FLAGS( wxStaticBitmapStyle )
wxBEGIN_FLAGS( wxStaticBitmapStyle )
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

wxEND_FLAGS( wxStaticBitmapStyle )

wxIMPLEMENT_DYNAMIC_CLASS_XTI(wxStaticBitmap, wxControl, "wx/statbmp.h");

wxBEGIN_PROPERTIES_TABLE(wxStaticBitmap)
    wxPROPERTY_FLAGS( WindowStyle, wxStaticBitmapStyle, long, \
                      SetWindowStyleFlag, GetWindowStyleFlag, \
                      wxEMPTY_PARAMETER_VALUE, 0 , wxT("Helpstring"), \
                      wxT("group")) wxEND_PROPERTIES_TABLE()

wxEMPTY_HANDLERS_TABLE(wxStaticBitmap)

wxCONSTRUCTOR_5( wxStaticBitmap, wxWindow*, Parent, wxWindowID, Id, \
                 wxBitmap, Bitmap, wxPoint, Position, wxSize, Size )



#endif 