
#if defined(__WXX11__) || defined(__WXGTK__) || defined(__WXMOTIF__)

#include "wx/wxprec.h"

#include "wx/unix/utilsx11.h"

#ifndef WX_PRECOMP
    #include "wx/log.h"
    #include "wx/app.h"
    #include "wx/icon.h"
    #include "wx/image.h"
#endif

#include "wx/iconbndl.h"
#include "wx/apptrait.h"
#include "wx/private/launchbrowser.h"

#ifdef __VMS
#pragma message disable nosimpint
#endif
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#ifdef __VMS
#pragma message enable nosimpint
#endif

#ifdef __WXGTK__
#include <gtk/gtk.h>
#ifdef GDK_WINDOWING_X11
#include <gdk/gdkx.h>
#ifdef __WXGTK20__
#include "wx/gtk/private/gtk2-compat.h"     #endif
#endif
GdkWindow* wxGetTopLevelGDK();
#endif

#if !defined(__WXGTK__) || defined(GDK_WINDOWING_X11)

static Atom _NET_WM_STATE = 0;
static Atom _NET_WM_STATE_FULLSCREEN = 0;
static Atom _NET_WM_STATE_STAYS_ON_TOP = 0;
static Atom _NET_WM_WINDOW_TYPE = 0;
static Atom _NET_WM_WINDOW_TYPE_NORMAL = 0;
static Atom _KDE_NET_WM_WINDOW_TYPE_OVERRIDE = 0;
static Atom _WIN_LAYER = 0;
static Atom KWIN_RUNNING = 0;
#ifndef __WXGTK20__
static Atom _NET_SUPPORTING_WM_CHECK = 0;
static Atom _NET_SUPPORTED = 0;
#endif

#define wxMAKE_ATOM(name, display) \
    if (name == 0) name = XInternAtom((display), #name, False)


#define WindowCast(w) (Window)(wxPtrToUInt(w))

static bool IsMapped(Display *display, Window window)
{
    XWindowAttributes attr;
    XGetWindowAttributes(display, window, &attr);
    return (attr.map_state != IsUnmapped);
}



extern "C"
{
    typedef int (*wxX11ErrorHandler)(Display *, XErrorEvent *);

    static int wxX11ErrorsSuspender_handler(Display*, XErrorEvent*) { return 0; }
}

class wxX11ErrorsSuspender
{
public:
    wxX11ErrorsSuspender(Display *d) : m_display(d)
    {
        m_old = XSetErrorHandler(wxX11ErrorsSuspender_handler);
    }
    ~wxX11ErrorsSuspender()
    {
        XFlush(m_display);
        XSetErrorHandler(m_old);
    }

private:
    Display *m_display;
    wxX11ErrorHandler m_old;
};




#if wxUSE_IMAGE && !wxUSE_NANOX

static Atom _NET_WM_ICON = 0;

void
wxSetIconsX11(WXDisplay* display, WXWindow window, const wxIconBundle& ib)
{
    size_t size = 0;

    const size_t numIcons = ib.GetIconCount();
    for ( size_t i = 0; i < numIcons; ++i )
    {
        const wxIcon icon = ib.GetIconByIndex(i);

        size += 2 + icon.GetWidth() * icon.GetHeight();
    }

    wxMAKE_ATOM(_NET_WM_ICON, (Display*)display);

    if ( size > 0 )
    {
        unsigned long* data = new unsigned long[size];
        unsigned long* ptr = data;

        for ( size_t i = 0; i < numIcons; ++i )
        {
            const wxImage image = ib.GetIconByIndex(i).ConvertToImage();
            int width = image.GetWidth(),
                height = image.GetHeight();
            unsigned char* imageData = image.GetData();
            unsigned char* imageDataEnd = imageData + ( width * height * 3 );
            bool hasMask = image.HasMask();
            unsigned char rMask, gMask, bMask;
            unsigned char r, g, b, a;

            if( hasMask )
            {
                rMask = image.GetMaskRed();
                gMask = image.GetMaskGreen();
                bMask = image.GetMaskBlue();
            }
            else             {
                rMask =
                gMask =
                bMask = 0;
            }

            *ptr++ = width;
            *ptr++ = height;

            while ( imageData < imageDataEnd )
            {
                r = imageData[0];
                g = imageData[1];
                b = imageData[2];
                if( hasMask && r == rMask && g == gMask && b == bMask )
                    a = 0;
                else
                    a = 255;

                *ptr++ = ( a << 24 ) | ( r << 16 ) | ( g << 8 ) | b;

                imageData += 3;
            }
        }

        XChangeProperty( (Display*)display,
                         WindowCast(window),
                         _NET_WM_ICON,
                         XA_CARDINAL, 32,
                         PropModeReplace,
                         (unsigned char*)data, size );
        delete[] data;
    }
    else
    {
        XDeleteProperty( (Display*)display,
                         WindowCast(window),
                         _NET_WM_ICON );
    }
}

#endif 



#define  WIN_LAYER_NORMAL       4
#define  WIN_LAYER_ABOVE_DOCK  10

static void wxWinHintsSetLayer(Display *display, Window rootWnd,
                               Window window, int layer)
{
    wxX11ErrorsSuspender noerrors(display);

    XEvent xev;

    wxMAKE_ATOM( _WIN_LAYER, display );

    if (IsMapped(display, window))
    {
        xev.type = ClientMessage;
        xev.xclient.type = ClientMessage;
        xev.xclient.window = window;
        xev.xclient.message_type = _WIN_LAYER;
        xev.xclient.format = 32;
        xev.xclient.data.l[0] = (long)layer;
        xev.xclient.data.l[1] = CurrentTime;

        XSendEvent(display, rootWnd, False,
                   SubstructureNotifyMask, (XEvent*) &xev);
    }
    else
    {
        long data[1];

        data[0] = layer;
        XChangeProperty(display, window,
                        _WIN_LAYER, XA_CARDINAL, 32,
                        PropModeReplace, (unsigned char *)data, 1);
    }
}



#ifdef __WXGTK20__
static bool wxQueryWMspecSupport(Display* WXUNUSED(display),
                                 Window WXUNUSED(rootWnd),
                                 Atom (feature))
{
    GdkAtom gatom = gdk_x11_xatom_to_atom(feature);
    return gdk_x11_screen_supports_net_wm_hint(gdk_screen_get_default(), gatom);
}
#else
static bool wxQueryWMspecSupport(Display *display, Window rootWnd, Atom feature)
{
    wxMAKE_ATOM(_NET_SUPPORTING_WM_CHECK, display);
    wxMAKE_ATOM(_NET_SUPPORTED, display);

                                                        
    Atom type;
    Window *wins;
    Atom *atoms;
    int format;
    unsigned long after;
    unsigned long nwins, natoms;

        XGetWindowProperty(display, rootWnd,
                       _NET_SUPPORTING_WM_CHECK, 0, LONG_MAX,
                       False, XA_WINDOW, &type, &format, &nwins,
                       &after, (unsigned char **)&wins);
    if ( type != XA_WINDOW || nwins == 0 || wins[0] == None )
       return false;
    XFree(wins);

        XGetWindowProperty(display, rootWnd,
                       _NET_SUPPORTED, 0, LONG_MAX,
                       False, XA_ATOM, &type, &format, &natoms,
                       &after, (unsigned char **)&atoms);
    if ( type != XA_ATOM || atoms == NULL )
        return false;

        for (unsigned i = 0; i < natoms; i++)
    {
        if ( atoms[i] == feature )
        {
            XFree(atoms);
            return true;
        }
    }
    XFree(atoms);
    return false;
}
#endif


#define _NET_WM_STATE_REMOVE        0
#define _NET_WM_STATE_ADD           1

static void wxWMspecSetState(Display *display, Window rootWnd,
                             Window window, int operation, Atom state)
{
    wxMAKE_ATOM(_NET_WM_STATE, display);

    if ( IsMapped(display, window) )
    {
        XEvent xev;
        xev.type = ClientMessage;
        xev.xclient.type = ClientMessage;
        xev.xclient.serial = 0;
        xev.xclient.send_event = True;
        xev.xclient.display = display;
        xev.xclient.window = window;
        xev.xclient.message_type = _NET_WM_STATE;
        xev.xclient.format = 32;
        xev.xclient.data.l[0] = operation;
        xev.xclient.data.l[1] = state;
        xev.xclient.data.l[2] = None;

        XSendEvent(display, rootWnd,
                   False,
                   SubstructureRedirectMask | SubstructureNotifyMask,
                   &xev);
    }
        }

static void wxWMspecSetFullscreen(Display *display, Window rootWnd,
                                  Window window, bool fullscreen)
{
    wxMAKE_ATOM(_NET_WM_STATE_FULLSCREEN, display);
    wxWMspecSetState(display, rootWnd,
                     window,
                     fullscreen ? _NET_WM_STATE_ADD : _NET_WM_STATE_REMOVE,
                      _NET_WM_STATE_FULLSCREEN);
}


static bool wxKwinRunning(Display *display, Window rootWnd)
{
    wxMAKE_ATOM(KWIN_RUNNING, display);

    unsigned char* data;
    Atom type;
    int format;
    unsigned long nitems, after;
    if (XGetWindowProperty(display, rootWnd,
                           KWIN_RUNNING, 0, 1, False, KWIN_RUNNING,
                           &type, &format, &nitems, &after,
                           &data) != Success)
    {
        return false;
    }

    bool retval = (type == KWIN_RUNNING &&
                   nitems == 1 && data && ((long*)data)[0] == 1);
    XFree(data);
    return retval;
}

static void wxSetKDEFullscreen(Display *display, Window rootWnd,
                               Window w, bool fullscreen, wxRect *origRect)
{
    long data[2];
    unsigned lng;

    wxMAKE_ATOM(_NET_WM_WINDOW_TYPE, display);
    wxMAKE_ATOM(_NET_WM_WINDOW_TYPE_NORMAL, display);
    wxMAKE_ATOM(_KDE_NET_WM_WINDOW_TYPE_OVERRIDE, display);
    wxMAKE_ATOM(_NET_WM_STATE_STAYS_ON_TOP, display);

    if (fullscreen)
    {
        data[0] = _KDE_NET_WM_WINDOW_TYPE_OVERRIDE;
        data[1] = _NET_WM_WINDOW_TYPE_NORMAL;
        lng = 2;
    }
    else
    {
        data[0] = _NET_WM_WINDOW_TYPE_NORMAL;
        data[1] = None;
        lng = 1;
    }

        XSync(display, False);

    bool wasMapped = IsMapped(display, w);
    if (wasMapped)
    {
        XUnmapWindow(display, w);
        XSync(display, False);
    }

    XChangeProperty(display, w, _NET_WM_WINDOW_TYPE, XA_ATOM, 32,
                    PropModeReplace, (unsigned char *) &data[0], lng);
    XSync(display, False);

    if (wasMapped)
    {
        XMapRaised(display, w);
        XSync(display, False);
    }

    wxWMspecSetState(display, rootWnd, w,
                     fullscreen ? _NET_WM_STATE_ADD : _NET_WM_STATE_REMOVE,
                     _NET_WM_STATE_STAYS_ON_TOP);
    XSync(display, False);

    if (!fullscreen)
    {
                                                        XMoveResizeWindow(display, w,
                          origRect->x, origRect->y,
                          origRect->width, origRect->height);
        XSync(display, False);
    }
}


wxX11FullScreenMethod wxGetFullScreenMethodX11(WXDisplay* display,
                                               WXWindow rootWindow)
{
    Window root = WindowCast(rootWindow);
    Display *disp = (Display*)display;

        wxMAKE_ATOM(_NET_WM_STATE_FULLSCREEN, disp);
    if (wxQueryWMspecSupport(disp, root, _NET_WM_STATE_FULLSCREEN))
    {
        wxLogTrace(wxT("fullscreen"),
                   wxT("detected _NET_WM_STATE_FULLSCREEN support"));
        return wxX11_FS_WMSPEC;
    }

            if (wxKwinRunning(disp, root))
    {
        wxLogTrace(wxT("fullscreen"), wxT("detected kwin"));
        return wxX11_FS_KDE;
    }

        wxLogTrace(wxT("fullscreen"), wxT("unknown WM, using _WIN_LAYER"));
    return wxX11_FS_GENERIC;
}


void wxSetFullScreenStateX11(WXDisplay* display, WXWindow rootWindow,
                             WXWindow window, bool show,
                             wxRect *origRect,
                             wxX11FullScreenMethod method)
{
        
    Window wnd = WindowCast(window);
    Window root = WindowCast(rootWindow);
    Display *disp = (Display*)display;

    if (method == wxX11_FS_AUTODETECT)
        method = wxGetFullScreenMethodX11(display, rootWindow);

    switch (method)
    {
        case wxX11_FS_WMSPEC:
            wxWMspecSetFullscreen(disp, root, wnd, show);
            break;
        case wxX11_FS_KDE:
            wxSetKDEFullscreen(disp, root, wnd, show, origRect);
            break;
        default:
            wxWinHintsSetLayer(disp, root, wnd,
                               show ? WIN_LAYER_ABOVE_DOCK : WIN_LAYER_NORMAL);
            break;
    }
}




#include <X11/keysym.h>


int wxCharCodeXToWX(WXKeySym keySym)
{
    int id;
    switch (keySym)
    {
        case XK_Shift_L:
        case XK_Shift_R:
            id = WXK_SHIFT; break;
        case XK_Control_L:
        case XK_Control_R:
            id = WXK_CONTROL; break;
        case XK_Meta_L:
        case XK_Meta_R:
            id = WXK_ALT; break;
        case XK_Caps_Lock:
            id = WXK_CAPITAL; break;
        case XK_BackSpace:
            id = WXK_BACK; break;
        case XK_Delete:
            id = WXK_DELETE; break;
        case XK_Clear:
            id = WXK_CLEAR; break;
        case XK_Tab:
            id = WXK_TAB; break;
        case XK_numbersign:
            id = '#'; break;
        case XK_Return:
            id = WXK_RETURN; break;
        case XK_Escape:
            id = WXK_ESCAPE; break;
        case XK_Pause:
        case XK_Break:
            id = WXK_PAUSE; break;
        case XK_Num_Lock:
            id = WXK_NUMLOCK; break;
        case XK_Scroll_Lock:
            id = WXK_SCROLL; break;

        case XK_Home:
            id = WXK_HOME; break;
        case XK_End:
            id = WXK_END; break;
        case XK_Left:
            id = WXK_LEFT; break;
        case XK_Right:
            id = WXK_RIGHT; break;
        case XK_Up:
            id = WXK_UP; break;
        case XK_Down:
            id = WXK_DOWN; break;
        case XK_Next:
            id = WXK_PAGEDOWN; break;
        case XK_Prior:
            id = WXK_PAGEUP; break;
        case XK_Menu:
            id = WXK_MENU; break;
        case XK_Select:
            id = WXK_SELECT; break;
        case XK_Cancel:
            id = WXK_CANCEL; break;
        case XK_Print:
            id = WXK_PRINT; break;
        case XK_Execute:
            id = WXK_EXECUTE; break;
        case XK_Insert:
            id = WXK_INSERT; break;
        case XK_Help:
            id = WXK_HELP; break;

        case XK_KP_Multiply:
            id = WXK_NUMPAD_MULTIPLY; break;
        case XK_KP_Add:
            id = WXK_NUMPAD_ADD; break;
        case XK_KP_Subtract:
            id = WXK_NUMPAD_SUBTRACT; break;
        case XK_KP_Divide:
            id = WXK_NUMPAD_DIVIDE; break;
        case XK_KP_Decimal:
            id = WXK_NUMPAD_DECIMAL; break;
        case XK_KP_Equal:
            id = WXK_NUMPAD_EQUAL; break;
        case XK_KP_Space:
            id = WXK_NUMPAD_SPACE; break;
        case XK_KP_Tab:
            id = WXK_NUMPAD_TAB; break;
        case XK_KP_Enter:
            id = WXK_NUMPAD_ENTER; break;
        case XK_KP_0:
            id = WXK_NUMPAD0; break;
        case XK_KP_1:
            id = WXK_NUMPAD1; break;
        case XK_KP_2:
            id = WXK_NUMPAD2; break;
        case XK_KP_3:
            id = WXK_NUMPAD3; break;
        case XK_KP_4:
            id = WXK_NUMPAD4; break;
        case XK_KP_5:
            id = WXK_NUMPAD5; break;
        case XK_KP_6:
            id = WXK_NUMPAD6; break;
        case XK_KP_7:
            id = WXK_NUMPAD7; break;
        case XK_KP_8:
            id = WXK_NUMPAD8; break;
        case XK_KP_9:
            id = WXK_NUMPAD9; break;
        case XK_KP_Insert:
            id = WXK_NUMPAD_INSERT; break;
        case XK_KP_End:
            id = WXK_NUMPAD_END; break;
        case XK_KP_Down:
            id = WXK_NUMPAD_DOWN; break;
        case XK_KP_Page_Down:
            id = WXK_NUMPAD_PAGEDOWN; break;
        case XK_KP_Left:
            id = WXK_NUMPAD_LEFT; break;
        case XK_KP_Right:
            id = WXK_NUMPAD_RIGHT; break;
        case XK_KP_Home:
            id = WXK_NUMPAD_HOME; break;
        case XK_KP_Up:
            id = WXK_NUMPAD_UP; break;
        case XK_KP_Page_Up:
            id = WXK_NUMPAD_PAGEUP; break;
        case XK_F1:
            id = WXK_F1; break;
        case XK_F2:
            id = WXK_F2; break;
        case XK_F3:
            id = WXK_F3; break;
        case XK_F4:
            id = WXK_F4; break;
        case XK_F5:
            id = WXK_F5; break;
        case XK_F6:
            id = WXK_F6; break;
        case XK_F7:
            id = WXK_F7; break;
        case XK_F8:
            id = WXK_F8; break;
        case XK_F9:
            id = WXK_F9; break;
        case XK_F10:
            id = WXK_F10; break;
        case XK_F11:
            id = WXK_F11; break;
        case XK_F12:
            id = WXK_F12; break;
        case XK_F13:
            id = WXK_F13; break;
        case XK_F14:
            id = WXK_F14; break;
        case XK_F15:
            id = WXK_F15; break;
        case XK_F16:
            id = WXK_F16; break;
        case XK_F17:
            id = WXK_F17; break;
        case XK_F18:
            id = WXK_F18; break;
        case XK_F19:
            id = WXK_F19; break;
        case XK_F20:
            id = WXK_F20; break;
        case XK_F21:
            id = WXK_F21; break;
        case XK_F22:
            id = WXK_F22; break;
        case XK_F23:
            id = WXK_F23; break;
        case XK_F24:
            id = WXK_F24; break;
        default:
            id = (keySym <= 255) ? (int)keySym : -1;
    }

    return id;
}

WXKeySym wxCharCodeWXToX(int id)
{
    WXKeySym keySym;

    switch (id)
    {
        case WXK_CANCEL:        keySym = XK_Cancel; break;
        case WXK_BACK:          keySym = XK_BackSpace; break;
        case WXK_TAB:           keySym = XK_Tab; break;
        case WXK_CLEAR:         keySym = XK_Clear; break;
        case WXK_RETURN:        keySym = XK_Return; break;
        case WXK_SHIFT:         keySym = XK_Shift_L; break;
        case WXK_CONTROL:       keySym = XK_Control_L; break;
        case WXK_ALT:           keySym = XK_Meta_L; break;
        case WXK_CAPITAL:       keySym = XK_Caps_Lock; break;
        case WXK_MENU :         keySym = XK_Menu; break;
        case WXK_PAUSE:         keySym = XK_Pause; break;
        case WXK_ESCAPE:        keySym = XK_Escape; break;
        case WXK_SPACE:         keySym = ' '; break;
        case WXK_PAGEUP:        keySym = XK_Prior; break;
        case WXK_PAGEDOWN:      keySym = XK_Next; break;
        case WXK_END:           keySym = XK_End; break;
        case WXK_HOME :         keySym = XK_Home; break;
        case WXK_LEFT :         keySym = XK_Left; break;
        case WXK_UP:            keySym = XK_Up; break;
        case WXK_RIGHT:         keySym = XK_Right; break;
        case WXK_DOWN :         keySym = XK_Down; break;
        case WXK_SELECT:        keySym = XK_Select; break;
        case WXK_PRINT:         keySym = XK_Print; break;
        case WXK_EXECUTE:       keySym = XK_Execute; break;
        case WXK_INSERT:        keySym = XK_Insert; break;
        case WXK_DELETE:        keySym = XK_Delete; break;
        case WXK_HELP :         keySym = XK_Help; break;
        case WXK_NUMPAD0:       keySym = XK_KP_0; break; case WXK_NUMPAD_INSERT:     keySym = XK_KP_Insert; break;
        case WXK_NUMPAD1:       keySym = XK_KP_1; break; case WXK_NUMPAD_END:           keySym = XK_KP_End; break;
        case WXK_NUMPAD2:       keySym = XK_KP_2; break; case WXK_NUMPAD_DOWN:      keySym = XK_KP_Down; break;
        case WXK_NUMPAD3:       keySym = XK_KP_3; break; case WXK_NUMPAD_PAGEDOWN:  keySym = XK_KP_Page_Down; break;
        case WXK_NUMPAD4:       keySym = XK_KP_4; break; case WXK_NUMPAD_LEFT:         keySym = XK_KP_Left; break;
        case WXK_NUMPAD5:       keySym = XK_KP_5; break;
        case WXK_NUMPAD6:       keySym = XK_KP_6; break; case WXK_NUMPAD_RIGHT:       keySym = XK_KP_Right; break;
        case WXK_NUMPAD7:       keySym = XK_KP_7; break; case WXK_NUMPAD_HOME:       keySym = XK_KP_Home; break;
        case WXK_NUMPAD8:       keySym = XK_KP_8; break; case WXK_NUMPAD_UP:             keySym = XK_KP_Up; break;
        case WXK_NUMPAD9:       keySym = XK_KP_9; break; case WXK_NUMPAD_PAGEUP:   keySym = XK_KP_Page_Up; break;
        case WXK_NUMPAD_DECIMAL:    keySym = XK_KP_Decimal; break; case WXK_NUMPAD_DELETE:   keySym = XK_KP_Delete; break;
        case WXK_NUMPAD_MULTIPLY:   keySym = XK_KP_Multiply; break;
        case WXK_NUMPAD_ADD:             keySym = XK_KP_Add; break;
        case WXK_NUMPAD_SUBTRACT: keySym = XK_KP_Subtract; break;
        case WXK_NUMPAD_DIVIDE:        keySym = XK_KP_Divide; break;
        case WXK_NUMPAD_ENTER:        keySym = XK_KP_Enter; break;
        case WXK_NUMPAD_SEPARATOR:  keySym = XK_KP_Separator; break;
        case WXK_F1:            keySym = XK_F1; break;
        case WXK_F2:            keySym = XK_F2; break;
        case WXK_F3:            keySym = XK_F3; break;
        case WXK_F4:            keySym = XK_F4; break;
        case WXK_F5:            keySym = XK_F5; break;
        case WXK_F6:            keySym = XK_F6; break;
        case WXK_F7:            keySym = XK_F7; break;
        case WXK_F8:            keySym = XK_F8; break;
        case WXK_F9:            keySym = XK_F9; break;
        case WXK_F10:           keySym = XK_F10; break;
        case WXK_F11:           keySym = XK_F11; break;
        case WXK_F12:           keySym = XK_F12; break;
        case WXK_F13:           keySym = XK_F13; break;
        case WXK_F14:           keySym = XK_F14; break;
        case WXK_F15:           keySym = XK_F15; break;
        case WXK_F16:           keySym = XK_F16; break;
        case WXK_F17:           keySym = XK_F17; break;
        case WXK_F18:           keySym = XK_F18; break;
        case WXK_F19:           keySym = XK_F19; break;
        case WXK_F20:           keySym = XK_F20; break;
        case WXK_F21:           keySym = XK_F21; break;
        case WXK_F22:           keySym = XK_F22; break;
        case WXK_F23:           keySym = XK_F23; break;
        case WXK_F24:           keySym = XK_F24; break;
        case WXK_NUMLOCK:       keySym = XK_Num_Lock; break;
        case WXK_SCROLL:        keySym = XK_Scroll_Lock; break;
        default:                keySym = id <= 255 ? (KeySym)id : 0;
    }

    return keySym;
}

CodePair keySymTab[] = {
    {0x0020, 0x0020},        {0x0021, 0x0021},        {0x0022, 0x0022},        {0x0023, 0x0023},        {0x0024, 0x0024},        {0x0025, 0x0025},        {0x0026, 0x0026},        {0x0027, 0x0027},        {0x0027, 0x0027},        {0x0028, 0x0028},        {0x0029, 0x0029},        {0x002a, 0x002a},        {0x002b, 0x002b},        {0x002c, 0x002c},        {0x002d, 0x002d},        {0x002e, 0x002e},        {0x002f, 0x002f},        {0x0030, 0x0030},        {0x0031, 0x0031},        {0x0032, 0x0032},        {0x0033, 0x0033},        {0x0034, 0x0034},        {0x0035, 0x0035},        {0x0036, 0x0036},        {0x0037, 0x0037},        {0x0038, 0x0038},        {0x0039, 0x0039},        {0x003a, 0x003a},        {0x003b, 0x003b},        {0x003c, 0x003c},        {0x003d, 0x003d},        {0x003e, 0x003e},        {0x003f, 0x003f},        {0x0040, 0x0040},        {0x0041, 0x0041},        {0x0042, 0x0042},        {0x0043, 0x0043},        {0x0044, 0x0044},        {0x0045, 0x0045},        {0x0046, 0x0046},        {0x0047, 0x0047},        {0x0048, 0x0048},        {0x0049, 0x0049},        {0x004a, 0x004a},        {0x004b, 0x004b},        {0x004c, 0x004c},        {0x004d, 0x004d},        {0x004e, 0x004e},        {0x004f, 0x004f},        {0x0050, 0x0050},        {0x0051, 0x0051},        {0x0052, 0x0052},        {0x0053, 0x0053},        {0x0054, 0x0054},        {0x0055, 0x0055},        {0x0056, 0x0056},        {0x0057, 0x0057},        {0x0058, 0x0058},        {0x0059, 0x0059},        {0x005a, 0x005a},        {0x005b, 0x005b},        {0x005c, 0x005c},        {0x005d, 0x005d},        {0x005e, 0x005e},        {0x005f, 0x005f},        {0x0060, 0x0060},        {0x0060, 0x0060},        {0x0061, 0x0061},        {0x0062, 0x0062},        {0x0063, 0x0063},        {0x0064, 0x0064},        {0x0065, 0x0065},        {0x0066, 0x0066},        {0x0067, 0x0067},        {0x0068, 0x0068},        {0x0069, 0x0069},        {0x006a, 0x006a},        {0x006b, 0x006b},        {0x006c, 0x006c},        {0x006d, 0x006d},        {0x006e, 0x006e},        {0x006f, 0x006f},        {0x0070, 0x0070},        {0x0071, 0x0071},        {0x0072, 0x0072},        {0x0073, 0x0073},        {0x0074, 0x0074},        {0x0075, 0x0075},        {0x0076, 0x0076},        {0x0077, 0x0077},        {0x0078, 0x0078},        {0x0079, 0x0079},        {0x007a, 0x007a},        {0x007b, 0x007b},        {0x007c, 0x007c},        {0x007d, 0x007d},        {0x007e, 0x007e},        {0x00a0, 0x00a0},        {0x00a1, 0x00a1},        {0x00a2, 0x00a2},        {0x00a3, 0x00a3},        {0x00a4, 0x00a4},        {0x00a5, 0x00a5},        {0x00a6, 0x00a6},        {0x00a7, 0x00a7},        {0x00a8, 0x00a8},        {0x00a9, 0x00a9},        {0x00aa, 0x00aa},        {0x00ab, 0x00ab},        {0x00ac, 0x00ac},        {0x00ad, 0x00ad},        {0x00ae, 0x00ae},        {0x00af, 0x00af},        {0x00b0, 0x00b0},        {0x00b1, 0x00b1},        {0x00b2, 0x00b2},        {0x00b3, 0x00b3},        {0x00b4, 0x00b4},        {0x00b5, 0x00b5},        {0x00b6, 0x00b6},        {0x00b7, 0x00b7},        {0x00b8, 0x00b8},        {0x00b9, 0x00b9},        {0x00ba, 0x00ba},        {0x00bb, 0x00bb},        {0x00bc, 0x00bc},        {0x00bd, 0x00bd},        {0x00be, 0x00be},        {0x00bf, 0x00bf},        {0x00c0, 0x00c0},        {0x00c1, 0x00c1},        {0x00c2, 0x00c2},        {0x00c3, 0x00c3},        {0x00c4, 0x00c4},        {0x00c5, 0x00c5},        {0x00c6, 0x00c6},        {0x00c7, 0x00c7},        {0x00c8, 0x00c8},        {0x00c9, 0x00c9},        {0x00ca, 0x00ca},        {0x00cb, 0x00cb},        {0x00cc, 0x00cc},        {0x00cd, 0x00cd},        {0x00ce, 0x00ce},        {0x00cf, 0x00cf},        {0x00d0, 0x00d0},        {0x00d0, 0x00d0},        {0x00d1, 0x00d1},        {0x00d2, 0x00d2},        {0x00d3, 0x00d3},        {0x00d4, 0x00d4},        {0x00d5, 0x00d5},        {0x00d6, 0x00d6},        {0x00d7, 0x00d7},        {0x00d8, 0x00d8},        {0x00d9, 0x00d9},        {0x00da, 0x00da},        {0x00db, 0x00db},        {0x00dc, 0x00dc},        {0x00dd, 0x00dd},        {0x00de, 0x00de},        {0x00de, 0x00de},        {0x00df, 0x00df},        {0x00e0, 0x00e0},        {0x00e1, 0x00e1},        {0x00e2, 0x00e2},        {0x00e3, 0x00e3},        {0x00e4, 0x00e4},        {0x00e5, 0x00e5},        {0x00e6, 0x00e6},        {0x00e7, 0x00e7},        {0x00e8, 0x00e8},        {0x00e9, 0x00e9},        {0x00ea, 0x00ea},        {0x00eb, 0x00eb},        {0x00ec, 0x00ec},        {0x00ed, 0x00ed},        {0x00ee, 0x00ee},        {0x00ef, 0x00ef},        {0x00f0, 0x00f0},        {0x00f1, 0x00f1},        {0x00f2, 0x00f2},        {0x00f3, 0x00f3},        {0x00f4, 0x00f4},        {0x00f5, 0x00f5},        {0x00f6, 0x00f6},        {0x00f7, 0x00f7},        {0x00f8, 0x00f8},        {0x00f9, 0x00f9},        {0x00fa, 0x00fa},        {0x00fb, 0x00fb},        {0x00fc, 0x00fc},        {0x00fd, 0x00fd},        {0x00fe, 0x00fe},        {0x00ff, 0x00ff},        {0x01a1, 0x0104},        {0x01a2, 0x02d8},        {0x01a3, 0x0141},        {0x01a5, 0x013d},        {0x01a6, 0x015a},        {0x01a9, 0x0160},        {0x01aa, 0x015e},        {0x01ab, 0x0164},        {0x01ac, 0x0179},        {0x01ae, 0x017d},        {0x01af, 0x017b},        {0x01b1, 0x0105},        {0x01b2, 0x02db},        {0x01b3, 0x0142},        {0x01b5, 0x013e},        {0x01b6, 0x015b},        {0x01b7, 0x02c7},        {0x01b9, 0x0161},        {0x01ba, 0x015f},        {0x01bb, 0x0165},        {0x01bc, 0x017a},        {0x01bd, 0x02dd},        {0x01be, 0x017e},        {0x01bf, 0x017c},        {0x01c0, 0x0154},        {0x01c3, 0x0102},        {0x01c5, 0x0139},        {0x01c6, 0x0106},        {0x01c8, 0x010c},        {0x01ca, 0x0118},        {0x01cc, 0x011a},        {0x01cf, 0x010e},        {0x01d0, 0x0110},        {0x01d1, 0x0143},        {0x01d2, 0x0147},        {0x01d5, 0x0150},        {0x01d8, 0x0158},        {0x01d9, 0x016e},        {0x01db, 0x0170},        {0x01de, 0x0162},        {0x01e0, 0x0155},        {0x01e3, 0x0103},        {0x01e5, 0x013a},        {0x01e6, 0x0107},        {0x01e8, 0x010d},        {0x01ea, 0x0119},        {0x01ec, 0x011b},        {0x01ef, 0x010f},        {0x01f0, 0x0111},        {0x01f1, 0x0144},        {0x01f2, 0x0148},        {0x01f5, 0x0151},        {0x01f8, 0x0159},        {0x01f9, 0x016f},        {0x01fb, 0x0171},        {0x01fe, 0x0163},        {0x01ff, 0x02d9},        {0x02a1, 0x0126},        {0x02a6, 0x0124},        {0x02a9, 0x0130},        {0x02ab, 0x011e},        {0x02ac, 0x0134},        {0x02b1, 0x0127},        {0x02b6, 0x0125},        {0x02b9, 0x0131},        {0x02bb, 0x011f},        {0x02bc, 0x0135},        {0x02c5, 0x010a},        {0x02c6, 0x0108},        {0x02d5, 0x0120},        {0x02d8, 0x011c},        {0x02dd, 0x016c},        {0x02de, 0x015c},        {0x02e5, 0x010b},        {0x02e6, 0x0109},        {0x02f5, 0x0121},        {0x02f8, 0x011d},        {0x02fd, 0x016d},        {0x02fe, 0x015d},        {0x03a2, 0x0138},        {0x03a3, 0x0156},        {0x03a5, 0x0128},        {0x03a6, 0x013b},        {0x03aa, 0x0112},        {0x03ab, 0x0122},        {0x03ac, 0x0166},        {0x03b3, 0x0157},        {0x03b5, 0x0129},        {0x03b6, 0x013c},        {0x03ba, 0x0113},        {0x03bb, 0x0123},        {0x03bc, 0x0167},        {0x03bd, 0x014a},        {0x03bf, 0x014b},        {0x03c0, 0x0100},        {0x03c7, 0x012e},        {0x03cc, 0x0116},        {0x03cf, 0x012a},        {0x03d1, 0x0145},        {0x03d2, 0x014c},        {0x03d3, 0x0136},        {0x03d9, 0x0172},        {0x03dd, 0x0168},        {0x03de, 0x016a},        {0x03e0, 0x0101},        {0x03e7, 0x012f},        {0x03ec, 0x0117},        {0x03ef, 0x012b},        {0x03f1, 0x0146},        {0x03f2, 0x014d},        {0x03f3, 0x0137},        {0x03f9, 0x0173},        {0x03fd, 0x0169},        {0x03fe, 0x016b},        {0x047e, 0x203e},        {0x04a1, 0x3002},        {0x04a2, 0x300c},        {0x04a3, 0x300d},        {0x04a4, 0x3001},        {0x04a5, 0x30fb},        {0x04a6, 0x30f2},        {0x04a7, 0x30a1},        {0x04a8, 0x30a3},        {0x04a9, 0x30a5},        {0x04aa, 0x30a7},        {0x04ab, 0x30a9},        {0x04ac, 0x30e3},        {0x04ad, 0x30e5},        {0x04ae, 0x30e7},        {0x04af, 0x30c3},        {0x04b0, 0x30fc},        {0x04b1, 0x30a2},        {0x04b2, 0x30a4},        {0x04b3, 0x30a6},        {0x04b4, 0x30a8},        {0x04b5, 0x30aa},        {0x04b6, 0x30ab},        {0x04b7, 0x30ad},        {0x04b8, 0x30af},        {0x04b9, 0x30b1},        {0x04ba, 0x30b3},        {0x04bb, 0x30b5},        {0x04bc, 0x30b7},        {0x04bd, 0x30b9},        {0x04be, 0x30bb},        {0x04bf, 0x30bd},        {0x04c0, 0x30bf},        {0x04c1, 0x30c1},        {0x04c2, 0x30c4},        {0x04c3, 0x30c6},        {0x04c4, 0x30c8},        {0x04c5, 0x30ca},        {0x04c6, 0x30cb},        {0x04c7, 0x30cc},        {0x04c8, 0x30cd},        {0x04c9, 0x30ce},        {0x04ca, 0x30cf},        {0x04cb, 0x30d2},        {0x04cc, 0x30d5},        {0x04cd, 0x30d8},        {0x04ce, 0x30db},        {0x04cf, 0x30de},        {0x04d0, 0x30df},        {0x04d1, 0x30e0},        {0x04d2, 0x30e1},        {0x04d3, 0x30e2},        {0x04d4, 0x30e4},        {0x04d5, 0x30e6},        {0x04d6, 0x30e8},        {0x04d7, 0x30e9},        {0x04d8, 0x30ea},        {0x04d9, 0x30eb},        {0x04da, 0x30ec},        {0x04db, 0x30ed},        {0x04dc, 0x30ef},        {0x04dd, 0x30f3},        {0x04de, 0x309b},        {0x04df, 0x309c},        {0x05ac, 0x060c},        {0x05bb, 0x061b},        {0x05bf, 0x061f},        {0x05c1, 0x0621},        {0x05c2, 0x0622},        {0x05c3, 0x0623},        {0x05c4, 0x0624},        {0x05c5, 0x0625},        {0x05c6, 0x0626},        {0x05c7, 0x0627},        {0x05c8, 0x0628},        {0x05c9, 0x0629},        {0x05ca, 0x062a},        {0x05cb, 0x062b},        {0x05cc, 0x062c},        {0x05cd, 0x062d},        {0x05ce, 0x062e},        {0x05cf, 0x062f},        {0x05d0, 0x0630},        {0x05d1, 0x0631},        {0x05d2, 0x0632},        {0x05d3, 0x0633},        {0x05d4, 0x0634},        {0x05d5, 0x0635},        {0x05d6, 0x0636},        {0x05d7, 0x0637},        {0x05d8, 0x0638},        {0x05d9, 0x0639},        {0x05da, 0x063a},        {0x05e0, 0x0640},        {0x05e1, 0x0641},        {0x05e2, 0x0642},        {0x05e3, 0x0643},        {0x05e4, 0x0644},        {0x05e5, 0x0645},        {0x05e6, 0x0646},        {0x05e7, 0x0647},        {0x05e8, 0x0648},        {0x05e9, 0x0649},        {0x05ea, 0x064a},        {0x05eb, 0x064b},        {0x05ec, 0x064c},        {0x05ed, 0x064d},        {0x05ee, 0x064e},        {0x05ef, 0x064f},        {0x05f0, 0x0650},        {0x05f1, 0x0651},        {0x05f2, 0x0652},        {0x06a1, 0x0452},        {0x06a2, 0x0453},        {0x06a3, 0x0451},        {0x06a4, 0x0454},        {0x06a5, 0x0455},        {0x06a6, 0x0456},        {0x06a7, 0x0457},        {0x06a8, 0x0458},        {0x06a9, 0x0459},        {0x06aa, 0x045a},        {0x06ab, 0x045b},        {0x06ac, 0x045c},        {0x06ae, 0x045e},        {0x06af, 0x045f},        {0x06b0, 0x2116},        {0x06b1, 0x0402},        {0x06b2, 0x0403},        {0x06b3, 0x0401},        {0x06b4, 0x0404},        {0x06b5, 0x0405},        {0x06b6, 0x0406},        {0x06b7, 0x0407},        {0x06b8, 0x0408},        {0x06b9, 0x0409},        {0x06ba, 0x040a},        {0x06bb, 0x040b},        {0x06bc, 0x040c},        {0x06be, 0x040e},        {0x06bf, 0x040f},        {0x06c0, 0x044e},        {0x06c1, 0x0430},        {0x06c2, 0x0431},        {0x06c3, 0x0446},        {0x06c4, 0x0434},        {0x06c5, 0x0435},        {0x06c6, 0x0444},        {0x06c7, 0x0433},        {0x06c8, 0x0445},        {0x06c9, 0x0438},        {0x06ca, 0x0439},        {0x06cb, 0x043a},        {0x06cc, 0x043b},        {0x06cd, 0x043c},        {0x06ce, 0x043d},        {0x06cf, 0x043e},        {0x06d0, 0x043f},        {0x06d1, 0x044f},        {0x06d2, 0x0440},        {0x06d3, 0x0441},        {0x06d4, 0x0442},        {0x06d5, 0x0443},        {0x06d6, 0x0436},        {0x06d7, 0x0432},        {0x06d8, 0x044c},        {0x06d9, 0x044b},        {0x06da, 0x0437},        {0x06db, 0x0448},        {0x06dc, 0x044d},        {0x06dd, 0x0449},        {0x06de, 0x0447},        {0x06df, 0x044a},        {0x06e0, 0x042e},        {0x06e1, 0x0410},        {0x06e2, 0x0411},        {0x06e3, 0x0426},        {0x06e4, 0x0414},        {0x06e5, 0x0415},        {0x06e6, 0x0424},        {0x06e7, 0x0413},        {0x06e8, 0x0425},        {0x06e9, 0x0418},        {0x06ea, 0x0419},        {0x06eb, 0x041a},        {0x06ec, 0x041b},        {0x06ed, 0x041c},        {0x06ee, 0x041d},        {0x06ef, 0x041e},        {0x06f0, 0x041f},        {0x06f1, 0x042f},        {0x06f2, 0x0420},        {0x06f3, 0x0421},        {0x06f4, 0x0422},        {0x06f5, 0x0423},        {0x06f6, 0x0416},        {0x06f7, 0x0412},        {0x06f8, 0x042c},        {0x06f9, 0x042b},        {0x06fa, 0x0417},        {0x06fb, 0x0428},        {0x06fc, 0x042d},        {0x06fd, 0x0429},        {0x06fe, 0x0427},        {0x06ff, 0x042a},        {0x07a1, 0x0386},        {0x07a2, 0x0388},        {0x07a3, 0x0389},        {0x07a4, 0x038a},        {0x07a5, 0x03aa},        {0x07a7, 0x038c},        {0x07a8, 0x038e},        {0x07a9, 0x03ab},        {0x07ab, 0x038f},        {0x07ae, 0x0385},        {0x07af, 0x2015},        {0x07b1, 0x03ac},        {0x07b2, 0x03ad},        {0x07b3, 0x03ae},        {0x07b4, 0x03af},        {0x07b5, 0x03ca},        {0x07b6, 0x0390},        {0x07b7, 0x03cc},        {0x07b8, 0x03cd},        {0x07b9, 0x03cb},        {0x07ba, 0x03b0},        {0x07bb, 0x03ce},        {0x07c1, 0x0391},        {0x07c2, 0x0392},        {0x07c3, 0x0393},        {0x07c4, 0x0394},        {0x07c5, 0x0395},        {0x07c6, 0x0396},        {0x07c7, 0x0397},        {0x07c8, 0x0398},        {0x07c9, 0x0399},        {0x07ca, 0x039a},        {0x07cb, 0x039b},        {0x07cb, 0x039b},        {0x07cc, 0x039c},        {0x07cd, 0x039d},        {0x07ce, 0x039e},        {0x07cf, 0x039f},        {0x07d0, 0x03a0},        {0x07d1, 0x03a1},        {0x07d2, 0x03a3},        {0x07d4, 0x03a4},        {0x07d5, 0x03a5},        {0x07d6, 0x03a6},        {0x07d7, 0x03a7},        {0x07d8, 0x03a8},        {0x07d9, 0x03a9},        {0x07e1, 0x03b1},        {0x07e2, 0x03b2},        {0x07e3, 0x03b3},        {0x07e4, 0x03b4},        {0x07e5, 0x03b5},        {0x07e6, 0x03b6},        {0x07e7, 0x03b7},        {0x07e8, 0x03b8},        {0x07e9, 0x03b9},        {0x07ea, 0x03ba},        {0x07eb, 0x03bb},        {0x07ec, 0x03bc},        {0x07ed, 0x03bd},        {0x07ee, 0x03be},        {0x07ef, 0x03bf},        {0x07f0, 0x03c0},        {0x07f1, 0x03c1},        {0x07f2, 0x03c3},        {0x07f3, 0x03c2},        {0x07f4, 0x03c4},        {0x07f5, 0x03c5},        {0x07f6, 0x03c6},        {0x07f7, 0x03c7},        {0x07f8, 0x03c8},        {0x07f9, 0x03c9},        {0x08a1, 0x23b7},        {0x08a2, 0x250c},        {0x08a3, 0x2500},        {0x08a4, 0x2320},        {0x08a5, 0x2321},        {0x08a6, 0x2502},        {0x08a7, 0x23a1},        {0x08a8, 0x23a3},        {0x08a9, 0x23a4},        {0x08aa, 0x23a6},        {0x08ab, 0x239b},        {0x08ac, 0x239d},        {0x08ad, 0x239e},        {0x08ae, 0x23a0},        {0x08af, 0x23a8},        {0x08b0, 0x23ac},        {0x08b1, 0x0000},        {0x08b2, 0x0000},        {0x08b3, 0x0000},        {0x08b4, 0x0000},        {0x08b5, 0x0000},        {0x08b6, 0x0000},        {0x08b7, 0x0000},        {0x08bc, 0x2264},        {0x08bd, 0x2260},        {0x08be, 0x2265},        {0x08bf, 0x222b},        {0x08c0, 0x2234},        {0x08c1, 0x221d},        {0x08c2, 0x221e},        {0x08c5, 0x2207},        {0x08c8, 0x223c},        {0x08c9, 0x2243},        {0x08cd, 0x21d4},        {0x08ce, 0x21d2},        {0x08cf, 0x2261},        {0x08d6, 0x221a},        {0x08da, 0x2282},        {0x08db, 0x2283},        {0x08dc, 0x2229},        {0x08dd, 0x222a},        {0x08de, 0x2227},        {0x08df, 0x2228},        {0x08ef, 0x2202},        {0x08f6, 0x0192},        {0x08fb, 0x2190},        {0x08fc, 0x2191},        {0x08fd, 0x2192},        {0x08fe, 0x2193},        {0x09df, 0x0000},        {0x09e0, 0x25c6},        {0x09e1, 0x2592},        {0x09e2, 0x2409},        {0x09e3, 0x240c},        {0x09e4, 0x240d},        {0x09e5, 0x240a},        {0x09e8, 0x2424},        {0x09e9, 0x240b},        {0x09ea, 0x2518},        {0x09eb, 0x2510},        {0x09ec, 0x250c},        {0x09ed, 0x2514},        {0x09ee, 0x253c},        {0x09ef, 0x23ba},        {0x09f0, 0x23bb},        {0x09f1, 0x2500},        {0x09f2, 0x23bc},        {0x09f3, 0x23bd},        {0x09f4, 0x251c},        {0x09f5, 0x2524},        {0x09f6, 0x2534},        {0x09f7, 0x252c},        {0x09f8, 0x2502},        {0x0aa1, 0x2003},        {0x0aa2, 0x2002},        {0x0aa3, 0x2004},        {0x0aa4, 0x2005},        {0x0aa5, 0x2007},        {0x0aa6, 0x2008},        {0x0aa7, 0x2009},        {0x0aa8, 0x200a},        {0x0aa9, 0x2014},        {0x0aaa, 0x2013},        {0x0aac, 0x2423},        {0x0aae, 0x2026},        {0x0aaf, 0x2025},        {0x0ab0, 0x2153},        {0x0ab1, 0x2154},        {0x0ab2, 0x2155},        {0x0ab3, 0x2156},        {0x0ab4, 0x2157},        {0x0ab5, 0x2158},        {0x0ab6, 0x2159},        {0x0ab7, 0x215a},        {0x0ab8, 0x2105},        {0x0abb, 0x2012},        {0x0abc, 0x27e8},        {0x0abd, 0x002e},        {0x0abe, 0x27e9},        {0x0abf, 0x0000},        {0x0ac3, 0x215b},        {0x0ac4, 0x215c},        {0x0ac5, 0x215d},        {0x0ac6, 0x215e},        {0x0ac9, 0x2122},        {0x0aca, 0x2613},        {0x0acb, 0x0000},        {0x0acc, 0x25c1},        {0x0acd, 0x25b7},        {0x0ace, 0x25cb},        {0x0acf, 0x25af},        {0x0ad0, 0x2018},        {0x0ad1, 0x2019},        {0x0ad2, 0x201c},        {0x0ad3, 0x201d},        {0x0ad4, 0x211e},        {0x0ad6, 0x2032},        {0x0ad7, 0x2033},        {0x0ad9, 0x271d},        {0x0ada, 0x0000},        {0x0adb, 0x25ac},        {0x0adc, 0x25c0},        {0x0add, 0x25b6},        {0x0ade, 0x25cf},        {0x0adf, 0x25ae},        {0x0ae0, 0x25e6},        {0x0ae1, 0x25ab},        {0x0ae2, 0x25ad},        {0x0ae3, 0x25b3},        {0x0ae4, 0x25bd},        {0x0ae5, 0x2606},        {0x0ae6, 0x2022},        {0x0ae7, 0x25aa},        {0x0ae8, 0x25b2},        {0x0ae9, 0x25bc},        {0x0aea, 0x261c},        {0x0aeb, 0x261e},        {0x0aec, 0x2663},        {0x0aed, 0x2666},        {0x0aee, 0x2665},        {0x0af0, 0x2720},        {0x0af1, 0x2020},        {0x0af2, 0x2021},        {0x0af3, 0x2713},        {0x0af4, 0x2717},        {0x0af5, 0x266f},        {0x0af6, 0x266d},        {0x0af7, 0x2642},        {0x0af8, 0x2640},        {0x0af9, 0x260e},        {0x0afa, 0x2315},        {0x0afb, 0x2117},        {0x0afc, 0x2038},        {0x0afd, 0x201a},        {0x0afe, 0x201e},        {0x0aff, 0x0000},        {0x0ba3, 0x003c},        {0x0ba6, 0x003e},        {0x0ba8, 0x2228},        {0x0ba9, 0x2227},        {0x0bc0, 0x00af},        {0x0bc2, 0x22a5},        {0x0bc3, 0x2229},        {0x0bc4, 0x230a},        {0x0bc6, 0x005f},        {0x0bca, 0x2218},        {0x0bcc, 0x2395},        {0x0bce, 0x22a4},        {0x0bcf, 0x25cb},        {0x0bd3, 0x2308},        {0x0bd6, 0x222a},        {0x0bd8, 0x2283},        {0x0bda, 0x2282},        {0x0bdc, 0x22a2},        {0x0bfc, 0x22a3},        {0x0cdf, 0x2017},        {0x0ce0, 0x05d0},        {0x0ce1, 0x05d1},        {0x0ce1, 0x05d1},        {0x0ce2, 0x05d2},        {0x0ce2, 0x05d2},        {0x0ce3, 0x05d3},        {0x0ce3, 0x05d3},        {0x0ce4, 0x05d4},        {0x0ce5, 0x05d5},        {0x0ce6, 0x05d6},        {0x0ce6, 0x05d6},        {0x0ce7, 0x05d7},        {0x0ce7, 0x05d7},        {0x0ce8, 0x05d8},        {0x0ce8, 0x05d8},        {0x0ce9, 0x05d9},        {0x0cea, 0x05da},        {0x0ceb, 0x05db},        {0x0cec, 0x05dc},        {0x0ced, 0x05dd},        {0x0cee, 0x05de},        {0x0cef, 0x05df},        {0x0cf0, 0x05e0},        {0x0cf1, 0x05e1},        {0x0cf1, 0x05e1},        {0x0cf2, 0x05e2},        {0x0cf3, 0x05e3},        {0x0cf4, 0x05e4},        {0x0cf5, 0x05e5},        {0x0cf5, 0x05e5},        {0x0cf6, 0x05e6},        {0x0cf6, 0x05e6},        {0x0cf7, 0x05e7},        {0x0cf7, 0x05e7},        {0x0cf8, 0x05e8},        {0x0cf9, 0x05e9},        {0x0cfa, 0x05ea},        {0x0cfa, 0x05ea},        {0x0da1, 0x0e01},        {0x0da2, 0x0e02},        {0x0da3, 0x0e03},        {0x0da4, 0x0e04},        {0x0da5, 0x0e05},        {0x0da6, 0x0e06},        {0x0da7, 0x0e07},        {0x0da8, 0x0e08},        {0x0da9, 0x0e09},        {0x0daa, 0x0e0a},        {0x0dab, 0x0e0b},        {0x0dac, 0x0e0c},        {0x0dad, 0x0e0d},        {0x0dae, 0x0e0e},        {0x0daf, 0x0e0f},        {0x0db0, 0x0e10},        {0x0db1, 0x0e11},        {0x0db2, 0x0e12},        {0x0db3, 0x0e13},        {0x0db4, 0x0e14},        {0x0db5, 0x0e15},        {0x0db6, 0x0e16},        {0x0db7, 0x0e17},        {0x0db8, 0x0e18},        {0x0db9, 0x0e19},        {0x0dba, 0x0e1a},        {0x0dbb, 0x0e1b},        {0x0dbc, 0x0e1c},        {0x0dbd, 0x0e1d},        {0x0dbe, 0x0e1e},        {0x0dbf, 0x0e1f},        {0x0dc0, 0x0e20},        {0x0dc1, 0x0e21},        {0x0dc2, 0x0e22},        {0x0dc3, 0x0e23},        {0x0dc4, 0x0e24},        {0x0dc5, 0x0e25},        {0x0dc6, 0x0e26},        {0x0dc7, 0x0e27},        {0x0dc8, 0x0e28},        {0x0dc9, 0x0e29},        {0x0dca, 0x0e2a},        {0x0dcb, 0x0e2b},        {0x0dcc, 0x0e2c},        {0x0dcd, 0x0e2d},        {0x0dce, 0x0e2e},        {0x0dcf, 0x0e2f},        {0x0dd0, 0x0e30},        {0x0dd1, 0x0e31},        {0x0dd2, 0x0e32},        {0x0dd3, 0x0e33},        {0x0dd4, 0x0e34},        {0x0dd5, 0x0e35},        {0x0dd6, 0x0e36},        {0x0dd7, 0x0e37},        {0x0dd8, 0x0e38},        {0x0dd9, 0x0e39},        {0x0dda, 0x0e3a},        {0x0dde, 0x0000},        {0x0ddf, 0x0e3f},        {0x0de0, 0x0e40},        {0x0de1, 0x0e41},        {0x0de2, 0x0e42},        {0x0de3, 0x0e43},        {0x0de4, 0x0e44},        {0x0de5, 0x0e45},        {0x0de6, 0x0e46},        {0x0de7, 0x0e47},        {0x0de8, 0x0e48},        {0x0de9, 0x0e49},        {0x0dea, 0x0e4a},        {0x0deb, 0x0e4b},        {0x0dec, 0x0e4c},        {0x0ded, 0x0e4d},        {0x0df0, 0x0e50},        {0x0df1, 0x0e51},        {0x0df2, 0x0e52},        {0x0df3, 0x0e53},        {0x0df4, 0x0e54},        {0x0df5, 0x0e55},        {0x0df6, 0x0e56},        {0x0df7, 0x0e57},        {0x0df8, 0x0e58},        {0x0df9, 0x0e59},        {0x0ea1, 0x3131},        {0x0ea2, 0x3132},        {0x0ea3, 0x3133},        {0x0ea4, 0x3134},        {0x0ea5, 0x3135},        {0x0ea6, 0x3136},        {0x0ea7, 0x3137},        {0x0ea8, 0x3138},        {0x0ea9, 0x3139},        {0x0eaa, 0x313a},        {0x0eab, 0x313b},        {0x0eac, 0x313c},        {0x0ead, 0x313d},        {0x0eae, 0x313e},        {0x0eaf, 0x313f},        {0x0eb0, 0x3140},        {0x0eb1, 0x3141},        {0x0eb2, 0x3142},        {0x0eb3, 0x3143},        {0x0eb4, 0x3144},        {0x0eb5, 0x3145},        {0x0eb6, 0x3146},        {0x0eb7, 0x3147},        {0x0eb8, 0x3148},        {0x0eb9, 0x3149},        {0x0eba, 0x314a},        {0x0ebb, 0x314b},        {0x0ebc, 0x314c},        {0x0ebd, 0x314d},        {0x0ebe, 0x314e},        {0x0ebf, 0x314f},        {0x0ec0, 0x3150},        {0x0ec1, 0x3151},        {0x0ec2, 0x3152},        {0x0ec3, 0x3153},        {0x0ec4, 0x3154},        {0x0ec5, 0x3155},        {0x0ec6, 0x3156},        {0x0ec7, 0x3157},        {0x0ec8, 0x3158},        {0x0ec9, 0x3159},        {0x0eca, 0x315a},        {0x0ecb, 0x315b},        {0x0ecc, 0x315c},        {0x0ecd, 0x315d},        {0x0ece, 0x315e},        {0x0ecf, 0x315f},        {0x0ed0, 0x3160},        {0x0ed1, 0x3161},        {0x0ed2, 0x3162},        {0x0ed3, 0x3163},        {0x0ed4, 0x11a8},        {0x0ed5, 0x11a9},        {0x0ed6, 0x11aa},        {0x0ed7, 0x11ab},        {0x0ed8, 0x11ac},        {0x0ed9, 0x11ad},        {0x0eda, 0x11ae},        {0x0edb, 0x11af},        {0x0edc, 0x11b0},        {0x0edd, 0x11b1},        {0x0ede, 0x11b2},        {0x0edf, 0x11b3},        {0x0ee0, 0x11b4},        {0x0ee1, 0x11b5},        {0x0ee2, 0x11b6},        {0x0ee3, 0x11b7},        {0x0ee4, 0x11b8},        {0x0ee5, 0x11b9},        {0x0ee6, 0x11ba},        {0x0ee7, 0x11bb},        {0x0ee8, 0x11bc},        {0x0ee9, 0x11bd},        {0x0eea, 0x11be},        {0x0eeb, 0x11bf},        {0x0eec, 0x11c0},        {0x0eed, 0x11c1},        {0x0eee, 0x11c2},        {0x0eef, 0x316d},        {0x0ef0, 0x3171},        {0x0ef1, 0x3178},        {0x0ef2, 0x317f},        {0x0ef3, 0x3181},        {0x0ef4, 0x3184},        {0x0ef5, 0x3186},        {0x0ef6, 0x318d},        {0x0ef7, 0x318e},        {0x0ef8, 0x11eb},        {0x0ef9, 0x11f0},        {0x0efa, 0x11f9},        {0x0eff, 0x20a9},        {0x13bc, 0x0152},        {0x13bd, 0x0153},        {0x13be, 0x0178},        {0x20a0, 0x20a0},        {0x20a1, 0x20a1},        {0x20a2, 0x20a2},        {0x20a3, 0x20a3},        {0x20a4, 0x20a4},        {0x20a5, 0x20a5},        {0x20a6, 0x20a6},        {0x20a7, 0x20a7},        {0x20a8, 0x20a8},        {0x20a9, 0x20a9},        {0x20aa, 0x20aa},        {0x20ab, 0x20ab},        {0x20ac, 0x20ac},        {0xfd01, 0x0000},        {0xfd02, 0x0000},        {0xfd03, 0x0000},        {0xfd04, 0x0000},        {0xfd05, 0x0000},        {0xfd06, 0x0000},        {0xfd07, 0x0000},        {0xfd08, 0x0000},        {0xfd09, 0x0000},        {0xfd0a, 0x0000},        {0xfd0b, 0x0000},        {0xfd0c, 0x0000},        {0xfd0d, 0x0000},        {0xfd0e, 0x0000},        {0xfd0f, 0x0000},        {0xfd10, 0x0000},        {0xfd11, 0x0000},        {0xfd12, 0x0000},        {0xfd13, 0x0000},        {0xfd14, 0x0000},        {0xfd15, 0x0000},        {0xfd16, 0x0000},        {0xfd17, 0x0000},        {0xfd18, 0x0000},        {0xfd19, 0x0000},        {0xfd1a, 0x0000},        {0xfd1b, 0x0000},        {0xfd1c, 0x0000},        {0xfd1d, 0x0000},        {0xfd1e, 0x0000},        {0xfe01, 0x0000},        {0xfe02, 0x0000},        {0xfe03, 0x0000},        {0xfe04, 0x0000},        {0xfe05, 0x0000},        {0xfe06, 0x0000},        {0xfe07, 0x0000},        {0xfe08, 0x0000},        {0xfe09, 0x0000},        {0xfe0a, 0x0000},        {0xfe0b, 0x0000},        {0xfe0c, 0x0000},        {0xfe0d, 0x0000},        {0xfe0e, 0x0000},        {0xfe0f, 0x0000},        {0xfe20, 0x0000},        {0xfe21, 0x0000},        {0xfe22, 0x0000},        {0xfe23, 0x0000},        {0xfe24, 0x0000},        {0xfe25, 0x0000},        {0xfe26, 0x0000},        {0xfe27, 0x0000},        {0xfe28, 0x0000},        {0xfe29, 0x0000},        {0xfe2a, 0x0000},        {0xfe2b, 0x0000},        {0xfe2c, 0x0000},        {0xfe2d, 0x0000},        {0xfe2e, 0x0000},        {0xfe2f, 0x0000},        {0xfe30, 0x0000},        {0xfe31, 0x0000},        {0xfe32, 0x0000},        {0xfe33, 0x0000},        {0xfe34, 0x0000},        {0xfe50, 0x0300},        {0xfe51, 0x0301},        {0xfe52, 0x0302},        {0xfe53, 0x0303},        {0xfe54, 0x0304},        {0xfe55, 0x0306},        {0xfe56, 0x0307},        {0xfe57, 0x0308},        {0xfe58, 0x030a},        {0xfe59, 0x030b},        {0xfe5a, 0x030c},        {0xfe5b, 0x0327},        {0xfe5c, 0x0328},        {0xfe5d, 0x0345},        {0xfe5e, 0x3099},        {0xfe5f, 0x309a},        {0xfe70, 0x0000},        {0xfe71, 0x0000},        {0xfe72, 0x0000},        {0xfe73, 0x0000},        {0xfe74, 0x0000},        {0xfe75, 0x0000},        {0xfe76, 0x0000},        {0xfe77, 0x0000},        {0xfe78, 0x0000},        {0xfe79, 0x0000},        {0xfe7a, 0x0000},        {0xfed0, 0x0000},        {0xfed1, 0x0000},        {0xfed2, 0x0000},        {0xfed4, 0x0000},        {0xfed5, 0x0000},        {0xfee0, 0x0000},        {0xfee1, 0x0000},        {0xfee2, 0x0000},        {0xfee3, 0x0000},        {0xfee4, 0x0000},        {0xfee5, 0x0000},        {0xfee6, 0x0000},        {0xfee7, 0x0000},        {0xfee8, 0x0000},        {0xfee9, 0x0000},        {0xfeea, 0x0000},        {0xfeeb, 0x0000},        {0xfeec, 0x0000},        {0xfeed, 0x0000},        {0xfeee, 0x0000},        {0xfeef, 0x0000},        {0xfef0, 0x0000},        {0xfef1, 0x0000},        {0xfef2, 0x0000},        {0xfef3, 0x0000},        {0xfef4, 0x0000},        {0xfef5, 0x0000},        {0xfef6, 0x0000},        {0xfef7, 0x0000},        {0xfef8, 0x0000},        {0xfef9, 0x0000},        {0xfefa, 0x0000},        {0xfefb, 0x0000},        {0xfefc, 0x0000},        {0xfefd, 0x0000},        {0xff08, 0x0008},        {0xff09, 0x0009},        {0xff0a, 0x000a},        {0xff0b, 0x000b},        {0xff0d, 0x000d},        {0xff13, 0x0013},        {0xff14, 0x0014},        {0xff15, 0x0015},        {0xff1b, 0x001b},        {0xff20, 0x0000},        {0xff21, 0x0000},        {0xff22, 0x0000},        {0xff23, 0x0000},        {0xff24, 0x0000},        {0xff25, 0x0000},        {0xff26, 0x0000},        {0xff27, 0x0000},        {0xff28, 0x0000},        {0xff29, 0x0000},        {0xff2a, 0x0000},        {0xff2b, 0x0000},        {0xff2c, 0x0000},        {0xff2d, 0x0000},        {0xff2e, 0x0000},        {0xff2f, 0x0000},        {0xff30, 0x0000},        {0xff31, 0x0000},        {0xff32, 0x0000},        {0xff33, 0x0000},        {0xff34, 0x0000},        {0xff35, 0x0000},        {0xff36, 0x0000},        {0xff37, 0x0000},        {0xff38, 0x0000},        {0xff39, 0x0000},        {0xff3a, 0x0000},        {0xff3b, 0x0000},        {0xff3c, 0x0000},        {0xff3d, 0x0000},        {0xff3e, 0x0000},        {0xff3f, 0x0000},        {0xff50, 0x0000},        {0xff51, 0x0000},        {0xff52, 0x0000},        {0xff53, 0x0000},        {0xff54, 0x0000},        {0xff55, 0x0000},        {0xff56, 0x0000},        {0xff57, 0x0000},        {0xff58, 0x0000},        {0xff60, 0x0000},        {0xff61, 0x0000},        {0xff62, 0x0000},        {0xff63, 0x0000},        {0xff65, 0x0000},        {0xff66, 0x0000},        {0xff67, 0x0000},        {0xff68, 0x0000},        {0xff69, 0x0000},        {0xff6a, 0x0000},        {0xff6b, 0x0000},        {0xff7e, 0x0000},        {0xff7f, 0x0000},        {0xff80, 0x0020},        {0xff89, 0x0009},        {0xff8d, 0x000d},        {0xff91, 0x0000},        {0xff92, 0x0000},        {0xff93, 0x0000},        {0xff94, 0x0000},        {0xff95, 0x0000},        {0xff96, 0x0000},        {0xff97, 0x0000},        {0xff98, 0x0000},        {0xff99, 0x0000},        {0xff9a, 0x0000},        {0xff9b, 0x0000},        {0xff9c, 0x0000},        {0xff9d, 0x0000},        {0xff9e, 0x0000},        {0xff9f, 0x0000},        {0xffaa, 0x002a},        {0xffab, 0x002b},        {0xffac, 0x002c},        {0xffad, 0x002d},        {0xffae, 0x002e},        {0xffaf, 0x002f},        {0xffb0, 0x0030},        {0xffb1, 0x0031},        {0xffb2, 0x0032},        {0xffb3, 0x0033},        {0xffb4, 0x0034},        {0xffb5, 0x0035},        {0xffb6, 0x0036},        {0xffb7, 0x0037},        {0xffb8, 0x0038},        {0xffb9, 0x0039},        {0xffbd, 0x003d},        {0xffbe, 0x0000},        {0xffbf, 0x0000},        {0xffc0, 0x0000},        {0xffc1, 0x0000},        {0xffc2, 0x0000},        {0xffc3, 0x0000},        {0xffc4, 0x0000},        {0xffc5, 0x0000},        {0xffc6, 0x0000},        {0xffc7, 0x0000},        {0xffc8, 0x0000},        {0xffc9, 0x0000},        {0xffca, 0x0000},        {0xffcb, 0x0000},        {0xffcc, 0x0000},        {0xffcd, 0x0000},        {0xffce, 0x0000},        {0xffcf, 0x0000},        {0xffd0, 0x0000},        {0xffd1, 0x0000},        {0xffd2, 0x0000},        {0xffd3, 0x0000},        {0xffd4, 0x0000},        {0xffd5, 0x0000},        {0xffd6, 0x0000},        {0xffd7, 0x0000},        {0xffd8, 0x0000},        {0xffd9, 0x0000},        {0xffda, 0x0000},        {0xffdb, 0x0000},        {0xffdc, 0x0000},        {0xffdd, 0x0000},        {0xffde, 0x0000},        {0xffdf, 0x0000},        {0xffe0, 0x0000},        {0xffe1, 0x0000},        {0xffe2, 0x0000},        {0xffe3, 0x0000},        {0xffe4, 0x0000},        {0xffe5, 0x0000},        {0xffe6, 0x0000},        {0xffe7, 0x0000},        {0xffe8, 0x0000},        {0xffe9, 0x0000},        {0xffea, 0x0000},        {0xffeb, 0x0000},        {0xffec, 0x0000},        {0xffed, 0x0000},        {0xffee, 0x0000},        {0xffff, 0x0000},        {0xffffff, 0x0000},    
        
            
    {0x06ad, 0x0491},        {0x06bd, 0x0490},    
                
                {0x14a1, 0x0000},        {0x14a2, 0x0587},        {0x14a3, 0x0589},        {0x14a4, 0x0029},        {0x14a5, 0x0028},        {0x14a6, 0x00bb},        {0x14a7, 0x00ab},        {0x14a8, 0x2014},        {0x14a9, 0x002e},        {0x14aa, 0x055d},        {0x14ab, 0x002c},        {0x14ac, 0x2013},        {0x14ad, 0x058a},        {0x14ae, 0x2026},        {0x14af, 0x055c},        {0x14b0, 0x055b},        {0x14b1, 0x055e},        {0x14b2, 0x0531},        {0x14b3, 0x0561},        {0x14b4, 0x0532},        {0x14b5, 0x0562},        {0x14b6, 0x0533},        {0x14b7, 0x0563},        {0x14b8, 0x0534},        {0x14b9, 0x0564},        {0x14ba, 0x0535},        {0x14bb, 0x0565},        {0x14bc, 0x0536},        {0x14bd, 0x0566},        {0x14be, 0x0537},        {0x14bf, 0x0567},        {0x14c0, 0x0538},        {0x14c1, 0x0568},        {0x14c2, 0x0539},        {0x14c3, 0x0569},        {0x14c4, 0x053a},        {0x14c5, 0x056a},        {0x14c6, 0x053b},        {0x14c7, 0x056b},        {0x14c8, 0x053c},        {0x14c9, 0x056c},        {0x14ca, 0x053d},        {0x14cb, 0x056d},        {0x14cc, 0x053e},        {0x14cd, 0x056e},        {0x14ce, 0x053f},        {0x14cf, 0x056f},        {0x14d0, 0x0540},        {0x14d1, 0x0570},        {0x14d2, 0x0541},        {0x14d3, 0x0571},        {0x14d4, 0x0542},        {0x14d5, 0x0572},        {0x14d6, 0x0543},        {0x14d7, 0x0573},        {0x14d8, 0x0544},        {0x14d9, 0x0574},        {0x14da, 0x0545},        {0x14db, 0x0575},        {0x14dc, 0x0546},        {0x14dd, 0x0576},        {0x14de, 0x0547},        {0x14df, 0x0577},        {0x14e0, 0x0548},        {0x14e1, 0x0578},        {0x14e2, 0x0549},        {0x14e3, 0x0579},        {0x14e4, 0x054a},        {0x14e5, 0x057a},        {0x14e6, 0x054b},        {0x14e7, 0x057b},        {0x14e8, 0x054c},        {0x14e9, 0x057c},        {0x14ea, 0x054d},        {0x14eb, 0x057d},        {0x14ec, 0x054e},        {0x14ed, 0x057e},        {0x14ee, 0x054f},        {0x14ef, 0x057f},        {0x14f0, 0x0550},        {0x14f1, 0x0580},        {0x14f2, 0x0551},        {0x14f3, 0x0581},        {0x14f4, 0x0552},        {0x14f5, 0x0582},        {0x14f6, 0x0553},        {0x14f7, 0x0583},        {0x14f8, 0x0554},        {0x14f9, 0x0584},        {0x14fa, 0x0555},        {0x14fb, 0x0585},        {0x14fc, 0x0556},        {0x14fd, 0x0586},        {0x14fe, 0x055a},        {0x14ff, 0x00a7},    
                {0x15d0, 0x10d0},        {0x15d1, 0x10d1},        {0x15d2, 0x10d2},        {0x15d3, 0x10d3},        {0x15d4, 0x10d4},        {0x15d5, 0x10d5},        {0x15d6, 0x10d6},        {0x15d7, 0x10d7},        {0x15d8, 0x10d8},        {0x15d9, 0x10d9},        {0x15da, 0x10da},        {0x15db, 0x10db},        {0x15dc, 0x10dc},        {0x15dd, 0x10dd},        {0x15de, 0x10de},        {0x15df, 0x10df},        {0x15e0, 0x10e0},        {0x15e1, 0x10e1},        {0x15e2, 0x10e2},        {0x15e3, 0x10e3},        {0x15e4, 0x10e4},        {0x15e5, 0x10e5},        {0x15e6, 0x10e6},        {0x15e7, 0x10e7},        {0x15e8, 0x10e8},        {0x15e9, 0x10e9},        {0x15ea, 0x10ea},        {0x15eb, 0x10eb},        {0x15ec, 0x10ec},        {0x15ed, 0x10ed},        {0x15ee, 0x10ee},        {0x15ef, 0x10ef},        {0x15f0, 0x10f0},        {0x15f1, 0x10f1},        {0x15f2, 0x10f2},        {0x15f3, 0x10f3},        {0x15f4, 0x10f4},        {0x15f5, 0x10f5},        {0x15f6, 0x10f6},    
            
            {0x12a1, 0x1e02},        {0x12a2, 0x1e03},        {0x12a6, 0x1e0a},        {0x12a8, 0x1e80},        {0x12aa, 0x1e82},        {0x12ab, 0x1e0b},        {0x12ac, 0x1ef2},        {0x12b0, 0x1e1e},        {0x12b1, 0x1e1f},        {0x12b4, 0x1e40},        {0x12b5, 0x1e41},        {0x12b7, 0x1e56},        {0x12b8, 0x1e81},        {0x12b9, 0x1e57},        {0x12ba, 0x1e83},        {0x12bb, 0x1e60},        {0x12bc, 0x1ef3},        {0x12bd, 0x1e84},        {0x12be, 0x1e85},        {0x12bf, 0x1e61},        {0x12d0, 0x0174},        {0x12d7, 0x1e6a},        {0x12de, 0x0176},        {0x12f0, 0x0175},        {0x12f7, 0x1e6b},        {0x12fe, 0x0177},    
                {0x0590, 0x06f0},        {0x0591, 0x06f1},        {0x0592, 0x06f2},        {0x0593, 0x06f3},        {0x0594, 0x06f4},        {0x0595, 0x06f5},        {0x0596, 0x06f6},        {0x0597, 0x06f7},        {0x0598, 0x06f8},        {0x0599, 0x06f9},        {0x05a5, 0x066a},        {0x05a6, 0x0670},        {0x05a7, 0x0679},        {0x05a8, 0x067e},        {0x05a9, 0x0686},        {0x05aa, 0x0688},        {0x05ab, 0x0691},        {0x05ae, 0x06d4},        {0x05b0, 0x0660},        {0x05b1, 0x0661},        {0x05b2, 0x0662},        {0x05b3, 0x0663},        {0x05b4, 0x0664},        {0x05b5, 0x0665},        {0x05b6, 0x0666},        {0x05b7, 0x0667},        {0x05b8, 0x0668},        {0x05b9, 0x0669},        {0x05f3, 0x0653},        {0x05f4, 0x0654},        {0x05f5, 0x0655},        {0x05f6, 0x0698},        {0x05f7, 0x06a4},        {0x05f8, 0x06a9},        {0x05f9, 0x06af},        {0x05fa, 0x06ba},        {0x05fb, 0x06be},        {0x05fc, 0x06cc},        {0x05fd, 0x06d2},        {0x05fe, 0x06c1},    
            {0x0680, 0x0492},        {0x0681, 0x0496},        {0x0682, 0x049a},        {0x0683, 0x049c},        {0x0684, 0x04a2},        {0x0685, 0x04ae},        {0x0686, 0x04b0},        {0x0687, 0x04b2},        {0x0688, 0x04b6},        {0x0689, 0x04b8},        {0x068a, 0x04ba},        {0x068c, 0x04d8},        {0x068d, 0x04e2},        {0x068e, 0x04e8},        {0x068f, 0x04ee},        {0x0690, 0x0493},        {0x0691, 0x0497},        {0x0692, 0x049b},        {0x0693, 0x049d},        {0x0694, 0x04a3},        {0x0695, 0x04af},        {0x0696, 0x04b1},        {0x0697, 0x04b3},        {0x0698, 0x04b7},        {0x0699, 0x04b9},        {0x069a, 0x04bb},        {0x069c, 0x04d9},        {0x069d, 0x04e3},        {0x069e, 0x04e9},        {0x069f, 0x04ef},    
                    {0x16a2, 0x0000},        {0x16a3, 0x1e8a},        {0x16a5, 0x0000},        {0x16a6, 0x012c},        {0x16a7, 0x0000},        {0x16a8, 0x0000},        {0x16a9, 0x01b5},        {0x16aa, 0x01e6},        {0x16af, 0x019f},        {0x16b2, 0x0000},        {0x16b3, 0x1e8b},        {0x16b4, 0x0000},        {0x16b5, 0x0000},        {0x16b6, 0x012d},        {0x16b7, 0x0000},        {0x16b8, 0x0000},        {0x16b9, 0x01b6},        {0x16ba, 0x01e7},        {0x16bd, 0x01d2},        {0x16bf, 0x0275},        {0x16c6, 0x018f},        {0x16f6, 0x0259},    
                {0x16d1, 0x1e36},        {0x16d2, 0x0000},        {0x16d3, 0x0000},        {0x16e1, 0x1e37},        {0x16e2, 0x0000},        {0x16e3, 0x0000},    
                {0x1ea0, 0x1ea0},        {0x1ea1, 0x1ea1},        {0x1ea2, 0x1ea2},        {0x1ea3, 0x1ea3},        {0x1ea4, 0x1ea4},        {0x1ea5, 0x1ea5},        {0x1ea6, 0x1ea6},        {0x1ea7, 0x1ea7},        {0x1ea8, 0x1ea8},        {0x1ea9, 0x1ea9},        {0x1eaa, 0x1eaa},        {0x1eab, 0x1eab},        {0x1eac, 0x1eac},        {0x1ead, 0x1ead},        {0x1eae, 0x1eae},        {0x1eaf, 0x1eaf},        {0x1eb0, 0x1eb0},        {0x1eb1, 0x1eb1},        {0x1eb2, 0x1eb2},        {0x1eb3, 0x1eb3},        {0x1eb4, 0x1eb4},        {0x1eb5, 0x1eb5},        {0x1eb6, 0x1eb6},        {0x1eb7, 0x1eb7},        {0x1eb8, 0x1eb8},        {0x1eb9, 0x1eb9},        {0x1eba, 0x1eba},        {0x1ebb, 0x1ebb},        {0x1ebc, 0x1ebc},        {0x1ebd, 0x1ebd},        {0x1ebe, 0x1ebe},        {0x1ebf, 0x1ebf},        {0x1ec0, 0x1ec0},        {0x1ec1, 0x1ec1},        {0x1ec2, 0x1ec2},        {0x1ec3, 0x1ec3},        {0x1ec4, 0x1ec4},        {0x1ec5, 0x1ec5},        {0x1ec6, 0x1ec6},        {0x1ec7, 0x1ec7},        {0x1ec8, 0x1ec8},        {0x1ec9, 0x1ec9},        {0x1eca, 0x1eca},        {0x1ecb, 0x1ecb},        {0x1ecc, 0x1ecc},        {0x1ecd, 0x1ecd},        {0x1ece, 0x1ece},        {0x1ecf, 0x1ecf},        {0x1ed0, 0x1ed0},        {0x1ed1, 0x1ed1},        {0x1ed2, 0x1ed2},        {0x1ed3, 0x1ed3},        {0x1ed4, 0x1ed4},        {0x1ed5, 0x1ed5},        {0x1ed6, 0x1ed6},        {0x1ed7, 0x1ed7},        {0x1ed8, 0x1ed8},        {0x1ed9, 0x1ed9},        {0x1eda, 0x1eda},        {0x1edb, 0x1edb},        {0x1edc, 0x1edc},        {0x1edd, 0x1edd},        {0x1ede, 0x1ede},        {0x1edf, 0x1edf},        {0x1ee0, 0x1ee0},        {0x1ee1, 0x1ee1},        {0x1ee2, 0x1ee2},        {0x1ee3, 0x1ee3},        {0x1ee4, 0x1ee4},        {0x1ee5, 0x1ee5},        {0x1ee6, 0x1ee6},        {0x1ee7, 0x1ee7},        {0x1ee8, 0x1ee8},        {0x1ee9, 0x1ee9},        {0x1eea, 0x1eea},        {0x1eeb, 0x1eeb},        {0x1eec, 0x1eec},        {0x1eed, 0x1eed},        {0x1eee, 0x1eee},        {0x1eef, 0x1eef},        {0x1ef0, 0x1ef0},        {0x1ef1, 0x1ef1},        {0x1ef4, 0x1ef4},        {0x1ef5, 0x1ef5},        {0x1ef6, 0x1ef6},        {0x1ef7, 0x1ef7},        {0x1ef8, 0x1ef8},        {0x1ef9, 0x1ef9},    
    {0x1efa, 0x01a0},        {0x1efb, 0x01a1},        {0x1efc, 0x01af},        {0x1efd, 0x01b0},    
            {0x1e9f, 0x0303},        {0x1ef2, 0x0300},        {0x1ef3, 0x0301},        {0x1efe, 0x0309},        {0x1eff, 0x0323},    
            {0xfe60, 0x0323},        {0xfe61, 0x0309},        {0xfe62, 0x031b},    };
int wxUnicodeCharXToWX(WXKeySym keySym)
{
    int id = wxCharCodeXToWX (keySym);

    
    if ( id != WXK_NONE )
        return id;

    int min = 0;
    int max = sizeof(keySymTab) / sizeof(CodePair) - 1;
    int mid;

    
    if ( (keySym & 0xff000000) == 0x01000000 )
        return keySym & 0x00ffffff;

    
    while ( max >= min ) {
        mid = (min + max) / 2;
        if ( keySymTab[mid].keySym < keySym )
            min = mid + 1;
        else if ( keySymTab[mid].keySym > keySym )
            max = mid - 1;
        else {
            
            return keySymTab[mid].uniChar;
        }
    }

        return WXK_NONE;
}


bool wxGetKeyState(wxKeyCode key)
{
    wxASSERT_MSG(key != WXK_LBUTTON && key != WXK_RBUTTON && key !=
        WXK_MBUTTON, wxT("can't use wxGetKeyState() for mouse buttons"));

    Display *pDisplay = (Display*) wxGetDisplay();

    int iKey = wxCharCodeWXToX(key);
    int          iKeyMask = 0;
    Window       wDummy1, wDummy2;
    int          iDummy3, iDummy4, iDummy5, iDummy6;
    unsigned int iMask;
    KeyCode keyCode = XKeysymToKeycode(pDisplay,iKey);
    if (keyCode == NoSymbol)
        return false;

    if ( IsModifierKey(iKey) )      {
        XModifierKeymap *map = XGetModifierMapping(pDisplay);
        wxCHECK_MSG( map, false, wxT("failed to get X11 modifiers map") );

        for (int i = 0; i < 8; ++i)
        {
            if ( map->modifiermap[map->max_keypermod * i] == keyCode)
            {
                iKeyMask = 1 << i;
            }
        }

        XQueryPointer(pDisplay, DefaultRootWindow(pDisplay), &wDummy1, &wDummy2,
                        &iDummy3, &iDummy4, &iDummy5, &iDummy6, &iMask );
        XFreeModifiermap(map);
        return (iMask & iKeyMask) != 0;
    }

                        char key_vector[32];
    XQueryKeymap(pDisplay, key_vector);
    return key_vector[keyCode >> 3] & (1 << (keyCode & 7));
}

#endif 

bool wxLaunchDefaultApplication(const wxString& document, int flags)
{
    wxUnusedVar(flags);

                        wxString path, xdg_open;
    if ( wxGetEnv("PATH", &path) &&
         wxFindFileInPath(&xdg_open, path, "xdg-open") )
    {
        if ( wxExecute(xdg_open + " " + document) )
            return true;
    }

    return false;
}


bool
wxDoLaunchDefaultBrowser(const wxLaunchBrowserParams& params)
{
#ifdef __WXGTK__
#if GTK_CHECK_VERSION(2,14,0)
#ifndef __WXGTK3__
    if (gtk_check_version(2,14,0) == NULL)
#endif
    {
        GdkScreen* screen = gdk_window_get_screen(wxGetTopLevelGDK());
        if (gtk_show_uri(screen, params.url.utf8_str(), GDK_CURRENT_TIME, NULL))
            return true;
    }
#endif #endif 
                            wxString path, xdg_open;
    if ( wxGetEnv("PATH", &path) &&
         wxFindFileInPath(&xdg_open, path, "xdg-open") )
    {
        if ( wxExecute(xdg_open + " " + params.GetPathOrURL()) )
            return true;
    }

    wxString desktop = wxTheApp->GetTraits()->GetDesktopEnvironment();

            if (desktop == wxT("GNOME"))
    {
        wxArrayString errors;
        wxArrayString output;

                long res = wxExecute( wxT("gconftool-2 --get /desktop/gnome/applications/browser/exec"),
                              output, errors, wxEXEC_NODISABLE );
        if (res >= 0 && errors.GetCount() == 0)
        {
            wxString cmd = output[0];
            cmd << wxT(' ') << params.GetPathOrURL();
            if (wxExecute(cmd))
                return true;
        }
    }
    else if (desktop == wxT("KDE"))
    {
                if (wxExecute(wxT("kfmclient openURL ") + params.GetPathOrURL()))
            return true;
    }

    return false;
}

#endif 