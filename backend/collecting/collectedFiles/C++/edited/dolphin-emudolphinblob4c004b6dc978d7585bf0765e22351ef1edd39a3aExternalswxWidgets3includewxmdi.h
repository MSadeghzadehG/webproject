
#ifndef _WX_MDI_H_BASE_
#define _WX_MDI_H_BASE_

#include "wx/defs.h"

#if wxUSE_MDI

#include "wx/frame.h"
#include "wx/menu.h"

class WXDLLIMPEXP_FWD_CORE wxMDIParentFrame;
class WXDLLIMPEXP_FWD_CORE wxMDIChildFrame;
class WXDLLIMPEXP_FWD_CORE wxMDIClientWindowBase;
class WXDLLIMPEXP_FWD_CORE wxMDIClientWindow;


class WXDLLIMPEXP_CORE wxMDIParentFrameBase : public wxFrame
{
public:
    wxMDIParentFrameBase()
    {
        m_clientWindow = NULL;
        m_currentChild = NULL;
#if wxUSE_MENUS
        m_windowMenu = NULL;
#endif     }

    

#if wxUSE_MENUS
    virtual ~wxMDIParentFrameBase()
    {
        delete m_windowMenu;
    }
#endif 
        
        virtual wxMDIChildFrame *GetActiveChild() const
        { return m_currentChild; }
    virtual void SetActiveChild(wxMDIChildFrame *child)
        { m_currentChild = child; }


        wxMDIClientWindowBase *GetClientWindow() const { return m_clientWindow; }


        
#if wxUSE_MENUS
            wxMenu* GetWindowMenu() const { return m_windowMenu; }

                virtual void SetWindowMenu(wxMenu *menu)
    {
        if ( menu != m_windowMenu )
        {
            delete m_windowMenu;
            m_windowMenu = menu;
        }
    }
#endif 

        
    virtual void Cascade() { }
    virtual void Tile(wxOrientation WXUNUSED(orient) = wxHORIZONTAL) { }
    virtual void ArrangeIcons() { }
    virtual void ActivateNext() = 0;
    virtual void ActivatePrevious() = 0;

    

                                    virtual wxMDIClientWindow *OnCreateClient();

protected:
        virtual bool TryBefore(wxEvent& event) wxOVERRIDE;


                    wxMDIClientWindowBase *m_clientWindow;
    wxMDIChildFrame *m_currentChild;

#if wxUSE_MENUS
        wxMenu *m_windowMenu;
#endif };


class WXDLLIMPEXP_CORE wxMDIChildFrameBase : public wxFrame
{
public:
    wxMDIChildFrameBase() { m_mdiParent = NULL; }

    

        virtual void Activate() = 0;

                wxMDIParentFrame *GetMDIParent() const { return m_mdiParent; }

        wxMDIParentFrame *GetMDIParentFrame() const { return GetMDIParent(); }


                virtual bool IsTopLevel() const wxOVERRIDE { return false; }

                virtual bool IsTopNavigationDomain(NavigationKind kind) const wxOVERRIDE
    {
        switch ( kind )
        {
            case Navigation_Tab:
                return true;

            case Navigation_Accel:
                                                break;
        }

        return false;
    }

                virtual void Raise() wxOVERRIDE { Activate(); }

protected:
    wxMDIParentFrame *m_mdiParent;
};


class WXDLLIMPEXP_CORE wxTDIChildFrame : public wxMDIChildFrameBase
{
public:
    
#if wxUSE_STATUSBAR
                virtual wxStatusBar* CreateStatusBar(int WXUNUSED(number) = 1,
                                         long WXUNUSED(style) = 1,
                                         wxWindowID WXUNUSED(id) = 1,
                                         const wxString& WXUNUSED(name)
                                            = wxEmptyString) wxOVERRIDE
      { return NULL; }

    virtual wxStatusBar *GetStatusBar() const wxOVERRIDE
        { return NULL; }
    virtual void SetStatusText(const wxString &WXUNUSED(text),
                               int WXUNUSED(number)=0) wxOVERRIDE
        { }
    virtual void SetStatusWidths(int WXUNUSED(n),
                                 const int WXUNUSED(widths)[]) wxOVERRIDE
        { }
#endif 
#if wxUSE_TOOLBAR
                virtual wxToolBar *CreateToolBar(long WXUNUSED(style),
                                     wxWindowID WXUNUSED(id),
                                     const wxString& WXUNUSED(name)) wxOVERRIDE
        { return NULL; }
    virtual wxToolBar *GetToolBar() const wxOVERRIDE { return NULL; }
#endif 
        virtual void SetIcons(const wxIconBundle& WXUNUSED(icons)) wxOVERRIDE { }

        virtual wxString GetTitle() const wxOVERRIDE { return m_title; }
    virtual void SetTitle(const wxString& title) wxOVERRIDE = 0;

        virtual void Maximize(bool WXUNUSED(maximize) = true) wxOVERRIDE { }
    virtual bool IsMaximized() const wxOVERRIDE { return true; }
    virtual bool IsAlwaysMaximized() const wxOVERRIDE { return true; }
    virtual void Iconize(bool WXUNUSED(iconize) = true) wxOVERRIDE { }
    virtual bool IsIconized() const wxOVERRIDE { return false; }
    virtual void Restore() wxOVERRIDE { }

    virtual bool ShowFullScreen(bool WXUNUSED(show),
                                long WXUNUSED(style)) wxOVERRIDE { return false; }
    virtual bool IsFullScreen() const wxOVERRIDE { return false; }


                
    virtual void AddChild(wxWindowBase *child) wxOVERRIDE { wxWindow::AddChild(child); }

    virtual bool Destroy() wxOVERRIDE { return wxWindow::Destroy(); }

    #ifdef __WXMSW__
    virtual WXDWORD MSWGetStyle(long flags, WXDWORD *exstyle = NULL) const
    {
        return wxWindow::MSWGetStyle(flags, exstyle);
    }

    virtual WXHWND MSWGetParent() const
    {
        return wxWindow::MSWGetParent();
    }

    WXLRESULT MSWWindowProc(WXUINT message, WXWPARAM wParam, WXLPARAM lParam)
    {
        return wxWindow::MSWWindowProc(message, wParam, lParam);
    }
#endif 
protected:
    virtual void DoGetSize(int *width, int *height) const wxOVERRIDE
    {
        wxWindow::DoGetSize(width, height);
    }

    virtual void DoSetSize(int x, int y, int width, int height, int sizeFlags) wxOVERRIDE
    {
        wxWindow::DoSetSize(x, y, width, height, sizeFlags);
    }

    virtual void DoGetClientSize(int *width, int *height) const wxOVERRIDE
    {
        wxWindow::DoGetClientSize(width, height);
    }

    virtual void DoSetClientSize(int width, int height) wxOVERRIDE
    {
        wxWindow::DoSetClientSize(width, height);
    }

    virtual void DoMoveWindow(int x, int y, int width, int height) wxOVERRIDE
    {
        wxWindow::DoMoveWindow(x, y, width, height);
    }

        virtual void DoSetSizeHints(int WXUNUSED(minW), int WXUNUSED(minH),
                                int WXUNUSED(maxW), int WXUNUSED(maxH),
                                int WXUNUSED(incW), int WXUNUSED(incH)) wxOVERRIDE { }

    wxString m_title;
};


class WXDLLIMPEXP_CORE wxMDIClientWindowBase : public wxWindow
{
public:
    

            virtual bool CreateClient(wxMDIParentFrame *parent,
                              long style = wxVSCROLL | wxHSCROLL) = 0;
};


#ifndef wxUSE_GENERIC_MDI_AS_NATIVE
                #if defined(__WXMOTIF__) || \
        defined(__WXUNIVERSAL__)
        #define wxUSE_GENERIC_MDI_AS_NATIVE   1
    #else
        #define wxUSE_GENERIC_MDI_AS_NATIVE   0
    #endif
#endif 
#if wxUSE_GENERIC_MDI_AS_NATIVE
    #include "wx/generic/mdig.h"
#elif defined(__WXMSW__)
    #include "wx/msw/mdi.h"
#elif defined(__WXGTK20__)
    #include "wx/gtk/mdi.h"
#elif defined(__WXGTK__)
    #include "wx/gtk1/mdi.h"
#elif defined(__WXMAC__)
    #include "wx/osx/mdi.h"
#elif defined(__WXQT__)
    #include "wx/qt/mdi.h"
#endif

inline wxMDIClientWindow *wxMDIParentFrameBase::OnCreateClient()
{
    return new wxMDIClientWindow;
}

inline bool wxMDIParentFrameBase::TryBefore(wxEvent& event)
{
            if ( event.GetEventType() == wxEVT_MENU ||
            event.GetEventType() == wxEVT_UPDATE_UI )
    {
        wxMDIChildFrame * const child = GetActiveChild();
        if ( child )
        {
                                    wxWindow* const
                from = static_cast<wxWindow*>(event.GetPropagatedFrom());
            if ( !from || !from->IsDescendant(child) )
            {
                if ( child->ProcessWindowEventLocally(event) )
                    return true;
            }
        }
    }

    return wxFrame::TryBefore(event);
}

#endif 
#endif 