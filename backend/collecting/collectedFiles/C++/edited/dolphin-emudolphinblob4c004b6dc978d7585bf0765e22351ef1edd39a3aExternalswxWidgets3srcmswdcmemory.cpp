


#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#include "wx/dcmemory.h"
#include "wx/msw/dcmemory.h"

#ifndef WX_PRECOMP
    #include "wx/utils.h"
    #include "wx/log.h"
#endif

#include "wx/msw/private.h"


wxIMPLEMENT_ABSTRACT_CLASS(wxMemoryDCImpl, wxMSWDCImpl);

wxMemoryDCImpl::wxMemoryDCImpl( wxMemoryDC *owner )
        : wxMSWDCImpl( owner )
{
    CreateCompatible(NULL);
    Init();
}

wxMemoryDCImpl::wxMemoryDCImpl( wxMemoryDC *owner, wxBitmap& bitmap )
        : wxMSWDCImpl( owner )
{
    CreateCompatible(NULL);
    Init();
    DoSelect(bitmap);
}

wxMemoryDCImpl::wxMemoryDCImpl( wxMemoryDC *owner, wxDC *dc )
        : wxMSWDCImpl( owner )
{
    wxCHECK_RET( dc, wxT("NULL dc in wxMemoryDC ctor") );

    CreateCompatible(dc);

    Init();
}

void wxMemoryDCImpl::Init()
{
    if ( m_ok )
    {
        SetBrush(*wxWHITE_BRUSH);
        SetPen(*wxBLACK_PEN);

                        ::SetBkMode( GetHdc(), TRANSPARENT );
    }
}

bool wxMemoryDCImpl::CreateCompatible(wxDC *dc)
{
    wxDCImpl *impl = dc ? dc->GetImpl() : NULL ;
    wxMSWDCImpl *msw_impl = wxDynamicCast( impl, wxMSWDCImpl );
    if ( dc && !msw_impl)
    {
        m_ok = false;
        return false;
    }

    m_hDC = (WXHDC)::CreateCompatibleDC(dc ? GetHdcOf(*msw_impl) : NULL);

        m_bOwnsDC = true;

    m_ok = m_hDC != 0;

    return m_ok;
}

void wxMemoryDCImpl::DoSelect( const wxBitmap& bitmap )
{
        if ( m_oldBitmap )
    {
        ::SelectObject(GetHdc(), (HBITMAP) m_oldBitmap);
        if ( m_selectedBitmap.IsOk() )
        {
            m_selectedBitmap.SetSelectedInto(NULL);
            m_selectedBitmap = wxNullBitmap;
        }
    }

        wxASSERT_MSG( !bitmap.GetSelectedInto() ||
                  (bitmap.GetSelectedInto() == GetOwner()),
                  wxT("Bitmap is selected in another wxMemoryDC, delete the first wxMemoryDC or use SelectObject(NULL)") );

    m_selectedBitmap = bitmap;
    WXHBITMAP hBmp = m_selectedBitmap.GetHBITMAP();
    if ( !hBmp )
        return;

    m_selectedBitmap.SetSelectedInto(GetOwner());
    hBmp = (WXHBITMAP)::SelectObject(GetHdc(), (HBITMAP)hBmp);

    if ( !hBmp )
    {
        wxLogLastError(wxT("SelectObject(memDC, bitmap)"));

        wxFAIL_MSG(wxT("Couldn't select a bitmap into wxMemoryDC"));
    }
    else if ( !m_oldBitmap )
    {
        m_oldBitmap = hBmp;
    }
}

void wxMemoryDCImpl::DoGetSize(int *width, int *height) const
{
    if ( m_selectedBitmap.IsOk() )
    {
        *width = m_selectedBitmap.GetWidth();
        *height = m_selectedBitmap.GetHeight();
    }
    else
    {
        *width = 0;
        *height = 0;
    }
}


#define wxUSE_MEMORY_DC_DRAW_RECTANGLE 0

#if wxUSE_MEMORY_DC_DRAW_RECTANGLE

static void wxDrawRectangle(wxDC& dc, wxCoord x, wxCoord y, wxCoord width, wxCoord height)
{
    wxBrush brush(dc.GetBrush());
    wxPen pen(dc.GetPen());
    if (brush.IsOk() && brush.GetStyle() != wxTRANSPARENT)
    {
        HBRUSH hBrush = (HBRUSH) brush.GetResourceHandle() ;
        if (hBrush)
        {
            RECT rect;
            rect.left = x; rect.top = y;
            rect.right = x + width - 1;
            rect.bottom = y + height - 1;
            ::FillRect((HDC) dc.GetHDC(), &rect, hBrush);
        }
    }
    width --; height --;
    if (pen.IsOk() && pen.GetStyle() != wxTRANSPARENT)
    {
        dc.DrawLine(x, y, x + width, y);
        dc.DrawLine(x, y, x, y + height);
        dc.DrawLine(x, y+height, x+width, y + height);
        dc.DrawLine(x+width, y+height, x+width, y);
    }
}

#endif 
void wxMemoryDCImpl::DoDrawRectangle(wxCoord x, wxCoord y, wxCoord width, wxCoord height)
{
        #if wxUSE_MEMORY_DC_DRAW_RECTANGLE
    if (m_brush.IsOk() && m_pen.IsOk() &&
        (m_brush.GetStyle() == wxSOLID || m_brush.GetStyle() == wxTRANSPARENT) &&
        (m_pen.GetStyle() == wxSOLID || m_pen.GetStyle() == wxTRANSPARENT) &&
        (GetLogicalFunction() == wxCOPY))
    {
        wxDrawRectangle(* this, x, y, width, height);
    }
    else
#endif     {
        wxMSWDCImpl::DoDrawRectangle(x, y, width, height);
    }
}
