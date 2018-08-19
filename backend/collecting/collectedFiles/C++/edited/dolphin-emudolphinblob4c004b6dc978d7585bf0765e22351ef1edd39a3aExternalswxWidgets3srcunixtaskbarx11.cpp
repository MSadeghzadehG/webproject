


#include "wx/wxprec.h"

#if wxUSE_TASKBARICON && !defined(__WXGTK20__)

#include "wx/taskbar.h"

#ifndef  WX_PRECOMP
    #include "wx/log.h"
    #include "wx/frame.h"
    #include "wx/dcclient.h"
    #include "wx/statbmp.h"
    #include "wx/sizer.h"
    #include "wx/bitmap.h"
    #include "wx/image.h"
#endif

#ifdef __VMS
#pragma message disable nosimpint
#endif
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#ifdef __VMS
#pragma message enable nosimpint
#endif


    class WXDLLIMPEXP_ADV wxTaskBarIconAreaBase : public wxFrame
    {
    public:
        wxTaskBarIconAreaBase()
            : wxFrame(NULL, wxID_ANY, wxT("systray icon"),
                      wxDefaultPosition, wxDefaultSize,
                      wxDEFAULT_FRAME_STYLE | wxFRAME_NO_TASKBAR |
                      wxSIMPLE_BORDER | wxFRAME_SHAPED) {}

        static bool IsProtocolSupported() { return false; }
    };


#if defined(__WXGTK__)
    #include <gtk/gtk.h>
    #include <gdk/gdkx.h>
    #define GetDisplay()        GDK_DISPLAY()
    #define GetXWindow(wxwin)   GDK_WINDOW_XWINDOW((wxwin)->m_widget->window)
#elif defined(__WXX11__) || defined(__WXMOTIF__)
    #include "wx/x11/privx.h"
    #define GetDisplay()        ((Display*)wxGlobalDisplay())
    #define GetXWindow(wxwin)   ((Window)(wxwin)->GetHandle())
#else
    #error "You must define X11 accessors for this port!"
#endif



class WXDLLIMPEXP_ADV wxTaskBarIconArea : public wxTaskBarIconAreaBase
{
public:
    wxTaskBarIconArea(wxTaskBarIcon *icon, const wxBitmap &bmp);
    void SetTrayIcon(const wxBitmap& bmp);
    bool IsOk() { return true; }

protected:
    void SetLegacyWMProperties();

    void OnSizeChange(wxSizeEvent& event);
    void OnPaint(wxPaintEvent& evt);
    void OnMouseEvent(wxMouseEvent& event);
    void OnMenuEvent(wxCommandEvent& event);

    wxTaskBarIcon *m_icon;
    wxPoint        m_pos;
    wxBitmap       m_bmp;

    wxDECLARE_EVENT_TABLE();
};

wxBEGIN_EVENT_TABLE(wxTaskBarIconArea, wxTaskBarIconAreaBase)
    EVT_SIZE(wxTaskBarIconArea::OnSizeChange)
    EVT_MOUSE_EVENTS(wxTaskBarIconArea::OnMouseEvent)
    EVT_MENU(wxID_ANY, wxTaskBarIconArea::OnMenuEvent)
    EVT_PAINT(wxTaskBarIconArea::OnPaint)
wxEND_EVENT_TABLE()

wxTaskBarIconArea::wxTaskBarIconArea(wxTaskBarIcon *icon, const wxBitmap &bmp)
    : wxTaskBarIconAreaBase(), m_icon(icon), m_bmp(bmp)
{
            SetClientSize(wxSize(bmp.GetWidth(), bmp.GetHeight()));

    SetTrayIcon(bmp);

    if (!IsProtocolSupported())
    {
        wxLogTrace(wxT("systray"),
                   wxT("using legacy KDE1,2 and GNOME 1.2 methods"));
        SetLegacyWMProperties();
    }
}

void wxTaskBarIconArea::SetTrayIcon(const wxBitmap& bmp)
{
    m_bmp = bmp;

        wxSize winsize(GetClientSize());
    wxSize bmpsize(m_bmp.GetWidth(), m_bmp.GetHeight());
    wxSize iconsize(wxMin(winsize.x, bmpsize.x), wxMin(winsize.y, bmpsize.y));

        if (bmpsize != iconsize)
    {
        wxImage img = m_bmp.ConvertToImage();
        img.Rescale(iconsize.x, iconsize.y);
        m_bmp = wxBitmap(img);
    }

    wxRegion region;
    region.Union(m_bmp);

        if (winsize != iconsize)
    {
        m_pos.x = (winsize.x - iconsize.x) / 2;
        m_pos.y = (winsize.y - iconsize.y) / 2;
        region.Offset(m_pos.x, m_pos.y);
    }

        SetShape(region);
    Refresh();
}

void wxTaskBarIconArea::SetLegacyWMProperties()
{
#ifdef __WXGTK__
    gtk_widget_realize(m_widget);
#endif

    long data[1];

        Atom _KDE_NET_WM_SYSTEM_TRAY_WINDOW_FOR =
        XInternAtom(GetDisplay(), "_KDE_NET_WM_SYSTEM_TRAY_WINDOW_FOR", False);
    data[0] = 0;
    XChangeProperty(GetDisplay(), GetXWindow(this),
                    _KDE_NET_WM_SYSTEM_TRAY_WINDOW_FOR,
                    XA_WINDOW, 32,
                    PropModeReplace, (unsigned char*)data, 1);

        Atom KWM_DOCKWINDOW =
        XInternAtom(GetDisplay(), "KWM_DOCKWINDOW", False);
    data[0] = 1;
    XChangeProperty(GetDisplay(), GetXWindow(this),
                    KWM_DOCKWINDOW,
                    KWM_DOCKWINDOW, 32,
                    PropModeReplace, (unsigned char*)data, 1);
}

void wxTaskBarIconArea::OnSizeChange(wxSizeEvent& WXUNUSED(event))
{
    wxLogTrace(wxT("systray"), wxT("icon size changed to %i x %i"),
               GetSize().x, GetSize().y);
        wxBitmap bmp(m_bmp);
    SetTrayIcon(bmp);
}

void wxTaskBarIconArea::OnPaint(wxPaintEvent& WXUNUSED(event))
{
    wxPaintDC dc(this);
    dc.DrawBitmap(m_bmp, m_pos.x, m_pos.y, true);
}

void wxTaskBarIconArea::OnMouseEvent(wxMouseEvent& event)
{
    wxEventType type = 0;
    wxEventType mtype = event.GetEventType();

    if (mtype == wxEVT_LEFT_DOWN)
        type = wxEVT_TASKBAR_LEFT_DOWN;
    else if (mtype == wxEVT_LEFT_UP)
        type = wxEVT_TASKBAR_LEFT_UP;
    else if (mtype == wxEVT_LEFT_DCLICK)
        type = wxEVT_TASKBAR_LEFT_DCLICK;
    else if (mtype == wxEVT_RIGHT_DOWN)
        type = wxEVT_TASKBAR_RIGHT_DOWN;
    else if (mtype == wxEVT_RIGHT_UP)
        type = wxEVT_TASKBAR_RIGHT_UP;
    else if (mtype == wxEVT_RIGHT_DCLICK)
        type = wxEVT_TASKBAR_RIGHT_DCLICK;
    else if (mtype == wxEVT_MOTION)
        type = wxEVT_TASKBAR_MOVE;
    else
        return;

   wxTaskBarIconEvent e(type, m_icon);
   m_icon->ProcessEvent(e);
}

void wxTaskBarIconArea::OnMenuEvent(wxCommandEvent& event)
{
    m_icon->ProcessEvent(event);
}


bool wxTaskBarIconBase::IsAvailable()
{
    return wxTaskBarIconArea::IsProtocolSupported();
}


wxIMPLEMENT_DYNAMIC_CLASS(wxTaskBarIcon, wxEvtHandler);

wxTaskBarIcon::wxTaskBarIcon() : m_iconWnd(NULL)
{
}

wxTaskBarIcon::~wxTaskBarIcon()
{
    if (m_iconWnd)
    {
        m_iconWnd->Disconnect(wxEVT_DESTROY,
            wxWindowDestroyEventHandler(wxTaskBarIcon::OnDestroy), NULL, this);
        RemoveIcon();
    }
}

bool wxTaskBarIcon::IsOk() const
{
    return true;
}

bool wxTaskBarIcon::IsIconInstalled() const
{
    return m_iconWnd != NULL;
}

void wxTaskBarIcon::OnDestroy(wxWindowDestroyEvent&)
{
            m_iconWnd = NULL;
}

bool wxTaskBarIcon::SetIcon(const wxIcon& icon, const wxString& tooltip)
{
    wxBitmap bmp;
    bmp.CopyFromIcon(icon);

    if (!m_iconWnd)
    {
        m_iconWnd = new wxTaskBarIconArea(this, bmp);
        if (m_iconWnd->IsOk())
        {
            m_iconWnd->Connect(wxEVT_DESTROY,
                wxWindowDestroyEventHandler(wxTaskBarIcon::OnDestroy),
                NULL, this);
            m_iconWnd->Show();
        }
        else
        {
            m_iconWnd->Destroy();
            m_iconWnd = NULL;
            return false;
        }
    }
    else
    {
        m_iconWnd->SetTrayIcon(bmp);
    }

#if wxUSE_TOOLTIPS
    if (!tooltip.empty())
        m_iconWnd->SetToolTip(tooltip);
    else
        m_iconWnd->SetToolTip(NULL);
#else
    wxUnusedVar(tooltip);
#endif
    return true;
}

bool wxTaskBarIcon::RemoveIcon()
{
    if (!m_iconWnd)
        return false;
    m_iconWnd->Destroy();
    m_iconWnd = NULL;
    return true;
}

bool wxTaskBarIcon::PopupMenu(wxMenu *menu)
{
    if (!m_iconWnd)
        return false;
    m_iconWnd->PopupMenu(menu);
    return true;
}

#endif 