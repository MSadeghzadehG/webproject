
#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#if wxUSE_BMPBUTTON

#include "wx/bmpbuttn.h"

#ifndef WX_PRECOMP
    #include "wx/log.h"
    #include "wx/dcmemory.h"
    #include "wx/image.h"
#endif

#include "wx/artprov.h"
#include "wx/renderer.h"


wxDEFINE_FLAGS( wxBitmapButtonStyle )
wxBEGIN_FLAGS( wxBitmapButtonStyle )
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

    wxFLAGS_MEMBER(wxBU_AUTODRAW)
    wxFLAGS_MEMBER(wxBU_LEFT)
    wxFLAGS_MEMBER(wxBU_RIGHT)
    wxFLAGS_MEMBER(wxBU_TOP)
    wxFLAGS_MEMBER(wxBU_BOTTOM)
wxEND_FLAGS( wxBitmapButtonStyle )

wxIMPLEMENT_DYNAMIC_CLASS_XTI(wxBitmapButton, wxButton, "wx/bmpbuttn.h");

wxBEGIN_PROPERTIES_TABLE(wxBitmapButton)
    wxPROPERTY_FLAGS( WindowStyle, wxBitmapButtonStyle, long, \
                      SetWindowStyleFlag, GetWindowStyleFlag, \
                      wxEMPTY_PARAMETER_VALUE, 0 , wxT("Helpstring"), \
                      wxT("group")) wxEND_PROPERTIES_TABLE()

wxEMPTY_HANDLERS_TABLE(wxBitmapButton)

wxCONSTRUCTOR_5( wxBitmapButton, wxWindow*, Parent, wxWindowID, Id, \
                 wxBitmap, Bitmap, wxPoint, Position, wxSize, Size )



namespace
{

#ifdef wxHAS_DRAW_TITLE_BAR_BITMAP

wxBitmap
GetCloseButtonBitmap(wxWindow *win,
                     const wxSize& size,
                     const wxColour& colBg,
                     int flags = 0)
{
    wxBitmap bmp;
    bmp.CreateScaled(size.x, size.y, wxBITMAP_SCREEN_DEPTH, win->GetContentScaleFactor());
    wxMemoryDC dc(bmp);
    dc.SetBackground(colBg);
    dc.Clear();
    wxRendererNative::Get().
        DrawTitleBarBitmap(win, dc, win->FromDIP(size), wxTITLEBAR_BUTTON_CLOSE, flags);
    return bmp;
}

#endif 
} 

wxBitmapButton*
wxBitmapButtonBase::NewCloseButton(wxWindow* parent, wxWindowID winid)
{
    wxCHECK_MSG( parent, NULL, wxS("Must have a valid parent") );

    const wxColour colBg = parent->GetBackgroundColour();

#ifdef wxHAS_DRAW_TITLE_BAR_BITMAP
    const wxSize sizeBmp = wxArtProvider::GetSizeHint(wxART_BUTTON);
    wxBitmap bmp = GetCloseButtonBitmap(parent, sizeBmp, colBg);
#else     wxBitmap bmp = wxArtProvider::GetBitmap(wxART_CLOSE, wxART_BUTTON);
#endif 
    wxBitmapButton* const button = new wxBitmapButton
                                       (
                                        parent,
                                        winid,
                                        bmp,
                                        wxDefaultPosition,
                                        wxDefaultSize,
                                        wxBORDER_NONE
                                       );

#ifdef wxHAS_DRAW_TITLE_BAR_BITMAP
    button->SetBitmapPressed(
        GetCloseButtonBitmap(parent, sizeBmp, colBg, wxCONTROL_PRESSED));

    button->SetBitmapCurrent(
        GetCloseButtonBitmap(parent, sizeBmp, colBg, wxCONTROL_CURRENT));
#endif 
        button->SetBackgroundColour(colBg);

    return button;
}

#endif 