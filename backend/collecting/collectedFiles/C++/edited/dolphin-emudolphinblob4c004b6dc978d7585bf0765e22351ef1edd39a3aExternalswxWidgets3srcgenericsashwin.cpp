
#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#if wxUSE_SASH

#include "wx/sashwin.h"

#ifndef WX_PRECOMP
    #include "wx/dialog.h"
    #include "wx/frame.h"
    #include "wx/settings.h"
    #include "wx/dcclient.h"
    #include "wx/dcscreen.h"
    #include "wx/math.h"
#endif

#include <stdlib.h>

#include "wx/laywin.h"

wxDEFINE_EVENT( wxEVT_SASH_DRAGGED, wxSashEvent );

wxIMPLEMENT_DYNAMIC_CLASS(wxSashWindow, wxWindow);
wxIMPLEMENT_DYNAMIC_CLASS(wxSashEvent, wxCommandEvent);

wxBEGIN_EVENT_TABLE(wxSashWindow, wxWindow)
    EVT_PAINT(wxSashWindow::OnPaint)
    EVT_SIZE(wxSashWindow::OnSize)
    EVT_MOUSE_EVENTS(wxSashWindow::OnMouseEvent)
#if defined( __WXMSW__ ) || defined( __WXMAC__)
    EVT_SET_CURSOR(wxSashWindow::OnSetCursor)
#endif 
wxEND_EVENT_TABLE()

bool wxSashWindow::Create(wxWindow *parent, wxWindowID id, const wxPoint& pos,
    const wxSize& size, long style, const wxString& name)
{
    return wxWindow::Create(parent, id, pos, size, style, name);
}

wxSashWindow::~wxSashWindow()
{
    delete m_sashCursorWE;
    delete m_sashCursorNS;
}

void wxSashWindow::Init()
{
    m_draggingEdge = wxSASH_NONE;
    m_dragMode = wxSASH_DRAG_NONE;
    m_oldX = 0;
    m_oldY = 0;
    m_firstX = 0;
    m_firstY = 0;
    m_borderSize = 3;
    m_extraBorderSize = 0;
    m_minimumPaneSizeX = 0;
    m_minimumPaneSizeY = 0;
    m_maximumPaneSizeX = 10000;
    m_maximumPaneSizeY = 10000;
    m_sashCursorWE = new wxCursor(wxCURSOR_SIZEWE);
    m_sashCursorNS = new wxCursor(wxCURSOR_SIZENS);
    m_mouseCaptured = false;
    m_currentCursor = NULL;

        InitColours();
}

void wxSashWindow::OnPaint(wxPaintEvent& WXUNUSED(event))
{
    wxPaintDC dc(this);

    DrawBorders(dc);
    DrawSashes(dc);
}

void wxSashWindow::OnMouseEvent(wxMouseEvent& event)
{
    wxCoord x = 0, y = 0;
    event.GetPosition(&x, &y);

    wxSashEdgePosition sashHit = SashHitTest(x, y);

    if (event.LeftDown())
    {
        CaptureMouse();
        m_mouseCaptured = true;

        if ( sashHit != wxSASH_NONE )
        {
                                                                                    wxWindow* parent = this;

            while (parent && !wxDynamicCast(parent, wxDialog) &&
                             !wxDynamicCast(parent, wxFrame))
              parent = parent->GetParent();

            wxScreenDC::StartDrawingOnTop(parent);

                                                m_dragMode = wxSASH_DRAG_LEFT_DOWN;
            m_draggingEdge = sashHit;
            m_firstX = x;
            m_firstY = y;

            if ( (sashHit == wxSASH_LEFT) || (sashHit == wxSASH_RIGHT) )
            {
                if (m_currentCursor != m_sashCursorWE)
                {
                    SetCursor(*m_sashCursorWE);
                }
                m_currentCursor = m_sashCursorWE;
            }
            else
            {
                if (m_currentCursor != m_sashCursorNS)
                {
                    SetCursor(*m_sashCursorNS);
                }
                m_currentCursor = m_sashCursorNS;
            }
        }
    }
    else if ( event.LeftUp() && m_dragMode == wxSASH_DRAG_LEFT_DOWN )
    {
                if (m_mouseCaptured)
            ReleaseMouse();
        m_mouseCaptured = false;

        wxScreenDC::EndDrawingOnTop();
        m_dragMode = wxSASH_DRAG_NONE;
        m_draggingEdge = wxSASH_NONE;
    }
    else if (event.LeftUp() && m_dragMode == wxSASH_DRAG_DRAGGING)
    {
                m_dragMode = wxSASH_DRAG_NONE;
        if (m_mouseCaptured)
            ReleaseMouse();
        m_mouseCaptured = false;

                DrawSashTracker(m_draggingEdge, m_oldX, m_oldY);

                        wxScreenDC::EndDrawingOnTop();

        int w, h;
        GetSize(&w, &h);
        int xp, yp;
        GetPosition(&xp, &yp);

        wxSashEdgePosition edge = m_draggingEdge;
        m_draggingEdge = wxSASH_NONE;

        wxRect dragRect;
        wxSashDragStatus status = wxSASH_STATUS_OK;

                int newHeight = wxDefaultCoord,
            newWidth = wxDefaultCoord;

                                        y += yp;
        x += xp;

        switch (edge)
        {
            case wxSASH_TOP:
                if ( y > yp + h )
                {
                                        status = wxSASH_STATUS_OUT_OF_RANGE;
                }
                else
                {
                    newHeight = h - (y - yp);
                }
                break;

            case wxSASH_BOTTOM:
                if ( y < yp )
                {
                                        status = wxSASH_STATUS_OUT_OF_RANGE;
                }
                else
                {
                    newHeight = y - yp;
                }
                break;

            case wxSASH_LEFT:
                if ( x > xp + w )
                {
                                        status = wxSASH_STATUS_OUT_OF_RANGE;
                }
                else
                {
                    newWidth = w - (x - xp);
                }
                break;

            case wxSASH_RIGHT:
                if ( x < xp )
                {
                                                            status = wxSASH_STATUS_OUT_OF_RANGE;
                }
                else
                {
                    newWidth = x - xp;
                }
                break;

            case wxSASH_NONE:
                                break;
        }

        if ( newHeight == wxDefaultCoord )
        {
                        newHeight = h;
        }
        else
        {
                        newHeight = wxMax(newHeight, m_minimumPaneSizeY);
            newHeight = wxMin(newHeight, m_maximumPaneSizeY);
        }

        if ( newWidth == wxDefaultCoord )
        {
                        newWidth = w;
        }
        else
        {
                        newWidth = wxMax(newWidth, m_minimumPaneSizeX);
            newWidth = wxMin(newWidth, m_maximumPaneSizeX);
        }

        dragRect = wxRect(x, y, newWidth, newHeight);

        wxSashEvent eventSash(GetId(), edge);
        eventSash.SetEventObject(this);
        eventSash.SetDragStatus(status);
        eventSash.SetDragRect(dragRect);
        GetEventHandler()->ProcessEvent(eventSash);
    }
    else if ( event.LeftUp() )
    {
        if (m_mouseCaptured)
           ReleaseMouse();
        m_mouseCaptured = false;
    }
    else if ((event.Moving() || event.Leaving()) && !event.Dragging())
    {
                if ( sashHit != wxSASH_NONE )
        {
            if ( (sashHit == wxSASH_LEFT) || (sashHit == wxSASH_RIGHT) )
            {
                if (m_currentCursor != m_sashCursorWE)
                {
                    SetCursor(*m_sashCursorWE);
                }
                m_currentCursor = m_sashCursorWE;
            }
            else
            {
                if (m_currentCursor != m_sashCursorNS)
                {
                    SetCursor(*m_sashCursorNS);
                }
                m_currentCursor = m_sashCursorNS;
            }
        }
        else
        {
            SetCursor(wxNullCursor);
            m_currentCursor = NULL;
        }
    }
    else if ( event.Dragging() &&
              ((m_dragMode == wxSASH_DRAG_DRAGGING) ||
               (m_dragMode == wxSASH_DRAG_LEFT_DOWN)) )
    {
        if ( (m_draggingEdge == wxSASH_LEFT) || (m_draggingEdge == wxSASH_RIGHT) )
        {
            if (m_currentCursor != m_sashCursorWE)
            {
                SetCursor(*m_sashCursorWE);
            }
            m_currentCursor = m_sashCursorWE;
        }
        else
        {
            if (m_currentCursor != m_sashCursorNS)
            {
                SetCursor(*m_sashCursorNS);
            }
            m_currentCursor = m_sashCursorNS;
        }

        if (m_dragMode == wxSASH_DRAG_LEFT_DOWN)
        {
            m_dragMode = wxSASH_DRAG_DRAGGING;
            DrawSashTracker(m_draggingEdge, x, y);
        }
        else
        {
            if ( m_dragMode == wxSASH_DRAG_DRAGGING )
            {
                                DrawSashTracker(m_draggingEdge, m_oldX, m_oldY);

                                DrawSashTracker(m_draggingEdge, x, y);
            }
        }
        m_oldX = x;
        m_oldY = y;
    }
    else if ( event.LeftDClick() )
    {
            }
    else
    {
    }
}

void wxSashWindow::OnSize(wxSizeEvent& WXUNUSED(event))
{
    SizeWindows();
}

wxSashEdgePosition wxSashWindow::SashHitTest(int x, int y, int WXUNUSED(tolerance))
{
    int cx, cy;
    GetClientSize(& cx, & cy);

    int i;
    for (i = 0; i < 4; i++)
    {
        wxSashEdge& edge = m_sashes[i];
        wxSashEdgePosition position = (wxSashEdgePosition) i ;

        if (edge.m_show)
        {
            switch (position)
            {
                case wxSASH_TOP:
                {
                    if (y >= 0 && y <= GetEdgeMargin(position))
                        return wxSASH_TOP;
                    break;
                }
                case wxSASH_RIGHT:
                {
                    if ((x >= cx - GetEdgeMargin(position)) && (x <= cx))
                        return wxSASH_RIGHT;
                    break;
                }
                case wxSASH_BOTTOM:
                {
                    if ((y >= cy - GetEdgeMargin(position)) && (y <= cy))
                        return wxSASH_BOTTOM;
                    break;
                }
                case wxSASH_LEFT:
                {
                    if ((x <= GetEdgeMargin(position)) && (x >= 0))
                        return wxSASH_LEFT;
                    break;
                }
                case wxSASH_NONE:
                {
                    break;
                }
            }
        }
    }
    return wxSASH_NONE;
}

void wxSashWindow::DrawBorders(wxDC& dc)
{
    int w, h;
    GetClientSize(&w, &h);

    wxPen mediumShadowPen(m_mediumShadowColour, 1, wxPENSTYLE_SOLID);
    wxPen darkShadowPen(m_darkShadowColour, 1, wxPENSTYLE_SOLID);
    wxPen lightShadowPen(m_lightShadowColour, 1, wxPENSTYLE_SOLID);
    wxPen hilightPen(m_hilightColour, 1, wxPENSTYLE_SOLID);

    if ( GetWindowStyleFlag() & wxSW_3DBORDER )
    {
        dc.SetPen(mediumShadowPen);
        dc.DrawLine(0, 0, w-1, 0);
        dc.DrawLine(0, 0, 0, h - 1);

        dc.SetPen(darkShadowPen);
        dc.DrawLine(1, 1, w-2, 1);
        dc.DrawLine(1, 1, 1, h-2);

        dc.SetPen(hilightPen);
        dc.DrawLine(0, h-1, w-1, h-1);
        dc.DrawLine(w-1, 0, w-1, h);                                      
        dc.SetPen(lightShadowPen);
        dc.DrawLine(w-2, 1, w-2, h-2);         dc.DrawLine(1, h-2, w-1, h-2);         }
    else if ( GetWindowStyleFlag() & wxSW_BORDER )
    {
        dc.SetBrush(*wxTRANSPARENT_BRUSH);
        dc.SetPen(*wxBLACK_PEN);
        dc.DrawRectangle(0, 0, w-1, h-1);
    }

    dc.SetPen(wxNullPen);
    dc.SetBrush(wxNullBrush);
}

void wxSashWindow::DrawSashes(wxDC& dc)
{
    int i;
    for (i = 0; i < 4; i++)
        if (m_sashes[i].m_show)
            DrawSash((wxSashEdgePosition) i, dc);
}

void wxSashWindow::DrawSash(wxSashEdgePosition edge, wxDC& dc)
{
    int w, h;
    GetClientSize(&w, &h);

    wxPen facePen(m_faceColour, 1, wxPENSTYLE_SOLID);
    wxBrush faceBrush(m_faceColour, wxBRUSHSTYLE_SOLID);
    wxPen mediumShadowPen(m_mediumShadowColour, 1, wxPENSTYLE_SOLID);
    wxPen darkShadowPen(m_darkShadowColour, 1, wxPENSTYLE_SOLID);
    wxPen lightShadowPen(m_lightShadowColour, 1, wxPENSTYLE_SOLID);
    wxPen hilightPen(m_hilightColour, 1, wxPENSTYLE_SOLID);
    wxColour blackClr(0, 0, 0);
    wxColour whiteClr(255, 255, 255);
    wxPen blackPen(blackClr, 1, wxPENSTYLE_SOLID);
    wxPen whitePen(whiteClr, 1, wxPENSTYLE_SOLID);

    if ( edge == wxSASH_LEFT || edge == wxSASH_RIGHT )
    {
        int sashPosition = (edge == wxSASH_LEFT) ? 0 : ( w - GetEdgeMargin(edge) );

        dc.SetPen(facePen);
        dc.SetBrush(faceBrush);
        dc.DrawRectangle(sashPosition, 0, GetEdgeMargin(edge), h);

        if (GetWindowStyleFlag() & wxSW_3DSASH)
        {
            if (edge == wxSASH_LEFT)
            {
                                                dc.SetPen(mediumShadowPen);
                dc.DrawLine(GetEdgeMargin(edge), 0, GetEdgeMargin(edge), h);
            }
            else
            {
                                                dc.SetPen(hilightPen);
                dc.DrawLine(w - GetEdgeMargin(edge), 0, w - GetEdgeMargin(edge), h);
            }
        }
    }
    else     {
        int sashPosition = (edge == wxSASH_TOP) ? 0 : ( h - GetEdgeMargin(edge) );

        dc.SetPen(facePen);
        dc.SetBrush(faceBrush);
        dc.DrawRectangle(0, sashPosition, w, GetEdgeMargin(edge));

        if (GetWindowStyleFlag() & wxSW_3DSASH)
        {
            if (edge == wxSASH_BOTTOM)
            {
                                                dc.SetPen(hilightPen);
                dc.DrawLine(0, h - GetEdgeMargin(edge), w, h - GetEdgeMargin(edge));
            }
            else
            {
                                                dc.SetPen(mediumShadowPen);
                dc.DrawLine(1, GetEdgeMargin(edge), w-1, GetEdgeMargin(edge));
            }
        }
    }

    dc.SetPen(wxNullPen);
    dc.SetBrush(wxNullBrush);
}

void wxSashWindow::DrawSashTracker(wxSashEdgePosition edge, int x, int y)
{
    int w, h;
    GetClientSize(&w, &h);

    wxScreenDC screenDC;
    int x1, y1;
    int x2, y2;

    if ( edge == wxSASH_LEFT || edge == wxSASH_RIGHT )
    {
        x1 = x; y1 = 2;
        x2 = x; y2 = h-2;

        if ( (edge == wxSASH_LEFT) && (x1 > w) )
        {
            x1 = w; x2 = w;
        }
        else if ( (edge == wxSASH_RIGHT) && (x1 < 0) )
        {
            x1 = 0; x2 = 0;
        }
    }
    else
    {
        x1 = 2; y1 = y;
        x2 = w-2; y2 = y;

        if ( (edge == wxSASH_TOP) && (y1 > h) )
        {
            y1 = h;
            y2 = h;
        }
        else if ( (edge == wxSASH_BOTTOM) && (y1 < 0) )
        {
            y1 = 0;
            y2 = 0;
        }
    }

    ClientToScreen(&x1, &y1);
    ClientToScreen(&x2, &y2);

    wxPen sashTrackerPen(*wxBLACK, 2, wxPENSTYLE_SOLID);

    screenDC.SetLogicalFunction(wxINVERT);
    screenDC.SetPen(sashTrackerPen);
    screenDC.SetBrush(*wxTRANSPARENT_BRUSH);

    screenDC.DrawLine(x1, y1, x2, y2);

    screenDC.SetLogicalFunction(wxCOPY);

    screenDC.SetPen(wxNullPen);
    screenDC.SetBrush(wxNullBrush);
}

void wxSashWindow::SizeWindows()
{
    int cw, ch;
    GetClientSize(&cw, &ch);

    if (GetChildren().GetCount() == 1)
    {
        wxWindow* child = GetChildren().GetFirst()->GetData();

        int x = 0;
        int y = 0;
        int width = cw;
        int height = ch;

                if (m_sashes[0].m_show)
        {
            y = m_borderSize;
            height -= m_borderSize;
        }
        y += m_extraBorderSize;

                if (m_sashes[3].m_show)
        {
            x = m_borderSize;
            width -= m_borderSize;
        }
        x += m_extraBorderSize;

                if (m_sashes[1].m_show)
        {
            width -= m_borderSize;
        }
        width -= 2*m_extraBorderSize;

                if (m_sashes[2].m_show)
        {
            height -= m_borderSize;
        }
        height -= 2*m_extraBorderSize;

        child->SetSize(x, y, width, height);
    }
    else if (GetChildren().GetCount() > 1)
    {
                                                wxLayoutAlgorithm layout;
        layout.LayoutWindow(this);
    }

    wxClientDC dc(this);
    DrawBorders(dc);
    DrawSashes(dc);
}

void wxSashWindow::InitColours()
{
        m_faceColour = wxSystemSettings::GetColour(wxSYS_COLOUR_3DFACE);
    m_mediumShadowColour = wxSystemSettings::GetColour(wxSYS_COLOUR_3DSHADOW);
    m_darkShadowColour = wxSystemSettings::GetColour(wxSYS_COLOUR_3DDKSHADOW);
    m_lightShadowColour = wxSystemSettings::GetColour(wxSYS_COLOUR_3DLIGHT);
    m_hilightColour = wxSystemSettings::GetColour(wxSYS_COLOUR_3DHILIGHT);
}

void wxSashWindow::SetSashVisible(wxSashEdgePosition edge, bool sash)
{
     m_sashes[edge].m_show = sash;
     if (sash)
        m_sashes[edge].m_margin = m_borderSize;
     else
        m_sashes[edge].m_margin = 0;
}

#if defined( __WXMSW__ ) || defined( __WXMAC__)

void wxSashWindow::OnSetCursor(wxSetCursorEvent& event)
{
            
    if ( SashHitTest(event.GetX(), event.GetY()) != wxSASH_NONE)
    {
                event.Skip();
    }
    }

#endif 
#endif 