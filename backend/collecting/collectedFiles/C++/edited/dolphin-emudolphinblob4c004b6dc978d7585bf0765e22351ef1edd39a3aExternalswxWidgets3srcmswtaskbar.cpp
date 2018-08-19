
#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#if wxUSE_TASKBARICON

#ifndef WX_PRECOMP
    #include "wx/window.h"
    #include "wx/frame.h"
    #include "wx/utils.h"
    #include "wx/menu.h"
    #include "wx/app.h"
#endif

#include "wx/msw/wrapshl.h"

#include <string.h>
#include "wx/taskbar.h"
#include "wx/platinfo.h"
#include "wx/msw/private.h"

#ifndef NIN_BALLOONTIMEOUT
    #define NIN_BALLOONTIMEOUT      0x0404
    #define NIN_BALLOONUSERCLICK    0x0405
#endif

#ifndef NIM_SETVERSION
    #define NIM_SETVERSION  0x00000004
#endif

#ifndef NIF_INFO
    #define NIF_INFO        0x00000010
#endif

#ifndef NOTIFYICONDATA_V2_SIZE
    #ifdef UNICODE
        #define NOTIFYICONDATA_V2_SIZE 0x03A8
    #else
        #define NOTIFYICONDATA_V2_SIZE 0x01E8
    #endif
#endif

static UINT gs_msgTaskbar = 0;
static UINT gs_msgRestartTaskbar = 0;


wxIMPLEMENT_DYNAMIC_CLASS(wxTaskBarIcon, wxEvtHandler);



class wxTaskBarIconWindow : public wxFrame
{
public:
    wxTaskBarIconWindow(wxTaskBarIcon *icon)
        : wxFrame(NULL, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0),
          m_icon(icon)
    {
    }

    WXLRESULT MSWWindowProc(WXUINT msg,
                            WXWPARAM wParam, WXLPARAM lParam)
    {
        if (msg == gs_msgRestartTaskbar || msg == gs_msgTaskbar)
        {
            return m_icon->WindowProc(msg, wParam, lParam);
        }
        else
        {
            return wxFrame::MSWWindowProc(msg, wParam, lParam);
        }
    }

private:
    wxTaskBarIcon *m_icon;
};



struct NotifyIconData : public NOTIFYICONDATA
{
    NotifyIconData(WXHWND hwnd)
    {
        memset(this, 0, sizeof(NOTIFYICONDATA));

                                cbSize = wxPlatformInfo::Get().CheckOSVersion(6, 0)
                    ? sizeof(NOTIFYICONDATA)
                    : NOTIFYICONDATA_V2_SIZE;

        hWnd = (HWND) hwnd;
        uCallbackMessage = gs_msgTaskbar;
        uFlags = NIF_MESSAGE;

                        uID = 99;
    }
};


wxTaskBarIcon::wxTaskBarIcon(wxTaskBarIconType WXUNUSED(iconType))
{
    m_win = NULL;
    m_iconAdded = false;
    RegisterWindowMessages();
}

wxTaskBarIcon::~wxTaskBarIcon()
{
    if ( m_iconAdded )
        RemoveIcon();

    if ( m_win )
    {
                                        delete m_win;
    }
}

bool wxTaskBarIcon::SetIcon(const wxIcon& icon, const wxString& tooltip)
{
                if (!m_win)
    {
        m_win = new wxTaskBarIconWindow(this);
    }

    m_icon = icon;
    m_strTooltip = tooltip;

    NotifyIconData notifyData(GetHwndOf(m_win));

    if (icon.IsOk())
    {
        notifyData.uFlags |= NIF_ICON;
        notifyData.hIcon = GetHiconOf(icon);
    }

            notifyData.uFlags |= NIF_TIP;
    if ( !tooltip.empty() )
    {
        wxStrlcpy(notifyData.szTip, tooltip.t_str(), WXSIZEOF(notifyData.szTip));
    }

    bool ok = Shell_NotifyIcon(m_iconAdded ? NIM_MODIFY
                                            : NIM_ADD, &notifyData) != 0;

    if ( !ok )
    {
        wxLogLastError(wxT("Shell_NotifyIcon(NIM_MODIFY/ADD)"));
    }

    if ( !m_iconAdded && ok )
        m_iconAdded = true;

    return ok;
}

#if wxUSE_TASKBARICON_BALLOONS

bool
wxTaskBarIcon::ShowBalloon(const wxString& title,
                           const wxString& text,
                           unsigned msec,
                           int flags,
                           const wxIcon& icon)
{
    wxCHECK_MSG( m_iconAdded, false,
                    wxT("can't be used before the icon is created") );

    const HWND hwnd = GetHwndOf(m_win);

            NotifyIconData notifyData(hwnd);
    notifyData.uFlags = 0;
    notifyData.uVersion = 3 ;

    if ( !Shell_NotifyIcon(NIM_SETVERSION, &notifyData) )
    {
        wxLogLastError(wxT("Shell_NotifyIcon(NIM_SETVERSION)"));
    }

        notifyData = NotifyIconData(hwnd);
    notifyData.uFlags |= NIF_INFO;
    notifyData.uTimeout = msec;
    wxStrlcpy(notifyData.szInfo, text.t_str(), WXSIZEOF(notifyData.szInfo));
    wxStrlcpy(notifyData.szInfoTitle, title.t_str(),
                WXSIZEOF(notifyData.szInfoTitle));

    wxUnusedVar(icon); 
#ifdef NIIF_LARGE_ICON
        if ( icon.IsOk() && wxPlatformInfo::Get().CheckOSVersion(6, 0) )
    {
        notifyData.hBalloonIcon = GetHiconOf(icon);
        notifyData.dwInfoFlags |= NIIF_USER | NIIF_LARGE_ICON;
    }
    else
#endif
    if ( flags & wxICON_INFORMATION )
        notifyData.dwInfoFlags |= NIIF_INFO;
    else if ( flags & wxICON_WARNING )
        notifyData.dwInfoFlags |= NIIF_WARNING;
    else if ( flags & wxICON_ERROR )
        notifyData.dwInfoFlags |= NIIF_ERROR;

    bool ok = Shell_NotifyIcon(NIM_MODIFY, &notifyData) != 0;
    if ( !ok )
    {
        wxLogLastError(wxT("Shell_NotifyIcon(NIM_MODIFY)"));
    }

    return ok;
}

#endif 
bool wxTaskBarIcon::RemoveIcon()
{
    if (!m_iconAdded)
        return false;

    m_iconAdded = false;

    NotifyIconData notifyData(GetHwndOf(m_win));

    bool ok = Shell_NotifyIcon(NIM_DELETE, &notifyData) != 0;
    if ( !ok )
    {
        wxLogLastError(wxT("Shell_NotifyIcon(NIM_DELETE)"));
    }

    return ok;
}

#if wxUSE_MENUS
bool wxTaskBarIcon::PopupMenu(wxMenu *menu)
{
    wxASSERT_MSG( m_win != NULL, wxT("taskbar icon not initialized") );

    static bool s_inPopup = false;

    if (s_inPopup)
        return false;

    s_inPopup = true;

    int         x, y;
    wxGetMousePosition(&x, &y);

    m_win->Move(x, y);

    m_win->PushEventHandler(this);

    menu->UpdateUI();

                ::SetForegroundWindow(GetHwndOf(m_win));

    bool rval = m_win->PopupMenu(menu, 0, 0);

    ::PostMessage(GetHwndOf(m_win), WM_NULL, 0, 0L);

    m_win->PopEventHandler(false);

    s_inPopup = false;

    return rval;
}
#endif 
void wxTaskBarIcon::RegisterWindowMessages()
{
    static bool s_registered = false;

    if ( !s_registered )
    {
                gs_msgRestartTaskbar = RegisterWindowMessage(wxT("TaskbarCreated"));

                gs_msgTaskbar = ::RegisterWindowMessage(wxT("wxTaskBarIconMessage"));

        s_registered = true;
    }
}


long wxTaskBarIcon::WindowProc(unsigned int msg,
                               unsigned int WXUNUSED(wParam),
                               long lParam)
{
    if ( msg == gs_msgRestartTaskbar )       {
        m_iconAdded = false;
        SetIcon(m_icon, m_strTooltip);
        return 0;
    }

        wxASSERT( msg == gs_msgTaskbar );

    wxEventType eventType = 0;
    switch ( lParam )
    {
        case WM_LBUTTONDOWN:
            eventType = wxEVT_TASKBAR_LEFT_DOWN;
            break;

        case WM_LBUTTONUP:
            eventType = wxEVT_TASKBAR_LEFT_UP;
            break;

        case WM_RBUTTONDOWN:
            eventType = wxEVT_TASKBAR_RIGHT_DOWN;
            break;

        case WM_RBUTTONUP:
            eventType = wxEVT_TASKBAR_RIGHT_UP;
            break;

        case WM_LBUTTONDBLCLK:
            eventType = wxEVT_TASKBAR_LEFT_DCLICK;
            break;

        case WM_RBUTTONDBLCLK:
            eventType = wxEVT_TASKBAR_RIGHT_DCLICK;
            break;

        case WM_MOUSEMOVE:
            eventType = wxEVT_TASKBAR_MOVE;
            break;

        case NIN_BALLOONTIMEOUT:
            eventType = wxEVT_TASKBAR_BALLOON_TIMEOUT;
            break;

        case NIN_BALLOONUSERCLICK:
            eventType = wxEVT_TASKBAR_BALLOON_CLICK;
            break;
    }

    if ( eventType )
    {
        wxTaskBarIconEvent event(eventType, this);

        ProcessEvent(event);
    }

    return 0;
}

#endif 
