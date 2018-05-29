


#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#if wxUSE_AUI

#include "wx/aui/framemanager.h"
#include "wx/aui/floatpane.h"
#include "wx/aui/dockart.h"

#ifndef WX_PRECOMP
#endif

#ifdef __WXMSW__
#include "wx/msw/private.h"
#endif

wxIMPLEMENT_CLASS(wxAuiFloatingFrame, wxAuiFloatingFrameBaseClass);

wxAuiFloatingFrame::wxAuiFloatingFrame(wxWindow* parent,
                wxAuiManager* owner_mgr,
                const wxAuiPaneInfo& pane,
                wxWindowID id ,
                long style )
                : wxAuiFloatingFrameBaseClass(parent, id, wxEmptyString,
                        pane.floating_pos, pane.floating_size,
                        style |
                        (pane.HasCloseButton()?wxCLOSE_BOX:0) |
                        (pane.HasMaximizeButton()?wxMAXIMIZE_BOX:0) |
                        (pane.IsFixed()?0:wxRESIZE_BORDER)
                        )
{
    m_ownerMgr = owner_mgr;
    m_moving = false;
    m_mgr.SetManagedWindow(this);
    m_solidDrag = true;

        #ifdef __WXMSW__
    BOOL b = TRUE;
    SystemParametersInfo(38 , 0, &b, 0);
    m_solidDrag = b ? true : false;
#endif

    SetExtraStyle(wxWS_EX_PROCESS_IDLE);
}

wxAuiFloatingFrame::~wxAuiFloatingFrame()
{
        if (m_ownerMgr && m_ownerMgr->m_actionWindow == this)
    {
        m_ownerMgr->m_actionWindow = NULL;
    }

    m_mgr.UnInit();
}

void wxAuiFloatingFrame::SetPaneWindow(const wxAuiPaneInfo& pane)
{
    m_paneWindow = pane.window;
    m_paneWindow->Reparent(this);

    wxAuiPaneInfo contained_pane = pane;
    contained_pane.Dock().Center().Show().
                    CaptionVisible(false).
                    PaneBorder(false).
                    Layer(0).Row(0).Position(0);

        wxSize pane_min_size = pane.window->GetMinSize();

            wxSize cur_max_size = GetMaxSize();
    if (cur_max_size.IsFullySpecified() &&
          (cur_max_size.x < pane.min_size.x ||
           cur_max_size.y < pane.min_size.y)
       )
    {
        SetMaxSize(pane_min_size);
    }

    SetMinSize(pane.window->GetMinSize());

    m_mgr.AddPane(m_paneWindow, contained_pane);
    m_mgr.Update();

    if (pane.min_size.IsFullySpecified())
    {
                                wxSize tmp = GetSize();
        GetSizer()->SetSizeHints(this);
        SetSize(tmp);
    }

    SetTitle(pane.caption);

                                        const bool hasFloatingSize = pane.floating_size != wxDefaultSize;
    if (pane.IsFixed())
    {
        SetWindowStyleFlag(GetWindowStyleFlag() & ~wxRESIZE_BORDER);
    }

    if ( hasFloatingSize )
    {
        SetSize(pane.floating_size);
    }
    else
    {
        wxSize size = pane.best_size;
        if (size == wxDefaultSize)
            size = pane.min_size;
        if (size == wxDefaultSize)
            size = m_paneWindow->GetSize();
        if (m_ownerMgr && pane.HasGripper())
        {
            if (pane.HasGripperTop())
                size.y += m_ownerMgr->m_art->GetMetric(wxAUI_DOCKART_GRIPPER_SIZE);
            else
                size.x += m_ownerMgr->m_art->GetMetric(wxAUI_DOCKART_GRIPPER_SIZE);
        }

        SetClientSize(size);
    }
}

wxAuiManager* wxAuiFloatingFrame::GetOwnerManager() const
{
    return m_ownerMgr;
}

bool wxAuiFloatingFrame::IsTopNavigationDomain(NavigationKind kind) const
{
    switch ( kind )
    {
        case Navigation_Tab:
            break;

        case Navigation_Accel:
                                                return false;
    }

    return wxAuiFloatingFrameBaseClass::IsTopNavigationDomain(kind);
}

void wxAuiFloatingFrame::OnSize(wxSizeEvent& WXUNUSED(event))
{
    if (m_ownerMgr)
    {
        m_ownerMgr->OnFloatingPaneResized(m_paneWindow, GetRect());
    }
}

void wxAuiFloatingFrame::OnClose(wxCloseEvent& evt)
{
    if (m_ownerMgr)
    {
        m_ownerMgr->OnFloatingPaneClosed(m_paneWindow, evt);
    }
    if (!evt.GetVeto())
    {
        m_mgr.DetachPane(m_paneWindow);
        Destroy();
    }
}

void wxAuiFloatingFrame::OnMoveEvent(wxMoveEvent& event)
{
    if (!m_solidDrag)
    {
                                if (!isMouseDown())
            return;
        OnMoveStart();
        OnMoving(event.GetRect(), wxNORTH);
        m_moving = true;
        return;
    }


    wxRect winRect = GetRect();

    if (winRect == m_lastRect)
        return;

        if (m_lastRect.IsEmpty())
    {
        m_lastRect = winRect;
        return;
    }

        #ifndef __WXOSX__
            if ((abs(winRect.x - m_lastRect.x) > 3) ||
        (abs(winRect.y - m_lastRect.y) > 3))
    {
        m_last3Rect = m_last2Rect;
        m_last2Rect = m_lastRect;
        m_lastRect = winRect;

                        if (m_ownerMgr)
        {
            m_ownerMgr->GetPane(m_paneWindow).
                floating_pos = winRect.GetPosition();
        }

        return;
    }
#endif

        if (m_lastRect.GetSize() != winRect.GetSize())
    {
        m_last3Rect = m_last2Rect;
        m_last2Rect = m_lastRect;
        m_lastRect = winRect;
        return;
    }

    wxDirection dir = wxALL;

    int horiz_dist = abs(winRect.x - m_last3Rect.x);
    int vert_dist = abs(winRect.y - m_last3Rect.y);

    if (vert_dist >= horiz_dist)
    {
        if (winRect.y < m_last3Rect.y)
            dir = wxNORTH;
        else
            dir = wxSOUTH;
    }
    else
    {
        if (winRect.x < m_last3Rect.x)
            dir = wxWEST;
        else
            dir = wxEAST;
    }

    m_last3Rect = m_last2Rect;
    m_last2Rect = m_lastRect;
    m_lastRect = winRect;

    if (!isMouseDown())
        return;

    if (!m_moving)
    {
        OnMoveStart();
        m_moving = true;
    }

    if (m_last3Rect.IsEmpty())
        return;

    if ( event.GetEventType() == wxEVT_MOVING )
        OnMoving(event.GetRect(), dir);
    else
        OnMoving(wxRect(event.GetPosition(),GetSize()), dir);
}

void wxAuiFloatingFrame::OnIdle(wxIdleEvent& event)
{
    if (m_moving)
    {
        if (!isMouseDown())
        {
            m_moving = false;
            OnMoveFinished();
        }
        else
        {
            event.RequestMore();
        }
    }
}

void wxAuiFloatingFrame::OnMoveStart()
{
        if (m_ownerMgr)
    {
        m_ownerMgr->OnFloatingPaneMoveStart(m_paneWindow);
    }
}

void wxAuiFloatingFrame::OnMoving(const wxRect& WXUNUSED(window_rect), wxDirection dir)
{
        if (m_ownerMgr)
    {
        m_ownerMgr->OnFloatingPaneMoving(m_paneWindow, dir);
    }
    m_lastDirection = dir;
}

void wxAuiFloatingFrame::OnMoveFinished()
{
        if (m_ownerMgr)
    {
        m_ownerMgr->OnFloatingPaneMoved(m_paneWindow, m_lastDirection);
    }
}

void wxAuiFloatingFrame::OnActivate(wxActivateEvent& event)
{
    if (m_ownerMgr && event.GetActive())
    {
        m_ownerMgr->OnFloatingPaneActivated(m_paneWindow);
    }
}

bool wxAuiFloatingFrame::isMouseDown()
{
    return wxGetMouseState().LeftIsDown();
}


wxBEGIN_EVENT_TABLE(wxAuiFloatingFrame, wxAuiFloatingFrameBaseClass)
    EVT_SIZE(wxAuiFloatingFrame::OnSize)
    EVT_MOVE(wxAuiFloatingFrame::OnMoveEvent)
    EVT_MOVING(wxAuiFloatingFrame::OnMoveEvent)
    EVT_CLOSE(wxAuiFloatingFrame::OnClose)
    EVT_IDLE(wxAuiFloatingFrame::OnIdle)
    EVT_ACTIVATE(wxAuiFloatingFrame::OnActivate)
wxEND_EVENT_TABLE()


#endif 