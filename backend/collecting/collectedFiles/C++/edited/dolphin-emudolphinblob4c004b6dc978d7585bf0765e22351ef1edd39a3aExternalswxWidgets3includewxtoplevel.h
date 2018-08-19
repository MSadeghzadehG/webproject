
#ifndef _WX_TOPLEVEL_BASE_H_
#define _WX_TOPLEVEL_BASE_H_


#include "wx/nonownedwnd.h"
#include "wx/iconbndl.h"
#include "wx/weakref.h"

extern WXDLLIMPEXP_DATA_CORE(const char) wxFrameNameStr[];

class WXDLLIMPEXP_FWD_CORE wxTopLevelWindowBase;




#define wxSTAY_ON_TOP           0x8000
#define wxICONIZE               0x4000
#define wxMINIMIZE              wxICONIZE
#define wxMAXIMIZE              0x2000
#define wxCLOSE_BOX             0x1000  
#define wxSYSTEM_MENU           0x0800
#define wxMINIMIZE_BOX          0x0400
#define wxMAXIMIZE_BOX          0x0200

#define wxTINY_CAPTION          0x0080  #define wxRESIZE_BORDER         0x0040  
#if WXWIN_COMPATIBILITY_2_8
            #define wxTINY_CAPTION_HORIZ    wxTINY_CAPTION
    #define wxTINY_CAPTION_VERT     wxTINY_CAPTION
#endif

#define wxDEFAULT_FRAME_STYLE \
            (wxSYSTEM_MENU | \
             wxRESIZE_BORDER | \
             wxMINIMIZE_BOX | \
             wxMAXIMIZE_BOX | \
             wxCLOSE_BOX | \
             wxCAPTION | \
             wxCLIP_CHILDREN)


#define wxTOPLEVEL_EX_DIALOG        0x00000008

enum
{
    wxFULLSCREEN_NOMENUBAR   = 0x0001,
    wxFULLSCREEN_NOTOOLBAR   = 0x0002,
    wxFULLSCREEN_NOSTATUSBAR = 0x0004,
    wxFULLSCREEN_NOBORDER    = 0x0008,
    wxFULLSCREEN_NOCAPTION   = 0x0010,

    wxFULLSCREEN_ALL         = wxFULLSCREEN_NOMENUBAR | wxFULLSCREEN_NOTOOLBAR |
                               wxFULLSCREEN_NOSTATUSBAR | wxFULLSCREEN_NOBORDER |
                               wxFULLSCREEN_NOCAPTION
};

enum
{
    wxUSER_ATTENTION_INFO = 1,
    wxUSER_ATTENTION_ERROR = 2
};


class WXDLLIMPEXP_CORE wxTopLevelWindowBase : public wxNonOwnedWindow
{
public:
        wxTopLevelWindowBase();
    virtual ~wxTopLevelWindowBase();

        
        virtual void Maximize(bool maximize = true) = 0;

        virtual void Restore() = 0;

        virtual void Iconize(bool iconize = true) = 0;

        virtual bool IsMaximized() const = 0;

            virtual bool IsAlwaysMaximized() const;

        virtual bool IsIconized() const = 0;

        wxIcon GetIcon() const;

        const wxIconBundle& GetIcons() const { return m_icons; }

        void SetIcon(const wxIcon& icon);

        virtual void SetIcons(const wxIconBundle& icons) { m_icons = icons; }

    virtual bool EnableFullScreenView(bool WXUNUSED(enable) = true)
    {
        return false;
    }

        virtual bool ShowFullScreen(bool show, long style = wxFULLSCREEN_ALL) = 0;

            virtual void ShowWithoutActivating() {
        wxFAIL_MSG("ShowWithoutActivating not implemented on this platform.");
    }

        virtual bool IsFullScreen() const = 0;

            virtual void SetTitle(const wxString& title) = 0;
    virtual wxString GetTitle() const = 0;

        virtual bool EnableCloseButton(bool WXUNUSED(enable) = true) { return false; }
    virtual bool EnableMaximizeButton(bool WXUNUSED(enable) = true) { return false; }
    virtual bool EnableMinimizeButton(bool WXUNUSED(enable) = true) { return false; }

            virtual void RequestUserAttention(int flags = wxUSER_ATTENTION_INFO);

                virtual bool IsActive() { return IsDescendant(FindFocus()); }

                        virtual bool ShouldPreventAppExit() const { return true; }

        void CentreOnScreen(int dir = wxBOTH) { DoCentre(dir | wxCENTRE_ON_SCREEN); }
    void CenterOnScreen(int dir = wxBOTH) { CentreOnScreen(dir); }

            static wxSize GetDefaultSize();


                    
        wxWindow *GetDefaultItem() const
        { return m_winTmpDefault ? m_winTmpDefault : m_winDefault; }

        wxWindow *SetDefaultItem(wxWindow *win)
        { wxWindow *old = GetDefaultItem(); m_winDefault = win; return old; }

        wxWindow *GetTmpDefaultItem() const { return m_winTmpDefault; }

            wxWindow *SetTmpDefaultItem(wxWindow *win)
        { wxWindow *old = GetDefaultItem(); m_winTmpDefault = win; return old; }

        
        virtual bool Destroy() wxOVERRIDE;
    virtual bool IsTopLevel() const wxOVERRIDE { return true; }
    virtual bool IsTopNavigationDomain(NavigationKind kind) const wxOVERRIDE;
    virtual bool IsVisible() const { return IsShown(); }

        void OnCloseWindow(wxCloseEvent& event);
    void OnSize(wxSizeEvent& WXUNUSED(event)) { DoLayout(); }

        virtual void GetRectForTopLevelChildren(int *x, int *y, int *w, int *h);

            void OnActivate(wxActivateEvent &WXUNUSED(event)) { }

        virtual void DoUpdateWindowUI(wxUpdateUIEvent& event) wxOVERRIDE ;

        virtual void SetMinSize(const wxSize& minSize) wxOVERRIDE;
    virtual void SetMaxSize(const wxSize& maxSize) wxOVERRIDE;

    virtual void OSXSetModified(bool modified) { m_modified = modified; }
    virtual bool OSXIsModified() const { return m_modified; }

    virtual void SetRepresentedFilename(const wxString& WXUNUSED(filename)) { }

protected:
            virtual void DoClientToScreen(int *x, int *y) const wxOVERRIDE;
    virtual void DoScreenToClient(int *x, int *y) const wxOVERRIDE;

        virtual void DoCentre(int dir) wxOVERRIDE;

            virtual void DoGetScreenPosition(int *x, int *y) const wxOVERRIDE
    {
        DoGetPosition(x, y);
    }

            virtual bool IsOneOfBars(const wxWindow *WXUNUSED(win)) const
        { return false; }

        bool IsLastBeforeExit() const;

        bool SendIconizeEvent(bool iconized = true);

            void DoLayout();

    static int WidthDefault(int w) { return w == wxDefaultCoord ? GetDefaultSize().x : w; }
    static int HeightDefault(int h) { return h == wxDefaultCoord ? GetDefaultSize().y : h; }


        wxIconBundle m_icons;

        wxWindowRef m_winDefault;

        wxWindowRef m_winTmpDefault;

    bool m_modified;

    wxDECLARE_NO_COPY_CLASS(wxTopLevelWindowBase);
    wxDECLARE_EVENT_TABLE();
};


#if defined(__WXMSW__)
    #include "wx/msw/toplevel.h"
    #define wxTopLevelWindowNative wxTopLevelWindowMSW
#elif defined(__WXGTK20__)
    #include "wx/gtk/toplevel.h"
    #define wxTopLevelWindowNative wxTopLevelWindowGTK
#elif defined(__WXGTK__)
    #include "wx/gtk1/toplevel.h"
    #define wxTopLevelWindowNative wxTopLevelWindowGTK
#elif defined(__WXX11__)
    #include "wx/x11/toplevel.h"
    #define wxTopLevelWindowNative wxTopLevelWindowX11
#elif defined(__WXDFB__)
    #include "wx/dfb/toplevel.h"
    #define wxTopLevelWindowNative wxTopLevelWindowDFB
#elif defined(__WXMAC__)
    #include "wx/osx/toplevel.h"
    #define wxTopLevelWindowNative wxTopLevelWindowMac
#elif defined(__WXMOTIF__)
    #include "wx/motif/toplevel.h"
    #define wxTopLevelWindowNative wxTopLevelWindowMotif
#elif defined(__WXQT__)
    #include "wx/qt/toplevel.h"
#define wxTopLevelWindowNative wxTopLevelWindowQt
#endif

#ifdef __WXUNIVERSAL__
    #include "wx/univ/toplevel.h"
#else     class WXDLLIMPEXP_CORE wxTopLevelWindow : public wxTopLevelWindowNative
    {
    public:
                wxTopLevelWindow() { }
        wxTopLevelWindow(wxWindow *parent,
                   wxWindowID winid,
                   const wxString& title,
                   const wxPoint& pos = wxDefaultPosition,
                   const wxSize& size = wxDefaultSize,
                   long style = wxDEFAULT_FRAME_STYLE,
                   const wxString& name = wxFrameNameStr)
            : wxTopLevelWindowNative(parent, winid, title,
                                     pos, size, style, name)
        {
        }

        wxDECLARE_DYNAMIC_CLASS_NO_COPY(wxTopLevelWindow);
    };
#endif 
#endif 