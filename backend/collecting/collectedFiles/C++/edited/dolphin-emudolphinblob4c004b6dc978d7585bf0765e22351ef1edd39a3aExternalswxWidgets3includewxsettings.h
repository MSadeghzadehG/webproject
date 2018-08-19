
#ifndef _WX_SETTINGS_H_BASE_
#define _WX_SETTINGS_H_BASE_

#include "wx/colour.h"
#include "wx/font.h"

class WXDLLIMPEXP_FWD_CORE wxWindow;

enum wxSystemFont
{
    wxSYS_OEM_FIXED_FONT = 10,
    wxSYS_ANSI_FIXED_FONT,
    wxSYS_ANSI_VAR_FONT,
    wxSYS_SYSTEM_FONT,
    wxSYS_DEVICE_DEFAULT_FONT,

            wxSYS_DEFAULT_PALETTE,

            wxSYS_SYSTEM_FIXED_FONT,

    wxSYS_DEFAULT_GUI_FONT,

        wxSYS_ICONTITLE_FONT = wxSYS_DEFAULT_GUI_FONT
};

enum wxSystemColour
{
    wxSYS_COLOUR_SCROLLBAR,
    wxSYS_COLOUR_DESKTOP,
    wxSYS_COLOUR_ACTIVECAPTION,
    wxSYS_COLOUR_INACTIVECAPTION,
    wxSYS_COLOUR_MENU,
    wxSYS_COLOUR_WINDOW,
    wxSYS_COLOUR_WINDOWFRAME,
    wxSYS_COLOUR_MENUTEXT,
    wxSYS_COLOUR_WINDOWTEXT,
    wxSYS_COLOUR_CAPTIONTEXT,
    wxSYS_COLOUR_ACTIVEBORDER,
    wxSYS_COLOUR_INACTIVEBORDER,
    wxSYS_COLOUR_APPWORKSPACE,
    wxSYS_COLOUR_HIGHLIGHT,
    wxSYS_COLOUR_HIGHLIGHTTEXT,
    wxSYS_COLOUR_BTNFACE,
    wxSYS_COLOUR_BTNSHADOW,
    wxSYS_COLOUR_GRAYTEXT,
    wxSYS_COLOUR_BTNTEXT,
    wxSYS_COLOUR_INACTIVECAPTIONTEXT,
    wxSYS_COLOUR_BTNHIGHLIGHT,
    wxSYS_COLOUR_3DDKSHADOW,
    wxSYS_COLOUR_3DLIGHT,
    wxSYS_COLOUR_INFOTEXT,
    wxSYS_COLOUR_INFOBK,
    wxSYS_COLOUR_LISTBOX,
    wxSYS_COLOUR_HOTLIGHT,
    wxSYS_COLOUR_GRADIENTACTIVECAPTION,
    wxSYS_COLOUR_GRADIENTINACTIVECAPTION,
    wxSYS_COLOUR_MENUHILIGHT,
    wxSYS_COLOUR_MENUBAR,
    wxSYS_COLOUR_LISTBOXTEXT,
    wxSYS_COLOUR_LISTBOXHIGHLIGHTTEXT,

    wxSYS_COLOUR_MAX,

        wxSYS_COLOUR_BACKGROUND = wxSYS_COLOUR_DESKTOP,
    wxSYS_COLOUR_3DFACE = wxSYS_COLOUR_BTNFACE,
    wxSYS_COLOUR_3DSHADOW = wxSYS_COLOUR_BTNSHADOW,
    wxSYS_COLOUR_BTNHILIGHT = wxSYS_COLOUR_BTNHIGHLIGHT,
    wxSYS_COLOUR_3DHIGHLIGHT = wxSYS_COLOUR_BTNHIGHLIGHT,
    wxSYS_COLOUR_3DHILIGHT = wxSYS_COLOUR_BTNHIGHLIGHT,
    wxSYS_COLOUR_FRAMEBK = wxSYS_COLOUR_BTNFACE
};

enum wxSystemMetric
{
    wxSYS_MOUSE_BUTTONS = 1,
    wxSYS_BORDER_X,
    wxSYS_BORDER_Y,
    wxSYS_CURSOR_X,
    wxSYS_CURSOR_Y,
    wxSYS_DCLICK_X,
    wxSYS_DCLICK_Y,
    wxSYS_DRAG_X,
    wxSYS_DRAG_Y,
    wxSYS_EDGE_X,
    wxSYS_EDGE_Y,
    wxSYS_HSCROLL_ARROW_X,
    wxSYS_HSCROLL_ARROW_Y,
    wxSYS_HTHUMB_X,
    wxSYS_ICON_X,
    wxSYS_ICON_Y,
    wxSYS_ICONSPACING_X,
    wxSYS_ICONSPACING_Y,
    wxSYS_WINDOWMIN_X,
    wxSYS_WINDOWMIN_Y,
    wxSYS_SCREEN_X,
    wxSYS_SCREEN_Y,
    wxSYS_FRAMESIZE_X,
    wxSYS_FRAMESIZE_Y,
    wxSYS_SMALLICON_X,
    wxSYS_SMALLICON_Y,
    wxSYS_HSCROLL_Y,
    wxSYS_VSCROLL_X,
    wxSYS_VSCROLL_ARROW_X,
    wxSYS_VSCROLL_ARROW_Y,
    wxSYS_VTHUMB_Y,
    wxSYS_CAPTION_Y,
    wxSYS_MENU_Y,
    wxSYS_NETWORK_PRESENT,
    wxSYS_PENWINDOWS_PRESENT,
    wxSYS_SHOW_SOUNDS,
    wxSYS_SWAP_BUTTONS,
    wxSYS_DCLICK_MSEC
};

enum wxSystemFeature
{
    wxSYS_CAN_DRAW_FRAME_DECORATIONS = 1,
    wxSYS_CAN_ICONIZE_FRAME,
    wxSYS_TABLET_PRESENT
};

enum wxSystemScreenType
{
    wxSYS_SCREEN_NONE = 0,  
    wxSYS_SCREEN_TINY,          wxSYS_SCREEN_PDA,           wxSYS_SCREEN_SMALL,         wxSYS_SCREEN_DESKTOP    };



class WXDLLIMPEXP_CORE wxSystemSettingsNative
{
public:
        static wxColour GetColour(wxSystemColour index);

        static wxFont GetFont(wxSystemFont index);

        static int GetMetric(wxSystemMetric index, wxWindow * win = NULL);

        static bool HasFeature(wxSystemFeature index);
};


class WXDLLIMPEXP_CORE wxSystemSettings : public wxSystemSettingsNative
{
public:
#ifdef __WXUNIVERSAL__
                static wxColour GetColour(wxSystemColour index);

            static int GetMetric(wxSystemMetric index, wxWindow *win = NULL);
#endif 
            static wxSystemScreenType GetScreenType();

        static void SetScreenType( wxSystemScreenType screen );

        static wxSystemScreenType ms_screen;

};

#endif
    
