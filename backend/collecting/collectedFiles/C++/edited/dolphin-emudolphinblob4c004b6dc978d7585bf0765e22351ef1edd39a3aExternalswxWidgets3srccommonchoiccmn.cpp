


#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#if wxUSE_CHOICE

#include "wx/choice.h"

#include "wx/private/textmeasure.h"

#ifndef WX_PRECOMP
#endif

const char wxChoiceNameStr[] = "choice";


wxDEFINE_FLAGS( wxChoiceStyle )
wxBEGIN_FLAGS( wxChoiceStyle )
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

wxEND_FLAGS( wxChoiceStyle )

wxIMPLEMENT_DYNAMIC_CLASS_XTI(wxChoice, wxControl, "wx/choice.h");

wxBEGIN_PROPERTIES_TABLE(wxChoice)
wxEVENT_PROPERTY( Select, wxEVT_CHOICE, wxCommandEvent )

wxPROPERTY( Font, wxFont, SetFont, GetFont , wxEMPTY_PARAMETER_VALUE, \
           0 , wxT("Helpstring"), wxT("group"))
wxPROPERTY_COLLECTION( Choices, wxArrayString, wxString, AppendString, \
                      GetStrings, 0 , wxT("Helpstring"), wxT("group"))
wxPROPERTY( Selection,int, SetSelection, GetSelection, wxEMPTY_PARAMETER_VALUE, \
           0 , wxT("Helpstring"), wxT("group"))



wxPROPERTY_FLAGS( WindowStyle, wxChoiceStyle, long, SetWindowStyleFlag, \
                 GetWindowStyleFlag, wxEMPTY_PARAMETER_VALUE, 0 , \
                 wxT("Helpstring"), wxT("group")) wxEND_PROPERTIES_TABLE()

wxEMPTY_HANDLERS_TABLE(wxChoice)

wxCONSTRUCTOR_4( wxChoice, wxWindow*, Parent, wxWindowID, Id, \
                wxPoint, Position, wxSize, Size )


wxChoiceBase::~wxChoiceBase()
{
    }

wxSize wxChoiceBase::DoGetBestSize() const
{
        wxSize best(80, -1);

    const unsigned int nItems = GetCount();
    if ( nItems > 0 )
    {
        wxTextMeasure txm(this);
        best.x = txm.GetLargestStringExtent(GetStrings()).x;
    }

    return best;
}


void wxChoiceBase::Command(wxCommandEvent& event)
{
    SetSelection(event.GetInt());
    (void)GetEventHandler()->ProcessEvent(event);
}

#endif 