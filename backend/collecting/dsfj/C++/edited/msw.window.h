
#ifndef _WX_WINDOW_H_
#define _WX_WINDOW_H_

#include "wx/settings.h"        
class WXDLLIMPEXP_FWD_CORE wxButton;

#define wxUSE_DEFERRED_SIZING 1


class WXDLLIMPEXP_CORE wxWindowMSW : public wxWindowBase
{
    friend class wxSpinCtrl;
    friend class wxSlider;
    friend class wxRadioBox;
public:
    wxWindowMSW() { Init(); }

    wxWindowMSW(wxWindow *parent,
                wxWindowID id,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                long style = 0,
                const wxString& name = wxPanelNameStr)
    {
        Init();
        Create(parent, id, pos, size, style, name);
    }

    virtual ~wxWindowMSW();

    bool Create(wxWindow *parent,
                wxWindowID id,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                long style = 0,
                const wxString& name = wxPanelNameStr);

        virtual void SetLabel(const wxString& label);
    virtual wxString GetLabel() const;

    virtual void Raise();
    virtual void Lower();

        virtual double GetContentScaleFactor() const { return 1.0; }

    virtual bool BeginRepositioningChildren();
    virtual void EndRepositioningChildren();

    virtual bool Show(bool show = true);
    virtual bool ShowWithEffect(wxShowEffect effect,
                                unsigned timeout = 0)
    {
        return MSWShowWithEffect(true, effect, timeout);
    }
    virtual bool HideWithEffect(wxShowEffect effect,
                                unsigned timeout = 0)
    {
        return MSWShowWithEffect(false, effect, timeout);
    }

    virtual void SetFocus();
    virtual void SetFocusFromKbd();

    virtual bool Reparent(wxWindowBase *newParent);

    virtual void WarpPointer(int x, int y);

    virtual void Refresh( bool eraseBackground = true,
                          const wxRect *rect = (const wxRect *) NULL );
    virtual void Update();

    virtual void SetWindowStyleFlag(long style);
    virtual void SetExtraStyle(long exStyle);
    virtual bool SetCursor( const wxCursor &cursor );
    virtual bool SetFont( const wxFont &font );

    virtual int GetCharHeight() const;
    virtual int GetCharWidth() const;

    virtual void SetScrollbar( int orient, int pos, int thumbVisible,
                               int range, bool refresh = true );
    virtual void SetScrollPos( int orient, int pos, bool refresh = true );
    virtual int GetScrollPos( int orient ) const;
    virtual int GetScrollThumb( int orient ) const;
    virtual int GetScrollRange( int orient ) const;
    virtual void ScrollWindow( int dx, int dy,
                               const wxRect* rect = NULL );

    virtual bool ScrollLines(int lines);
    virtual bool ScrollPages(int pages);

    virtual void SetLayoutDirection(wxLayoutDirection dir);
    virtual wxLayoutDirection GetLayoutDirection() const;
    virtual wxCoord AdjustForLayoutDirection(wxCoord x,
                                             wxCoord width,
                                             wxCoord widthTotal) const;

    virtual void SetId(wxWindowID winid);

#if wxUSE_DRAG_AND_DROP
    virtual void SetDropTarget( wxDropTarget *dropTarget );
#endif 
        virtual void DragAcceptFiles(bool accept);

#ifndef __WXUNIVERSAL__
            virtual bool LoadNativeDialog(wxWindow* parent, wxWindowID id);
    virtual bool LoadNativeDialog(wxWindow* parent, const wxString& name);
    wxWindow* GetWindowChild1(wxWindowID id);
    wxWindow* GetWindowChild(wxWindowID id);
#endif 
#if wxUSE_HOTKEY
        virtual bool RegisterHotKey(int hotkeyId, int modifiers, int keycode);
    virtual bool UnregisterHotKey(int hotkeyId);
#endif 
        
    WXHWND GetHWND() const { return m_hWnd; }
    void SetHWND(WXHWND hWnd) { m_hWnd = hWnd; }
    virtual WXWidget GetHandle() const { return GetHWND(); }

    void AssociateHandle(WXWidget handle);
    void DissociateHandle();

        bool IsSizeDeferred() const;

                            
                    typedef bool (*MSWMessageHandler)(wxWindowMSW *win,
                                      WXUINT nMsg,
                                      WXWPARAM wParam,
                                      WXLPARAM lParam);

        static bool MSWRegisterMessageHandler(int msg, MSWMessageHandler handler);

        static void MSWUnregisterMessageHandler(int msg, MSWMessageHandler handler);


        
        
    void OnPaint(wxPaintEvent& event);

public:
        void SubclassWin(WXHWND hWnd);
    void UnsubclassWin();

    WXFARPROC MSWGetOldWndProc() const { return m_oldWndProc; }
    void MSWSetOldWndProc(WXFARPROC proc) { m_oldWndProc = proc; }

                bool IsOfStandardClass() const { return m_oldWndProc != NULL; }

    wxWindow *FindItem(long id, WXHWND hWnd = NULL) const;
    wxWindow *FindItemByHWND(WXHWND hWnd, bool controlOnly = false) const;

        virtual bool ContainsHWND(WXHWND WXUNUSED(hWnd)) const { return false; }

#if wxUSE_TOOLTIPS
        virtual bool HasToolTips() const { return GetToolTip() != NULL; }
#endif 
                        virtual WXDWORD MSWGetStyle(long flags, WXDWORD *exstyle = NULL) const ;

                    WXDWORD MSWGetCreateWindowFlags(WXDWORD *exflags = NULL) const
        { return MSWGetStyle(GetWindowStyle(), exflags); }

                void MSWUpdateStyle(long flagsOld, long exflagsOld);

        virtual WXHWND MSWGetParent() const;

        static const wxChar *MSWGetRegisteredClassName();

                    bool MSWCreate(const wxChar *wclass,
                   const wxChar *title = NULL,
                   const wxPoint& pos = wxDefaultPosition,
                   const wxSize& size = wxDefaultSize,
                   WXDWORD style = 0,
                   WXDWORD exendedStyle = 0);

    virtual bool MSWCommand(WXUINT param, WXWORD id);

#ifndef __WXUNIVERSAL__
        virtual wxWindow* CreateWindowFromHWND(wxWindow* parent, WXHWND hWnd);

        virtual void AdoptAttributesFromHWND();
#endif 
        virtual void SetupColours();

                    
    void UnpackCommand(WXWPARAM wParam, WXLPARAM lParam,
                       WXWORD *id, WXHWND *hwnd, WXWORD *cmd);
    void UnpackActivate(WXWPARAM wParam, WXLPARAM lParam,
                        WXWORD *state, WXWORD *minimized, WXHWND *hwnd);
    void UnpackScroll(WXWPARAM wParam, WXLPARAM lParam,
                      WXWORD *code, WXWORD *pos, WXHWND *hwnd);
    void UnpackCtlColor(WXWPARAM wParam, WXLPARAM lParam,
                        WXHDC *hdc, WXHWND *hwnd);
    void UnpackMenuSelect(WXWPARAM wParam, WXLPARAM lParam,
                          WXWORD *item, WXWORD *flags, WXHMENU *hmenu);

                
                        
        virtual bool MSWOnScroll(int orientation, WXWORD nSBCode,
                             WXWORD pos, WXHWND control);

        virtual bool MSWOnNotify(int idCtrl, WXLPARAM lParam, WXLPARAM *result);

        virtual bool MSWOnDrawItem(int id, WXDRAWITEMSTRUCT *item);
    virtual bool MSWOnMeasureItem(int id, WXMEASUREITEMSTRUCT *item);

        bool HandleCreate(WXLPCREATESTRUCT cs, bool *mayCreate);
    bool HandleInitDialog(WXHWND hWndFocus);
    bool HandleDestroy();

    bool HandlePaint();
    bool HandlePrintClient(WXHDC hDC);
    bool HandleEraseBkgnd(WXHDC hDC);

    bool HandleMinimize();
    bool HandleMaximize();
    bool HandleSize(int x, int y, WXUINT flag);
    bool HandleSizing(wxRect& rect);
    bool HandleGetMinMaxInfo(void *mmInfo);
    bool HandleEnterSizeMove();
    bool HandleExitSizeMove();

    bool HandleShow(bool show, int status);
    bool HandleActivate(int flag, bool minimized, WXHWND activate);

    bool HandleCommand(WXWORD id, WXWORD cmd, WXHWND control);

    bool HandleCtlColor(WXHBRUSH *hBrush, WXHDC hdc, WXHWND hWnd);

    bool HandlePaletteChanged(WXHWND hWndPalChange);
    bool HandleQueryNewPalette();
    bool HandleSysColorChange();
    bool HandleDisplayChange();
    bool HandleCaptureChanged(WXHWND gainedCapture);
    virtual bool HandleSettingChange(WXWPARAM wParam, WXLPARAM lParam);

    bool HandleQueryEndSession(long logOff, bool *mayEnd);
    bool HandleEndSession(bool endSession, long logOff);

    bool HandleSetFocus(WXHWND wnd);
    bool HandleKillFocus(WXHWND wnd);

    bool HandleDropFiles(WXWPARAM wParam);

    bool HandleMouseEvent(WXUINT msg, int x, int y, WXUINT flags);
    bool HandleMouseMove(int x, int y, WXUINT flags);
    bool HandleMouseWheel(wxMouseWheelAxis axis,
                          WXWPARAM wParam, WXLPARAM lParam);

    bool HandleChar(WXWPARAM wParam, WXLPARAM lParam);
    bool HandleKeyDown(WXWPARAM wParam, WXLPARAM lParam);
    bool HandleKeyUp(WXWPARAM wParam, WXLPARAM lParam);
#if wxUSE_HOTKEY
    bool HandleHotKey(WXWPARAM wParam, WXLPARAM lParam);
#endif
    int HandleMenuChar(int chAccel, WXLPARAM lParam);
        bool HandleClipboardEvent( WXUINT nMsg );

    bool HandleQueryDragIcon(WXHICON *hIcon);

    bool HandleSetCursor(WXHWND hWnd, short nHitTest, int mouseMsg);

    bool HandlePower(WXWPARAM wParam, WXLPARAM lParam, bool *vetoed);


                                    virtual bool MSWHandleMessage(WXLRESULT *result,
                                  WXUINT message,
                                  WXWPARAM wParam,
                                  WXLPARAM lParam);

            virtual WXLRESULT MSWWindowProc(WXUINT nMsg, WXWPARAM wParam, WXLPARAM lParam);

        virtual WXLRESULT MSWDefWindowProc(WXUINT nMsg, WXWPARAM wParam, WXLPARAM lParam);

    
            virtual bool MSWShouldPreProcessMessage(WXMSG* pMsg);

        virtual bool MSWProcessMessage(WXMSG* pMsg);

        virtual bool MSWTranslateMessage(WXMSG* pMsg);

        virtual void MSWDestroyWindow();


                        
                            virtual void MSWAdjustBrushOrg(int* WXUNUSED(xOrg),
                                   int* WXUNUSED(yOrg)) const
    {
    }

                virtual WXHBRUSH MSWGetCustomBgBrush() { return 0; }

                                                    virtual WXHBRUSH MSWGetBgBrushForChild(WXHDC hDC, wxWindowMSW *child);

            WXHBRUSH MSWGetBgBrush(WXHDC hDC);

    enum MSWThemeColour
    {
        ThemeColourText = 0,
        ThemeColourBackground,
        ThemeColourBorder
    };

            wxColour MSWGetThemeColour(const wchar_t *themeName,
                               int themePart,
                               int themeState,
                               MSWThemeColour themeColour,
                               wxSystemColour fallback) const;

                    virtual bool MSWPrintChild(WXHDC WXUNUSED(hDC), wxWindow * WXUNUSED(child))
    {
        return false;
    }

                virtual bool MSWShouldPropagatePrintChild()
    {
        return true;
    }

                            virtual bool MSWHasInheritableBackground() const
    {
        return InheritsBackgroundColour();
    }

#if !defined(__WXUNIVERSAL__)
    #define wxHAS_MSW_BACKGROUND_ERASE_HOOK
#endif

#ifdef wxHAS_MSW_BACKGROUND_ERASE_HOOK
                            void MSWSetEraseBgHook(wxWindow *child);

        bool MSWHasEraseBgHook() const;

            virtual bool MSWEraseBgHook(WXHDC WXUNUSED(hDC)) { return false; }
#endif 
        bool MSWShowWithEffect(bool show,
                           wxShowEffect effect,
                           unsigned timeout);

        void OnSysColourChanged(wxSysColourChangedEvent& event);

        void InitMouseEvent(wxMouseEvent& event, int x, int y, WXUINT flags);

        bool IsMouseInWindow() const;

        virtual bool IsDoubleBuffered() const;

    void SetDoubleBuffered(bool on);

        void GenerateMouseLeave();

            virtual void OnInternalIdle();

#if wxUSE_MENUS && !defined(__WXUNIVERSAL__)
    virtual bool HandleMenuSelect(WXWORD nItem, WXWORD nFlags, WXHMENU hMenu);

        bool HandleMenuPopup(wxEventType evtType, WXHMENU hMenu);

        virtual bool DoSendMenuOpenCloseEvent(wxEventType evtType, wxMenu* menu);

        virtual wxMenu* MSWFindMenuFromHMENU(WXHMENU hMenu);
#endif 
            static wxButton* MSWGetDefaultButtonFor(wxWindow* win);

                    static bool MSWClickButtonIfPossible(wxButton* btn);

protected:
                virtual wxBorder GetDefaultBorderForControl() const;

        virtual wxBorder GetDefaultBorder() const;

            virtual wxBorder TranslateBorder(wxBorder border) const;

#if wxUSE_MENUS_NATIVE
    virtual bool DoPopupMenu( wxMenu *menu, int x, int y );
#endif 
        WXHWND                m_hWnd;

        WXFARPROC             m_oldWndProc;

        bool                  m_mouseInWindow:1;
    bool                  m_lastKeydownProcessed:1;

        int                   m_xThumbSize;
    int                   m_yThumbSize;

        virtual void DoGetTextExtent(const wxString& string,
                                 int *x, int *y,
                                 int *descent = NULL,
                                 int *externalLeading = NULL,
                                 const wxFont *font = NULL) const;
    virtual void DoClientToScreen( int *x, int *y ) const;
    virtual void DoScreenToClient( int *x, int *y ) const;
    virtual void DoGetPosition( int *x, int *y ) const;
    virtual void DoGetSize( int *width, int *height ) const;
    virtual void DoGetClientSize( int *width, int *height ) const;
    virtual void DoSetSize(int x, int y,
                           int width, int height,
                           int sizeFlags = wxSIZE_AUTO);
    virtual void DoSetClientSize(int width, int height);

    virtual wxSize DoGetBorderSize() const;

    virtual void DoCaptureMouse();
    virtual void DoReleaseMouse();

    virtual void DoEnable(bool enable);

    virtual void DoFreeze();
    virtual void DoThaw();

                            bool DoMoveSibling(WXHWND hwnd, int x, int y, int width, int height);

                    virtual void DoMoveWindow(int x, int y, int width, int height);

#if wxUSE_TOOLTIPS
    virtual void DoSetToolTip( wxToolTip *tip );

            bool HandleTooltipNotify(WXUINT code,
                             WXLPARAM lParam,
                             const wxString& ttip);
#endif 
                void InitAnyKeyEvent(wxKeyEvent& event,
                         WXWPARAM wParam,
                         WXLPARAM lParam) const;

                        wxKeyEvent CreateKeyEvent(wxEventType evType,
                              WXWPARAM wParam,
                              WXLPARAM lParam = 0) const;

                    wxKeyEvent CreateCharEvent(wxEventType evType,
                               WXWPARAM wParam,
                               WXLPARAM lParam) const;


            bool DoEraseBackground(WXHDC hDC);

                            void MSWUpdateUIState(int action, int state = 0);

            virtual void MSWGetCreateWindowCoords(const wxPoint& pos,
                                          const wxSize& size,
                                          int& x, int& y,
                                          int& w, int& h) const;

    bool MSWEnableHWND(WXHWND hWnd, bool enable);

                    virtual wxWindow* MSWFindItem(long WXUNUSED(id), WXHWND WXUNUSED(hWnd)) const
    {
        return NULL;
    }

private:
        void Init();

        bool HandleMove(int x, int y);
    bool HandleMoving(wxRect& rect);
    bool HandleJoystickEvent(WXUINT msg, int x, int y, WXUINT flags);
    bool HandleNotify(int idCtrl, WXLPARAM lParam, WXLPARAM *result);

#ifndef __WXUNIVERSAL__
                bool MSWSafeIsDialogMessage(WXMSG* msg);
#endif 
#if wxUSE_DEFERRED_SIZING
protected:
        virtual void MSWEndDeferWindowPos()
    {
        m_pendingPosition = wxDefaultPosition;
        m_pendingSize = wxDefaultSize;
    }

        WXHANDLE m_hDWP;

                wxPoint     m_pendingPosition;
    wxSize      m_pendingSize;
#endif 
private:
    wxDECLARE_DYNAMIC_CLASS(wxWindowMSW);
    wxDECLARE_NO_COPY_CLASS(wxWindowMSW);
    wxDECLARE_EVENT_TABLE();
};

class wxWindowCreationHook
{
public:
    wxWindowCreationHook(wxWindowMSW *winBeingCreated);
    ~wxWindowCreationHook();
};

#endif 