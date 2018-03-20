


#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#if wxUSE_HYPERLINKCTRL


#include "wx/hyperlink.h"

#ifndef WX_PRECOMP
    #include "wx/menu.h"
    #include "wx/log.h"
    #include "wx/dataobj.h"
#endif


wxDEFINE_FLAGS( wxHyperlinkStyle )
wxBEGIN_FLAGS( wxHyperlinkStyle )
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

wxFLAGS_MEMBER(wxHL_CONTEXTMENU)
wxFLAGS_MEMBER(wxHL_ALIGN_LEFT)
wxFLAGS_MEMBER(wxHL_ALIGN_RIGHT)
wxFLAGS_MEMBER(wxHL_ALIGN_CENTRE)
wxEND_FLAGS( wxHyperlinkStyle )

wxIMPLEMENT_DYNAMIC_CLASS_XTI( wxHyperlinkCtrl, wxControl, "wx/hyperlink.h");

wxIMPLEMENT_DYNAMIC_CLASS(wxHyperlinkEvent, wxCommandEvent);
wxDEFINE_EVENT( wxEVT_HYPERLINK, wxHyperlinkEvent );

wxBEGIN_PROPERTIES_TABLE(wxHyperlinkCtrl)
wxPROPERTY( Label, wxString, SetLabel, GetLabel, wxString(), \
           0 , wxT("The link label"), wxT("group") )

wxPROPERTY( URL, wxString, SetURL, GetURL, wxString(), \
           0 , wxT("The link URL"), wxT("group") )
wxPROPERTY_FLAGS( WindowStyle, wxHyperlinkStyle, long, SetWindowStyleFlag, \
                 GetWindowStyleFlag, wxEMPTY_PARAMETER_VALUE, 0 ,     \
                 wxT("The link style"), wxT("group")) wxEND_PROPERTIES_TABLE()

wxEMPTY_HANDLERS_TABLE(wxHyperlinkCtrl)

wxCONSTRUCTOR_7( wxHyperlinkCtrl, wxWindow*, Parent, wxWindowID, Id, wxString, \
                Label, wxString, URL, wxPoint, Position, wxSize, Size, long, WindowStyle )


const char wxHyperlinkCtrlNameStr[] = "hyperlink";


void
wxHyperlinkCtrlBase::CheckParams(const wxString& label,
                                 const wxString& url,
                                 long style)
{
#if wxDEBUG_LEVEL
    wxASSERT_MSG(!url.empty() || !label.empty(),
                 wxT("Both URL and label are empty ?"));

    int alignment = (int)((style & wxHL_ALIGN_LEFT) != 0) +
                    (int)((style & wxHL_ALIGN_CENTRE) != 0) +
                    (int)((style & wxHL_ALIGN_RIGHT) != 0);
    wxASSERT_MSG(alignment == 1,
        wxT("Specify exactly one align flag!"));
#else     wxUnusedVar(label);
    wxUnusedVar(url);
    wxUnusedVar(style);
#endif }

void wxHyperlinkCtrlBase::SendEvent()
{
    wxString url = GetURL();
    wxHyperlinkEvent linkEvent(this, GetId(), url);
    if (!GetEventHandler()->ProcessEvent(linkEvent))         {
        if (!wxLaunchDefaultBrowser(url))
        {
            wxLogWarning(wxT("Could not launch the default browser with url '%s' !"), url.c_str());
        }
    }
}

#endif 