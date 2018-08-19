


#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#if wxUSE_DRAGIMAGE

#ifndef WX_PRECOMP
    #include <stdio.h>
    #include "wx/window.h"
    #include "wx/frame.h"
    #include "wx/dcclient.h"
    #include "wx/dcscreen.h"
    #include "wx/dcmemory.h"
    #include "wx/settings.h"
    #include "wx/intl.h"
    #include "wx/log.h"
    #include "wx/image.h"
#endif

#define wxUSE_IMAGE_IN_DRAGIMAGE 1

#include "wx/generic/dragimgg.h"


wxIMPLEMENT_DYNAMIC_CLASS(wxGenericDragImage, wxObject);



wxGenericDragImage::~wxGenericDragImage()
{
    if (m_windowDC)
    {
        delete m_windowDC;
    }
}

void wxGenericDragImage::Init()
{
    m_isDirty = false;
    m_isShown = false;
    m_windowDC = NULL;
    m_window = NULL;
    m_fullScreen = false;
#ifdef wxHAS_NATIVE_OVERLAY
    m_dcOverlay = NULL;
#else
    m_pBackingBitmap = NULL;
#endif
}




bool wxGenericDragImage::Create(const wxCursor& cursor)
{
    m_cursor = cursor;

    return true;
}

bool wxGenericDragImage::Create(const wxBitmap& image, const wxCursor& cursor)
{
        
    m_cursor = cursor;
    m_bitmap = image;

    return true ;
}

bool wxGenericDragImage::Create(const wxIcon& image, const wxCursor& cursor)
{
        
    m_cursor = cursor;
    m_icon = image;

    return true ;
}

bool wxGenericDragImage::Create(const wxString& str, const wxCursor& cursor)
{
    wxFont font(wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT));

    wxCoord w = 0, h = 0;
    wxScreenDC dc;
    dc.SetFont(font);
    dc.GetTextExtent(str, & w, & h);
    dc.SetFont(wxNullFont);

    wxMemoryDC dc2;

        wxBitmap bitmap((int) ((w+2) * 1.5), (int) h+2);
    dc2.SelectObject(bitmap);

    dc2.SetFont(font);
    dc2.SetBackground(* wxWHITE_BRUSH);
    dc2.Clear();
    dc2.SetBackgroundMode(wxBRUSHSTYLE_TRANSPARENT);
    dc2.SetTextForeground(* wxLIGHT_GREY);
    dc2.DrawText(str, 0, 0);
    dc2.DrawText(str, 1, 0);
    dc2.DrawText(str, 2, 0);
    dc2.DrawText(str, 1, 1);
    dc2.DrawText(str, 2, 1);
    dc2.DrawText(str, 1, 2);
    dc2.DrawText(str, 2, 2);

    dc2.SetTextForeground(* wxBLACK);
    dc2.DrawText(str, 1, 1);

    dc2.SelectObject(wxNullBitmap);

#if wxUSE_IMAGE_IN_DRAGIMAGE && (!defined(__WXMSW__) || wxUSE_WXDIB)
        wxImage image = bitmap.ConvertToImage();
    image.SetMaskColour(255, 255, 255);
    bitmap = wxBitmap(image);
#endif

    return Create(bitmap, cursor);
}

#if wxUSE_TREECTRL
bool wxGenericDragImage::Create(const wxTreeCtrl& treeCtrl, wxTreeItemId& id)
{
    wxString str = treeCtrl.GetItemText(id);
    return Create(str);
}
#endif

#if wxUSE_LISTCTRL
bool wxGenericDragImage::Create(const wxListCtrl& listCtrl, long id)
{
    wxString str = listCtrl.GetItemText(id);
    return Create(str);
}
#endif

bool wxGenericDragImage::BeginDrag(const wxPoint& hotspot,
                                   wxWindow* window,
                                   bool fullScreen,
                                   wxRect* rect)
{
    wxCHECK_MSG( window, false, wxT("Window must not be null in BeginDrag."));

        m_offset = hotspot;
    m_window = window;
    m_fullScreen = fullScreen;

    if (rect)
        m_boundingRect = * rect;

    m_isDirty = false;
    m_isShown = false;

    if (m_cursor.IsOk())
    {
        m_oldCursor = window->GetCursor();
        window->SetCursor(m_cursor);
    }

    window->CaptureMouse();

        
    wxSize clientSize;
    wxPoint pt;
    if (!m_fullScreen)
    {
        clientSize = window->GetClientSize();
        m_boundingRect.x = 0; m_boundingRect.y = 0;
        m_boundingRect.width = clientSize.x; m_boundingRect.height = clientSize.y;
    }
    else
    {
        int w, h;
        wxDisplaySize(& w, & h);
        clientSize.x = w; clientSize.y = h;
        if (rect)
        {
            pt.x = m_boundingRect.x; pt.y = m_boundingRect.y;
            clientSize.x = m_boundingRect.width; clientSize.y = m_boundingRect.height;
        }
        else
        {
            m_boundingRect.x = 0; m_boundingRect.y = 0;
            m_boundingRect.width = w; m_boundingRect.height = h;
        }
    }

#ifndef wxHAS_NATIVE_OVERLAY
    wxBitmap* backing = (m_pBackingBitmap ? m_pBackingBitmap : (wxBitmap*) & m_backingBitmap);

    if (!backing->IsOk() || (backing->GetWidth() < clientSize.x || backing->GetHeight() < clientSize.y))
        (*backing) = wxBitmap(clientSize.x, clientSize.y);
#endif 
    if (!m_fullScreen)
    {
        m_windowDC = new wxClientDC(window);
    }
    else
    {
        m_windowDC = new wxScreenDC;

#if 0
                ((wxScreenDC*) m_windowDC)->StartDrawingOnTop(rect);
#endif

        m_windowDC->SetClippingRegion(m_boundingRect.x, m_boundingRect.y,
            m_boundingRect.width, m_boundingRect.height);
    }

    return true;
}

bool wxGenericDragImage::BeginDrag(const wxPoint& hotspot, wxWindow* window, wxWindow* fullScreenRect)
{
    wxRect rect;

    int x = fullScreenRect->GetPosition().x;
    int y = fullScreenRect->GetPosition().y;

    wxSize sz = fullScreenRect->GetSize();

    if (fullScreenRect->GetParent() && !wxDynamicCast(fullScreenRect, wxFrame))
        fullScreenRect->GetParent()->ClientToScreen(& x, & y);

    rect.x = x; rect.y = y;
    rect.width = sz.x; rect.height = sz.y;

    return BeginDrag(hotspot, window, true, & rect);
}

bool wxGenericDragImage::EndDrag()
{
    if (m_window)
    {
#ifdef __WXMSW__
                        if (wxWindow::GetCapture() == m_window)
#endif
            m_window->ReleaseMouse();

        if (m_cursor.IsOk() && m_oldCursor.IsOk())
        {
            m_window->SetCursor(m_oldCursor);
        }
    }

    if (m_windowDC)
    {
#ifdef wxHAS_NATIVE_OVERLAY
        m_overlay.Reset();
#else
        m_windowDC->DestroyClippingRegion();
#endif
        wxDELETE(m_windowDC);
    }

#ifndef wxHAS_NATIVE_OVERLAY
    m_repairBitmap = wxNullBitmap;
#endif

    return true;
}

bool wxGenericDragImage::Move(const wxPoint& pt)
{
    wxASSERT_MSG( (m_windowDC != NULL), wxT("No window DC in wxGenericDragImage::Move()") );

    wxPoint pt2(pt);
    if (m_fullScreen)
        pt2 = m_window->ClientToScreen(pt);

        wxPoint oldPos = m_position;

    bool eraseOldImage = (m_isDirty && m_isShown);

    if (m_isShown)
        RedrawImage(oldPos - m_offset, pt2 - m_offset, eraseOldImage, true);

    m_position = pt2;

    if (m_isShown)
        m_isDirty = true;

    return true;
}

bool wxGenericDragImage::Show()
{
    wxASSERT_MSG( (m_windowDC != NULL), wxT("No window DC in wxGenericDragImage::Show()") );

    
    if (!m_isShown)
    {
                
#ifndef wxHAS_NATIVE_OVERLAY
        wxBitmap* backing = (m_pBackingBitmap ? m_pBackingBitmap : (wxBitmap*) & m_backingBitmap);
        wxMemoryDC memDC;
        memDC.SelectObject(* backing);

        UpdateBackingFromWindow(* m_windowDC, memDC, m_boundingRect, wxRect(0, 0, m_boundingRect.width, m_boundingRect.height));

                memDC.SelectObject(wxNullBitmap);
#endif 
        RedrawImage(m_position - m_offset, m_position - m_offset, false, true);
    }

    m_isShown = true;
    m_isDirty = true;

    return true;
}

bool wxGenericDragImage::UpdateBackingFromWindow(wxDC& windowDC, wxMemoryDC& destDC,
    const wxRect& sourceRect, const wxRect& destRect) const
{
    return destDC.Blit(destRect.x, destRect.y, destRect.width, destRect.height, & windowDC,
        sourceRect.x, sourceRect.y);
}

bool wxGenericDragImage::Hide()
{
    wxASSERT_MSG( (m_windowDC != NULL), wxT("No window DC in wxGenericDragImage::Hide()") );

    
    if (m_isShown && m_isDirty)
    {
        RedrawImage(m_position - m_offset, m_position - m_offset, true, false);
    }

    m_isShown = false;
    m_isDirty = false;

    return true;
}

bool wxGenericDragImage::RedrawImage(const wxPoint& oldPos,
                                     const wxPoint& newPos,
                                     bool eraseOld, bool drawNew)
{
    if (!m_windowDC)
        return false;

#ifdef wxHAS_NATIVE_OVERLAY
    wxUnusedVar(oldPos);

    wxDCOverlay dcoverlay( m_overlay, (wxWindowDC*) m_windowDC ) ;
    if ( eraseOld )
        dcoverlay.Clear() ;
    if (drawNew)
        DoDrawImage(*m_windowDC, newPos);
#else     wxBitmap* backing = (m_pBackingBitmap ? m_pBackingBitmap : (wxBitmap*) & m_backingBitmap);
    if (!backing->IsOk())
        return false;

    wxRect oldRect(GetImageRect(oldPos));
    wxRect newRect(GetImageRect(newPos));

    wxRect fullRect;

        if (eraseOld && drawNew)
    {
        int oldRight = oldRect.GetRight();
        int oldBottom = oldRect.GetBottom();
        int newRight = newRect.GetRight();
        int newBottom = newRect.GetBottom();

        wxPoint topLeft = wxPoint(wxMin(oldPos.x, newPos.x), wxMin(oldPos.y, newPos.y));
        wxPoint bottomRight = wxPoint(wxMax(oldRight, newRight), wxMax(oldBottom, newBottom));

        fullRect.x = topLeft.x; fullRect.y = topLeft.y;
        fullRect.SetRight(bottomRight.x);
        fullRect.SetBottom(bottomRight.y);
    }
    else if (eraseOld)
        fullRect = oldRect;
    else if (drawNew)
        fullRect = newRect;

            int excess = 50;

    if (!m_repairBitmap.IsOk() || (m_repairBitmap.GetWidth() < fullRect.GetWidth() || m_repairBitmap.GetHeight() < fullRect.GetHeight()))
    {
        m_repairBitmap = wxBitmap(fullRect.GetWidth() + excess, fullRect.GetHeight() + excess);
    }

    wxMemoryDC memDC;
    memDC.SelectObject(* backing);

    wxMemoryDC memDCTemp;
    memDCTemp.SelectObject(m_repairBitmap);

                    
    memDCTemp.Blit(0, 0, fullRect.GetWidth(), fullRect.GetHeight(), & memDC, fullRect.x - m_boundingRect.x, fullRect.y - m_boundingRect.y);

        if (drawNew)
    {
        wxPoint pos(newPos.x - fullRect.x, newPos.y - fullRect.y) ;
        DoDrawImage(memDCTemp, pos);
    }

            m_windowDC->Blit(fullRect.x, fullRect.y, fullRect.width, fullRect.height, & memDCTemp, 0, 0);

    memDCTemp.SelectObject(wxNullBitmap);
    memDC.SelectObject(wxNullBitmap);
#endif 
    return true;
}

bool wxGenericDragImage::DoDrawImage(wxDC& dc, const wxPoint& pos) const
{
    if (m_bitmap.IsOk())
    {
        dc.DrawBitmap(m_bitmap, pos.x, pos.y, (m_bitmap.GetMask() != 0));
        return true;
    }
    else if (m_icon.IsOk())
    {
        dc.DrawIcon(m_icon, pos.x, pos.y);
        return true;
    }
    else
        return false;
}

wxRect wxGenericDragImage::GetImageRect(const wxPoint& pos) const
{
    if (m_bitmap.IsOk())
    {
        return wxRect(pos.x, pos.y, m_bitmap.GetWidth(), m_bitmap.GetHeight());
    }
    else if (m_icon.IsOk())
    {
        return wxRect(pos.x, pos.y, m_icon.GetWidth(), m_icon.GetHeight());
    }
    else
    {
        return wxRect(pos.x, pos.y, 0, 0);
    }
}

#endif 