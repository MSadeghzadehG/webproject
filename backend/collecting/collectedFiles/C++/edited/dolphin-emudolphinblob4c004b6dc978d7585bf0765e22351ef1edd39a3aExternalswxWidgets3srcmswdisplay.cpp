


#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#if wxUSE_DISPLAY

#include "wx/display.h"

#ifndef WX_PRECOMP
    #include "wx/dynarray.h"
    #include "wx/app.h"
    #include "wx/frame.h"
#endif

#include "wx/dynlib.h"
#include "wx/sysopt.h"

#include "wx/display_impl.h"
#include "wx/msw/wrapwin.h"
#include "wx/msw/missing.h"
#include "wx/msw/private.h"
#include "wx/msw/private/hiddenwin.h"

                    #if !defined(HMONITOR_DECLARED) && !defined(MNS_NOCHECK)
        DECLARE_HANDLE(HMONITOR);
        typedef BOOL(CALLBACK * MONITORENUMPROC )(HMONITOR, HDC, LPRECT, LPARAM);
        typedef struct tagMONITORINFO
        {
            DWORD   cbSize;
            RECT    rcMonitor;
            RECT    rcWork;
            DWORD   dwFlags;
        } MONITORINFO, *LPMONITORINFO;
        typedef struct tagMONITORINFOEX : public tagMONITORINFO
        {
            TCHAR       szDevice[CCHDEVICENAME];
        } MONITORINFOEX, *LPMONITORINFOEX;
        #define MONITOR_DEFAULTTONULL       0x00000000
        #define MONITOR_DEFAULTTOPRIMARY    0x00000001
        #define MONITOR_DEFAULTTONEAREST    0x00000002
        #define MONITORINFOF_PRIMARY        0x00000001
        #define HMONITOR_DECLARED
    #endif

static const wxChar displayDllName[] = wxT("user32.dll");


class wxDisplayMSW : public wxDisplayImpl
{
public:
    wxDisplayMSW(unsigned n, HMONITOR hmon)
        : wxDisplayImpl(n),
          m_hmon(hmon)
    {
    }

    virtual wxRect GetGeometry() const;
    virtual wxRect GetClientArea() const;
    virtual wxString GetName() const;
    virtual bool IsPrimary() const;

    virtual wxVideoMode GetCurrentMode() const;
    virtual wxArrayVideoModes GetModes(const wxVideoMode& mode) const;
    virtual bool ChangeMode(const wxVideoMode& mode);

protected:
        static wxVideoMode ConvertToVideoMode(const DEVMODE& dm)
    {
                                return wxVideoMode(dm.dmPelsWidth,
                           dm.dmPelsHeight,
                           dm.dmBitsPerPel,
                           dm.dmDisplayFrequency > 1 ? dm.dmDisplayFrequency : 0);
    }

            bool GetMonInfo(MONITORINFOEX& monInfo) const;

    HMONITOR m_hmon;

private:
    wxDECLARE_NO_COPY_CLASS(wxDisplayMSW);
};



WX_DEFINE_ARRAY(HMONITOR, wxMonitorHandleArray);

class wxDisplayFactoryMSW : public wxDisplayFactory
{
public:
                wxDisplayFactoryMSW();

        virtual ~wxDisplayFactoryMSW();

    bool IsOk() const { return !m_displays.empty(); }

    virtual wxDisplayImpl *CreateDisplay(unsigned n);
    virtual unsigned GetCount() { return unsigned(m_displays.size()); }
    virtual int GetFromPoint(const wxPoint& pt);
    virtual int GetFromWindow(const wxWindow *window);

            static void RefreshMonitors() { ms_factory->DoRefreshMonitors(); }


private:
        static BOOL CALLBACK MultimonEnumProc(HMONITOR hMonitor,
                                          HDC hdcMonitor,
                                          LPRECT lprcMonitor,
                                          LPARAM dwData);

            int FindDisplayFromHMONITOR(HMONITOR hmon) const;

        void DoRefreshMonitors();


                static wxDisplayFactoryMSW* ms_factory;


            wxMonitorHandleArray m_displays;

            HWND m_hiddenHwnd;
    const wxChar* m_hiddenClass;

    wxDECLARE_NO_COPY_CLASS(wxDisplayFactoryMSW);
};

wxDisplayFactoryMSW* wxDisplayFactoryMSW::ms_factory = NULL;


 wxDisplayFactory *wxDisplay::CreateFactory()
{
    wxDisplayFactoryMSW *factoryMM = new wxDisplayFactoryMSW;

    if ( factoryMM->IsOk() )
        return factoryMM;

    delete factoryMM;

        return new wxDisplayFactorySingle;
}



bool wxDisplayMSW::GetMonInfo(MONITORINFOEX& monInfo) const
{
    if ( !::GetMonitorInfo(m_hmon, &monInfo) )
    {
        wxLogLastError(wxT("GetMonitorInfo"));
        return false;
    }

    return true;
}

wxRect wxDisplayMSW::GetGeometry() const
{
    WinStruct<MONITORINFOEX> monInfo;

    wxRect rect;
    if ( GetMonInfo(monInfo) )
        wxCopyRECTToRect(monInfo.rcMonitor, rect);

    return rect;
}

wxRect wxDisplayMSW::GetClientArea() const
{
    WinStruct<MONITORINFOEX> monInfo;

    wxRect rectClient;
    if ( GetMonInfo(monInfo) )
        wxCopyRECTToRect(monInfo.rcWork, rectClient);

    return rectClient;
}

wxString wxDisplayMSW::GetName() const
{
    WinStruct<MONITORINFOEX> monInfo;

    wxString name;
    if ( GetMonInfo(monInfo) )
        name = monInfo.szDevice;

    return name;
}

bool wxDisplayMSW::IsPrimary() const
{
    WinStruct<MONITORINFOEX> monInfo;

    if ( !GetMonInfo(monInfo) )
        return false;

    return (monInfo.dwFlags & MONITORINFOF_PRIMARY) != 0;
}

wxVideoMode wxDisplayMSW::GetCurrentMode() const
{
    wxVideoMode mode;

                const wxString name = GetName();
    const wxChar * const deviceName = name.empty()
                                          ? (const wxChar*)NULL
                                          : (const wxChar*)name.c_str();

    DEVMODE dm;
    dm.dmSize = sizeof(dm);
    dm.dmDriverExtra = 0;

    if ( !::EnumDisplaySettings(deviceName, ENUM_CURRENT_SETTINGS, &dm) )
    {
        wxLogLastError(wxT("EnumDisplaySettings(ENUM_CURRENT_SETTINGS)"));
    }
    else
    {
        mode = ConvertToVideoMode(dm);
    }

    return mode;
}

wxArrayVideoModes wxDisplayMSW::GetModes(const wxVideoMode& modeMatch) const
{
    wxArrayVideoModes modes;

                const wxString name = GetName();
    const wxChar * const deviceName = name.empty()
                                            ? (const wxChar*)NULL
                                            : (const wxChar*)name.c_str();

    DEVMODE dm;
    dm.dmSize = sizeof(dm);
    dm.dmDriverExtra = 0;

    for ( int iModeNum = 0;
          ::EnumDisplaySettings(deviceName, iModeNum, &dm);
          iModeNum++ )
    {
        const wxVideoMode mode = ConvertToVideoMode(dm);
        if ( mode.Matches(modeMatch) )
        {
            modes.Add(mode);
        }
    }

    return modes;
}

bool wxDisplayMSW::ChangeMode(const wxVideoMode& mode)
{
        DEVMODE dm;
    DEVMODE *pDevMode;

    int flags;

    if ( mode == wxDefaultVideoMode )
    {
                pDevMode = NULL;
        flags = 0;
    }
    else     {
        wxCHECK_MSG( mode.GetWidth() && mode.GetHeight(), false,
                        wxT("at least the width and height must be specified") );

        wxZeroMemory(dm);
        dm.dmSize = sizeof(dm);
        dm.dmDriverExtra = 0;
        dm.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT;
        dm.dmPelsWidth = mode.GetWidth();
        dm.dmPelsHeight = mode.GetHeight();

        if ( mode.GetDepth() )
        {
            dm.dmFields |= DM_BITSPERPEL;
            dm.dmBitsPerPel = mode.GetDepth();
        }

        if ( mode.GetRefresh() )
        {
            dm.dmFields |= DM_DISPLAYFREQUENCY;
            dm.dmDisplayFrequency = mode.GetRefresh();
        }

        pDevMode = &dm;

        flags = CDS_FULLSCREEN;
    }


        switch ( ::ChangeDisplaySettingsEx
             (
                GetName().t_str(),                  pDevMode,                           NULL,                               flags,
                NULL                             ) )
    {
        case DISP_CHANGE_SUCCESSFUL:
                        {
                                                                wxWindow *winTop = wxTheApp->GetTopWindow();
                wxFrame *frameTop = wxDynamicCast(winTop, wxFrame);
                if (frameTop && frameTop->IsFullScreen())
                {
                    wxVideoMode current = GetCurrentMode();
                    frameTop->SetClientSize(current.GetWidth(), current.GetHeight());
                }
            }
            return true;

        case DISP_CHANGE_BADMODE:
                        break;

        default:
            wxFAIL_MSG( wxT("unexpected ChangeDisplaySettingsEx() return value") );
    }

    return false;
}



LRESULT APIENTRY
wxDisplayWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if ( msg == WM_SETTINGCHANGE )
    {
        wxDisplayFactoryMSW::RefreshMonitors();

        return 0;
    }

    return ::DefWindowProc(hwnd, msg, wParam, lParam);
}

wxDisplayFactoryMSW::wxDisplayFactoryMSW()
{
            wxASSERT_MSG( !ms_factory, wxS("Using more than one factory?") );

    ms_factory = this;

    m_hiddenHwnd = NULL;
    m_hiddenClass = NULL;

    DoRefreshMonitors();

                m_hiddenHwnd = wxCreateHiddenWindow
                   (
                    &m_hiddenClass,
                    wxT("wxDisplayHiddenWindow"),
                    wxDisplayWndProc
                   );
}

wxDisplayFactoryMSW::~wxDisplayFactoryMSW()
{
    if ( m_hiddenHwnd )
    {
        if ( !::DestroyWindow(m_hiddenHwnd) )
        {
            wxLogLastError(wxT("DestroyWindow(wxDisplayHiddenWindow)"));
        }

        if ( m_hiddenClass )
        {
            if ( !::UnregisterClass(m_hiddenClass, wxGetInstance()) )
            {
                wxLogLastError(wxT("UnregisterClass(wxDisplayHiddenWindow)"));
            }
        }
    }

    ms_factory = NULL;
}

void wxDisplayFactoryMSW::DoRefreshMonitors()
{
    m_displays.Clear();

    if ( !::EnumDisplayMonitors(NULL, NULL, MultimonEnumProc, (LPARAM)this) )
    {
        wxLogLastError(wxT("EnumDisplayMonitors"));
    }
}


BOOL CALLBACK
wxDisplayFactoryMSW::MultimonEnumProc(
    HMONITOR hMonitor,                  HDC WXUNUSED(hdcMonitor),           LPRECT WXUNUSED(lprcMonitor),       LPARAM dwData)                  {
    wxDisplayFactoryMSW *const self = (wxDisplayFactoryMSW *)dwData;

    self->m_displays.Add(hMonitor);

        return TRUE;
}

wxDisplayImpl *wxDisplayFactoryMSW::CreateDisplay(unsigned n)
{
    wxCHECK_MSG( n < m_displays.size(), NULL, wxT("An invalid index was passed to wxDisplay") );

    return new wxDisplayMSW(n, m_displays[n]);
}

int wxDisplayFactoryMSW::FindDisplayFromHMONITOR(HMONITOR hmon) const
{
    if ( hmon )
    {
        const size_t count = m_displays.size();
        for ( size_t n = 0; n < count; n++ )
        {
            if ( hmon == m_displays[n] )
                return n;
        }
    }

    return wxNOT_FOUND;
}

int wxDisplayFactoryMSW::GetFromPoint(const wxPoint& pt)
{
    POINT pt2;
    pt2.x = pt.x;
    pt2.y = pt.y;

    return FindDisplayFromHMONITOR(::MonitorFromPoint(pt2,
                                                       MONITOR_DEFAULTTONULL));
}

int wxDisplayFactoryMSW::GetFromWindow(const wxWindow *window)
{
#ifdef __WXMSW__
    return FindDisplayFromHMONITOR(::MonitorFromWindow(GetHwndOf(window),
                                                        MONITOR_DEFAULTTONULL));
#else
    const wxSize halfsize = window->GetSize() / 2;
    wxPoint pt = window->GetScreenPosition();
    pt.x += halfsize.x;
    pt.y += halfsize.y;
    return GetFromPoint(pt);
#endif
}

#endif 
void wxClientDisplayRect(int *x, int *y, int *width, int *height)
{
            RECT r;

    SystemParametersInfo(SPI_GETWORKAREA, 0, &r, 0);
    if (x)      *x = r.left;
    if (y)      *y = r.top;
    if (width)  *width = r.right - r.left;
    if (height) *height = r.bottom - r.top;
}
