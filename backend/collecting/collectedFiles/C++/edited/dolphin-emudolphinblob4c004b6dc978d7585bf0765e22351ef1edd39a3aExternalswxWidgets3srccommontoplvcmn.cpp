


#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#include "wx/toplevel.h"

#ifndef WX_PRECOMP
    #include "wx/dcclient.h"
    #include "wx/app.h"
#endif 
#include "wx/display.h"


wxBEGIN_EVENT_TABLE(wxTopLevelWindowBase, wxWindow)
    EVT_CLOSE(wxTopLevelWindowBase::OnCloseWindow)
    EVT_SIZE(wxTopLevelWindowBase::OnSize)
wxEND_EVENT_TABLE()


wxIMPLEMENT_ABSTRACT_CLASS(wxTopLevelWindow, wxWindow);


wxTopLevelWindowBase::wxTopLevelWindowBase()
{
        m_isShown = false;
}

wxTopLevelWindowBase::~wxTopLevelWindowBase()
{
        if ( wxTheApp && wxTheApp->GetTopWindow() == this )
        wxTheApp->SetTopWindow(NULL);

    wxTopLevelWindows.DeleteObject(this);

                                for ( wxObjectList::iterator i = wxPendingDelete.begin();
          i != wxPendingDelete.end();
          )
    {
        wxWindow * const win = wxDynamicCast(*i, wxWindow);
        if ( win && wxGetTopLevelParent(win->GetParent()) == this )
        {
            wxPendingDelete.erase(i);

            delete win;

                                    i = wxPendingDelete.begin();
        }
        else
        {
            ++i;
        }
    }

    if ( IsLastBeforeExit() )
    {
                wxTheApp->ExitMainLoop();
    }
}

bool wxTopLevelWindowBase::Destroy()
{
                    if ( wxWindow* parent = GetParent() )
    {
        if ( parent->IsBeingDeleted() )
            return wxNonOwnedWindow::Destroy();
    }

            if ( !wxPendingDelete.Member(this) )
        wxPendingDelete.Append(this);

                            for ( wxWindowList::const_iterator i = wxTopLevelWindows.begin(),
                                     end = wxTopLevelWindows.end();
          i != end;
          ++i )
    {
        wxTopLevelWindow * const win = static_cast<wxTopLevelWindow *>(*i);
        if ( win != this && win->IsShown() )
        {
                                    Hide();

            break;
        }
    }

    return true;
}

bool wxTopLevelWindowBase::IsLastBeforeExit() const
{
            if ( !wxTheApp || !wxTheApp->GetExitOnFrameDelete() )
        return false;

                        if ( GetParent() && !GetParent()->IsBeingDeleted() )
        return false;

    wxWindowList::const_iterator i;
    const wxWindowList::const_iterator end = wxTopLevelWindows.end();

        for ( i = wxTopLevelWindows.begin(); i != end; ++i )
    {
        wxTopLevelWindow * const win = static_cast<wxTopLevelWindow *>(*i);
        if ( win->ShouldPreventAppExit() )
        {
                        return false;
        }
    }

        for ( i = wxTopLevelWindows.begin(); i != end; ++i )
    {
                wxTopLevelWindow * const win = static_cast<wxTopLevelWindow *>(*i);
        if ( !wxPendingDelete.Member(win) && !win->Close() )
        {
                                                                                    return false;
        }
    }

    return true;
}


void wxTopLevelWindowBase::SetMinSize(const wxSize& minSize)
{
    SetSizeHints(minSize, GetMaxSize());
}

void wxTopLevelWindowBase::SetMaxSize(const wxSize& maxSize)
{
    SetSizeHints(GetMinSize(), maxSize);
}

void wxTopLevelWindowBase::GetRectForTopLevelChildren(int *x, int *y, int *w, int *h)
{
    GetPosition(x,y);
    GetSize(w,h);
}


wxSize wxTopLevelWindowBase::GetDefaultSize()
{
    wxSize size = wxGetClientDisplayRect().GetSize();
#ifndef __WXOSX_IPHONE__
        if ( size.x >= 1024 )
        size.x = 400;
    else if ( size.x >= 800 )
        size.x = 300;
    else if ( size.x >= 320 )
        size.x = 240;

    if ( size.y >= 768 )
        size.y = 250;
    else if ( size.y > 200 )
    {
        size.y *= 2;
        size.y /= 3;
    }
#endif
    return size;
}

void wxTopLevelWindowBase::DoCentre(int dir)
{
                        if ( IsAlwaysMaximized() || IsMaximized() )
        return;

                int nDisplay = wxDisplay::GetFromWindow(GetParent() ? GetParent() : this);
    wxDisplay dpy(nDisplay == wxNOT_FOUND ? 0 : nDisplay);
    const wxRect rectDisplay(dpy.GetClientArea());

        wxRect rectParent;
    if ( !(dir & wxCENTRE_ON_SCREEN) && GetParent() )
    {
                        rectParent = GetParent()->GetScreenRect();

                                        if ( !rectParent.Intersects(rectDisplay) )
        {
                        rectParent = rectDisplay;
        }
    }
    else
    {
                        rectParent = rectDisplay;
    }

    if ( !(dir & wxBOTH) )
        dir |= wxBOTH; 
        wxRect rect = GetRect().CentreIn(rectParent, dir & ~wxCENTRE_ON_SCREEN);

                if ( !rectDisplay.Contains(rect.GetTopLeft()) )
    {
                int dx = rectDisplay.GetLeft() - rect.GetLeft();
        int dy = rectDisplay.GetTop() - rect.GetTop();
        rect.Offset(dx > 0 ? dx : 0, dy > 0 ? dy : 0);
    }

    if ( !rectDisplay.Contains(rect.GetBottomRight()) )
    {
                int dx = rectDisplay.GetRight() - rect.GetRight();
        int dy = rectDisplay.GetBottom() - rect.GetBottom();
        rect.Offset(dx < 0 ? dx : 0, dy < 0 ? dy : 0);
    }

                
        SetSize(rect, wxSIZE_ALLOW_MINUS_ONE);
}


void wxTopLevelWindowBase::DoScreenToClient(int *x, int *y) const
{
    wxWindow::DoScreenToClient(x, y);

        wxPoint pt(GetClientAreaOrigin());
    if ( x )
        *x -= pt.x;
    if ( y )
        *y -= pt.y;
}

void wxTopLevelWindowBase::DoClientToScreen(int *x, int *y) const
{
            wxPoint pt(GetClientAreaOrigin());
    if ( x )
        *x += pt.x;
    if ( y )
        *y += pt.y;

    wxWindow::DoClientToScreen(x, y);
}

bool wxTopLevelWindowBase::IsAlwaysMaximized() const
{
    return false;
}


wxIcon wxTopLevelWindowBase::GetIcon() const
{
    return m_icons.IsEmpty() ? wxIcon() : m_icons.GetIcon( -1 );
}

void wxTopLevelWindowBase::SetIcon(const wxIcon& icon)
{
            wxIconBundle icons;
    if ( icon.IsOk() )
        icons.AddIcon(icon);

    SetIcons(icons);
}


bool wxTopLevelWindowBase::IsTopNavigationDomain(NavigationKind kind) const
{
                    switch ( kind )
    {
        case Navigation_Tab:
        case Navigation_Accel:
            break;
    }

    return true;
}

void wxTopLevelWindowBase::DoLayout()
{
                    if ( IsBeingDeleted() )
        return;


        if ( GetAutoLayout() )
    {
        Layout();
    }
    else
    {
                wxWindow *child = NULL;
        for ( wxWindowList::compatibility_iterator node = GetChildren().GetFirst();
              node;
              node = node->GetNext() )
        {
            wxWindow *win = node->GetData();

                                                if ( !win->IsTopLevel() && !IsOneOfBars(win) )
            {
                if ( child )
                {
                    return;                     }

                child = win;
            }
        }

                if ( child && child->IsShown() )
        {
                        int clientW, clientH;
            DoGetClientSize(&clientW, &clientH);

            child->SetSize(0, 0, clientW, clientH);
        }
    }
}

void wxTopLevelWindowBase::OnCloseWindow(wxCloseEvent& WXUNUSED(event))
{
    Destroy();
}

bool wxTopLevelWindowBase::SendIconizeEvent(bool iconized)
{
    wxIconizeEvent event(GetId(), iconized);
    event.SetEventObject(this);

    return GetEventHandler()->ProcessEvent(event);
}

void wxTopLevelWindowBase::DoUpdateWindowUI(wxUpdateUIEvent& event)
{
                wxWindowBase::DoUpdateWindowUI(event);

        if ( event.GetSetText() )
    {
        if ( event.GetText() != GetTitle() )
            SetTitle(event.GetText());
    }
}

void wxTopLevelWindowBase::RequestUserAttention(int WXUNUSED(flags))
{
        Raise();
}
