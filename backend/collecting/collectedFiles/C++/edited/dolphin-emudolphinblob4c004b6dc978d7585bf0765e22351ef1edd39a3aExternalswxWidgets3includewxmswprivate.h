
#ifndef _WX_PRIVATE_H_
#define _WX_PRIVATE_H_

#include "wx/msw/wrapwin.h"

#include "wx/log.h"

#if wxUSE_GUI
    #include "wx/window.h"
#endif 
class WXDLLIMPEXP_FWD_CORE wxFont;
class WXDLLIMPEXP_FWD_CORE wxWindow;
class WXDLLIMPEXP_FWD_CORE wxWindowBase;


#ifndef MAX_PATH
    #define MAX_PATH  260
#endif


#if wxUSE_GUI

extern WXDLLIMPEXP_DATA_CORE(HICON) wxSTD_FRAME_ICON;
extern WXDLLIMPEXP_DATA_CORE(HICON) wxSTD_MDIPARENTFRAME_ICON;
extern WXDLLIMPEXP_DATA_CORE(HICON) wxSTD_MDICHILDFRAME_ICON;
extern WXDLLIMPEXP_DATA_CORE(HICON) wxDEFAULT_FRAME_ICON;
extern WXDLLIMPEXP_DATA_CORE(HICON) wxDEFAULT_MDIPARENTFRAME_ICON;
extern WXDLLIMPEXP_DATA_CORE(HICON) wxDEFAULT_MDICHILDFRAME_ICON;
extern WXDLLIMPEXP_DATA_CORE(HFONT) wxSTATUS_LINE_FONT;

#endif 

extern WXDLLIMPEXP_DATA_BASE(HINSTANCE) wxhInstance;

extern "C"
{
    WXDLLIMPEXP_BASE HINSTANCE wxGetInstance();
}

WXDLLIMPEXP_BASE void wxSetInstance(HINSTANCE hInst);


#if defined(STRICT) || defined(__GNUC__)
    typedef WNDPROC WndProcCast;
#else
    typedef FARPROC WndProcCast;
#endif


#define CASTWNDPROC (WndProcCast)




#if !defined(APIENTRY)      #define APIENTRY FAR PASCAL
#endif



#define STATIC_CLASS     wxT("STATIC")
#define STATIC_FLAGS     (SS_LEFT|WS_CHILD|WS_VISIBLE)
#define CHECK_CLASS      wxT("BUTTON")
#define CHECK_FLAGS      (BS_AUTOCHECKBOX|WS_TABSTOP|WS_CHILD)
#define CHECK_IS_FAFA    FALSE
#define RADIO_CLASS      wxT("BUTTON")
#define RADIO_FLAGS      (BS_AUTORADIOBUTTON|WS_CHILD|WS_VISIBLE)
#define RADIO_SIZE       20
#define RADIO_IS_FAFA    FALSE
#define PURE_WINDOWS
#define GROUP_CLASS      wxT("BUTTON")
#define GROUP_FLAGS      (BS_GROUPBOX|WS_CHILD|WS_VISIBLE)




#define MEANING_CHARACTER '0'
#define DEFAULT_ITEM_WIDTH  100
#define DEFAULT_ITEM_HEIGHT 80

#define EDIT_HEIGHT_FROM_CHAR_HEIGHT(cy)    (cy+8)

extern LONG APIENTRY
  wxSubclassedGenericControlProc(WXHWND hWnd, WXUINT message, WXWPARAM wParam, WXLPARAM lParam);


#define wxZeroMemory(obj)   ::ZeroMemory(&obj, sizeof(obj))

#if defined(__CYGWIN__)
    #define wxGetOSFHandle(fd) ((HANDLE)get_osfhandle(fd))
#elif defined(__VISUALC__) \
   || defined(__BORLANDC__) \
   || defined(__MINGW32__)
    #define wxGetOSFHandle(fd) ((HANDLE)_get_osfhandle(fd))
    #define wxOpenOSFHandle(h, flags) (_open_osfhandle(wxPtrToUInt(h), flags))

    wxDECL_FOR_STRICT_MINGW32(FILE*, _fdopen, (int, const char*))
    #define wx_fdopen _fdopen
#endif

template <wxUIntPtr INVALID_VALUE = (wxUIntPtr)INVALID_HANDLE_VALUE>
class AutoHANDLE
{
public:
    wxEXPLICIT AutoHANDLE(HANDLE handle = InvalidHandle()) : m_handle(handle) { }

    bool IsOk() const { return m_handle != InvalidHandle(); }
    operator HANDLE() const { return m_handle; }

    ~AutoHANDLE() { if ( IsOk() ) DoClose(); }

    void Close()
    {
        wxCHECK_RET(IsOk(), wxT("Handle must be valid"));

        DoClose();

        m_handle = InvalidHandle();
    }

protected:
            static HANDLE InvalidHandle()
    {
        return static_cast<HANDLE>(INVALID_VALUE);
    }

    void DoClose()
    {
        if ( !::CloseHandle(m_handle) )
            wxLogLastError(wxT("CloseHandle"));
    }

    WXHANDLE m_handle;
};

template <class T>
struct WinStruct : public T
{
    WinStruct()
    {
        ::ZeroMemory(this, sizeof(T));

                this->cbSize = sizeof(T);
    }
};


#define wxMSW_CONV_LPCTSTR(s) static_cast<const wxChar *>((s).t_str())
#define wxMSW_CONV_LPTSTR(s) const_cast<wxChar *>(wxMSW_CONV_LPCTSTR(s))
#define wxMSW_CONV_LPARAM(s) reinterpret_cast<LPARAM>(wxMSW_CONV_LPCTSTR(s))


#if wxUSE_GUI

#include "wx/gdicmn.h"
#include "wx/colour.h"

inline COLORREF wxColourToRGB(const wxColour& c)
{
    return RGB(c.Red(), c.Green(), c.Blue());
}

inline COLORREF wxColourToPalRGB(const wxColour& c)
{
    return PALETTERGB(c.Red(), c.Green(), c.Blue());
}

inline wxColour wxRGBToColour(COLORREF rgb)
{
    return wxColour(GetRValue(rgb), GetGValue(rgb), GetBValue(rgb));
}

inline void wxRGBToColour(wxColour& c, COLORREF rgb)
{
    c.Set(GetRValue(rgb), GetGValue(rgb), GetBValue(rgb));
}

enum wxSTD_COLOUR
{
    wxSTD_COL_BTNTEXT,
    wxSTD_COL_BTNSHADOW,
    wxSTD_COL_BTNFACE,
    wxSTD_COL_BTNHIGHLIGHT,
    wxSTD_COL_MAX
};

struct WXDLLIMPEXP_CORE wxCOLORMAP
{
    COLORREF from, to;
};

extern wxCOLORMAP *wxGetStdColourMap();

inline wxRect wxRectFromRECT(const RECT& rc)
{
    return wxRect(rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top);
}

inline void wxCopyRECTToRect(const RECT& rc, wxRect& rect)
{
    rect = wxRectFromRECT(rc);
}

inline void wxCopyRectToRECT(const wxRect& rect, RECT& rc)
{
            rc.top = rect.y;
    rc.left = rect.x;
    rc.right = rect.x + rect.width;
    rc.bottom = rect.y + rect.height;
}

extern void HIMETRICToPixel(LONG *x, LONG *y);
extern void HIMETRICToPixel(LONG *x, LONG *y, HDC hdcRef);
extern void PixelToHIMETRIC(LONG *x, LONG *y);
extern void PixelToHIMETRIC(LONG *x, LONG *y, HDC hdcRef);

extern HBITMAP wxInvertMask(HBITMAP hbmpMask, int w = 0, int h = 0);

extern HICON wxBitmapToHICON(const wxBitmap& bmp);

extern
HCURSOR wxBitmapToHCURSOR(const wxBitmap& bmp, int hotSpotX, int hotSpotY);


#if wxUSE_OWNER_DRAWN

enum wxDSBStates
{
    wxDSB_NORMAL = 0,
    wxDSB_SELECTED,
    wxDSB_DISABLED
};

extern
BOOL wxDrawStateBitmap(HDC hDC, HBITMAP hBitmap, int x, int y, UINT uState);

#endif 
#ifndef GET_X_LPARAM
    #define GET_X_LPARAM(lp) ((int)(short)LOWORD(lp))
    #define GET_Y_LPARAM(lp) ((int)(short)HIWORD(lp))
#endif 
inline bool wxIsModifierDown(int vk)
{
            return ::GetKeyState(vk) < 0;
}

inline bool wxIsShiftDown()
{
    return wxIsModifierDown(VK_SHIFT);
}

inline bool wxIsCtrlDown()
{
    return wxIsModifierDown(VK_CONTROL);
}

inline bool wxIsAltDown()
{
    return wxIsModifierDown(VK_MENU);
}

inline bool wxIsAnyModifierDown()
{
    return wxIsShiftDown() || wxIsCtrlDown() || wxIsAltDown();
}

inline RECT wxGetWindowRect(HWND hwnd)
{
    RECT rect;

    if ( !::GetWindowRect(hwnd, &rect) )
    {
        wxLogLastError(wxT("GetWindowRect"));
    }

    return rect;
}

inline RECT wxGetClientRect(HWND hwnd)
{
    RECT rect;

    if ( !::GetClientRect(hwnd, &rect) )
    {
        wxLogLastError(wxT("GetClientRect"));
    }

    return rect;
}


class ScreenHDC
{
public:
    ScreenHDC() { m_hdc = ::GetDC(NULL);    }
   ~ScreenHDC() { ::ReleaseDC(NULL, m_hdc); }

    operator HDC() const { return m_hdc; }

private:
    HDC m_hdc;

    wxDECLARE_NO_COPY_CLASS(ScreenHDC);
};

class WindowHDC
{
public:
    WindowHDC() : m_hwnd(NULL), m_hdc(NULL) { }
    WindowHDC(HWND hwnd) { m_hdc = ::GetDC(m_hwnd = hwnd); }
   ~WindowHDC() { if ( m_hwnd && m_hdc ) { ::ReleaseDC(m_hwnd, m_hdc); } }

    operator HDC() const { return m_hdc; }

private:
   HWND m_hwnd;
   HDC m_hdc;

   wxDECLARE_NO_COPY_CLASS(WindowHDC);
};

class MemoryHDC
{
public:
    MemoryHDC(HDC hdc = 0) { m_hdc = ::CreateCompatibleDC(hdc); }
   ~MemoryHDC() { ::DeleteDC(m_hdc); }

    operator HDC() const { return m_hdc; }

private:
    HDC m_hdc;

    wxDECLARE_NO_COPY_CLASS(MemoryHDC);
};

class SelectInHDC
{
private:
    void DoInit(HGDIOBJ hgdiobj) { m_hgdiobj = ::SelectObject(m_hdc, hgdiobj); }

public:
    SelectInHDC() : m_hdc(NULL), m_hgdiobj(NULL) { }
    SelectInHDC(HDC hdc, HGDIOBJ hgdiobj) : m_hdc(hdc) { DoInit(hgdiobj); }

    void Init(HDC hdc, HGDIOBJ hgdiobj)
    {
        wxASSERT_MSG( !m_hdc, wxT("initializing twice?") );

        m_hdc = hdc;

        DoInit(hgdiobj);
    }

    ~SelectInHDC() { if ( m_hdc ) ::SelectObject(m_hdc, m_hgdiobj); }

        operator bool() const { return m_hgdiobj != 0; }

private:
    HDC m_hdc;
    HGDIOBJ m_hgdiobj;

    wxDECLARE_NO_COPY_CLASS(SelectInHDC);
};

class AutoGDIObject
{
protected:
    AutoGDIObject() { m_gdiobj = NULL; }
    AutoGDIObject(HGDIOBJ gdiobj) : m_gdiobj(gdiobj) { }
    ~AutoGDIObject() { if ( m_gdiobj ) ::DeleteObject(m_gdiobj); }

    void InitGdiobj(HGDIOBJ gdiobj)
    {
        wxASSERT_MSG( !m_gdiobj, wxT("initializing twice?") );

        m_gdiobj = gdiobj;
    }

    HGDIOBJ GetObject() const { return m_gdiobj; }

private:
    HGDIOBJ m_gdiobj;
};


class AutoHBRUSH : private AutoGDIObject
{
public:
    AutoHBRUSH(COLORREF col)
        : AutoGDIObject(::CreateSolidBrush(col)) { }

    operator HBRUSH() const { return (HBRUSH)GetObject(); }
};

class AutoHFONT : private AutoGDIObject
{
private:
public:
    AutoHFONT()
        : AutoGDIObject() { }

    AutoHFONT(const LOGFONT& lf)
        : AutoGDIObject(::CreateFontIndirect(&lf)) { }

    void Init(const LOGFONT& lf) { InitGdiobj(::CreateFontIndirect(&lf)); }

    operator HFONT() const { return (HFONT)GetObject(); }
};

class AutoHPEN : private AutoGDIObject
{
public:
    AutoHPEN(COLORREF col)
        : AutoGDIObject(::CreatePen(PS_SOLID, 0, col)) { }

    operator HPEN() const { return (HPEN)GetObject(); }
};

class AutoHBITMAP : private AutoGDIObject
{
public:
    AutoHBITMAP()
        : AutoGDIObject() { }

    AutoHBITMAP(HBITMAP hbmp) : AutoGDIObject(hbmp) { }

    void Init(HBITMAP hbmp) { InitGdiobj(hbmp); }

    operator HBITMAP() const { return (HBITMAP)GetObject(); }
};

class CompatibleBitmap : public AutoHBITMAP
{
public:
    CompatibleBitmap(HDC hdc, int w, int h)
        : AutoHBITMAP(::CreateCompatibleBitmap(hdc, w, h))
    {
    }
};

class MonoBitmap : public AutoHBITMAP
{
public:
    MonoBitmap(int w, int h)
        : AutoHBITMAP(::CreateBitmap(w, h, 1, 1, 0))
    {
    }
};

class AutoHRGN : private AutoGDIObject
{
public:
    AutoHRGN(HRGN hrgn) : AutoGDIObject(hrgn) { }

    operator HRGN() const { return (HRGN)GetObject(); }
};

class AutoIconInfo : public ICONINFO
{
public:
    AutoIconInfo() { wxZeroMemory(*this); }

    bool GetFrom(HICON hIcon)
    {
        if ( !::GetIconInfo(hIcon, this) )
        {
            wxLogLastError(wxT("GetIconInfo"));
            return false;
        }

        return true;
    }

    ~AutoIconInfo()
    {
        if ( hbmColor )
            ::DeleteObject(hbmColor);
        if ( hbmMask )
            ::DeleteObject(hbmMask);
    }
};

class HDCClipper
{
public:
    HDCClipper(HDC hdc, HRGN hrgn)
        : m_hdc(hdc)
    {
        if ( !::SelectClipRgn(hdc, hrgn) )
        {
            wxLogLastError(wxT("SelectClipRgn"));
        }
    }

    ~HDCClipper()
    {
        ::SelectClipRgn(m_hdc, NULL);
    }

private:
    HDC m_hdc;

    wxDECLARE_NO_COPY_CLASS(HDCClipper);
};

    class HDCMapModeChanger
    {
    public:
        HDCMapModeChanger(HDC hdc, int mm)
            : m_hdc(hdc)
        {
            m_modeOld = ::SetMapMode(hdc, mm);
            if ( !m_modeOld )
            {
                wxLogLastError(wxT("SelectClipRgn"));
            }
        }

        ~HDCMapModeChanger()
        {
            if ( m_modeOld )
                ::SetMapMode(m_hdc, m_modeOld);
        }

    private:
        HDC m_hdc;
        int m_modeOld;

        wxDECLARE_NO_COPY_CLASS(HDCMapModeChanger);
    };

    #define wxCHANGE_HDC_MAP_MODE(hdc, mm) \
        HDCMapModeChanger wxMAKE_UNIQUE_NAME(wxHDCMapModeChanger)(hdc, mm)

class GlobalPtr
{
public:
        GlobalPtr()
    {
        m_hGlobal = NULL;
    }

        void Init(size_t size, unsigned flags = GMEM_MOVEABLE)
    {
        m_hGlobal = ::GlobalAlloc(flags, size);
        if ( !m_hGlobal )
        {
            wxLogLastError(wxT("GlobalAlloc"));
        }
    }

    GlobalPtr(size_t size, unsigned flags = GMEM_MOVEABLE)
    {
        Init(size, flags);
    }

    ~GlobalPtr()
    {
        if ( m_hGlobal && ::GlobalFree(m_hGlobal) )
        {
            wxLogLastError(wxT("GlobalFree"));
        }
    }

        operator HGLOBAL() const { return m_hGlobal; }

private:
    HGLOBAL m_hGlobal;

    wxDECLARE_NO_COPY_CLASS(GlobalPtr);
};

class GlobalPtrLock
{
public:
            GlobalPtrLock()
    {
        m_hGlobal = NULL;
        m_ptr = NULL;
    }

            void Init(HGLOBAL hGlobal)
    {
        m_hGlobal = hGlobal;

                        m_ptr = GlobalLock(hGlobal);
        if ( !m_ptr )
        {
            wxLogLastError(wxT("GlobalLock"));
        }
    }

        GlobalPtrLock(HGLOBAL hGlobal)
    {
        Init(hGlobal);
    }

    ~GlobalPtrLock()
    {
        if ( m_hGlobal && !GlobalUnlock(m_hGlobal) )
        {
                        DWORD dwLastError = ::GetLastError();
            if ( dwLastError != NO_ERROR )
            {
                wxLogApiError(wxT("GlobalUnlock"), dwLastError);
            }
        }
    }

    void *Get() const { return m_ptr; }
    operator void *() const { return m_ptr; }

private:
    HGLOBAL m_hGlobal;
    void *m_ptr;

    wxDECLARE_NO_COPY_CLASS(GlobalPtrLock);
};

class ClassRegistrar
{
public:
        ClassRegistrar() { m_registered = -1; }

        bool IsInitialized() const { return m_registered != -1; }

        bool IsRegistered() const { return m_registered == 1; }

        bool Register(const WNDCLASS& wc)
    {
                wxASSERT_MSG( m_registered == -1,
                        wxT("calling ClassRegistrar::Register() twice?") );

        m_registered = ::RegisterClass(&wc) ? 1 : 0;
        if ( !IsRegistered() )
        {
            wxLogLastError(wxT("RegisterClassEx()"));
        }
        else
        {
            m_clsname = wc.lpszClassName;
        }

        return m_registered == 1;
    }

            const wxString& GetName() const { return m_clsname; }

        ~ClassRegistrar()
    {
        if ( IsRegistered() )
        {
            if ( !::UnregisterClass(m_clsname.t_str(), wxGetInstance()) )
            {
                wxLogLastError(wxT("UnregisterClass"));
            }
        }
    }

private:
            int m_registered;

        wxString m_clsname;
};


#define GetHwnd()               ((HWND)GetHWND())
#define GetHwndOf(win)          ((HWND)((win)->GetHWND()))
#define GetWinHwnd              GetHwndOf

#define GetHdc()                ((HDC)GetHDC())
#define GetHdcOf(dc)            ((HDC)(dc).GetHDC())

#define GetHbitmap()            ((HBITMAP)GetHBITMAP())
#define GetHbitmapOf(bmp)       ((HBITMAP)(bmp).GetHBITMAP())

#define GetHicon()              ((HICON)GetHICON())
#define GetHiconOf(icon)        ((HICON)(icon).GetHICON())

#define GetHaccel()             ((HACCEL)GetHACCEL())
#define GetHaccelOf(table)      ((HACCEL)((table).GetHACCEL()))

#define GetHbrush()             ((HBRUSH)GetResourceHandle())
#define GetHbrushOf(brush)      ((HBRUSH)(brush).GetResourceHandle())

#define GetHmenu()              ((HMENU)GetHMenu())
#define GetHmenuOf(menu)        ((HMENU)(menu)->GetHMenu())

#define GetHcursor()            ((HCURSOR)GetHCURSOR())
#define GetHcursorOf(cursor)    ((HCURSOR)(cursor).GetHCURSOR())

#define GetHfont()              ((HFONT)GetHFONT())
#define GetHfontOf(font)        ((HFONT)(font).GetHFONT())

#define GetHimagelist()         ((HIMAGELIST)GetHIMAGELIST())
#define GetHimagelistOf(imgl)   ((HIMAGELIST)(imgl)->GetHIMAGELIST())

#define GetHpalette()           ((HPALETTE)GetHPALETTE())
#define GetHpaletteOf(pal)      ((HPALETTE)(pal).GetHPALETTE())

#define GetHpen()               ((HPEN)GetResourceHandle())
#define GetHpenOf(pen)          ((HPEN)(pen).GetResourceHandle())

#define GetHrgn()               ((HRGN)GetHRGN())
#define GetHrgnOf(rgn)          ((HRGN)(rgn).GetHRGN())

#endif 

inline wxString wxGetFullModuleName(HMODULE hmod)
{
    wxString fullname;
    if ( !::GetModuleFileName
            (
                hmod,
                wxStringBuffer(fullname, MAX_PATH),
                MAX_PATH
            ) )
    {
        wxLogLastError(wxT("GetModuleFileName"));
    }

    return fullname;
}

inline wxString wxGetFullModuleName()
{
    return wxGetFullModuleName((HMODULE)wxGetInstance());
}

enum wxWinVersion
{
    wxWinVersion_Unknown = 0,

    wxWinVersion_3 = 0x0300,
    wxWinVersion_NT3 = wxWinVersion_3,

    wxWinVersion_4 = 0x0400,
    wxWinVersion_95 = wxWinVersion_4,
    wxWinVersion_NT4 = wxWinVersion_4,
    wxWinVersion_98 = 0x0410,

    wxWinVersion_5 = 0x0500,
    wxWinVersion_ME = wxWinVersion_5,
    wxWinVersion_NT5 = wxWinVersion_5,
    wxWinVersion_2000 = wxWinVersion_5,
    wxWinVersion_XP = 0x0501,
    wxWinVersion_2003 = 0x0501,
    wxWinVersion_XP_SP2 = 0x0502,
    wxWinVersion_2003_SP1 = 0x0502,

    wxWinVersion_6 = 0x0600,
    wxWinVersion_Vista = wxWinVersion_6,
    wxWinVersion_NT6 = wxWinVersion_6,

    wxWinVersion_7 = 0x601,

    wxWinVersion_8 = 0x602,
    wxWinVersion_8_1 = 0x603,

    wxWinVersion_10 = 0x1000
};

WXDLLIMPEXP_BASE wxWinVersion wxGetWinVersion();

#if wxUSE_GUI && defined(__WXMSW__)

extern HCURSOR wxGetCurrentBusyCursor();    extern const wxCursor *wxGetGlobalCursor(); 
WXDLLIMPEXP_CORE void wxGetCursorPosMSW(POINT* pt);

WXDLLIMPEXP_CORE void wxGetCharSize(WXHWND wnd, int *x, int *y, const wxFont& the_font);
WXDLLIMPEXP_CORE void wxFillLogFont(LOGFONT *logFont, const wxFont *font);
WXDLLIMPEXP_CORE wxFont wxCreateFontFromLogFont(const LOGFONT *logFont);
WXDLLIMPEXP_CORE wxFontEncoding wxGetFontEncFromCharSet(int charset);

WXDLLIMPEXP_CORE void wxSliderEvent(WXHWND control, WXWORD wParam, WXWORD pos);
WXDLLIMPEXP_CORE void wxScrollBarEvent(WXHWND hbar, WXWORD wParam, WXWORD pos);

extern WXDLLIMPEXP_CORE void wxFindMaxSize(WXHWND hwnd, RECT *rect);

extern WXDLLIMPEXP_CORE wxString wxGetWindowText(WXHWND hWnd);

extern WXDLLIMPEXP_CORE wxString wxGetWindowClass(WXHWND hWnd);

extern WXDLLIMPEXP_CORE int wxGetWindowId(WXHWND hWnd);

extern WXDLLIMPEXP_CORE bool wxCheckWindowWndProc(WXHWND hWnd, WXFARPROC wndProc);

inline bool wxStyleHasBorder(long style)
{
    return (style & (wxSIMPLE_BORDER | wxRAISED_BORDER |
                     wxSUNKEN_BORDER | wxDOUBLE_BORDER)) != 0;
}

inline long wxGetWindowExStyle(const wxWindowMSW *win)
{
    return ::GetWindowLong(GetHwndOf(win), GWL_EXSTYLE);
}

inline bool wxHasWindowExStyle(const wxWindowMSW *win, long style)
{
    return (wxGetWindowExStyle(win) & style) != 0;
}

inline long wxSetWindowExStyle(const wxWindowMSW *win, long style)
{
    return ::SetWindowLong(GetHwndOf(win), GWL_EXSTYLE, style);
}

inline bool
wxUpdateExStyleForLayoutDirection(WXHWND hWnd,
                                  wxLayoutDirection dir,
                                  LONG_PTR flagsForRTL)
{
    wxCHECK_MSG( hWnd, false,
                 wxS("Can't set layout direction for invalid window") );

    const LONG_PTR styleOld = ::GetWindowLongPtr(hWnd, GWL_EXSTYLE);

    LONG_PTR styleNew = styleOld;
    switch ( dir )
    {
        case wxLayout_LeftToRight:
            styleNew &= ~flagsForRTL;
            break;

        case wxLayout_RightToLeft:
            styleNew |= flagsForRTL;
            break;

        case wxLayout_Default:
            wxFAIL_MSG(wxS("Invalid layout direction"));
    }

    if ( styleNew == styleOld )
        return false;

    ::SetWindowLongPtr(hWnd, GWL_EXSTYLE, styleNew);

    return true;
}

inline bool wxUpdateLayoutDirection(WXHWND hWnd, wxLayoutDirection dir)
{
    return wxUpdateExStyleForLayoutDirection(hWnd, dir, WS_EX_LAYOUTRTL);
}

inline bool wxUpdateEditLayoutDirection(WXHWND hWnd, wxLayoutDirection dir)
{
    return wxUpdateExStyleForLayoutDirection(hWnd, dir,
                                             WS_EX_RIGHT |
                                             WS_EX_RTLREADING |
                                             WS_EX_LEFTSCROLLBAR);
}

inline wxLayoutDirection wxGetEditLayoutDirection(WXHWND hWnd)
{
    wxCHECK_MSG( hWnd, wxLayout_Default, wxS("invalid window") );

                    const LONG_PTR style = ::GetWindowLongPtr(hWnd, GWL_EXSTYLE);

    return style & WS_EX_RTLREADING ? wxLayout_RightToLeft
                                    : wxLayout_LeftToRight;
}


extern WXDLLIMPEXP_CORE wxWindow* wxFindWinFromHandle(HWND hwnd);

extern WXDLLIMPEXP_CORE wxWindow *wxGetWindowFromHWND(WXHWND hwnd);

extern WXDLLIMPEXP_CORE wxSize wxGetHiconSize(HICON hicon);

WXDLLIMPEXP_CORE void wxDrawLine(HDC hdc, int x1, int y1, int x2, int y2);

inline void wxFillRect(HWND hwnd, HDC hdc, HBRUSH hbr)
{
    RECT rc;
    ::GetClientRect(hwnd, &rc);
    ::FillRect(hdc, &rc, hbr);
}


#ifdef __WIN64__

inline void *wxGetWindowProc(HWND hwnd)
{
    return (void *)::GetWindowLongPtr(hwnd, GWLP_WNDPROC);
}

inline void *wxGetWindowUserData(HWND hwnd)
{
    return (void *)::GetWindowLongPtr(hwnd, GWLP_USERDATA);
}

inline WNDPROC wxSetWindowProc(HWND hwnd, WNDPROC func)
{
    return (WNDPROC)::SetWindowLongPtr(hwnd, GWLP_WNDPROC, (LONG_PTR)func);
}

inline void *wxSetWindowUserData(HWND hwnd, void *data)
{
    return (void *)::SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)data);
}

#else 
inline WNDPROC wxGetWindowProc(HWND hwnd)
{
    return (WNDPROC)(LONG_PTR)::GetWindowLong(hwnd, GWL_WNDPROC);
}

inline void *wxGetWindowUserData(HWND hwnd)
{
    return (void *)(LONG_PTR)::GetWindowLong(hwnd, GWL_USERDATA);
}

inline WNDPROC wxSetWindowProc(HWND hwnd, WNDPROC func)
{
    return (WNDPROC)(LONG_PTR)::SetWindowLong(hwnd, GWL_WNDPROC, (LONG_PTR)func);
}

inline void *wxSetWindowUserData(HWND hwnd, void *data)
{
    return (void *)(LONG_PTR)::SetWindowLong(hwnd, GWL_USERDATA, (LONG_PTR)data);
}

#endif 
#endif 
#endif 