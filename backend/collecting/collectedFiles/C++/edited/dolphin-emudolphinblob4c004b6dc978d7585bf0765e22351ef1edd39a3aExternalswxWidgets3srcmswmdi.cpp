


#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#if wxUSE_MDI && !defined(__WXUNIVERSAL__)

#include "wx/mdi.h"

#ifndef WX_PRECOMP
    #include "wx/frame.h"
    #include "wx/menu.h"
    #include "wx/app.h"
    #include "wx/utils.h"
    #include "wx/dialog.h"
    #include "wx/statusbr.h"
    #include "wx/settings.h"
    #include "wx/intl.h"
    #include "wx/log.h"
    #include "wx/sizer.h"
    #include "wx/toolbar.h"
#endif

#include "wx/stockitem.h"
#include "wx/msw/private.h"

#include <string.h>


extern wxMenu *wxCurrentPopupMenu;

extern void wxRemoveHandleAssociation(wxWindow *win);

namespace
{


const int wxFIRST_MDI_CHILD = 4100;

const int wxLAST_MDI_CHILD = wxFIRST_MDI_CHILD + 8;

const int wxID_MDI_MORE_WINDOWS = wxLAST_MDI_CHILD + 1;

const char *WINDOW_MENU_LABEL = gettext_noop("&Window");


void MDISetMenu(wxWindow *win, HMENU hmenuFrame, HMENU hmenuWindow);

void MDIInsertWindowMenu(wxWindow *win, WXHMENU hMenu, HMENU subMenu);

void MDIRemoveWindowMenu(wxWindow *win, WXHMENU hMenu);

void UnpackMDIActivate(WXWPARAM wParam, WXLPARAM lParam,
                       WXWORD *activate, WXHWND *hwndAct, WXHWND *hwndDeact);

inline HMENU GetMDIWindowMenu(wxMDIParentFrame *frame)
{
    wxMenu *menu = frame->GetWindowMenu();
    return menu ? GetHmenuOf(menu) : 0;
}

} 


wxIMPLEMENT_DYNAMIC_CLASS(wxMDIParentFrame, wxFrame);
wxIMPLEMENT_DYNAMIC_CLASS(wxMDIChildFrame, wxFrame);
wxIMPLEMENT_DYNAMIC_CLASS(wxMDIClientWindow, wxWindow);

wxBEGIN_EVENT_TABLE(wxMDIParentFrame, wxFrame)
    EVT_ACTIVATE(wxMDIParentFrame::OnActivate)
    EVT_SIZE(wxMDIParentFrame::OnSize)
    EVT_ICONIZE(wxMDIParentFrame::OnIconized)
    EVT_SYS_COLOUR_CHANGED(wxMDIParentFrame::OnSysColourChanged)

#if wxUSE_MENUS
    EVT_MENU_RANGE(wxFIRST_MDI_CHILD, wxLAST_MDI_CHILD,
                   wxMDIParentFrame::OnMDIChild)
    EVT_MENU_RANGE(wxID_MDI_WINDOW_FIRST, wxID_MDI_WINDOW_LAST,
                   wxMDIParentFrame::OnMDICommand)
#endif wxEND_EVENT_TABLE()

wxBEGIN_EVENT_TABLE(wxMDIChildFrame, wxFrame)
    EVT_IDLE(wxMDIChildFrame::OnIdle)
wxEND_EVENT_TABLE()

wxBEGIN_EVENT_TABLE(wxMDIClientWindow, wxWindow)
    EVT_SCROLL(wxMDIClientWindow::OnScroll)
wxEND_EVENT_TABLE()


void wxMDIParentFrame::Init()
{
#if wxUSE_MENUS && wxUSE_ACCEL
    m_accelWindowMenu = NULL;
#endif 
  m_activationNotHandled = false;
}

bool wxMDIParentFrame::Create(wxWindow *parent,
                              wxWindowID id,
                              const wxString& title,
                              const wxPoint& pos,
                              const wxSize& size,
                              long style,
                              const wxString& name)
{
      if ( !(style & wxFRAME_NO_WINDOW_MENU) )
  {
            m_windowMenu = new wxMenu;

      m_windowMenu->Append(wxID_MDI_WINDOW_CASCADE, _("&Cascade"));
      m_windowMenu->Append(wxID_MDI_WINDOW_TILE_HORZ, _("Tile &Horizontally"));
      m_windowMenu->Append(wxID_MDI_WINDOW_TILE_VERT, _("Tile &Vertically"));
      m_windowMenu->AppendSeparator();
      m_windowMenu->Append(wxID_MDI_WINDOW_ARRANGE_ICONS, _("&Arrange Icons"));
      m_windowMenu->Append(wxID_MDI_WINDOW_NEXT, _("&Next"));
      m_windowMenu->Append(wxID_MDI_WINDOW_PREV, _("&Previous"));
  }

  if (!parent)
    wxTopLevelWindows.Append(this);

  SetName(name);
  m_windowStyle = style;

  if ( parent )
      parent->AddChild(this);

  if ( id != wxID_ANY )
    m_windowId = id;
  else
    m_windowId = NewControlId();

  WXDWORD exflags;
  WXDWORD msflags = MSWGetCreateWindowFlags(&exflags);
  msflags &= ~WS_VSCROLL;
  msflags &= ~WS_HSCROLL;

  if ( !wxWindow::MSWCreate(wxApp::GetRegisteredClassName(wxT("wxMDIFrame")),
                            title.t_str(),
                            pos, size,
                            msflags,
                            exflags) )
  {
      return false;
  }

  SetOwnBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_APPWORKSPACE));

    m_isShown = false;

  return true;
}

wxMDIParentFrame::~wxMDIParentFrame()
{
    #if wxUSE_TOOLBAR
    m_frameToolBar = NULL;
#endif
#if wxUSE_STATUSBAR
    m_frameStatusBar = NULL;
#endif 
#if wxUSE_MENUS && wxUSE_ACCEL
    delete m_accelWindowMenu;
#endif 
    DestroyChildren();

            if ( m_hMenu )
        ::DestroyMenu((HMENU)m_hMenu);

    if ( m_clientWindow )
    {
        if ( m_clientWindow->MSWGetOldWndProc() )
            m_clientWindow->UnsubclassWin();

        m_clientWindow->SetHWND(0);
        delete m_clientWindow;
    }
}


wxMDIChildFrame *wxMDIParentFrame::GetActiveChild() const
{
    HWND hWnd = (HWND)::SendMessage(GetWinHwnd(GetClientWindow()),
                                    WM_MDIGETACTIVE, 0, 0L);
    if ( !hWnd )
        return NULL;

    return static_cast<wxMDIChildFrame *>(wxFindWinFromHandle(hWnd));
}

int wxMDIParentFrame::GetChildFramesCount() const
{
    int count = 0;
    for ( wxWindowList::const_iterator i = GetChildren().begin();
          i != GetChildren().end();
          ++i )
    {
        if ( wxDynamicCast(*i, wxMDIChildFrame) )
            count++;
    }

    return count;
}

#if wxUSE_MENUS

void wxMDIParentFrame::AddMDIChild(wxMDIChildFrame * WXUNUSED(child))
{
    switch ( GetChildFramesCount() )
    {
        case 1:
                                    AddWindowMenu();

                                    UpdateWindowMenu(false);
            break;

        case 2:
                                    UpdateWindowMenu(true);
            break;
    }
}

void wxMDIParentFrame::RemoveMDIChild(wxMDIChildFrame * WXUNUSED(child))
{
    switch ( GetChildFramesCount() )
    {
        case 1:
                                    RemoveWindowMenu();

                                    break;

        case 2:
                                    UpdateWindowMenu(false);
            break;
    }
}


void wxMDIParentFrame::AddWindowMenu()
{
    if ( m_windowMenu )
    {
                        m_windowMenu->Attach(GetMenuBar());

        MDIInsertWindowMenu(GetClientWindow(), m_hMenu, GetMDIWindowMenu(this));
    }
}

void wxMDIParentFrame::RemoveWindowMenu()
{
    if ( m_windowMenu )
    {
        MDIRemoveWindowMenu(GetClientWindow(), m_hMenu);

        m_windowMenu->Detach();
    }
}

void wxMDIParentFrame::UpdateWindowMenu(bool enable)
{
    if ( m_windowMenu )
    {
        m_windowMenu->Enable(wxID_MDI_WINDOW_NEXT, enable);
        m_windowMenu->Enable(wxID_MDI_WINDOW_PREV, enable);
    }
}

#if wxUSE_MENUS_NATIVE

void wxMDIParentFrame::InternalSetMenuBar()
{
    if ( GetActiveChild() )
    {
        AddWindowMenu();
    }
    else     {
                        MDISetMenu(GetClientWindow(), (HMENU)m_hMenu, NULL);
    }
}

#endif 
void wxMDIParentFrame::SetWindowMenu(wxMenu* menu)
{
    if ( menu != m_windowMenu )
    {
                        const bool hasWindowMenu = GetActiveChild() != NULL;

        if ( hasWindowMenu )
            RemoveWindowMenu();

        delete m_windowMenu;

        m_windowMenu = menu;

        if ( hasWindowMenu )
            AddWindowMenu();
    }

#if wxUSE_ACCEL
    wxDELETE(m_accelWindowMenu);

    if ( menu && menu->HasAccels() )
        m_accelWindowMenu = menu->CreateAccelTable();
#endif }


void wxMDIParentFrame::DoMenuUpdates(wxMenu* menu)
{
    wxMDIChildFrame *child = GetActiveChild();
    if ( child )
    {
        wxMenuBar* bar = child->GetMenuBar();

        if (menu)
        {
            menu->UpdateUI();
        }
        else
        {
            if ( bar != NULL )
            {
                int nCount = bar->GetMenuCount();
                for (int n = 0; n < nCount; n++)
                    bar->GetMenu(n)->UpdateUI();
            }
        }
    }
    else
    {
        wxFrameBase::DoMenuUpdates(menu);
    }
}

wxMenuItem *wxMDIParentFrame::FindItemInMenuBar(int menuId) const
{
                wxMenuItem *item = GetActiveChild()
                            ? GetActiveChild()->FindItemInMenuBar(menuId)
                            : NULL;
    if ( !item )
        item = wxFrame::FindItemInMenuBar(menuId);

    if ( !item && m_windowMenu )
        item = m_windowMenu->FindItem(menuId);

    return item;
}

wxMenu* wxMDIParentFrame::MSWFindMenuFromHMENU(WXHMENU hMenu)
{
    wxMenu* menu = GetActiveChild()
                        ? GetActiveChild()->MSWFindMenuFromHMENU(hMenu)
                        : NULL;
    if ( !menu )
        menu = wxFrame::MSWFindMenuFromHMENU(hMenu);

    if ( !menu && m_windowMenu && GetHmenuOf(m_windowMenu) == hMenu )
        menu = m_windowMenu;

    return menu;
}

WXHMENU wxMDIParentFrame::MSWGetActiveMenu() const
{
    wxMDIChildFrame * const child  = GetActiveChild();
    if ( child )
    {
        const WXHMENU hmenu = child->MSWGetActiveMenu();
        if ( hmenu )
            return hmenu;
    }

    return wxFrame::MSWGetActiveMenu();
}

#endif 

void wxMDIParentFrame::UpdateClientSize()
{
    int width, height;
    GetClientSize(&width, &height);

    if ( wxSizer* sizer = GetSizer() )
    {
        sizer->SetDimension(0, 0, width, height);
    }
    else
    {
        if ( GetClientWindow() )
            GetClientWindow()->SetSize(0, 0, width, height);
    }
}

void wxMDIParentFrame::OnSize(wxSizeEvent& WXUNUSED(event))
{
    UpdateClientSize();

    }

void wxMDIParentFrame::OnIconized(wxIconizeEvent& event)
{
    event.Skip();

    if ( !event.IsIconized() )
        UpdateClientSize();
}

void wxMDIParentFrame::OnSysColourChanged(wxSysColourChangedEvent& event)
{
    if ( m_clientWindow )
    {
        m_clientWindow->SetBackgroundColour(wxSystemSettings::GetColour(wxSYS_COLOUR_APPWORKSPACE));
        m_clientWindow->Refresh();
    }

    event.Skip();
}

WXHICON wxMDIParentFrame::GetDefaultIcon() const
{
        return (WXHICON)0;
}


void wxMDIParentFrame::Cascade()
{
    ::SendMessage(GetWinHwnd(GetClientWindow()), WM_MDICASCADE, 0, 0);
}

void wxMDIParentFrame::Tile(wxOrientation orient)
{
    wxASSERT_MSG( orient == wxHORIZONTAL || orient == wxVERTICAL,
                  wxT("invalid orientation value") );

    ::SendMessage(GetWinHwnd(GetClientWindow()), WM_MDITILE,
                  orient == wxHORIZONTAL ? MDITILE_HORIZONTAL
                                         : MDITILE_VERTICAL, 0);
}

void wxMDIParentFrame::ArrangeIcons()
{
    ::SendMessage(GetWinHwnd(GetClientWindow()), WM_MDIICONARRANGE, 0, 0);
}

void wxMDIParentFrame::ActivateNext()
{
    ::SendMessage(GetWinHwnd(GetClientWindow()), WM_MDINEXT, 0, 0);
}

void wxMDIParentFrame::ActivatePrevious()
{
    ::SendMessage(GetWinHwnd(GetClientWindow()), WM_MDINEXT, 0, 1);
}


WXLRESULT wxMDIParentFrame::MSWWindowProc(WXUINT message,
                                          WXWPARAM wParam,
                                          WXLPARAM lParam)
{
    WXLRESULT rc = 0;
    bool processed = false;

    switch ( message )
    {
        case WM_ACTIVATE:
            {
                WXWORD state, minimized;
                WXHWND hwnd;
                UnpackActivate(wParam, lParam, &state, &minimized, &hwnd);

                processed = HandleActivate(state, minimized != 0, hwnd);
            }
            break;

        case WM_COMMAND:
                                                            {
                WXWORD id, cmd;
                WXHWND hwnd;
                UnpackCommand(wParam, lParam, &id, &hwnd, &cmd);

                if ( id == wxID_MDI_MORE_WINDOWS ||
                     (cmd == 0  &&
                        id >= SC_SIZE ) )
                {
                    MSWDefWindowProc(message, wParam, lParam);
                    processed = true;
                }
            }
            break;

        case WM_CREATE:
            m_clientWindow = OnCreateClient();
                        if ( !m_clientWindow->CreateClient(this, GetWindowStyleFlag()) )
            {
                wxLogMessage(_("Failed to create MDI parent frame."));

                rc = -1;
            }

            processed = true;
            break;
    }

    if ( !processed )
        rc = wxFrame::MSWWindowProc(message, wParam, lParam);

    return rc;
}

void wxMDIParentFrame::OnActivate(wxActivateEvent& WXUNUSED(event))
{
                                                                                m_activationNotHandled = true;
}

bool wxMDIParentFrame::HandleActivate(int state, bool minimized, WXHWND activate)
{
    bool processed = false;

                m_activationNotHandled = false;

    if ( wxWindow::HandleActivate(state, minimized, activate) )
    {
                        processed = !m_activationNotHandled;
    }

                if ( GetActiveChild() )
    {
        if ( GetActiveChild()->HandleActivate(state, minimized, activate) )
            processed = true;
    }

    return processed;
}

#if wxUSE_MENUS

void wxMDIParentFrame::OnMDIChild(wxCommandEvent& event)
{
    wxWindowList::compatibility_iterator node = GetChildren().GetFirst();
    while ( node )
    {
        wxWindow *child = node->GetData();
        if ( child->GetHWND() )
        {
            int childId = wxGetWindowId(child->GetHWND());
            if ( childId == event.GetId() )
            {
                wxStaticCast(child, wxMDIChildFrame)->Activate();
                return;
            }
        }

        node = node->GetNext();
    }

    wxFAIL_MSG( "unknown MDI child selected?" );
}

void wxMDIParentFrame::OnMDICommand(wxCommandEvent& event)
{
    WXWPARAM wParam = 0;
    WXLPARAM lParam = 0;
    int msg;
    switch ( event.GetId() )
    {
        case wxID_MDI_WINDOW_CASCADE:
            msg = WM_MDICASCADE;
            wParam = MDITILE_SKIPDISABLED;
            break;

        case wxID_MDI_WINDOW_TILE_HORZ:
            wParam |= MDITILE_HORIZONTAL;
            
        case wxID_MDI_WINDOW_TILE_VERT:
            if ( !wParam )
                wParam = MDITILE_VERTICAL;
            msg = WM_MDITILE;
            wParam |= MDITILE_SKIPDISABLED;
            break;

        case wxID_MDI_WINDOW_ARRANGE_ICONS:
            msg = WM_MDIICONARRANGE;
            break;

        case wxID_MDI_WINDOW_NEXT:
            msg = WM_MDINEXT;
            lParam = 0;                     break;

        case wxID_MDI_WINDOW_PREV:
            msg = WM_MDINEXT;
            lParam = 1;                     break;

        default:
            wxFAIL_MSG( "unknown MDI command" );
            return;
    }

    ::SendMessage(GetWinHwnd(GetClientWindow()), msg, wParam, lParam);
}

#endif 
WXLRESULT wxMDIParentFrame::MSWDefWindowProc(WXUINT message,
                                        WXWPARAM wParam,
                                        WXLPARAM lParam)
{
    WXHWND clientWnd;
    if ( GetClientWindow() )
        clientWnd = GetClientWindow()->GetHWND();
    else
        clientWnd = 0;

    return DefFrameProc(GetHwnd(), (HWND)clientWnd, message, wParam, lParam);
}

bool wxMDIParentFrame::MSWTranslateMessage(WXMSG* msg)
{
    MSG *pMsg = (MSG *)msg;

        wxMDIChildFrame * const child = GetActiveChild();
    if ( child && child->MSWTranslateMessage(msg) )
    {
        return true;
    }

            if ( wxFrame::MSWTranslateMessage(msg) )
    {
        return true;
    }

#if wxUSE_MENUS && wxUSE_ACCEL
                if ( m_accelWindowMenu && m_accelWindowMenu->Translate(this, msg) )
        return true;
#endif 
        if ( pMsg->message == WM_KEYDOWN || pMsg->message == WM_SYSKEYDOWN )
    {
        if ( ::TranslateMDISysAccel(GetWinHwnd(GetClientWindow()), pMsg))
            return true;
    }

    return false;
}


void wxMDIChildFrame::Init()
{
    m_needsResize = true;
}

bool wxMDIChildFrame::Create(wxMDIParentFrame *parent,
                             wxWindowID id,
                             const wxString& title,
                             const wxPoint& pos,
                             const wxSize& size,
                             long style,
                             const wxString& name)
{
    m_mdiParent = parent;

  SetName(name);

  if ( id != wxID_ANY )
    m_windowId = id;
  else
    m_windowId = NewControlId();

  if ( parent )
  {
      parent->AddChild(this);
  }

  int x = pos.x;
  int y = pos.y;
  int width = size.x;
  int height = size.y;

  MDICREATESTRUCT mcs;

  wxString className =
      wxApp::GetRegisteredClassName(wxT("wxMDIChildFrame"), COLOR_WINDOW);
  if ( !(style & wxFULL_REPAINT_ON_RESIZE) )
      className += wxApp::GetNoRedrawClassSuffix();

  mcs.szClass = className.t_str();
  mcs.szTitle = title.t_str();
  mcs.hOwner = wxGetInstance();
  if (x != wxDefaultCoord)
      mcs.x = x;
  else
      mcs.x = CW_USEDEFAULT;

  if (y != wxDefaultCoord)
      mcs.y = y;
  else
      mcs.y = CW_USEDEFAULT;

  if (width != wxDefaultCoord)
      mcs.cx = width;
  else
      mcs.cx = CW_USEDEFAULT;

  if (height != wxDefaultCoord)
      mcs.cy = height;
  else
      mcs.cy = CW_USEDEFAULT;

  DWORD msflags = WS_OVERLAPPED | WS_CLIPCHILDREN;
  if (style & wxMINIMIZE_BOX)
    msflags |= WS_MINIMIZEBOX;
  if (style & wxMAXIMIZE_BOX)
    msflags |= WS_MAXIMIZEBOX;
  if (style & wxRESIZE_BORDER)
    msflags |= WS_THICKFRAME;
  if (style & wxSYSTEM_MENU)
    msflags |= WS_SYSMENU;
  if ((style & wxMINIMIZE) || (style & wxICONIZE))
    msflags |= WS_MINIMIZE;
  if (style & wxMAXIMIZE)
    msflags |= WS_MAXIMIZE;
  if (style & wxCAPTION)
    msflags |= WS_CAPTION;

  mcs.style = msflags;

  mcs.lParam = 0;

  wxWindowCreationHook hook(this);

  m_hWnd = (WXHWND)::SendMessage(GetWinHwnd(parent->GetClientWindow()),
                                 WM_MDICREATE, 0, (LPARAM)&mcs);

  if ( !m_hWnd )
  {
      wxLogLastError(wxT("WM_MDICREATE"));
      return false;
  }

  SubclassWin(m_hWnd);

  parent->AddMDIChild(this);

  return true;
}

wxMDIChildFrame::~wxMDIChildFrame()
{
        if ( !m_hWnd )
        return;

    wxMDIParentFrame * const parent = GetMDIParent();

    parent->RemoveMDIChild(this);

        #if wxUSE_TOOLBAR
    m_frameToolBar = NULL;
#endif
#if wxUSE_STATUSBAR
    m_frameStatusBar = NULL;
#endif 
    DestroyChildren();

    MDIRemoveWindowMenu(NULL, m_hMenu);

            MDISetMenu(parent->GetClientWindow(),
               (HMENU)parent->MSWGetActiveMenu(),
               GetMDIWindowMenu(parent));

    MSWDestroyWindow();
}

bool wxMDIChildFrame::Show(bool show)
{
    if (!wxFrame::Show(show))
        return false;

                if ( show )
        ::BringWindowToTop(GetHwnd());

            wxMDIParentFrame * const parent = GetMDIParent();
    MDISetMenu(parent->GetClientWindow(), NULL, NULL);

    return true;
}

void
wxMDIChildFrame::DoSetSize(int x, int y, int width, int height, int sizeFlags)
{
            wxMDIChildFrameBase::DoSetSize(x, y, width, height, sizeFlags);
}

void wxMDIChildFrame::DoSetClientSize(int width, int height)
{
  HWND hWnd = GetHwnd();

  RECT rect;
  ::GetClientRect(hWnd, &rect);

  RECT rect2;
  GetWindowRect(hWnd, &rect2);

        int actual_width = rect2.right - rect2.left - rect.right + width;
  int actual_height = rect2.bottom - rect2.top - rect.bottom + height;

#if wxUSE_STATUSBAR
  if (GetStatusBar() && GetStatusBar()->IsShown())
  {
    int sx, sy;
    GetStatusBar()->GetSize(&sx, &sy);
    actual_height += sy;
  }
#endif 
  POINT point;
  point.x = rect2.left;
  point.y = rect2.top;

      wxMDIParentFrame * const mdiParent = GetMDIParent();
  ::ScreenToClient(GetHwndOf(mdiParent->GetClientWindow()), &point);

  MoveWindow(hWnd, point.x, point.y, actual_width, actual_height, (BOOL)true);

  wxSize size(width, height);
  wxSizeEvent event(size, m_windowId);
  event.SetEventObject( this );
  HandleWindowEvent(event);
}

void wxMDIChildFrame::DoGetScreenPosition(int *x, int *y) const
{
  HWND hWnd = GetHwnd();

  RECT rect;
  ::GetWindowRect(hWnd, &rect);
  if (x)
     *x = rect.left;
  if (y)
     *y = rect.top;
}


void wxMDIChildFrame::DoGetPosition(int *x, int *y) const
{
  RECT rect;
  GetWindowRect(GetHwnd(), &rect);
  POINT point;
  point.x = rect.left;
  point.y = rect.top;

      wxMDIParentFrame * const mdiParent = GetMDIParent();
  ::ScreenToClient(GetHwndOf(mdiParent->GetClientWindow()), &point);

  if (x)
      *x = point.x;
  if (y)
      *y = point.y;
}

void wxMDIChildFrame::InternalSetMenuBar()
{
    wxMDIParentFrame * const parent = GetMDIParent();

    MDIInsertWindowMenu(parent->GetClientWindow(),
                     m_hMenu, GetMDIWindowMenu(parent));
}

void wxMDIChildFrame::DetachMenuBar()
{
    MDIRemoveWindowMenu(NULL, m_hMenu);
    wxFrame::DetachMenuBar();
}

WXHICON wxMDIChildFrame::GetDefaultIcon() const
{
        return (WXHICON)0;
}


void wxMDIChildFrame::Maximize(bool maximize)
{
    wxMDIParentFrame * const parent = GetMDIParent();
    if ( parent && parent->GetClientWindow() )
    {
        if ( !IsShown() )
        {
                                                ::SendMessage(GetWinHwnd(parent->GetClientWindow()), WM_SETREDRAW,
                          FALSE, 0L);
        }

        ::SendMessage(GetWinHwnd(parent->GetClientWindow()),
                      maximize ? WM_MDIMAXIMIZE : WM_MDIRESTORE,
                      (WPARAM)GetHwnd(), 0);

        if ( !IsShown() )
        {
                        ::ShowWindow(GetHwnd(), SW_HIDE);

                        ::SendMessage(GetWinHwnd(parent->GetClientWindow()), WM_SETREDRAW,
                          TRUE, 0L);
        }
    }
}

void wxMDIChildFrame::Restore()
{
    wxMDIParentFrame * const parent = GetMDIParent();
    if ( parent && parent->GetClientWindow() )
    {
        ::SendMessage(GetWinHwnd(parent->GetClientWindow()), WM_MDIRESTORE,
                      (WPARAM) GetHwnd(), 0);
    }
}

void wxMDIChildFrame::Activate()
{
    wxMDIParentFrame * const parent = GetMDIParent();
    if ( parent && parent->GetClientWindow() )
    {
                        if ( IsIconized() )
            Restore();

        ::SendMessage(GetWinHwnd(parent->GetClientWindow()), WM_MDIACTIVATE,
                      (WPARAM) GetHwnd(), 0);
    }
}


WXLRESULT wxMDIChildFrame::MSWWindowProc(WXUINT message,
                                         WXWPARAM wParam,
                                         WXLPARAM lParam)
{
    WXLRESULT rc = 0;
    bool processed = false;

    switch ( message )
    {
        case WM_GETMINMAXINFO:
            processed = HandleGetMinMaxInfo((MINMAXINFO *)lParam);
            break;

        case WM_MDIACTIVATE:
            {
                WXWORD act;
                WXHWND hwndAct, hwndDeact;
                UnpackMDIActivate(wParam, lParam, &act, &hwndAct, &hwndDeact);

                processed = HandleMDIActivate(act, hwndAct, hwndDeact);
            }
            
        case WM_MOVE:
                        
            
        case WM_SIZE:
                                    MSWDefWindowProc(message, wParam, lParam);
            break;

        case WM_WINDOWPOSCHANGING:
            processed = HandleWindowPosChanging((LPWINDOWPOS)lParam);
            break;
    }

    if ( !processed )
        rc = wxFrame::MSWWindowProc(message, wParam, lParam);

    return rc;
}

bool wxMDIChildFrame::HandleMDIActivate(long WXUNUSED(activate),
                                        WXHWND hwndAct,
                                        WXHWND hwndDeact)
{
    wxMDIParentFrame * const parent = GetMDIParent();

    WXHMENU hMenuToSet = 0;

    bool activated;

    if ( m_hWnd == hwndAct )
    {
        activated = true;
        parent->SetActiveChild(this);

        WXHMENU hMenuChild = m_hMenu;
        if ( hMenuChild )
            hMenuToSet = hMenuChild;
    }
    else if ( m_hWnd == hwndDeact )
    {
        wxASSERT_MSG( parent->GetActiveChild() == this,
                      wxT("can't deactivate MDI child which wasn't active!") );

        activated = false;
        parent->SetActiveChild(NULL);

        WXHMENU hMenuParent = parent->m_hMenu;

                        if ( hMenuParent && !hwndAct )
            hMenuToSet = hMenuParent;
    }
    else
    {
                return false;
    }

    if ( hMenuToSet )
    {
        MDISetMenu(parent->GetClientWindow(),
                   (HMENU)hMenuToSet, GetMDIWindowMenu(parent));
    }

    wxActivateEvent event(wxEVT_ACTIVATE, activated, m_windowId);
    event.SetEventObject( this );

    ResetWindowStyle(NULL);

    return HandleWindowEvent(event);
}

bool wxMDIChildFrame::HandleWindowPosChanging(void *pos)
{
    WINDOWPOS *lpPos = (WINDOWPOS *)pos;

    if (!(lpPos->flags & SWP_NOSIZE))
    {
        RECT rectClient;
        DWORD dwExStyle = ::GetWindowLong(GetHwnd(), GWL_EXSTYLE);
        DWORD dwStyle = ::GetWindowLong(GetHwnd(), GWL_STYLE);
        if (ResetWindowStyle((void *) & rectClient) && (dwStyle & WS_MAXIMIZE))
        {
            ::AdjustWindowRectEx(&rectClient, dwStyle, false, dwExStyle);
            lpPos->x = rectClient.left;
            lpPos->y = rectClient.top;
            lpPos->cx = rectClient.right - rectClient.left;
            lpPos->cy = rectClient.bottom - rectClient.top;
        }
    }

    return false;
}

bool wxMDIChildFrame::HandleGetMinMaxInfo(void *mmInfo)
{
            MSWDefWindowProc(WM_GETMINMAXINFO, 0, (LPARAM)mmInfo);

            return false;
}


WXLRESULT wxMDIChildFrame::MSWDefWindowProc(WXUINT message, WXWPARAM wParam, WXLPARAM lParam)
{
    return DefMDIChildProc(GetHwnd(),
                           (UINT)message, (WPARAM)wParam, (LPARAM)lParam);
}

bool wxMDIChildFrame::MSWTranslateMessage(WXMSG* msg)
{
            return MSWDoTranslateMessage(GetMDIParent(), msg);
}


void wxMDIChildFrame::MSWDestroyWindow()
{
    wxMDIParentFrame * const parent = GetMDIParent();

            
    HWND oldHandle = (HWND)GetHWND();
    SendMessage(GetWinHwnd(parent->GetClientWindow()), WM_MDIDESTROY,
                (WPARAM)oldHandle, 0);

    if (parent->GetActiveChild() == NULL)
        ResetWindowStyle(NULL);

    if (m_hMenu)
    {
        ::DestroyMenu((HMENU) m_hMenu);
        m_hMenu = 0;
    }
    wxRemoveHandleAssociation(this);
    m_hWnd = 0;
}

bool wxMDIChildFrame::ResetWindowStyle(void *vrect)
{
    RECT *rect = (RECT *)vrect;
    wxMDIParentFrame * const pFrameWnd = GetMDIParent();
    wxMDIChildFrame* pChild = pFrameWnd->GetActiveChild();

    if (!pChild || (pChild == this))
    {
        HWND hwndClient = GetWinHwnd(pFrameWnd->GetClientWindow());
        DWORD dwStyle = ::GetWindowLong(hwndClient, GWL_EXSTYLE);

                        DWORD dwThisStyle = pChild
            ? ::GetWindowLong(GetWinHwnd(pChild), GWL_STYLE) : 0;
        DWORD dwNewStyle = dwStyle;
        if ( dwThisStyle & WS_MAXIMIZE )
            dwNewStyle &= ~(WS_EX_CLIENTEDGE);
        else
            dwNewStyle |= WS_EX_CLIENTEDGE;

        if (dwStyle != dwNewStyle)
        {
                        ::RedrawWindow(hwndClient, NULL, NULL,
                           RDW_INVALIDATE | RDW_ALLCHILDREN);
            ::SetWindowLong(hwndClient, GWL_EXSTYLE, dwNewStyle);
            ::SetWindowPos(hwndClient, NULL, 0, 0, 0, 0,
                           SWP_FRAMECHANGED | SWP_NOACTIVATE |
                           SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER |
                           SWP_NOCOPYBITS);
            if (rect)
                ::GetClientRect(hwndClient, rect);

            return true;
        }
    }

    return false;
}


bool wxMDIClientWindow::CreateClient(wxMDIParentFrame *parent, long style)
{
    m_backgroundColour = wxSystemSettings::GetColour(wxSYS_COLOUR_APPWORKSPACE);

    CLIENTCREATESTRUCT ccs;
    m_windowStyle = style;
    m_parent = parent;

    ccs.hWindowMenu = GetMDIWindowMenu(parent);
    ccs.idFirstChild = wxFIRST_MDI_CHILD;

    DWORD msStyle = MDIS_ALLCHILDSTYLES | WS_VISIBLE | WS_CHILD |
                    WS_CLIPCHILDREN | WS_CLIPSIBLINGS;

    if ( style & wxHSCROLL )
        msStyle |= WS_HSCROLL;
    if ( style & wxVSCROLL )
        msStyle |= WS_VSCROLL;

    DWORD exStyle = WS_EX_CLIENTEDGE;

    wxWindowCreationHook hook(this);
    m_hWnd = (WXHWND)::CreateWindowEx
                       (
                        exStyle,
                        wxT("MDICLIENT"),
                        NULL,
                        msStyle,
                        0, 0, 0, 0,
                        GetWinHwnd(parent),
                        NULL,
                        wxGetInstance(),
                        (LPSTR)(LPCLIENTCREATESTRUCT)&ccs);
    if ( !m_hWnd )
    {
        wxLogLastError(wxT("CreateWindowEx(MDI client)"));

        return false;
    }

    SubclassWin(m_hWnd);

    return true;
}

void wxMDIClientWindow::OnScroll(wxScrollEvent& event)
{
                    
    if ( event.GetOrientation() == wxHORIZONTAL )
        m_scrollX = event.GetPosition();     else
        m_scrollY = event.GetPosition(); 
    event.Skip();
}

void wxMDIClientWindow::DoSetSize(int x, int y, int width, int height, int sizeFlags)
{
                
    const wxPoint oldPos = GetPosition();

    wxWindow::DoSetSize(x, y, width, height, sizeFlags | wxSIZE_FORCE);

    const wxPoint newPos = GetPosition();

    if ((newPos.x != oldPos.x) || (newPos.y != oldPos.y))
    {
        if (GetParent())
        {
            wxWindowList::compatibility_iterator node = GetParent()->GetChildren().GetFirst();
            while (node)
            {
                wxWindow *child = node->GetData();
                if (wxDynamicCast(child, wxMDIChildFrame))
                {
                   ::RedrawWindow(GetHwndOf(child),
                                  NULL,
                                  NULL,
                                  RDW_FRAME |
                                  RDW_ALLCHILDREN |
                                  RDW_INVALIDATE);
                }
                node = node->GetNext();
            }
        }
    }
}

void wxMDIChildFrame::OnIdle(wxIdleEvent& event)
{
                    if ( m_needsResize )
    {
        m_needsResize = false; 
        SendSizeEvent();
    }

    event.Skip();
}


namespace
{

void MDISetMenu(wxWindow *win, HMENU hmenuFrame, HMENU hmenuWindow)
{
    if ( hmenuFrame || hmenuWindow )
    {
                                ::SetLastError(ERROR_SUCCESS);

        if ( !::SendMessage(GetWinHwnd(win),
                            WM_MDISETMENU,
                            (WPARAM)hmenuFrame,
                            (LPARAM)hmenuWindow) )
        {
            const DWORD err = ::GetLastError();
            if ( err != ERROR_SUCCESS )
            {
                wxLogApiError(wxT("SendMessage(WM_MDISETMENU)"), err);
            }
        }
    }

        wxWindow *parent = win->GetParent();
    wxCHECK_RET( parent, wxT("MDI client without parent frame? weird...") );

    ::SendMessage(GetWinHwnd(win), WM_MDIREFRESHMENU, 0, 0L);

    ::DrawMenuBar(GetWinHwnd(parent));
}

class MenuIterator
{
public:
    explicit MenuIterator(HMENU hmenu)
        : m_hmenu(hmenu),
          m_numItems(::GetMenuItemCount(hmenu)),
          m_pos(-1)
    {
        m_mii.fMask = MIIM_STRING;
        m_mii.dwTypeData = m_buf;
    }

    bool GetNext(wxString& str)
    {
                for ( m_pos++; m_pos < m_numItems; m_pos++ )
        {
                                    m_mii.cch = WXSIZEOF(m_buf);

            if ( !::GetMenuItemInfo(m_hmenu, m_pos, TRUE, &m_mii) )
            {
                wxLogLastError(wxString::Format("GetMenuItemInfo(%d)", m_pos));
                continue;
            }

            if ( !m_mii.cch )
            {
                                continue;
            }

            str = m_buf;
            return true;
        }

        return false;
    }

    int GetPos() const { return m_pos; }

private:
    const HMENU m_hmenu;
    const int m_numItems;
    int m_pos;

    wxChar m_buf[1024];
    WinStruct<MENUITEMINFO> m_mii;

    wxDECLARE_NO_COPY_CLASS(MenuIterator);
};

void MDIInsertWindowMenu(wxWindow *win, WXHMENU hMenu, HMENU menuWin)
{
    HMENU hmenu = (HMENU)hMenu;

    if ( menuWin )
    {
                bool inserted = false;
        wxString buf;
        MenuIterator it(hmenu);
        while ( it.GetNext(buf) )
        {
            const wxString label = wxStripMenuCodes(buf);
            if ( label == wxGetStockLabel(wxID_HELP, wxSTOCK_NOFLAGS) )
            {
                inserted = true;
                ::InsertMenu(hmenu, it.GetPos(),
                             MF_BYPOSITION | MF_POPUP | MF_STRING,
                             (UINT_PTR)menuWin,
                             wxString(wxGetTranslation(WINDOW_MENU_LABEL)).t_str());
                break;
            }
        }

        if ( !inserted )
        {
            ::AppendMenu(hmenu, MF_POPUP,
                         (UINT_PTR)menuWin,
                         wxString(wxGetTranslation(WINDOW_MENU_LABEL)).t_str());
        }
    }

    MDISetMenu(win, hmenu, menuWin);
}

void MDIRemoveWindowMenu(wxWindow *win, WXHMENU hMenu)
{
    HMENU hmenu = (HMENU)hMenu;

    if ( hmenu )
    {
        wxString buf;
        MenuIterator it(hmenu);
        while ( it.GetNext(buf) )
        {
            if ( wxStrcmp(buf, wxGetTranslation(WINDOW_MENU_LABEL)) == 0 )
            {
                if ( !::RemoveMenu(hmenu, it.GetPos(), MF_BYPOSITION) )
                {
                    wxLogLastError(wxT("RemoveMenu"));
                }

                break;
            }
        }
    }

    if ( win )
    {
                MDISetMenu(win, hmenu, NULL);
    }
}

void UnpackMDIActivate(WXWPARAM wParam, WXLPARAM lParam,
                              WXWORD *activate, WXHWND *hwndAct, WXHWND *hwndDeact)
{
    *activate = true;
    *hwndAct = (WXHWND)lParam;
    *hwndDeact = (WXHWND)wParam;
}

} 
#endif 