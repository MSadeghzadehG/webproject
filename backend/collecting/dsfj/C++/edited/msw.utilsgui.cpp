


#include "wx/wxprec.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#ifndef WX_PRECOMP
    #include "wx/cursor.h"
    #include "wx/window.h"
    #include "wx/utils.h"
#endif 
#include "wx/msw/private.h"     
#include "wx/msw/wrapwin.h"
#include <shlwapi.h>


void wxBell()
{
    ::MessageBeep((UINT)-1);        }


static HCURSOR gs_wxBusyCursor = 0;     static HCURSOR gs_wxBusyCursorOld = 0;  static int gs_wxBusyCursorCount = 0;

extern HCURSOR wxGetCurrentBusyCursor()
{
    return gs_wxBusyCursor;
}

void wxBeginBusyCursor(const wxCursor *cursor)
{
    if ( gs_wxBusyCursorCount++ == 0 )
    {
        gs_wxBusyCursor = (HCURSOR)cursor->GetHCURSOR();
        gs_wxBusyCursorOld = ::SetCursor(gs_wxBusyCursor);
    }
    }

void wxEndBusyCursor()
{
    wxCHECK_RET( gs_wxBusyCursorCount > 0,
                 wxT("no matching wxBeginBusyCursor() for wxEndBusyCursor()") );

    if ( --gs_wxBusyCursorCount == 0 )
    {
        ::SetCursor(gs_wxBusyCursorOld);
        gs_wxBusyCursorOld = 0;
    }
}

bool wxIsBusy()
{
  return gs_wxBusyCursorCount > 0;
}

bool wxCheckForInterrupt(wxWindow *wnd)
{
    wxCHECK( wnd, false );

    MSG msg;
    while ( ::PeekMessage(&msg, GetHwndOf(wnd), 0, 0, PM_REMOVE) )
    {
        ::TranslateMessage(&msg);
        ::DispatchMessage(&msg);
    }

    return true;
}


void wxGetMousePosition( int* x, int* y )
{
    POINT pt;
    wxGetCursorPosMSW( & pt );
    if ( x ) *x = pt.x;
    if ( y ) *y = pt.y;
}

bool wxColourDisplay()
{
                        static int s_isColour = -1;

    if ( s_isColour == -1 )
    {
        ScreenHDC dc;
        int noCols = ::GetDeviceCaps(dc, NUMCOLORS);

        s_isColour = (noCols == -1) || (noCols > 2);
    }

    return s_isColour != 0;
}

int wxDisplayDepth()
{
    ScreenHDC dc;
    return GetDeviceCaps(dc, PLANES) * GetDeviceCaps(dc, BITSPIXEL);
}

void wxDisplaySize(int *width, int *height)
{
    ScreenHDC dc;

    if ( width )
        *width = ::GetDeviceCaps(dc, HORZRES);
    if ( height )
        *height = ::GetDeviceCaps(dc, VERTRES);
}

void wxDisplaySizeMM(int *width, int *height)
{
    ScreenHDC dc;

    if ( width )
        *width = ::GetDeviceCaps(dc, HORZSIZE);
    if ( height )
        *height = ::GetDeviceCaps(dc, VERTSIZE);
}


wxString WXDLLEXPORT wxGetWindowText(WXHWND hWnd)
{
    wxString str;

    if ( hWnd )
    {
        int len = GetWindowTextLength((HWND)hWnd) + 1;
        ::GetWindowText((HWND)hWnd, wxStringBuffer(str, len), len);
    }

    return str;
}

wxString WXDLLEXPORT wxGetWindowClass(WXHWND hWnd)
{
    wxString str;

    if ( hWnd )
    {
        int len = 256; 
        for ( ;; )
        {
            int count = ::GetClassName((HWND)hWnd, wxStringBuffer(str, len), len);

            if ( count == len )
            {
                                                len *= 2;
            }
            else
            {
                break;
            }
        }
    }

    return str;
}

int WXDLLEXPORT wxGetWindowId(WXHWND hWnd)
{
    return ::GetWindowLong((HWND)hWnd, GWL_ID);
}


void PixelToHIMETRIC(LONG *x, LONG *y, HDC hdcRef)
{
    int iWidthMM = GetDeviceCaps(hdcRef, HORZSIZE),
        iHeightMM = GetDeviceCaps(hdcRef, VERTSIZE),
        iWidthPels = GetDeviceCaps(hdcRef, HORZRES),
        iHeightPels = GetDeviceCaps(hdcRef, VERTRES);

        *x = ::MulDiv(*x, iWidthMM * 100, iWidthPels);
    *y = ::MulDiv(*y, iHeightMM * 100, iHeightPels);
}

void HIMETRICToPixel(LONG *x, LONG *y, HDC hdcRef)
{
    int iWidthMM = GetDeviceCaps(hdcRef, HORZSIZE),
        iHeightMM = GetDeviceCaps(hdcRef, VERTSIZE),
        iWidthPels = GetDeviceCaps(hdcRef, HORZRES),
        iHeightPels = GetDeviceCaps(hdcRef, VERTRES);

    *x = ::MulDiv(*x, iWidthPels, iWidthMM * 100);
    *y = ::MulDiv(*y, iHeightPels, iHeightMM * 100);
}

void HIMETRICToPixel(LONG *x, LONG *y)
{
    HIMETRICToPixel(x, y, ScreenHDC());
}

void PixelToHIMETRIC(LONG *x, LONG *y)
{
    PixelToHIMETRIC(x, y, ScreenHDC());
}

void wxDrawLine(HDC hdc, int x1, int y1, int x2, int y2)
{
    MoveToEx(hdc, x1, y1, NULL); LineTo((HDC) hdc, x2, y2);
}



extern bool wxEnableFileNameAutoComplete(HWND hwnd)
{
    HRESULT hr = ::SHAutoComplete(hwnd, 0x10 );
    if ( FAILED(hr) )
    {
        wxLogApiError(wxT("SHAutoComplete"), hr);
        return false;
    }

    return true;
}
