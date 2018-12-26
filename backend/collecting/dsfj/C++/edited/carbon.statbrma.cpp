
#include "wx/wxprec.h"

#if wxUSE_STATUSBAR

#include "wx/statusbr.h"

#ifndef WX_PRECOMP
    #include "wx/dc.h"
    #include "wx/dcclient.h"
    #include "wx/toplevel.h"
#endif

#include "wx/osx/private.h"

#define wxFIELD_TEXT_MARGIN 2


wxBEGIN_EVENT_TABLE(wxStatusBarMac, wxStatusBarGeneric)
    EVT_PAINT(wxStatusBarMac::OnPaint)
wxEND_EVENT_TABLE()


wxStatusBarMac::wxStatusBarMac(wxWindow *parent,
        wxWindowID id,
        long style,
        const wxString& name)
        :
        wxStatusBarGeneric()
{
    SetParent( NULL );
    Create( parent, id, style, name );
}

wxStatusBarMac::wxStatusBarMac()
        :
        wxStatusBarGeneric()
{
    SetParent( NULL );
}

wxStatusBarMac::~wxStatusBarMac()
{
}

bool wxStatusBarMac::Create(wxWindow *parent, wxWindowID id,
                            long style ,
                            const wxString& name)
{
    if ( !wxStatusBarGeneric::Create( parent, id, style, name ) )
        return false;

    if ( parent->MacGetTopLevelWindow()->GetExtraStyle() & wxFRAME_EX_METAL )
        SetBackgroundStyle( wxBG_STYLE_TRANSPARENT );

        SetWindowVariant( wxWINDOW_VARIANT_SMALL );

    return true;
}

void wxStatusBarMac::DrawFieldText(wxDC& dc, const wxRect& rect, int i, int WXUNUSED(textHeight))
{
    int w, h;
    GetSize( &w , &h );

    if ( !MacIsReallyHilited() )
        dc.SetTextForeground( wxColour( 0x80, 0x80, 0x80 ) );

    wxString text(GetStatusText( i ));

    

    int xpos = rect.x + wxFIELD_TEXT_MARGIN + 1;
    int ypos = 1;

    if ( MacGetTopLevelWindow()->GetExtraStyle() & wxFRAME_EX_METAL )
        ypos++;

    dc.SetClippingRegion(rect.x, 0, rect.width, h);
    dc.DrawText(text, xpos, ypos);
    dc.DestroyClippingRegion();
}

void wxStatusBarMac::DrawField(wxDC& dc, int i, int textHeight)
{
    wxRect rect;
    GetFieldRect(i, rect);

    DrawFieldText(dc, rect, i, textHeight);
}

void wxStatusBarMac::DoUpdateStatusText(int number)
{
    wxRect rect;
    GetFieldRect(number, rect);

    int w, h;
    GetSize( &w, &h );

    rect.y = 0;
    rect.height = h ;

    Refresh( true, &rect );
        #if 0
    Update();
#endif
}

void wxStatusBarMac::OnPaint(wxPaintEvent& WXUNUSED(event))
{
    wxPaintDC dc(this);
    dc.Clear();

    int w, h;
    GetSize( &w, &h );

    if ( MacIsReallyHilited() )
    {
        wxPen white( *wxWHITE , 1 , wxPENSTYLE_SOLID );
                if ( MacGetTopLevelWindow()->GetExtraStyle() & wxFRAME_EX_METAL )
            dc.SetPen(wxPen(wxColour(0x40, 0x40, 0x40), 1, wxPENSTYLE_SOLID));
        else
            dc.SetPen(wxPen(wxColour(0xB1, 0xB1, 0xB1), 1, wxPENSTYLE_SOLID));

        dc.DrawLine(0, 0, w, 0);
        dc.SetPen(white);
        dc.DrawLine(0, 1, w, 1);
    }
    else
    {
                dc.SetPen(wxPen(wxColour(0xB1, 0xB1, 0xB1), 1, wxPENSTYLE_SOLID));

        dc.DrawLine(0, 0, w, 0);
    }

    if ( GetFont().IsOk() )
        dc.SetFont(GetFont());
    dc.SetBackgroundMode(wxTRANSPARENT);

        int textHeight = dc.GetCharHeight();

    for ( size_t i = 0; i < m_panes.GetCount(); i ++ )
        DrawField(dc, i, textHeight);
}

void wxStatusBarMac::MacHiliteChanged()
{
    Refresh();
    Update();
}

#endif 
