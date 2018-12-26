


#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#if wxUSE_BUTTON

#ifndef WX_PRECOMP
    #include "wx/button.h"
    #include "wx/toplevel.h"
#endif 
extern WXDLLEXPORT_DATA(const char) wxButtonNameStr[] = "button";


wxDEFINE_FLAGS( wxButtonStyle )
wxBEGIN_FLAGS( wxButtonStyle )
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

wxFLAGS_MEMBER(wxBU_LEFT)
wxFLAGS_MEMBER(wxBU_RIGHT)
wxFLAGS_MEMBER(wxBU_TOP)
wxFLAGS_MEMBER(wxBU_BOTTOM)
wxFLAGS_MEMBER(wxBU_EXACTFIT)
wxEND_FLAGS( wxButtonStyle )

wxIMPLEMENT_DYNAMIC_CLASS_XTI(wxButton, wxControl, "wx/button.h");

wxBEGIN_PROPERTIES_TABLE(wxButton)
wxEVENT_PROPERTY( Click, wxEVT_BUTTON, wxCommandEvent )

wxPROPERTY( Font, wxFont, SetFont, GetFont, wxEMPTY_PARAMETER_VALUE, \
           0 , wxT("The font associated with the button label"), wxT("group"))
wxPROPERTY( Label, wxString, SetLabel, GetLabel, wxString(), \
           0 , wxT("The button label"), wxT("group") )

wxPROPERTY_FLAGS( WindowStyle, wxButtonStyle, long, SetWindowStyleFlag, \
                 GetWindowStyleFlag, wxEMPTY_PARAMETER_VALUE, 0 ,     \
                 wxT("The button style"), wxT("group")) wxEND_PROPERTIES_TABLE()

wxEMPTY_HANDLERS_TABLE(wxButton)

wxCONSTRUCTOR_6( wxButton, wxWindow*, Parent, wxWindowID, Id, wxString, \
                Label, wxPoint, Position, wxSize, Size, long, WindowStyle )



wxWindow *wxButtonBase::SetDefault()
{
    wxTopLevelWindow * const
        tlw = wxDynamicCast(wxGetTopLevelParent(this), wxTopLevelWindow);

    wxCHECK_MSG( tlw, NULL, wxT("button without top level window?") );

    return tlw->SetDefaultItem(this);
}

void wxAnyButtonBase::SetBitmapPosition(wxDirection dir)
{
    wxASSERT_MSG( !(dir & ~wxDIRECTION_MASK), "non-direction flag used" );
    wxASSERT_MSG( !!(dir & wxLEFT) +
                    !!(dir & wxRIGHT) +
                      !!(dir & wxTOP) +
                       !!(dir & wxBOTTOM) == 1,
                   "exactly one direction flag must be set" );

    DoSetBitmapPosition(dir);

}
#endif 