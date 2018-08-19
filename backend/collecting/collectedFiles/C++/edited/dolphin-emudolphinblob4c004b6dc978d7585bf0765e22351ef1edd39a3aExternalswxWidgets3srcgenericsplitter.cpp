
#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#if wxUSE_SPLITTER

#include "wx/splitter.h"

#ifndef WX_PRECOMP
    #include "wx/string.h"
    #include "wx/utils.h"
    #include "wx/log.h"

    #include "wx/dcclient.h"
    #include "wx/dcscreen.h"

    #include "wx/window.h"
    #include "wx/dialog.h"
    #include "wx/frame.h"

    #include "wx/settings.h"
#endif

#include "wx/renderer.h"

#include <stdlib.h>

wxDEFINE_EVENT( wxEVT_SPLITTER_SASH_POS_CHANGED, wxSplitterEvent );
wxDEFINE_EVENT( wxEVT_SPLITTER_SASH_POS_CHANGING, wxSplitterEvent );
wxDEFINE_EVENT( wxEVT_SPLITTER_DOUBLECLICKED, wxSplitterEvent );
wxDEFINE_EVENT( wxEVT_SPLITTER_UNSPLIT, wxSplitterEvent );

wxIMPLEMENT_DYNAMIC_CLASS(wxSplitterWindow, wxWindow);



wxIMPLEMENT_DYNAMIC_CLASS(wxSplitterEvent, wxNotifyEvent);

wxBEGIN_EVENT_TABLE(wxSplitterWindow, wxWindow)
    EVT_PAINT(wxSplitterWindow::OnPaint)
    EVT_SIZE(wxSplitterWindow::OnSize)
    EVT_MOUSE_EVENTS(wxSplitterWindow::OnMouseEvent)
    EVT_MOUSE_CAPTURE_LOST(wxSplitterWindow::OnMouseCaptureLost)

#if defined( __WXMSW__ ) || defined( __WXMAC__)
    EVT_SET_CURSOR(wxSplitterWindow::OnSetCursor)
#endif wxEND_EVENT_TABLE()

static bool IsLive(wxSplitterWindow* wnd)
{
                #if defined( __WXMAC__ ) && defined(TARGET_API_MAC_OSX) && TARGET_API_MAC_OSX == 1
    return true; #else
    return wnd->HasFlag(wxSP_LIVE_UPDATE);
#endif
}

bool wxSplitterWindow::Create(wxWindow *parent, wxWindowID id,
                                   const wxPoint& pos,
                                   const wxSize& size,
                                   long style,
                                   const wxString& name)
{
        style |= wxTAB_TRAVERSAL;

    if ( !wxWindow::Create(parent, id, pos, size, style, name) )
        return false;

    m_lastSize = GetClientSize();

    m_permitUnsplitAlways = (style & wxSP_PERMIT_UNSPLIT) != 0;

        #if !defined(__WXGTK__) || defined(__WXGTK20__)
            SetBackgroundStyle(wxBG_STYLE_CUSTOM);
#endif

    return true;
}

void wxSplitterWindow::Init()
{
    m_splitMode = wxSPLIT_VERTICAL;
    m_permitUnsplitAlways = true;
    m_windowOne = NULL;
    m_windowTwo = NULL;
    m_dragMode = wxSPLIT_DRAG_NONE;
    m_oldX = 0;
    m_oldY = 0;
    m_sashStart = 0;
    m_sashPosition = 0;
    m_requestedSashPosition = INT_MAX;
    m_sashGravity = 0.0;
    m_lastSize = wxSize(0,0);
    m_minimumPaneSize = 0;
    m_sashCursorWE = wxCursor(wxCURSOR_SIZEWE);
    m_sashCursorNS = wxCursor(wxCURSOR_SIZENS);
    m_sashTrackerPen = new wxPen(*wxBLACK, 2, wxPENSTYLE_SOLID);

    m_needUpdating = false;
    m_isHot = false;
}

wxSplitterWindow::~wxSplitterWindow()
{
    delete m_sashTrackerPen;
}


void wxSplitterWindow::RedrawIfHotSensitive(bool isHot)
{
    if ( wxRendererNative::Get().GetSplitterParams(this).isHotSensitive )
    {
        m_isHot = isHot;

        wxClientDC dc(this);
        DrawSash(dc);
    }
    }

void wxSplitterWindow::OnEnterSash()
{
    SetResizeCursor();

    RedrawIfHotSensitive(true);
}

void wxSplitterWindow::OnLeaveSash()
{
    SetCursor(*wxSTANDARD_CURSOR);

    RedrawIfHotSensitive(false);
}

void wxSplitterWindow::SetResizeCursor()
{
    SetCursor(m_splitMode == wxSPLIT_VERTICAL ? m_sashCursorWE
                                              : m_sashCursorNS);
}


void wxSplitterWindow::OnPaint(wxPaintEvent& WXUNUSED(event))
{
    wxPaintDC dc(this);
#ifdef __WXOSX__
                dc.Clear();
#endif

    DrawSash(dc);
}

void wxSplitterWindow::OnInternalIdle()
{
    wxWindow::OnInternalIdle();

                    if ( m_needUpdating )
    {
        m_needUpdating = false;
        SizeWindows();
    }
}

void wxSplitterWindow::OnMouseEvent(wxMouseEvent& event)
{
    int x = (int)event.GetX(),
        y = (int)event.GetY();

    if ( GetWindowStyle() & wxSP_NOSASH )
    {
        event.Skip();
        return;
    }

    bool isLive = IsLive(this);

    if (event.LeftDown())
    {
        if ( SashHitTest(x, y) )
        {
                        m_dragMode = wxSPLIT_DRAG_DRAGGING;

                        CaptureMouse();
            SetResizeCursor();

            if ( !isLive )
            {
                                                m_sashPositionCurrent = m_sashPosition;

                m_oldX = (m_splitMode == wxSPLIT_VERTICAL ? m_sashPositionCurrent : x);
                m_oldY = (m_splitMode != wxSPLIT_VERTICAL ? m_sashPositionCurrent : y);
                DrawSashTracker(m_oldX, m_oldY);
            }

            m_ptStart = wxPoint(x,y);
            m_sashStart = m_sashPosition;
            return;
        }
    }
    else if (event.LeftUp() && m_dragMode == wxSPLIT_DRAG_DRAGGING)
    {
                m_dragMode = wxSPLIT_DRAG_NONE;

                ReleaseMouse();
        SetCursor(* wxSTANDARD_CURSOR);

                if ( !IsSplit() )
        {
            return;
        }

                if ( !isLive )
        {
            DrawSashTracker(m_oldX, m_oldY);
        }

                                int diff = m_splitMode == wxSPLIT_VERTICAL ? x - m_ptStart.x : y - m_ptStart.y;

        int posSashNew = OnSashPositionChanging(m_sashStart + diff);
        if ( posSashNew == -1 )
        {
                        return;
        }

        if ( m_permitUnsplitAlways || m_minimumPaneSize == 0 )
        {
                        if ( posSashNew == 0 )
            {
                                wxWindow *removedWindow = m_windowOne;
                m_windowOne = m_windowTwo;
                m_windowTwo = NULL;
                OnUnsplit(removedWindow);
                wxSplitterEvent eventUnsplit(wxEVT_SPLITTER_UNSPLIT, this);
                eventUnsplit.m_data.win = removedWindow;
                (void)DoSendEvent(eventUnsplit);
                SetSashPositionAndNotify(0);
            }
            else if ( posSashNew == GetWindowSize() )
            {
                                wxWindow *removedWindow = m_windowTwo;
                m_windowTwo = NULL;
                OnUnsplit(removedWindow);
                wxSplitterEvent eventUnsplit(wxEVT_SPLITTER_UNSPLIT, this);
                eventUnsplit.m_data.win = removedWindow;
                (void)DoSendEvent(eventUnsplit);
                SetSashPositionAndNotify(0);
            }
            else
            {
                SetSashPositionAndNotify(posSashNew);
            }
        }
        else
        {
            SetSashPositionAndNotify(posSashNew);
        }

        SizeWindows();
    }      else if ((event.Moving() || event.Leaving() || event.Entering()) && (m_dragMode == wxSPLIT_DRAG_NONE))
    {
        if ( event.Leaving() || !SashHitTest(x, y) )
            OnLeaveSash();
        else
            OnEnterSash();
    }
    else if (event.Dragging() && (m_dragMode == wxSPLIT_DRAG_DRAGGING))
    {
        int diff = m_splitMode == wxSPLIT_VERTICAL ? x - m_ptStart.x : y - m_ptStart.y;

        int posSashNew = OnSashPositionChanging(m_sashStart + diff);
        if ( posSashNew == -1 )
        {
                        return;
        }

        if ( !isLive )
        {
            if ( posSashNew == m_sashPositionCurrent )
                return;

            m_sashPositionCurrent = posSashNew;

                        DrawSashTracker(m_oldX, m_oldY);

            m_oldX = (m_splitMode == wxSPLIT_VERTICAL ? m_sashPositionCurrent : x);
            m_oldY = (m_splitMode != wxSPLIT_VERTICAL ? m_sashPositionCurrent : y);

#ifdef __WXMSW__
                                                                                    if ( (short)m_oldX < 0 )
                m_oldX = 0;
            if ( (short)m_oldY < 0 )
                m_oldY = 0;
#endif 
                        DrawSashTracker(m_oldX, m_oldY);
        }
        else
        {
            if ( posSashNew == m_sashPosition )
                return;

            DoSetSashPosition(posSashNew);

                        m_requestedSashPosition = INT_MAX;
            m_needUpdating = true;
        }
    }
    else if ( event.LeftDClick() && m_windowTwo )
    {
        OnDoubleClickSash(x, y);
    }
    else
    {
        event.Skip();
    }
}

void wxSplitterWindow::OnMouseCaptureLost(wxMouseCaptureLostEvent& WXUNUSED(event))
{
    if (m_dragMode != wxSPLIT_DRAG_DRAGGING)
        return;

    m_dragMode = wxSPLIT_DRAG_NONE;

    SetCursor(* wxSTANDARD_CURSOR);

        if ( !IsLive(this) )
    {
        DrawSashTracker(m_oldX, m_oldY);
    }
}

void wxSplitterWindow::OnSize(wxSizeEvent& event)
{
                wxWindow *parent = wxGetTopLevelParent(this);
    bool iconized;

    wxTopLevelWindow *winTop = wxDynamicCast(parent, wxTopLevelWindow);
    if ( winTop )
    {
        iconized = winTop->IsIconized();
    }
    else
    {
        wxFAIL_MSG(wxT("should have a top level parent!"));

        iconized = false;
    }

    if ( iconized )
    {
        m_lastSize = wxSize(0,0);

        event.Skip();

        return;
    }

    const wxSize curSize = event.GetSize();

                        if ( m_windowTwo && m_requestedSashPosition == INT_MAX )
    {
        int size = m_splitMode == wxSPLIT_VERTICAL ? curSize.x : curSize.y;

        int old_size = m_splitMode == wxSPLIT_VERTICAL ? m_lastSize.x : m_lastSize.y;

                if ( size != old_size )
        {
            int newPosition = -1;

                        int delta = (int) ( (size - old_size)*m_sashGravity );
            if ( delta != 0 )
            {
                newPosition = m_sashPosition + delta;
                if( newPosition < m_minimumPaneSize )
                    newPosition = m_minimumPaneSize;
            }

                        newPosition = AdjustSashPosition(newPosition == -1
                                                 ? m_sashPosition
                                                 : newPosition);
            if ( newPosition != m_sashPosition )
                SetSashPositionAndNotify(newPosition);
        }
    }

    m_lastSize = curSize;

    SizeWindows();
}

void wxSplitterWindow::SetSashGravity(double gravity)
{
    wxCHECK_RET( gravity >= 0. && gravity <= 1.,
                    wxT("invalid gravity value") );

    m_sashGravity = gravity;
}

bool wxSplitterWindow::SashHitTest(int x, int y)
{
    if ( m_windowTwo == NULL || m_sashPosition == 0)
        return false; 
    int z = m_splitMode == wxSPLIT_VERTICAL ? x : y;
    int hitMax = m_sashPosition + GetSashSize() - 1;

    return z >= m_sashPosition && z <= hitMax;
}

void wxSplitterWindow::SetSashInvisible(bool invisible)
{
    if ( IsSashInvisible() != invisible )
        ToggleWindowStyle(wxSP_NOSASH);
}

int wxSplitterWindow::GetSashSize() const
{
    return IsSashInvisible() ? 0 : GetDefaultSashSize();
}

int wxSplitterWindow::GetDefaultSashSize() const
{
    return wxRendererNative::Get().GetSplitterParams(this).widthSash;
}

int wxSplitterWindow::GetBorderSize() const
{
    return wxRendererNative::Get().GetSplitterParams(this).border;
}

void wxSplitterWindow::DrawSash(wxDC& dc)
{
    if (HasFlag(wxSP_3DBORDER))
        wxRendererNative::Get().DrawSplitterBorder
                            (
                                this,
                                dc,
                                GetClientRect()
                            );

        if ( m_sashPosition == 0 || !m_windowTwo )
        return;

        if ( IsSashInvisible() )
        return;

    wxRendererNative::Get().DrawSplitterSash
                            (
                                this,
                                dc,
                                GetClientSize(),
                                m_sashPosition,
                                m_splitMode == wxSPLIT_VERTICAL ? wxVERTICAL
                                                                : wxHORIZONTAL,
                                m_isHot ? (int)wxCONTROL_CURRENT : 0
                            );
}

void wxSplitterWindow::DrawSashTracker(int x, int y)
{
    int w, h;
    GetClientSize(&w, &h);

    wxScreenDC screenDC;
    int x1, y1;
    int x2, y2;

    if ( m_splitMode == wxSPLIT_VERTICAL )
    {
        x1 = x2 = wxClip(x, 0, w) + m_sashTrackerPen->GetWidth()/2;
        y1 = 2;
        y2 = h-2;
    }
    else
    {
        y1 = y2 = wxClip(y, 0, h) + m_sashTrackerPen->GetWidth()/2;
        x1 = 2;
        x2 = w-2;
    }

    ClientToScreen(&x1, &y1);
    ClientToScreen(&x2, &y2);

    screenDC.SetLogicalFunction(wxINVERT);
    screenDC.SetPen(*m_sashTrackerPen);
    screenDC.SetBrush(*wxTRANSPARENT_BRUSH);

    screenDC.DrawLine(x1, y1, x2, y2);

    screenDC.SetLogicalFunction(wxCOPY);
}

int wxSplitterWindow::GetWindowSize() const
{
    wxSize size = GetClientSize();

    return m_splitMode == wxSPLIT_VERTICAL ? size.x : size.y;
}

int wxSplitterWindow::AdjustSashPosition(int sashPos) const
{
    wxWindow *win;

    win = GetWindow1();
    if ( win )
    {
                        int minSize = m_splitMode == wxSPLIT_VERTICAL ? win->GetMinWidth()
                                                      : win->GetMinHeight();

        if ( minSize == -1 || m_minimumPaneSize > minSize )
            minSize = m_minimumPaneSize;

        minSize += GetBorderSize();

        if ( sashPos < minSize )
            sashPos = minSize;
    }

    win = GetWindow2();
    if ( win )
    {
        int minSize = m_splitMode == wxSPLIT_VERTICAL ? win->GetMinWidth()
                                                      : win->GetMinHeight();

        if ( minSize == -1 || m_minimumPaneSize > minSize )
            minSize = m_minimumPaneSize;

        int maxSize = GetWindowSize() - minSize - GetBorderSize() - GetSashSize();
        if ( maxSize > 0 && sashPos > maxSize && maxSize >= m_minimumPaneSize)
            sashPos = maxSize;
    }

    return sashPos;
}

bool wxSplitterWindow::DoSetSashPosition(int sashPos)
{
    int newSashPosition = AdjustSashPosition(sashPos);

    if ( newSashPosition == m_sashPosition )
        return false;

    m_sashPosition = newSashPosition;

    return true;
}

void wxSplitterWindow::SetSashPositionAndNotify(int sashPos)
{
                    m_requestedSashPosition = INT_MAX;

                DoSetSashPosition(sashPos);

    wxSplitterEvent event(wxEVT_SPLITTER_SASH_POS_CHANGED, this);
    event.m_data.pos = m_sashPosition;

    (void)DoSendEvent(event);
}

void wxSplitterWindow::SizeWindows()
{
    int oldSashPosition = m_sashPosition;

        if ( m_requestedSashPosition != INT_MAX )
    {
        int newSashPosition = ConvertSashPosition(m_requestedSashPosition);
        if ( newSashPosition != m_sashPosition )
        {
            DoSetSashPosition(newSashPosition);
        }

        if ( newSashPosition <= m_sashPosition
            && newSashPosition >= m_sashPosition - GetBorderSize() )
        {
                        m_requestedSashPosition = INT_MAX;
        }
    }

    int w, h;
    GetClientSize(&w, &h);

    if ( GetWindow1() && !GetWindow2() )
    {
        GetWindow1()->SetSize(GetBorderSize(), GetBorderSize(),
                              w - 2*GetBorderSize(), h - 2*GetBorderSize());
    }
    else if ( GetWindow1() && GetWindow2() )
    {
        const int border = GetBorderSize(),
                  sash = GetSashSize();

        int size1 = GetSashPosition() - border,
            size2 = GetSashPosition() + sash;

        int x2, y2, w1, h1, w2, h2;
        if ( GetSplitMode() == wxSPLIT_VERTICAL )
        {
            w1 = size1;
            w2 = w - 2*border - sash - w1;
            if (w2 < 0)
                w2 = 0;
            h2 = h - 2*border;
            if (h2 < 0)
                h2 = 0;
            h1 = h2;
            x2 = size2;
            y2 = border;
        }
        else         {
            w2 = w - 2*border;
            if (w2 < 0)
                w2 = 0;
            w1 = w2;
            h1 = size1;
            h2 = h - 2*border - sash - h1;
            if (h2 < 0)
                h2 = 0;
            x2 = border;
            y2 = size2;
        }

        GetWindow2()->SetSize(x2, y2, w2, h2);
        GetWindow1()->SetSize(border, border, w1, h1);
    }

    if ( oldSashPosition != m_sashPosition )
    {
        wxClientDC dc(this);
        DrawSash(dc);
    }
}

void wxSplitterWindow::Initialize(wxWindow *window)
{
    wxASSERT_MSG( (!window || window->GetParent() == this),
                  wxT("windows in the splitter should have it as parent!") );

    if (window && !window->IsShown())
        window->Show();

    m_windowOne = window;
    m_windowTwo = NULL;
    DoSetSashPosition(0);
}

bool wxSplitterWindow::DoSplit(wxSplitMode mode,
                               wxWindow *window1, wxWindow *window2,
                               int sashPosition)
{
    if ( IsSplit() )
        return false;

    wxCHECK_MSG( window1 && window2, false,
                 wxT("cannot split with NULL window(s)") );

    wxCHECK_MSG( window1->GetParent() == this && window2->GetParent() == this, false,
                  wxT("windows in the splitter should have it as parent!") );

    if (! window1->IsShown())
        window1->Show();
    if (! window2->IsShown())
        window2->Show();

    m_splitMode = mode;
    m_windowOne = window1;
    m_windowTwo = window2;


    SetSashPosition(sashPosition, true);
    return true;
}

int wxSplitterWindow::ConvertSashPosition(int sashPosition) const
{
    if ( sashPosition > 0 )
    {
        return sashPosition;
    }
    else if ( sashPosition < 0 )
    {
                return GetWindowSize() + sashPosition;
    }
    else     {
                return GetWindowSize() / 2;
    }
}

bool wxSplitterWindow::Unsplit(wxWindow *toRemove)
{
    if ( ! IsSplit() )
        return false;

    wxWindow *win;
    if ( toRemove == NULL || toRemove == m_windowTwo)
    {
        win = m_windowTwo ;
        m_windowTwo = NULL;
    }
    else if ( toRemove == m_windowOne )
    {
        win = m_windowOne ;
        m_windowOne = m_windowTwo;
        m_windowTwo = NULL;
    }
    else
    {
        wxFAIL_MSG(wxT("splitter: attempt to remove a non-existent window"));

        return false;
    }

    OnUnsplit(win);
    DoSetSashPosition(0);
    SizeWindows();

    return true;
}

bool wxSplitterWindow::ReplaceWindow(wxWindow *winOld, wxWindow *winNew)
{
    wxCHECK_MSG( winOld, false, wxT("use one of Split() functions instead") );
    wxCHECK_MSG( winNew, false, wxT("use Unsplit() functions instead") );

    if ( winOld == m_windowTwo )
    {
        m_windowTwo = winNew;
    }
    else if ( winOld == m_windowOne )
    {
        m_windowOne = winNew;
    }
    else
    {
        wxFAIL_MSG(wxT("splitter: attempt to replace a non-existent window"));

        return false;
    }

    SizeWindows();

    return true;
}

void wxSplitterWindow::SetMinimumPaneSize(int min)
{
    m_minimumPaneSize = min;
    int pos = m_requestedSashPosition != INT_MAX ? m_requestedSashPosition : m_sashPosition;
    SetSashPosition(pos); }

void wxSplitterWindow::SetSashPosition(int position, bool redraw)
{
            m_requestedSashPosition = position;

    DoSetSashPosition(ConvertSashPosition(position));

    if ( redraw )
    {
        SizeWindows();
    }
}

void wxSplitterWindow::UpdateSize()
{
    SizeWindows();
}

bool wxSplitterWindow::DoSendEvent(wxSplitterEvent& event)
{
    return !GetEventHandler()->ProcessEvent(event) || event.IsAllowed();
}

wxSize wxSplitterWindow::DoGetBestSize() const
{
        wxSize size1, size2;
    if ( m_windowOne )
        size1 = m_windowOne->GetEffectiveMinSize();
    if ( m_windowTwo )
        size2 = m_windowTwo->GetEffectiveMinSize();

                int *pSash;
    wxSize sizeBest;
    if ( m_splitMode == wxSPLIT_VERTICAL )
    {
        sizeBest.y = wxMax(size1.y, size2.y);
        sizeBest.x = wxMax(size1.x, m_minimumPaneSize) +
                        wxMax(size2.x, m_minimumPaneSize);

        pSash = &sizeBest.x;
    }
    else     {
        sizeBest.x = wxMax(size1.x, size2.x);
        sizeBest.y = wxMax(size1.y, m_minimumPaneSize) +
                        wxMax(size2.y, m_minimumPaneSize);

        pSash = &sizeBest.y;
    }

        if ( m_windowOne && m_windowTwo )
        *pSash += GetSashSize();

        int border = 2*GetBorderSize();
    sizeBest.x += border;
    sizeBest.y += border;

    return sizeBest;
}


bool wxSplitterWindow::OnSashPositionChange(int WXUNUSED(newSashPosition))
{
        return true;
}

int wxSplitterWindow::OnSashPositionChanging(int newSashPosition)
{
        const int UNSPLIT_THRESHOLD = 4;

        if ( !OnSashPositionChange(newSashPosition) )
    {
                return -1;
    }

        int window_size = GetWindowSize();

    bool unsplit_scenario = false;
    if ( m_permitUnsplitAlways || m_minimumPaneSize == 0 )
    {
                if ( newSashPosition <= UNSPLIT_THRESHOLD )
        {
                        newSashPosition = 0;
            unsplit_scenario = true;
        }
        if ( newSashPosition >= window_size - UNSPLIT_THRESHOLD )
        {
                        newSashPosition = window_size;
            unsplit_scenario = true;
        }
    }

    if ( !unsplit_scenario )
    {
                newSashPosition = AdjustSashPosition(newSashPosition);

                        if ( newSashPosition < 0 || newSashPosition > window_size )
            newSashPosition = window_size / 2;
    }

                    wxSplitterEvent event(wxEVT_SPLITTER_SASH_POS_CHANGING, this);
    event.m_data.pos = newSashPosition;

    if ( !DoSendEvent(event) )
    {
                newSashPosition = -1;
    }
    else
    {
                newSashPosition = event.GetSashPosition();
    }

    return newSashPosition;
}

void wxSplitterWindow::OnDoubleClickSash(int x, int y)
{
    wxCHECK_RET(m_windowTwo, wxT("splitter: no window to remove"));

        wxSplitterEvent event(wxEVT_SPLITTER_DOUBLECLICKED, this);
    event.m_data.pt.x = x;
    event.m_data.pt.y = y;
    if ( DoSendEvent(event) )
    {
        if ( GetMinimumPaneSize() == 0 || m_permitUnsplitAlways )
        {
            wxWindow* win = m_windowTwo;
            if ( Unsplit(win) )
            {
                wxSplitterEvent unsplitEvent(wxEVT_SPLITTER_UNSPLIT, this);
                unsplitEvent.m_data.win = win;
                (void)DoSendEvent(unsplitEvent);
            }
        }
    }
    }

void wxSplitterWindow::OnUnsplit(wxWindow *winRemoved)
{
        winRemoved->Show(false);
}

#if defined( __WXMSW__ ) || defined( __WXMAC__)

void wxSplitterWindow::OnSetCursor(wxSetCursorEvent& event)
{
            
    if ( SashHitTest(event.GetX(), event.GetY()) )
    {
                event.Skip();
    }
    }

#endif 
#endif 
