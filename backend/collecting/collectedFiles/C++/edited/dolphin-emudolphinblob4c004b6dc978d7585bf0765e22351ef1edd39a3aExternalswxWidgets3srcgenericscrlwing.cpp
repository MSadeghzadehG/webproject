


#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#include "wx/scrolwin.h"

#ifndef WX_PRECOMP
    #include "wx/utils.h"
    #include "wx/panel.h"
    #include "wx/dcclient.h"
    #include "wx/timer.h"
    #include "wx/sizer.h"
    #include "wx/settings.h"
#endif

#ifdef __WXMAC__
#include "wx/scrolbar.h"
#endif

#ifdef __WXMSW__
    #include <windows.h>     #include "wx/msw/winundef.h"
#endif

#ifdef __WXMOTIF__
#ifdef __VMS__                # pragma message disable nosimpint
#endif
#include <Xm/Xm.h>
#ifdef __VMS__
# pragma message enable nosimpint
#endif
#endif




class WXDLLEXPORT wxScrollHelperEvtHandler : public wxEvtHandler
{
public:
    wxScrollHelperEvtHandler(wxScrollHelperBase *scrollHelper)
    {
        m_scrollHelper = scrollHelper;
    }

    virtual bool ProcessEvent(wxEvent& event) wxOVERRIDE;

private:
    wxScrollHelperBase *m_scrollHelper;

    wxDECLARE_NO_COPY_CLASS(wxScrollHelperEvtHandler);
};

#if wxUSE_TIMER

class wxAutoScrollTimer : public wxTimer
{
public:
    wxAutoScrollTimer(wxWindow *winToScroll,
                      wxScrollHelperBase *scroll,
                      wxEventType eventTypeToSend,
                      int pos, int orient);

    virtual void Notify() wxOVERRIDE;

private:
    wxWindow *m_win;
    wxScrollHelperBase *m_scrollHelper;
    wxEventType m_eventType;
    int m_pos,
        m_orient;

    wxDECLARE_NO_COPY_CLASS(wxAutoScrollTimer);
};



wxAutoScrollTimer::wxAutoScrollTimer(wxWindow *winToScroll,
                                     wxScrollHelperBase *scroll,
                                     wxEventType eventTypeToSend,
                                     int pos, int orient)
{
    m_win = winToScroll;
    m_scrollHelper = scroll;
    m_eventType = eventTypeToSend;
    m_pos = pos;
    m_orient = orient;
}

void wxAutoScrollTimer::Notify()
{
        if ( wxWindow::GetCapture() != m_win )
    {
        Stop();
    }
    else     {
                wxScrollWinEvent event1(m_eventType, m_pos, m_orient);
        event1.SetEventObject(m_win);
        event1.SetId(m_win->GetId());
        if ( m_scrollHelper->SendAutoScrollEvents(event1) &&
                m_win->GetEventHandler()->ProcessEvent(event1) )
        {
                        wxMouseEvent event2(wxEVT_MOTION);
            event2.SetPosition(wxGetMousePosition());

                                    wxWindow *parentTop = m_win;
            while ( parentTop->GetParent() )
                parentTop = parentTop->GetParent();
            wxPoint ptOrig = parentTop->GetPosition();
            event2.m_x -= ptOrig.x;
            event2.m_y -= ptOrig.y;

            event2.SetEventObject(m_win);

            wxMouseState mouseState = wxGetMouseState();

            event2.m_leftDown = mouseState.LeftIsDown();
            event2.m_middleDown = mouseState.MiddleIsDown();
            event2.m_rightDown = mouseState.RightIsDown();

            event2.m_shiftDown = mouseState.ShiftDown();
            event2.m_controlDown = mouseState.ControlDown();
            event2.m_altDown = mouseState.AltDown();
            event2.m_metaDown = mouseState.MetaDown();

            m_win->GetEventHandler()->ProcessEvent(event2);
        }
        else         {
            Stop();
        }
    }
}
#endif


bool wxScrollHelperEvtHandler::ProcessEvent(wxEvent& event)
{
    wxEventType evType = event.GetEventType();

                            bool processed = m_nextHandler->ProcessEvent(event);

                                if ( evType == wxEVT_SIZE )
    {
        m_scrollHelper->HandleOnSize((wxSizeEvent &)event);
        return true;
    }

                if (
#ifndef __WXUNIVERSAL__
                                                            !processed &&
#endif             evType == wxEVT_PAINT )
    {
        m_scrollHelper->HandleOnPaint((wxPaintEvent &)event);
        return true;
    }

            if ( processed )
        return true;

    if ( evType == wxEVT_CHILD_FOCUS )
    {
        m_scrollHelper->HandleOnChildFocus((wxChildFocusEvent &)event);
        return true;
    }

            bool wasSkipped = event.GetSkipped();
    if ( wasSkipped )
        event.Skip(false);

    if ( evType == wxEVT_SCROLLWIN_TOP ||
         evType == wxEVT_SCROLLWIN_BOTTOM ||
         evType == wxEVT_SCROLLWIN_LINEUP ||
         evType == wxEVT_SCROLLWIN_LINEDOWN ||
         evType == wxEVT_SCROLLWIN_PAGEUP ||
         evType == wxEVT_SCROLLWIN_PAGEDOWN ||
         evType == wxEVT_SCROLLWIN_THUMBTRACK ||
         evType == wxEVT_SCROLLWIN_THUMBRELEASE )
    {
        m_scrollHelper->HandleOnScroll((wxScrollWinEvent &)event);
        if ( !event.GetSkipped() )
        {
                                                            processed = true;
            wasSkipped = false;
        }
    }

    if ( evType == wxEVT_ENTER_WINDOW )
    {
        m_scrollHelper->HandleOnMouseEnter((wxMouseEvent &)event);
    }
    else if ( evType == wxEVT_LEAVE_WINDOW )
    {
        m_scrollHelper->HandleOnMouseLeave((wxMouseEvent &)event);
    }
#if wxUSE_MOUSEWHEEL
    #ifndef __WXGTK20__
    else if ( evType == wxEVT_MOUSEWHEEL )
    {
        m_scrollHelper->HandleOnMouseWheel((wxMouseEvent &)event);
        return true;
    }
#endif
#endif     else if ( evType == wxEVT_CHAR )
    {
        m_scrollHelper->HandleOnChar((wxKeyEvent &)event);
        if ( !event.GetSkipped() )
        {
            processed = true;
            wasSkipped = false;
        }
    }

    event.Skip(wasSkipped);

                                        event.DidntHonourProcessOnlyIn();

    return processed;
}



wxAnyScrollHelperBase::wxAnyScrollHelperBase(wxWindow* win)
{
    wxASSERT_MSG( win, wxT("associated window can't be NULL in wxScrollHelper") );

    m_win = win;
    m_targetWindow = NULL;

    m_kbdScrollingEnabled = true;
}


wxScrollHelperBase::wxScrollHelperBase(wxWindow *win)
    : wxAnyScrollHelperBase(win)
{
    m_xScrollPixelsPerLine =
    m_yScrollPixelsPerLine =
    m_xScrollPosition =
    m_yScrollPosition =
    m_xScrollLines =
    m_yScrollLines =
    m_xScrollLinesPerPage =
    m_yScrollLinesPerPage = 0;

    m_xScrollingEnabled =
    m_yScrollingEnabled = true;

    m_scaleX =
    m_scaleY = 1.0;
#if wxUSE_MOUSEWHEEL
    m_wheelRotation = 0;
#endif

    m_timerAutoScroll = NULL;

    m_handler = NULL;

    m_win->SetScrollHelper(static_cast<wxScrollHelper *>(this));

        DoSetTargetWindow(win);
}

wxScrollHelperBase::~wxScrollHelperBase()
{
    StopAutoScrolling();

    DeleteEvtHandler();
}


void wxScrollHelperBase::SetScrollbars(int pixelsPerUnitX,
                                       int pixelsPerUnitY,
                                       int noUnitsX,
                                       int noUnitsY,
                                       int xPos,
                                       int yPos,
                                       bool noRefresh)
{
        int xPosInPixels = (xPos + m_xScrollPosition)*m_xScrollPixelsPerLine,
        yPosInPixels = (yPos + m_yScrollPosition)*m_yScrollPixelsPerLine;

    bool do_refresh =
    (
      (noUnitsX != 0 && m_xScrollLines == 0) ||
      (noUnitsX < m_xScrollLines && xPosInPixels > pixelsPerUnitX * noUnitsX) ||

      (noUnitsY != 0 && m_yScrollLines == 0) ||
      (noUnitsY < m_yScrollLines && yPosInPixels > pixelsPerUnitY * noUnitsY) ||
      (xPos != m_xScrollPosition) ||
      (yPos != m_yScrollPosition)
    );

    m_xScrollPixelsPerLine = pixelsPerUnitX;
    m_yScrollPixelsPerLine = pixelsPerUnitY;
    m_xScrollPosition = xPos;
    m_yScrollPosition = yPos;

    int w = noUnitsX * pixelsPerUnitX;
    int h = noUnitsY * pixelsPerUnitY;

                
    
                m_targetWindow->SetVirtualSize( w ? w : wxDefaultCoord,
                                    h ? h : wxDefaultCoord);

    if (do_refresh && !noRefresh)
        m_targetWindow->Refresh(true, GetScrollRect());

#ifndef __WXUNIVERSAL__
                if ( m_targetWindow != m_win )
#endif     {
        AdjustScrollbars();
    }
#ifndef __WXUNIVERSAL__
    else
    {
            }
#endif }


void wxScrollHelperBase::DeleteEvtHandler()
{
        if ( m_win && m_handler )
    {
        if ( m_win->RemoveEventHandler(m_handler) )
        {
            delete m_handler;
        }
                
        m_handler = NULL;
    }
}

void wxScrollHelperBase::DoSetTargetWindow(wxWindow *target)
{
    m_targetWindow = target;
#ifdef __WXMAC__
    target->MacSetClipChildren( true ) ;
#endif

                if ( m_targetWindow == m_win )
    {
                DeleteEvtHandler();

        m_handler = new wxScrollHelperEvtHandler(this);
        m_targetWindow->PushEventHandler(m_handler);
    }
}

void wxScrollHelperBase::SetTargetWindow(wxWindow *target)
{
    wxCHECK_RET( target, wxT("target window must not be NULL") );

    if ( target == m_targetWindow )
        return;

    DoSetTargetWindow(target);
}


void wxScrollHelperBase::HandleOnScroll(wxScrollWinEvent& event)
{
    int nScrollInc = CalcScrollInc(event);
    if ( nScrollInc == 0 )
    {
                event.Skip();

        return;
    }

    bool needsRefresh = false;
    int dx = 0,
        dy = 0;
    int orient = event.GetOrientation();
    if (orient == wxHORIZONTAL)
    {
       if ( m_xScrollingEnabled )
       {
           dx = -m_xScrollPixelsPerLine * nScrollInc;
       }
       else
       {
           needsRefresh = true;
       }
    }
    else
    {
        if ( m_yScrollingEnabled )
        {
            dy = -m_yScrollPixelsPerLine * nScrollInc;
        }
        else
        {
            needsRefresh = true;
        }
    }

    if ( !needsRefresh )
    {
                        #ifdef __WXMAC__
                #else
        m_targetWindow->Update();
#endif
    }

    if (orient == wxHORIZONTAL)
    {
        m_xScrollPosition += nScrollInc;
        m_win->SetScrollPos(wxHORIZONTAL, m_xScrollPosition);
    }
    else
    {
        m_yScrollPosition += nScrollInc;
        m_win->SetScrollPos(wxVERTICAL, m_yScrollPosition);
    }

    if ( needsRefresh )
    {
        m_targetWindow->Refresh(true, GetScrollRect());
    }
    else
    {
        m_targetWindow->ScrollWindow(dx, dy, GetScrollRect());
    }
}

int wxScrollHelperBase::CalcScrollInc(wxScrollWinEvent& event)
{
    int pos = event.GetPosition();
    int orient = event.GetOrientation();

    int nScrollInc = 0;
    if (event.GetEventType() == wxEVT_SCROLLWIN_TOP)
    {
            if (orient == wxHORIZONTAL)
                nScrollInc = - m_xScrollPosition;
            else
                nScrollInc = - m_yScrollPosition;
    } else
    if (event.GetEventType() == wxEVT_SCROLLWIN_BOTTOM)
    {
            if (orient == wxHORIZONTAL)
                nScrollInc = m_xScrollLines - m_xScrollPosition;
            else
                nScrollInc = m_yScrollLines - m_yScrollPosition;
    } else
    if (event.GetEventType() == wxEVT_SCROLLWIN_LINEUP)
    {
            nScrollInc = -1;
    } else
    if (event.GetEventType() == wxEVT_SCROLLWIN_LINEDOWN)
    {
            nScrollInc = 1;
    } else
    if (event.GetEventType() == wxEVT_SCROLLWIN_PAGEUP)
    {
            if (orient == wxHORIZONTAL)
                nScrollInc = -GetScrollPageSize(wxHORIZONTAL);
            else
                nScrollInc = -GetScrollPageSize(wxVERTICAL);
    } else
    if (event.GetEventType() == wxEVT_SCROLLWIN_PAGEDOWN)
    {
            if (orient == wxHORIZONTAL)
                nScrollInc = GetScrollPageSize(wxHORIZONTAL);
            else
                nScrollInc = GetScrollPageSize(wxVERTICAL);
    } else
    if ((event.GetEventType() == wxEVT_SCROLLWIN_THUMBTRACK) ||
        (event.GetEventType() == wxEVT_SCROLLWIN_THUMBRELEASE))
    {
            if (orient == wxHORIZONTAL)
                nScrollInc = pos - m_xScrollPosition;
            else
                nScrollInc = pos - m_yScrollPosition;
    }

    if (orient == wxHORIZONTAL)
    {
        if ( m_xScrollPosition + nScrollInc < 0 )
        {
                        nScrollInc = -m_xScrollPosition;
        }
        else         {
            const int posMax = m_xScrollLines - m_xScrollLinesPerPage;
            if ( m_xScrollPosition + nScrollInc > posMax )
            {
                                nScrollInc = posMax - m_xScrollPosition;
            }
        }
    }
    else     {
        if ( m_yScrollPosition + nScrollInc < 0 )
        {
                        nScrollInc = -m_yScrollPosition;
        }
        else         {
            const int posMax = m_yScrollLines - m_yScrollLinesPerPage;
            if ( m_yScrollPosition + nScrollInc > posMax )
            {
                                nScrollInc = posMax - m_yScrollPosition;
            }
        }
    }

    return nScrollInc;
}

void wxScrollHelperBase::DoPrepareDC(wxDC& dc)
{
    wxPoint pt = dc.GetDeviceOrigin();
#ifdef __WXGTK__
                if (m_win->GetLayoutDirection() == wxLayout_RightToLeft)
        dc.SetDeviceOrigin( pt.x + m_xScrollPosition * m_xScrollPixelsPerLine,
                            pt.y - m_yScrollPosition * m_yScrollPixelsPerLine );
    else
#endif
        dc.SetDeviceOrigin( pt.x - m_xScrollPosition * m_xScrollPixelsPerLine,
                            pt.y - m_yScrollPosition * m_yScrollPixelsPerLine );
    dc.SetUserScale( m_scaleX, m_scaleY );
}

void wxScrollHelperBase::SetScrollRate( int xstep, int ystep )
{
    int old_x = m_xScrollPixelsPerLine * m_xScrollPosition;
    int old_y = m_yScrollPixelsPerLine * m_yScrollPosition;

    m_xScrollPixelsPerLine = xstep;
    m_yScrollPixelsPerLine = ystep;

    int new_x = m_xScrollPixelsPerLine * m_xScrollPosition;
    int new_y = m_yScrollPixelsPerLine * m_yScrollPosition;

    m_win->SetScrollPos( wxHORIZONTAL, m_xScrollPosition );
    m_win->SetScrollPos( wxVERTICAL, m_yScrollPosition );
    m_targetWindow->ScrollWindow( old_x - new_x, old_y - new_y );

    AdjustScrollbars();
}

void wxScrollHelperBase::GetScrollPixelsPerUnit (int *x_unit, int *y_unit) const
{
    if ( x_unit )
        *x_unit = m_xScrollPixelsPerLine;
    if ( y_unit )
        *y_unit = m_yScrollPixelsPerLine;
}


int wxScrollHelperBase::GetScrollLines( int orient ) const
{
    if ( orient == wxHORIZONTAL )
        return m_xScrollLines;
    else
        return m_yScrollLines;
}

int wxScrollHelperBase::GetScrollPageSize(int orient) const
{
    if ( orient == wxHORIZONTAL )
        return m_xScrollLinesPerPage;
    else
        return m_yScrollLinesPerPage;
}

void wxScrollHelperBase::SetScrollPageSize(int orient, int pageSize)
{
    if ( orient == wxHORIZONTAL )
        m_xScrollLinesPerPage = pageSize;
    else
        m_yScrollLinesPerPage = pageSize;
}

void wxScrollHelperBase::EnableScrolling (bool x_scroll, bool y_scroll)
{
    m_xScrollingEnabled = x_scroll;
    m_yScrollingEnabled = y_scroll;
}

void wxScrollHelperBase::DoGetViewStart (int *x, int *y) const
{
    if ( x )
        *x = m_xScrollPosition;
    if ( y )
        *y = m_yScrollPosition;
}

void wxScrollHelperBase::DoCalcScrolledPosition(int x, int y,
                                                int *xx, int *yy) const
{
    if ( xx )
        *xx = x - m_xScrollPosition * m_xScrollPixelsPerLine;
    if ( yy )
        *yy = y - m_yScrollPosition * m_yScrollPixelsPerLine;
}

void wxScrollHelperBase::DoCalcUnscrolledPosition(int x, int y,
                                                  int *xx, int *yy) const
{
    if ( xx )
        *xx = x + m_xScrollPosition * m_xScrollPixelsPerLine;
    if ( yy )
        *yy = y + m_yScrollPosition * m_yScrollPixelsPerLine;
}


bool wxScrollHelperBase::ScrollLayout()
{
    if ( m_win->GetSizer() && m_targetWindow == m_win )
    {
                
        int x = 0, y = 0, w = 0, h = 0;
        CalcScrolledPosition(0,0, &x,&y);
        m_win->GetVirtualSize(&w, &h);
        m_win->GetSizer()->SetDimension(x, y, w, h);
        return true;
    }

        return m_win->wxWindow::Layout();
}

void wxScrollHelperBase::ScrollDoSetVirtualSize(int x, int y)
{
    m_win->wxWindow::DoSetVirtualSize( x, y );
    AdjustScrollbars();

    if (m_win->GetAutoLayout())
        m_win->Layout();
}

wxSize wxScrollHelperBase::ScrollGetBestVirtualSize() const
{
    wxSize clientSize(m_win->GetClientSize());
    if ( m_win->GetSizer() )
        clientSize.IncTo(m_win->GetSizer()->CalcMin());

    return clientSize;
}


void wxScrollHelperBase::HandleOnSize(wxSizeEvent& WXUNUSED(event))
{
    if ( m_targetWindow->GetAutoLayout() )
    {
        wxSize size = m_targetWindow->GetBestVirtualSize();

                m_win->SetVirtualSize( size );
    }
    else
    {
        AdjustScrollbars();
    }
}

void wxAnyScrollHelperBase::HandleOnPaint(wxPaintEvent& WXUNUSED(event))
{
        wxPaintDC dc(m_win);
    DoPrepareDC(dc);

    OnDraw(dc);
}

void wxAnyScrollHelperBase::HandleOnChar(wxKeyEvent& event)
{
    if ( !m_kbdScrollingEnabled )
    {
        event.Skip();
        return;
    }

        wxScrollWinEvent newEvent;

    newEvent.SetPosition(0);
    newEvent.SetEventObject(m_win);
    newEvent.SetId(m_win->GetId());

        newEvent.SetOrientation(wxVERTICAL);

                bool sendHorizontalToo = false;

    switch ( event.GetKeyCode() )
    {
        case WXK_PAGEUP:
            newEvent.SetEventType(wxEVT_SCROLLWIN_PAGEUP);
            break;

        case WXK_PAGEDOWN:
            newEvent.SetEventType(wxEVT_SCROLLWIN_PAGEDOWN);
            break;

        case WXK_HOME:
            newEvent.SetEventType(wxEVT_SCROLLWIN_TOP);

            sendHorizontalToo = event.ControlDown();
            break;

        case WXK_END:
            newEvent.SetEventType(wxEVT_SCROLLWIN_BOTTOM);

            sendHorizontalToo = event.ControlDown();
            break;

        case WXK_LEFT:
            newEvent.SetOrientation(wxHORIZONTAL);
            wxFALLTHROUGH;

        case WXK_UP:
            newEvent.SetEventType(wxEVT_SCROLLWIN_LINEUP);
            break;

        case WXK_RIGHT:
            newEvent.SetOrientation(wxHORIZONTAL);
            wxFALLTHROUGH;

        case WXK_DOWN:
            newEvent.SetEventType(wxEVT_SCROLLWIN_LINEDOWN);
            break;

        default:
                        event.Skip();
            return;
    }

    m_win->ProcessWindowEvent(newEvent);

    if ( sendHorizontalToo )
    {
        newEvent.SetOrientation(wxHORIZONTAL);
        m_win->ProcessWindowEvent(newEvent);
    }
}


bool wxScrollHelperBase::SendAutoScrollEvents(wxScrollWinEvent& event) const
{
        wxWindow *win = (wxWindow *)event.GetEventObject();
    return win->HasScrollbar(event.GetOrientation());
}

void wxScrollHelperBase::StopAutoScrolling()
{
#if wxUSE_TIMER
    wxDELETE(m_timerAutoScroll);
#endif
}

void wxScrollHelperBase::HandleOnMouseEnter(wxMouseEvent& event)
{
    StopAutoScrolling();

    event.Skip();
}

void wxScrollHelperBase::HandleOnMouseLeave(wxMouseEvent& event)
{
        event.Skip();

                if ( wxWindow::GetCapture() == m_targetWindow )
    {
                int pos, orient;
        wxPoint pt = event.GetPosition();
        if ( pt.x < 0 )
        {
            orient = wxHORIZONTAL;
            pos = 0;
        }
        else if ( pt.y < 0 )
        {
            orient = wxVERTICAL;
            pos = 0;
        }
        else         {
            wxSize size = m_targetWindow->GetClientSize();
            if ( pt.x > size.x )
            {
                orient = wxHORIZONTAL;
                pos = m_xScrollLines;
            }
            else if ( pt.y > size.y )
            {
                orient = wxVERTICAL;
                pos = m_yScrollLines;
            }
            else             {
                                
                
                return;
            }
        }

                        if ( !m_targetWindow->HasScrollbar(orient) )
            return;

#if wxUSE_TIMER
        delete m_timerAutoScroll;
        m_timerAutoScroll = new wxAutoScrollTimer
                                (
                                    m_targetWindow, this,
                                    pos == 0 ? wxEVT_SCROLLWIN_LINEUP
                                             : wxEVT_SCROLLWIN_LINEDOWN,
                                    pos,
                                    orient
                                );
        m_timerAutoScroll->Start(50); #else
        wxUnusedVar(pos);
#endif
    }
}

#if wxUSE_MOUSEWHEEL

void wxScrollHelperBase::HandleOnMouseWheel(wxMouseEvent& event)
{
    m_wheelRotation += event.GetWheelRotation();
    int lines = m_wheelRotation / event.GetWheelDelta();
    m_wheelRotation -= lines * event.GetWheelDelta();

    if (lines != 0)
    {

        wxScrollWinEvent newEvent;

        newEvent.SetPosition(0);
        newEvent.SetOrientation( event.GetWheelAxis() == 0 ? wxVERTICAL : wxHORIZONTAL);
        newEvent.SetEventObject(m_win);

        if ( event.GetWheelAxis() == wxMOUSE_WHEEL_HORIZONTAL )
            lines = -lines;

        if (event.IsPageScroll())
        {
            if (lines > 0)
                newEvent.SetEventType(wxEVT_SCROLLWIN_PAGEUP);
            else
                newEvent.SetEventType(wxEVT_SCROLLWIN_PAGEDOWN);

            m_win->GetEventHandler()->ProcessEvent(newEvent);
        }
        else
        {
            lines *= event.GetLinesPerAction();
            if (lines > 0)
                newEvent.SetEventType(wxEVT_SCROLLWIN_LINEUP);
            else
                newEvent.SetEventType(wxEVT_SCROLLWIN_LINEDOWN);

            int times = abs(lines);
            for (; times > 0; times--)
                m_win->GetEventHandler()->ProcessEvent(newEvent);
        }
    }
}

#endif 
void wxScrollHelperBase::HandleOnChildFocus(wxChildFocusEvent& event)
{
            event.Skip();

        wxWindow *win = event.GetWindow();

    if ( win == m_targetWindow )
        return; 
#if defined( __WXOSX__ ) && wxUSE_SCROLLBAR
    if (wxDynamicCast(win, wxScrollBar))
        return;
#endif

                                                                                
    wxWindow *actual_focus=wxWindow::FindFocus();
    if (win != actual_focus &&
        wxDynamicCast(win, wxPanel) != 0 &&
        win->GetParent() == m_targetWindow)
                        return;

    const wxRect viewRect(m_targetWindow->GetClientRect());

                                        if ( win->GetParent() != m_targetWindow)
    {
        wxWindow *parent=win->GetParent();
        wxSize parent_size=parent->GetSize();
        if (parent_size.GetWidth() <= viewRect.GetWidth() &&
            parent_size.GetHeight() <= viewRect.GetHeight())
                        win=parent;
    }

            const wxRect
        winRect(m_targetWindow->ScreenToClient(win->GetScreenPosition()),
                win->GetSize());

        if ( viewRect.Contains(winRect) )
    {
                return;
    }

            if ( winRect.GetWidth() > viewRect.GetWidth() ||
            winRect.GetHeight() > viewRect.GetHeight() )
    {
                        return;
    }


        int stepx, stepy;
    GetScrollPixelsPerUnit(&stepx, &stepy);

    int startx, starty;
    GetViewStart(&startx, &starty);

        if ( stepy > 0 )
    {
        int diff = 0;

        if ( winRect.GetTop() < 0 )
        {
            diff = winRect.GetTop();
        }
        else if ( winRect.GetBottom() > viewRect.GetHeight() )
        {
            diff = winRect.GetBottom() - viewRect.GetHeight() + 1;
                                    diff += stepy - 1;
        }

        starty = (starty * stepy + diff) / stepy;
    }

        if ( stepx > 0 )
    {
        int diff = 0;

        if ( winRect.GetLeft() < 0 )
        {
            diff = winRect.GetLeft();
        }
        else if ( winRect.GetRight() > viewRect.GetWidth() )
        {
            diff = winRect.GetRight() - viewRect.GetWidth() + 1;
                                    diff += stepx - 1;
        }

        startx = (startx * stepx + diff) / stepx;
    }

    Scroll(startx, starty);
}


#ifdef wxHAS_GENERIC_SCROLLWIN


wxScrollHelper::wxScrollHelper(wxWindow *winToScroll)
    : wxScrollHelperBase(winToScroll)
{
    m_xVisibility =
    m_yVisibility = wxSHOW_SB_DEFAULT;
    m_adjustScrollFlagReentrancy = 0;
}

bool wxScrollHelper::IsScrollbarShown(int orient) const
{
    wxScrollbarVisibility visibility = orient == wxHORIZONTAL ? m_xVisibility
                                                              : m_yVisibility;

    return visibility != wxSHOW_SB_NEVER;
}

void wxScrollHelper::DoShowScrollbars(wxScrollbarVisibility horz,
                                      wxScrollbarVisibility vert)
{
    if ( horz != m_xVisibility || vert != m_yVisibility )
    {
        m_xVisibility = horz;
        m_yVisibility = vert;

        AdjustScrollbars();
    }
}

void
wxScrollHelper::DoAdjustScrollbar(int orient,
                                  int clientSize,
                                  int virtSize,
                                  int pixelsPerUnit,
                                  int& scrollUnits,
                                  int& scrollPosition,
                                  int& scrollLinesPerPage,
                                  wxScrollbarVisibility visibility)
{
            if ( pixelsPerUnit == 0 || clientSize >= virtSize )
    {
                scrollUnits =
        scrollPosition = 0;
        scrollLinesPerPage = 0;
    }
    else     {
                scrollUnits = (virtSize + pixelsPerUnit - 1) / pixelsPerUnit;

                scrollLinesPerPage = clientSize / pixelsPerUnit;

        if ( scrollLinesPerPage >= scrollUnits )
        {
                        scrollUnits =
            scrollPosition = 0;
            scrollLinesPerPage = 0;
        }
        else         {
            if ( scrollLinesPerPage < 1 )
                scrollLinesPerPage = 1;

                                    const int posMax = scrollUnits - scrollLinesPerPage;
            if ( scrollPosition > posMax )
                scrollPosition = posMax;
            else if ( scrollPosition < 0 )
                scrollPosition = 0;
        }
    }

                int range;
    switch ( visibility )
    {
        case wxSHOW_SB_NEVER:
            range = 0;
            break;

        case wxSHOW_SB_ALWAYS:
            range = scrollUnits ? scrollUnits : -1;
            break;

        default:
            wxFAIL_MSG( wxS("unknown scrollbar visibility") );
            wxFALLTHROUGH;

        case wxSHOW_SB_DEFAULT:
            range = scrollUnits;
            break;

    }

    m_win->SetScrollbar(orient, scrollPosition, scrollLinesPerPage, range);
}

void wxScrollHelper::AdjustScrollbars()
{
    wxRecursionGuard guard(m_adjustScrollFlagReentrancy);
    if ( guard.IsInside() )
    {
                                                        return;
    }

    int oldXScroll = m_xScrollPosition;
    int oldYScroll = m_yScrollPosition;

                            for ( int iterationCount = 0; iterationCount < 5; iterationCount++ )
    {
        wxSize clientSize = GetTargetSize();
        const wxSize virtSize = m_targetWindow->GetVirtualSize();

                                                                        const wxSize availSize = GetSizeAvailableForScrollTarget(
            m_win->GetSize() - m_win->GetWindowBorderSize());
        if ( availSize != clientSize )
        {
            if ( availSize.x >= virtSize.x && availSize.y >= virtSize.y )
            {
                                                                clientSize = availSize;
            }
        }


        DoAdjustScrollbar(wxHORIZONTAL,
                          clientSize.x,
                          virtSize.x,
                          m_xScrollPixelsPerLine,
                          m_xScrollLines,
                          m_xScrollPosition,
                          m_xScrollLinesPerPage,
                          m_xVisibility);

        DoAdjustScrollbar(wxVERTICAL,
                          clientSize.y,
                          virtSize.y,
                          m_yScrollPixelsPerLine,
                          m_yScrollLines,
                          m_yScrollPosition,
                          m_yScrollLinesPerPage,
                          m_yVisibility);


                        if ( GetTargetSize() == clientSize )
            break;
    }

#ifdef __WXMOTIF__
                
    if ( m_targetWindow->GetWindowStyle() & wxRETAINED )
    {
        Display* dpy = XtDisplay((Widget)m_targetWindow->GetMainWidget());

        int totalPixelWidth = m_xScrollLines * m_xScrollPixelsPerLine;
        int totalPixelHeight = m_yScrollLines * m_yScrollPixelsPerLine;
        if (m_targetWindow->GetBackingPixmap() &&
           !((m_targetWindow->GetPixmapWidth() == totalPixelWidth) &&
             (m_targetWindow->GetPixmapHeight() == totalPixelHeight)))
        {
            XFreePixmap (dpy, (Pixmap) m_targetWindow->GetBackingPixmap());
            m_targetWindow->SetBackingPixmap((WXPixmap) 0);
        }

        if (!m_targetWindow->GetBackingPixmap() &&
           (m_xScrollLines != 0) && (m_yScrollLines != 0))
        {
            int depth = wxDisplayDepth();
            m_targetWindow->SetPixmapWidth(totalPixelWidth);
            m_targetWindow->SetPixmapHeight(totalPixelHeight);
            m_targetWindow->SetBackingPixmap((WXPixmap) XCreatePixmap (dpy, RootWindow (dpy, DefaultScreen (dpy)),
              m_targetWindow->GetPixmapWidth(), m_targetWindow->GetPixmapHeight(), depth));
        }

    }
#endif 
    if (oldXScroll != m_xScrollPosition)
    {
       if (m_xScrollingEnabled)
            m_targetWindow->ScrollWindow( m_xScrollPixelsPerLine * (oldXScroll - m_xScrollPosition), 0,
                                          GetScrollRect() );
       else
            m_targetWindow->Refresh(true, GetScrollRect());
    }

    if (oldYScroll != m_yScrollPosition)
    {
        if (m_yScrollingEnabled)
            m_targetWindow->ScrollWindow( 0, m_yScrollPixelsPerLine * (oldYScroll-m_yScrollPosition),
                                          GetScrollRect() );
        else
            m_targetWindow->Refresh(true, GetScrollRect());
    }
}

void wxScrollHelper::DoScroll( int x_pos, int y_pos )
{
    if (!m_targetWindow)
        return;

    if (((x_pos == -1) || (x_pos == m_xScrollPosition)) &&
        ((y_pos == -1) || (y_pos == m_yScrollPosition))) return;

    int w = 0, h = 0;
    GetTargetSize(&w, &h);

        int new_x = m_xScrollPosition;
    int new_y = m_yScrollPosition;

    if ((x_pos != -1) && (m_xScrollPixelsPerLine))
    {
        new_x = x_pos;

                        int noPagePositions = w/m_xScrollPixelsPerLine;
        if (noPagePositions < 1) noPagePositions = 1;

                        new_x = wxMin( m_xScrollLines-noPagePositions, new_x );
        new_x = wxMax( 0, new_x );
    }
    if ((y_pos != -1) && (m_yScrollPixelsPerLine))
    {
        new_y = y_pos;

                        int noPagePositions = h/m_yScrollPixelsPerLine;
        if (noPagePositions < 1) noPagePositions = 1;

                        new_y = wxMin( m_yScrollLines-noPagePositions, new_y );
        new_y = wxMax( 0, new_y );
    }

    if ( new_x == m_xScrollPosition && new_y == m_yScrollPosition )
        return; 
                m_targetWindow->Update();

        if (m_xScrollPosition != new_x)
    {
        int old_x = m_xScrollPosition;
        m_xScrollPosition = new_x;
        m_win->SetScrollPos( wxHORIZONTAL, new_x );
        m_targetWindow->ScrollWindow( (old_x-new_x)*m_xScrollPixelsPerLine, 0,
                                      GetScrollRect() );
    }

    if (m_yScrollPosition != new_y)
    {
        int old_y = m_yScrollPosition;
        m_yScrollPosition = new_y;
        m_win->SetScrollPos( wxVERTICAL, new_y );
        m_targetWindow->ScrollWindow( 0, (old_y-new_y)*m_yScrollPixelsPerLine,
                                      GetScrollRect() );
    }
}

#endif 

wxSize wxScrolledT_Helper::FilterBestSize(const wxWindow *win,
                                          const wxScrollHelper *helper,
                                          const wxSize& origBest)
{
                    
    wxSize best = origBest;

    if ( win->GetAutoLayout() )
    {
                                int ppuX, ppuY;
        helper->GetScrollPixelsPerUnit(&ppuX, &ppuY);

                                                                        
        wxSize minSize = win->GetMinSize();

        if ( ppuX > 0 )
            best.x = minSize.x + wxSystemSettings::GetMetric(wxSYS_VSCROLL_X);

        if ( ppuY > 0 )
            best.y = minSize.y + wxSystemSettings::GetMetric(wxSYS_HSCROLL_Y);
    }

    return best;
}

#ifdef __WXMSW__
WXLRESULT wxScrolledT_Helper::FilterMSWWindowProc(WXUINT nMsg, WXLRESULT rc)
{
        if ( nMsg == WM_GETDLGCODE )
    {
        rc |= DLGC_WANTARROWS;
    }
    return rc;
}
#endif 
wxIMPLEMENT_DYNAMIC_CLASS(wxScrolledWindow, wxPanel);
