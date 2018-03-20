


#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#include "wx/toplevel.h"

#ifndef WX_PRECOMP
    #include "wx/app.h"
    #include "wx/dialog.h"
    #include "wx/string.h"
    #include "wx/log.h"
    #include "wx/intl.h"
    #include "wx/frame.h"
    #include "wx/menu.h"
    #include "wx/containr.h"            #include "wx/module.h"
#endif 
#include "wx/dynlib.h"
#include "wx/tooltip.h"

#include "wx/msw/private.h"

#include "wx/msw/winundef.h"
#include "wx/msw/missing.h"

#include "wx/display.h"

#ifndef ICON_BIG
    #define ICON_BIG 1
#endif

#ifndef ICON_SMALL
    #define ICON_SMALL 0
#endif

LONG APIENTRY
wxDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);


class wxTLWHiddenParentModule : public wxModule
{
public:
        virtual bool OnInit();
    virtual void OnExit();

        static HWND GetHWND();

private:
        static HWND ms_hwnd;

        static const wxChar *ms_className;

    wxDECLARE_DYNAMIC_CLASS(wxTLWHiddenParentModule);
};

wxIMPLEMENT_DYNAMIC_CLASS(wxTLWHiddenParentModule, wxModule);


wxBEGIN_EVENT_TABLE(wxTopLevelWindowMSW, wxTopLevelWindowBase)
    EVT_ACTIVATE(wxTopLevelWindowMSW::OnActivate)
wxEND_EVENT_TABLE()


void wxTopLevelWindowMSW::Init()
{
    m_iconized =
    m_maximizeOnShow = false;

        m_fsStyle = 0;
    m_fsOldWindowStyle = 0;
    m_fsIsMaximized = false;
    m_fsIsShowing = false;

    m_winLastFocused = NULL;

    m_menuSystem = NULL;
}

WXDWORD wxTopLevelWindowMSW::MSWGetStyle(long style, WXDWORD *exflags) const
{
            WXDWORD msflags = wxWindow::MSWGetStyle
                      (
                        (style & ~wxBORDER_MASK) | wxBORDER_NONE, exflags
                      ) & ~WS_CHILD & ~WS_VISIBLE;

                        
        if ( ( style & wxRESIZE_BORDER ) && !IsAlwaysMaximized())
        msflags |= WS_THICKFRAME;
    else if ( exflags && ((style & wxBORDER_DOUBLE) || (style & wxBORDER_RAISED)) )
        *exflags |= WS_EX_DLGMODALFRAME;
    else if ( !(style & wxBORDER_NONE) )
        msflags |= WS_BORDER;
    else
        msflags |= WS_POPUP;

    if ( style & wxCAPTION )
        msflags |= WS_CAPTION;
    else
        msflags |= WS_POPUP;

    
                        if ( !(GetExtraStyle() & wxWS_EX_CONTEXTHELP) )
    {
        if ( style & wxMINIMIZE_BOX )
            msflags |= WS_MINIMIZEBOX;
        if ( style & wxMAXIMIZE_BOX )
            msflags |= WS_MAXIMIZEBOX;
    }

            if ( style & (wxSYSTEM_MENU | wxCLOSE_BOX) )
        msflags |= WS_SYSMENU;

            if ( style & wxMINIMIZE )
        msflags |= WS_MINIMIZE;

    if ( style & wxMAXIMIZE )
        msflags |= WS_MAXIMIZE;

        if ( style & wxTINY_CAPTION )
        msflags |= WS_CAPTION;

    if ( exflags )
    {
        if ( !(GetExtraStyle() & wxTOPLEVEL_EX_DIALOG) )
        {
            if ( style & wxFRAME_TOOL_WINDOW )
            {
                                *exflags |= WS_EX_TOOLWINDOW;

                                style |= wxFRAME_NO_TASKBAR;
            }

                                                                                                                                                            if ( !(style & wxFRAME_NO_TASKBAR) && GetParent() )
            {
                                *exflags |= WS_EX_APPWINDOW;
            }
                    }

        if ( GetExtraStyle() & wxWS_EX_CONTEXTHELP )
            *exflags |= WS_EX_CONTEXTHELP;

        if ( style & wxSTAY_ON_TOP )
            *exflags |= WS_EX_TOPMOST;
    }

    return msflags;
}

WXHWND wxTopLevelWindowMSW::MSWGetParent() const
{
                    HWND hwndParent = NULL;
    if ( HasFlag(wxFRAME_FLOAT_ON_PARENT) )
    {
        const wxWindow *parent = GetParent();

        if ( !parent )
        {
                        wxFAIL_MSG( wxT("wxFRAME_FLOAT_ON_PARENT but no parent?") );
        }
        else
        {
            hwndParent = GetHwndOf(parent);
        }
    }
    
            if ( HasFlag(wxFRAME_NO_TASKBAR) && !hwndParent )
    {
                hwndParent = wxTLWHiddenParentModule::GetHWND();
    }

    return (WXHWND)hwndParent;
}

WXLRESULT wxTopLevelWindowMSW::MSWWindowProc(WXUINT message, WXWPARAM wParam, WXLPARAM lParam)
{
    WXLRESULT rc = 0;
    bool processed = false;

    switch ( message )
    {
        case WM_SYSCOMMAND:
            {
                                                                                                                                unsigned id = wParam & 0xfff0;

                                                                                                                                                                if ( id == SC_MINIMIZE )
                {
                                                                                DoSaveLastFocus();
                }
                else if ( id == SC_RESTORE )
                {
                                                                                processed = true;
                    rc = wxTopLevelWindowBase::MSWWindowProc(message,
                                                             wParam, lParam);

                    DoRestoreLastFocus();
                }

#ifndef __WXUNIVERSAL__
                                                                                                if ( m_menuSystem && id < SC_SIZE )
                {
                    if ( m_menuSystem->MSWCommand(0 , id) )
                        processed = true;
                }
#endif             }
            break;
    }

    if ( !processed )
        rc = wxTopLevelWindowBase::MSWWindowProc(message, wParam, lParam);

    return rc;
}

bool wxTopLevelWindowMSW::CreateDialog(const void *dlgTemplate,
                                       const wxString& title,
                                       const wxPoint& pos,
                                       const wxSize& size)
{
        wxWindow * const
        parent = static_cast<wxDialog *>(this)->GetParentForModalDialog();

    m_hWnd = (WXHWND)::CreateDialogIndirect
                       (
                        wxGetInstance(),
                        (DLGTEMPLATE*)dlgTemplate,
                        parent ? GetHwndOf(parent) : NULL,
                        (DLGPROC)wxDlgProc
                       );

    if ( !m_hWnd )
    {
        wxFAIL_MSG(wxT("Failed to create dialog. Incorrect DLGTEMPLATE?"));

        wxLogSysError(wxT("Can't create dialog using memory template"));

        return false;
    }

            if ( HasExtraStyle(wxWS_EX_CONTEXTHELP) )
    {
        wxFrame *winTop = wxDynamicCast(wxTheApp->GetTopWindow(), wxFrame);
        if ( winTop )
        {
            wxIcon icon = winTop->GetIcon();
            if ( icon.IsOk() )
            {
                ::SendMessage(GetHwnd(), WM_SETICON,
                              (WPARAM)TRUE,
                              (LPARAM)GetHiconOf(icon));
            }
        }
    }

    if ( !title.empty() )
    {
        ::SetWindowText(GetHwnd(), title.t_str());
    }

    SubclassWin(m_hWnd);

        int x, y, w, h;
    (void)MSWGetCreateWindowCoords(pos, size, x, y, w, h);

    if ( x == (int)CW_USEDEFAULT )
    {
                ::SetWindowPos(GetHwnd(), 0,
                       0, 0, w, h,
                       SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
    }
    else     {
        if ( !::MoveWindow(GetHwnd(), x, y, w, h, FALSE) )
        {
            wxLogLastError(wxT("MoveWindow"));
        }
    }

    return true;
}

bool wxTopLevelWindowMSW::CreateFrame(const wxString& title,
                                      const wxPoint& pos,
                                      const wxSize& size)
{
    WXDWORD exflags;
    WXDWORD flags = MSWGetCreateWindowFlags(&exflags);

    const wxSize sz = IsAlwaysMaximized() ? wxDefaultSize : size;

    if ( wxApp::MSWGetDefaultLayout(m_parent) == wxLayout_RightToLeft )
        exflags |= WS_EX_LAYOUTRTL;

    return MSWCreate(MSWGetRegisteredClassName(),
                     title.t_str(), pos, sz, flags, exflags);
}

bool wxTopLevelWindowMSW::Create(wxWindow *parent,
                                 wxWindowID id,
                                 const wxString& title,
                                 const wxPoint& pos,
                                 const wxSize& size,
                                 long style,
                                 const wxString& name)
{
    wxSize sizeReal = size;
    if ( !sizeReal.IsFullySpecified() )
    {
        sizeReal.SetDefaults(GetDefaultSize());
    }

                wxTopLevelWindows.Append(this);

    bool ret = CreateBase(parent, id, pos, sizeReal, style, name);
    if ( !ret )
        return false;

    if ( parent )
        parent->AddChild(this);

    if ( GetExtraStyle() & wxTOPLEVEL_EX_DIALOG )
    {
                                
                        static const int dlgsize = sizeof(DLGTEMPLATE) + (sizeof(WORD) * 3);
        DLGTEMPLATE *dlgTemplate = (DLGTEMPLATE *)malloc(dlgsize);
        memset(dlgTemplate, 0, dlgsize);

                dlgTemplate->x  = 34;
        dlgTemplate->y  = 22;
        dlgTemplate->cx = 144;
        dlgTemplate->cy = 75;

                                                        WXDWORD dwExtendedStyle;
        dlgTemplate->style = MSWGetStyle(style, &dwExtendedStyle);
        dlgTemplate->dwExtendedStyle = dwExtendedStyle;

                dlgTemplate->style |= WS_POPUP;

        if ( wxApp::MSWGetDefaultLayout(m_parent) == wxLayout_RightToLeft )
        {
            dlgTemplate->dwExtendedStyle |= WS_EX_LAYOUTRTL;
        }

                if ( style & (wxRESIZE_BORDER | wxCAPTION) )
            dlgTemplate->style |= DS_MODALFRAME;

        ret = CreateDialog(dlgTemplate, title, pos, sizeReal);
        free(dlgTemplate);
    }
    else     {
        ret = CreateFrame(title, pos, sizeReal);
    }

    if ( ret && !(GetWindowStyleFlag() & wxCLOSE_BOX) )
    {
        EnableCloseButton(false);
    }

                    if ( ret )
    {
        MSWUpdateUIState(UIS_INITIALIZE);
    }

    return ret;
}

wxTopLevelWindowMSW::~wxTopLevelWindowMSW()
{
    delete m_menuSystem;

    SendDestroyEvent();

                    if ( HasFlag(wxFRAME_FLOAT_ON_PARENT) )
    {
        wxWindow *parent = GetParent();
        if ( parent )
        {
            ::BringWindowToTop(GetHwndOf(parent));
        }
    }
}


void wxTopLevelWindowMSW::DoShowWindow(int nShowCmd)
{
    ::ShowWindow(GetHwnd(), nShowCmd);

        if ( nShowCmd != SW_HIDE )
    {
                        m_iconized = nShowCmd == SW_MINIMIZE;
    }

#if wxUSE_TOOLTIPS
        wxToolTip::UpdateVisibility();
#endif }

void wxTopLevelWindowMSW::ShowWithoutActivating()
{
    if ( !wxWindowBase::Show(true) )
        return;

    DoShowWindow(SW_SHOWNA);
}

bool wxTopLevelWindowMSW::Show(bool show)
{
        if ( !wxWindowBase::Show(show) )
        return false;

    int nShowCmd;
    if ( show )
    {
        if ( m_maximizeOnShow )
        {
                        nShowCmd = SW_MAXIMIZE;

            m_maximizeOnShow = false;
        }
        else if ( m_iconized )
        {
                                    nShowCmd = SW_MINIMIZE;
        }
        else if ( ::IsIconic(GetHwnd()) )
        {
                                                            if ( HasFlag(wxFRAME_TOOL_WINDOW) || !IsEnabled() )
                nShowCmd = SW_SHOWNOACTIVATE;
            else
                nShowCmd = SW_RESTORE;
        }
        else         {
                                                            if ( HasFlag(wxFRAME_TOOL_WINDOW) || !IsEnabled() )
                nShowCmd = SW_SHOWNA;
            else
                nShowCmd = SW_SHOW;
        }
    }
    else     {
        nShowCmd = SW_HIDE;
    }

#if wxUSE_DEFERRED_SIZING
                                    m_pendingSize = wxDefaultSize;
#endif 
    DoShowWindow(nShowCmd);

    return true;
}

void wxTopLevelWindowMSW::Raise()
{
    ::SetForegroundWindow(GetHwnd());
}


void wxTopLevelWindowMSW::Maximize(bool maximize)
{
    if ( IsShown() )
    {
                DoShowWindow(maximize ? SW_MAXIMIZE : SW_RESTORE);
    }
    else     {
                        m_maximizeOnShow = maximize;

#if wxUSE_DEFERRED_SIZING
                                        if ( maximize )
        {
                                                                                                            m_pendingSize = wxGetClientDisplayRect().GetSize();
        }
        #endif     }
}

bool wxTopLevelWindowMSW::IsMaximized() const
{
    return IsAlwaysMaximized() ||
           (::IsZoomed(GetHwnd()) != 0) ||
           m_maximizeOnShow;
}

void wxTopLevelWindowMSW::Iconize(bool iconize)
{
    if ( iconize == m_iconized )
    {
                        return;
    }

    if ( IsShown() )
    {
                DoShowWindow(iconize ? SW_MINIMIZE : SW_RESTORE);
    }
    else     {
                        m_iconized = iconize;
    }
}

bool wxTopLevelWindowMSW::IsIconized() const
{
    if ( !IsShown() )
        return m_iconized;

                    return ::IsIconic(GetHwnd()) != 0;
}

void wxTopLevelWindowMSW::Restore()
{
    DoShowWindow(SW_RESTORE);
}

void wxTopLevelWindowMSW::SetLayoutDirection(wxLayoutDirection dir)
{
    if ( dir == wxLayout_Default )
        dir = wxApp::MSWGetDefaultLayout(m_parent);

    if ( dir != wxLayout_Default )
        wxTopLevelWindowBase::SetLayoutDirection(dir);
}


void wxTopLevelWindowMSW::DoGetPosition(int *x, int *y) const
{
    if ( IsIconized() )
    {
        WINDOWPLACEMENT wp;
        wp.length = sizeof(WINDOWPLACEMENT);
        if ( ::GetWindowPlacement(GetHwnd(), &wp) )
        {
            RECT& rc = wp.rcNormalPosition;

                                    if ( !HasFlag(wxFRAME_TOOL_WINDOW) )
            {
                                                int n = wxDisplay::GetFromWindow(this);
                wxDisplay dpy(n == wxNOT_FOUND ? 0 : n);
                const wxPoint ptOfs = dpy.GetClientArea().GetPosition() -
                                      dpy.GetGeometry().GetPosition();

                rc.left += ptOfs.x;
                rc.top += ptOfs.y;
            }

            if ( x )
                *x = rc.left;
            if ( y )
                *y = rc.top;

            return;
        }

        wxLogLastError(wxT("GetWindowPlacement"));
    }
    
    wxTopLevelWindowBase::DoGetPosition(x, y);
}

void wxTopLevelWindowMSW::DoGetSize(int *width, int *height) const
{
    if ( IsIconized() )
    {
        WINDOWPLACEMENT wp;
        wp.length = sizeof(WINDOWPLACEMENT);
        if ( ::GetWindowPlacement(GetHwnd(), &wp) )
        {
            const RECT& rc = wp.rcNormalPosition;

            if ( width )
                *width = rc.right - rc.left;
            if ( height )
                *height = rc.bottom - rc.top;

            return;
        }

        wxLogLastError(wxT("GetWindowPlacement"));
    }
    
    wxTopLevelWindowBase::DoGetSize(width, height);
}

void
wxTopLevelWindowMSW::MSWGetCreateWindowCoords(const wxPoint& pos,
                                              const wxSize& size,
                                              int& x, int& y,
                                              int& w, int& h) const
{
        if ( pos.x == wxDefaultCoord )
    {
                        x =
        y = CW_USEDEFAULT;
    }
    else
    {
                                        static const int DEFAULT_Y = 200;

        x = pos.x;
        y = pos.y == wxDefaultCoord ? DEFAULT_Y : pos.y;
    }

    if ( size.x == wxDefaultCoord || size.y == wxDefaultCoord )
    {
                                                                                                                                                        wxSize sizeReal = size;
        sizeReal.SetDefaults(GetDefaultSize());

        w = sizeReal.x;
        h = sizeReal.y;
    }
    else
    {
        w = size.x;
        h = size.y;
    }
}


bool wxTopLevelWindowMSW::ShowFullScreen(bool show, long style)
{
    if ( show == IsFullScreen() )
    {
                return true;
    }

    m_fsIsShowing = show;

    if ( show )
    {
        m_fsStyle = style;

        
                m_fsOldWindowStyle = GetWindowLong(GetHwnd(), GWL_STYLE);

                m_fsOldSize = GetRect();
        m_fsIsMaximized = IsMaximized();

                LONG newStyle = m_fsOldWindowStyle;
        LONG offFlags = 0;

        if (style & wxFULLSCREEN_NOBORDER)
        {
            offFlags |= WS_BORDER;
            offFlags |= WS_THICKFRAME;
        }
        if (style & wxFULLSCREEN_NOCAPTION)
            offFlags |= WS_CAPTION | WS_SYSMENU;

        newStyle &= ~offFlags;

                                        newStyle |= WS_POPUP;

                ::SetWindowLong(GetHwnd(), GWL_STYLE, newStyle);

        wxRect rect;
#if wxUSE_DISPLAY
                int dpy = wxDisplay::GetFromWindow(this);
        if ( dpy != wxNOT_FOUND )
        {
            rect = wxDisplay(dpy).GetGeometry();
        }
        else #endif         {
                        wxCopyRECTToRect(wxGetWindowRect(::GetDesktopWindow()), rect);
        }

        SetSize(rect);

                long flags = SWP_FRAMECHANGED;

                        if ( !IsShown() )
        {
                                                            wxWindowBase::Show();

            flags |= SWP_SHOWWINDOW;
        }

        SetWindowPos(GetHwnd(), HWND_TOP,
                     rect.x, rect.y, rect.width, rect.height,
                     flags);

                wxSizeEvent event(rect.GetSize(), GetId());
        event.SetEventObject(this);
        HandleWindowEvent(event);
    }
    else     {
        Maximize(m_fsIsMaximized);
        SetWindowLong(GetHwnd(),GWL_STYLE, m_fsOldWindowStyle);
        SetWindowPos(GetHwnd(),HWND_TOP,m_fsOldSize.x, m_fsOldSize.y,
            m_fsOldSize.width, m_fsOldSize.height, SWP_FRAMECHANGED);
    }

    return true;
}


void wxTopLevelWindowMSW::SetTitle( const wxString& title)
{
    SetLabel(title);
}

wxString wxTopLevelWindowMSW::GetTitle() const
{
    return GetLabel();
}

bool wxTopLevelWindowMSW::DoSelectAndSetIcon(const wxIconBundle& icons,
                                             int smX,
                                             int smY,
                                             int i)
{
    const wxSize size(::GetSystemMetrics(smX), ::GetSystemMetrics(smY));

    wxIcon icon = icons.GetIcon(size, wxIconBundle::FALLBACK_NEAREST_LARGER);

    if ( !icon.IsOk() )
        return false;

    ::SendMessage(GetHwnd(), WM_SETICON, i, (LPARAM)GetHiconOf(icon));
    return true;
}

void wxTopLevelWindowMSW::SetIcons(const wxIconBundle& icons)
{
    wxTopLevelWindowBase::SetIcons(icons);

    if ( icons.IsEmpty() )
    {
                        wxASSERT_MSG( m_icons.IsEmpty(), "unsetting icons doesn't work" );
        return;
    }

    DoSelectAndSetIcon(icons, SM_CXSMICON, SM_CYSMICON, ICON_SMALL);
    DoSelectAndSetIcon(icons, SM_CXICON, SM_CYICON, ICON_BIG);
}

bool wxTopLevelWindowMSW::EnableCloseButton(bool enable)
{
        HMENU hmenu = GetSystemMenu(GetHwnd(), FALSE );
    if ( !hmenu )
    {
                        return !enable;
    }

            if ( ::EnableMenuItem(hmenu, SC_CLOSE,
                          MF_BYCOMMAND |
                          (enable ? MF_ENABLED : MF_GRAYED)) == -1 )
    {
        wxLogLastError(wxT("EnableMenuItem(SC_CLOSE)"));

        return false;
    }
        if ( !::DrawMenuBar(GetHwnd()) )
    {
        wxLogLastError(wxT("DrawMenuBar"));
    }

    return true;
}


bool wxTopLevelWindowMSW::EnableMaximizeButton(bool enable)
{
    if ( ( HasFlag(wxCAPTION) &&
         ( HasFlag(wxCLOSE_BOX) || HasFlag(wxSYSTEM_MENU) ) ) &&
         HasFlag(wxMAXIMIZE_BOX) )
    {
        if ( enable )
        {
            SetWindowStyleFlag(GetWindowStyleFlag() | wxMAXIMIZE_BOX);
        }
        else
        {
            SetWindowStyleFlag(GetWindowStyleFlag() ^ wxMAXIMIZE_BOX);
                        wxWindowBase::SetWindowStyleFlag(GetWindowStyle() | wxMAXIMIZE_BOX);
        }

        return true;
    }

    return false;
}

bool wxTopLevelWindowMSW::EnableMinimizeButton(bool enable)
{
    if ( ( HasFlag(wxCAPTION) &&
         ( HasFlag(wxCLOSE_BOX) || HasFlag(wxSYSTEM_MENU) ) ) &&
         HasFlag(wxMINIMIZE_BOX) )
    {
        if ( enable )
        {
            SetWindowStyleFlag(GetWindowStyleFlag() | wxMINIMIZE_BOX);
        }
        else
        {
            SetWindowStyleFlag(GetWindowStyleFlag() ^ wxMINIMIZE_BOX);
                        wxWindowBase::SetWindowStyleFlag(GetWindowStyle() | wxMINIMIZE_BOX);
        }

        return true;
    }

    return false;
}

void wxTopLevelWindowMSW::RequestUserAttention(int flags)
{
#if defined(FLASHW_STOP)
    WinStruct<FLASHWINFO> fwi;
    fwi.hwnd = GetHwnd();
    fwi.dwFlags = FLASHW_ALL;
    if ( flags & wxUSER_ATTENTION_INFO )
    {
                fwi.uCount = 3;
    }
    else     {
                fwi.dwFlags |= FLASHW_TIMERNOFG;
    }

    ::FlashWindowEx(&fwi);
#else
    {
        wxUnusedVar(flags);
        ::FlashWindow(GetHwnd(), TRUE);
    }
#endif }

wxMenu *wxTopLevelWindowMSW::MSWGetSystemMenu() const
{
#ifndef __WXUNIVERSAL__
    if ( !m_menuSystem )
    {
        HMENU hmenu = ::GetSystemMenu(GetHwnd(), FALSE);
        if ( !hmenu )
        {
            wxLogLastError(wxT("GetSystemMenu()"));
            return NULL;
        }

        wxTopLevelWindowMSW * const
            self = const_cast<wxTopLevelWindowMSW *>(this);

        self->m_menuSystem = wxMenu::MSWNewFromHMENU(hmenu);

                                                                                m_menuSystem->SetInvokingWindow(self);
    }
#endif 
    return m_menuSystem;
}


bool wxTopLevelWindowMSW::SetTransparent(wxByte alpha)
{
    LONG exstyle = GetWindowLong(GetHwnd(), GWL_EXSTYLE);

        if (alpha == 255)
    {
        SetWindowLong(GetHwnd(), GWL_EXSTYLE, exstyle & ~WS_EX_LAYERED);
        Refresh();
        return true;
    }

        if ((exstyle & WS_EX_LAYERED) == 0 )
        SetWindowLong(GetHwnd(), GWL_EXSTYLE, exstyle | WS_EX_LAYERED);

    if ( ::SetLayeredWindowAttributes(GetHwnd(), 0, (BYTE)alpha, LWA_ALPHA) )
        return true;

    return false;
}

bool wxTopLevelWindowMSW::CanSetTransparent()
{
    return true;
}

void wxTopLevelWindowMSW::DoFreeze()
{
            }

void wxTopLevelWindowMSW::DoThaw()
{
    }



void wxTopLevelWindowMSW::DoSaveLastFocus()
{
    if ( m_iconized )
        return;

        wxWindow* const winFocus = FindFocus();

    m_winLastFocused = IsDescendant(winFocus) ? winFocus : NULL;
}

void wxTopLevelWindowMSW::DoRestoreLastFocus()
{
    wxWindow *parent = m_winLastFocused ? m_winLastFocused->GetParent()
                                        : NULL;
    if ( !parent )
    {
        parent = this;
    }

    wxSetFocusToChild(parent, &m_winLastFocused);
}

void wxTopLevelWindowMSW::OnActivate(wxActivateEvent& event)
{
    if ( event.GetActive() )
    {
                                if ( m_iconized )
        {
            event.Skip();
            return;
        }

                                                wxLogTrace(wxT("focus"), wxT("wxTLW %p activated."), m_hWnd);

        wxWindow* const winFocus = FindFocus();
        if ( winFocus == this || !IsDescendant(winFocus) )
            DoRestoreLastFocus();
    }
    else     {
        DoSaveLastFocus();

        wxLogTrace(wxT("focus"),
                   wxT("wxTLW %p deactivated, last focused: %p."),
                   m_hWnd,
                   m_winLastFocused ? GetHwndOf(m_winLastFocused) : NULL);

        event.Skip();
    }
}

LONG APIENTRY
wxDlgProc(HWND WXUNUSED(hDlg),
          UINT message,
          WPARAM WXUNUSED(wParam),
          LPARAM WXUNUSED(lParam))
{
    switch ( message )
    {
        case WM_INITDIALOG:
        {
                                                return FALSE;
        }
    }

            return FALSE;
}


HWND wxTLWHiddenParentModule::ms_hwnd = NULL;

const wxChar *wxTLWHiddenParentModule::ms_className = NULL;

bool wxTLWHiddenParentModule::OnInit()
{
    ms_hwnd = NULL;
    ms_className = NULL;

    return true;
}

void wxTLWHiddenParentModule::OnExit()
{
    if ( ms_hwnd )
    {
        if ( !::DestroyWindow(ms_hwnd) )
        {
            wxLogLastError(wxT("DestroyWindow(hidden TLW parent)"));
        }

        ms_hwnd = NULL;
    }

    if ( ms_className )
    {
        if ( !::UnregisterClass(ms_className, wxGetInstance()) )
        {
            wxLogLastError(wxT("UnregisterClass(\"wxTLWHiddenParent\")"));
        }

        ms_className = NULL;
    }
}


HWND wxTLWHiddenParentModule::GetHWND()
{
    if ( !ms_hwnd )
    {
        if ( !ms_className )
        {
            static const wxChar *HIDDEN_PARENT_CLASS = wxT("wxTLWHiddenParent");

            WNDCLASS wndclass;
            wxZeroMemory(wndclass);

            wndclass.lpfnWndProc   = DefWindowProc;
            wndclass.hInstance     = wxGetInstance();
            wndclass.lpszClassName = HIDDEN_PARENT_CLASS;

            if ( !::RegisterClass(&wndclass) )
            {
                wxLogLastError(wxT("RegisterClass(\"wxTLWHiddenParent\")"));
            }
            else
            {
                ms_className = HIDDEN_PARENT_CLASS;
            }
        }

        ms_hwnd = ::CreateWindow(ms_className, wxEmptyString, 0, 0, 0, 0, 0, NULL,
                                 (HMENU)NULL, wxGetInstance(), NULL);
        if ( !ms_hwnd )
        {
            wxLogLastError(wxT("CreateWindow(hidden TLW parent)"));
        }
    }

    return ms_hwnd;
}
