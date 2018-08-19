
#ifndef _WX_GTK_WINDOW_H_
#define _WX_GTK_WINDOW_H_

#include "wx/dynarray.h"

#ifdef __WXGTK3__
    typedef struct _cairo cairo_t;
    typedef struct _GtkStyleProvider GtkStyleProvider;
    typedef struct _GtkCssProvider GtkCssProvider;
    #define WXUNUSED_IN_GTK2(x) x
    #define WXUNUSED_IN_GTK3(x)
#else
    #define WXUNUSED_IN_GTK2(x)
    #define WXUNUSED_IN_GTK3(x) x
#endif

typedef struct _GdkEventKey GdkEventKey;
typedef struct _GtkIMContext GtkIMContext;

WX_DEFINE_EXPORTED_ARRAY_PTR(GdkWindow *, wxArrayGdkWindows);

extern "C"
{

typedef void (*wxGTKCallback)();

}


class WXDLLIMPEXP_CORE wxWindowGTK : public wxWindowBase
{
public:
            wxWindowGTK();
    wxWindowGTK(wxWindow *parent,
                wxWindowID id,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                long style = 0,
                const wxString& name = wxPanelNameStr);
    bool Create(wxWindow *parent,
                wxWindowID id,
                const wxPoint& pos = wxDefaultPosition,
                const wxSize& size = wxDefaultSize,
                long style = 0,
                const wxString& name = wxPanelNameStr);
    virtual ~wxWindowGTK();

        
    virtual void Raise() wxOVERRIDE;
    virtual void Lower() wxOVERRIDE;

    virtual bool Show( bool show = true ) wxOVERRIDE;
    virtual bool IsShown() const wxOVERRIDE;

    virtual bool IsRetained() const wxOVERRIDE;

    virtual void SetFocus() wxOVERRIDE;

        virtual void SetCanFocus(bool canFocus) wxOVERRIDE;

    virtual bool Reparent( wxWindowBase *newParent ) wxOVERRIDE;

    virtual void WarpPointer(int x, int y) wxOVERRIDE;

    virtual void Refresh( bool eraseBackground = true,
                          const wxRect *rect = (const wxRect *) NULL ) wxOVERRIDE;
    virtual void Update() wxOVERRIDE;
    virtual void ClearBackground() wxOVERRIDE;

    virtual bool SetBackgroundColour( const wxColour &colour ) wxOVERRIDE;
    virtual bool SetForegroundColour( const wxColour &colour ) wxOVERRIDE;
    virtual bool SetCursor( const wxCursor &cursor ) wxOVERRIDE;
    virtual bool SetFont( const wxFont &font ) wxOVERRIDE;

    virtual bool SetBackgroundStyle(wxBackgroundStyle style) wxOVERRIDE ;
    virtual bool IsTransparentBackgroundSupported(wxString* reason = NULL) const wxOVERRIDE;

    virtual int GetCharHeight() const wxOVERRIDE;
    virtual int GetCharWidth() const wxOVERRIDE;
    virtual double GetContentScaleFactor() const wxOVERRIDE;

    virtual void SetScrollbar( int orient, int pos, int thumbVisible,
                               int range, bool refresh = true ) wxOVERRIDE;
    virtual void SetScrollPos( int orient, int pos, bool refresh = true ) wxOVERRIDE;
    virtual int GetScrollPos( int orient ) const wxOVERRIDE;
    virtual int GetScrollThumb( int orient ) const wxOVERRIDE;
    virtual int GetScrollRange( int orient ) const wxOVERRIDE;
    virtual void ScrollWindow( int dx, int dy,
                               const wxRect* rect = NULL ) wxOVERRIDE;
    virtual bool ScrollLines(int lines) wxOVERRIDE;
    virtual bool ScrollPages(int pages) wxOVERRIDE;

#if wxUSE_DRAG_AND_DROP
    virtual void SetDropTarget( wxDropTarget *dropTarget ) wxOVERRIDE;
#endif 
    virtual void AddChild( wxWindowBase *child ) wxOVERRIDE;
    virtual void RemoveChild( wxWindowBase *child ) wxOVERRIDE;

    virtual void SetLayoutDirection(wxLayoutDirection dir) wxOVERRIDE;
    virtual wxLayoutDirection GetLayoutDirection() const wxOVERRIDE;
    virtual wxCoord AdjustForLayoutDirection(wxCoord x,
                                             wxCoord width,
                                             wxCoord widthTotal) const wxOVERRIDE;

    virtual bool DoIsExposed( int x, int y ) const wxOVERRIDE;
    virtual bool DoIsExposed( int x, int y, int w, int h ) const wxOVERRIDE;

        void SetDoubleBuffered(bool on);
    virtual bool IsDoubleBuffered() const wxOVERRIDE;

        virtual void SetLabel(const wxString& label) wxOVERRIDE { m_gtkLabel = label; }
    virtual wxString GetLabel() const wxOVERRIDE            { return m_gtkLabel; }

        
    virtual WXWidget GetHandle() const wxOVERRIDE { return m_widget; }

            virtual void OnInternalIdle() wxOVERRIDE;

        void OnIdle(wxIdleEvent& WXUNUSED(event)) {}

        bool PreCreation( wxWindowGTK *parent, const wxPoint &pos, const wxSize &size );
    void PostCreation();

        void DoAddChild(wxWindowGTK *child);

        #ifdef __WXGTK3__
    void GTKSendPaintEvents(cairo_t* cr);
#else
    void GTKSendPaintEvents(const GdkRegion* region);
#endif

                            virtual GtkWidget* GetConnectWidget();
    void ConnectWidget( GtkWidget *widget );

                int GTKCallbackCommonPrologue(struct _GdkEventAny *event) const;

                bool GTKShouldIgnoreEvent() const;


                    virtual bool GTKProcessEvent(wxEvent& event) const;

        static wxLayoutDirection GTKGetLayout(GtkWidget *widget);
    static void GTKSetLayout(GtkWidget *widget, wxLayoutDirection dir);

            void GTKReleaseMouseAndNotify();
    static void GTKHandleCaptureLost();

    GdkWindow* GTKGetDrawingWindow() const;

    bool GTKHandleFocusIn();
    bool GTKHandleFocusOut();
    void GTKHandleFocusOutNoDeferring();
    static void GTKHandleDeferredFocusOut();

            virtual void GTKHandleRealized();
    void GTKHandleUnrealize();

protected:
                virtual bool GTKNeedsToFilterSameWindowFocus() const { return false; }

                    virtual bool GTKWidgetNeedsMnemonic() const;
    virtual void GTKWidgetDoSetMnemonic(GtkWidget* w);

                                virtual GdkWindow *GTKGetWindow(wxArrayGdkWindows& windows) const;

        bool GTKIsOwnWindow(GdkWindow *window) const;

public:
        PangoContext   *GTKGetPangoDefaultContext();

#if wxUSE_TOOLTIPS
        virtual void GTKApplyToolTip(const char* tip);
#endif 
            void GTKShowOnIdle() { m_showOnIdle = true; }

        bool GTKShowFromOnIdle();

        virtual bool GTKIsTransparentForMouse() const { return false; }

        wxEventType GTKGetScrollEventType(GtkRange* range);

        int                  m_x, m_y;
    int                  m_width, m_height;
    int m_clientWidth, m_clientHeight;
                bool m_useCachedClientSize;
        bool m_isGtkPositionValid;

        GtkWidget           *m_widget;              GtkWidget           *m_wxwindow;        
        wxString             m_gtkLabel;

        bool IsOfStandardClass() const { return m_wxwindow == NULL; }

        GtkWidget           *m_focusWidget;

    void GTKDisableFocusOutEvent();
    void GTKEnableFocusOutEvent();


    
                                GtkIMContext* m_imContext;

            GdkEventKey* m_imKeyEvent;

                    virtual int GTKIMFilterKeypress(GdkEventKey* event) const;

                            bool GTKOnInsertText(const char* text);

            bool GTKDoInsertTextFromIM(const char* text);


        enum ScrollDir { ScrollDir_Horz, ScrollDir_Vert, ScrollDir_Max };

        GtkRange* m_scrollBar[ScrollDir_Max];

        double m_scrollPos[ScrollDir_Max];

            static ScrollDir ScrollDirFromOrient(int orient)
    {
        return orient == wxVERTICAL ? ScrollDir_Vert : ScrollDir_Horz;
    }

        static int OrientFromScrollDir(ScrollDir dir)
    {
        return dir == ScrollDir_Horz ? wxHORIZONTAL : wxVERTICAL;
    }

        ScrollDir ScrollDirFromRange(GtkRange *range) const;

    void GTKUpdateCursor(
        bool isBusyOrGlobalCursor = false,
        bool isRealize = false,
        const wxCursor* overrideCursor = NULL);

        bool                 m_noExpose:1;              bool                 m_nativeSizeEvent:1;       bool                 m_isScrolling:1;           bool                 m_clipPaintRegion:1;       bool                 m_dirtyTabOrder:1;                                                         bool                 m_mouseButtonDown:1;
    bool                 m_showOnIdle:1;            bool m_needCursorReset:1;

    wxRegion             m_nativeUpdateRegion;  
protected:
        virtual void DoGetTextExtent(const wxString& string,
                                 int *x, int *y,
                                 int *descent = NULL,
                                 int *externalLeading = NULL,
                                 const wxFont *font = NULL) const wxOVERRIDE;
    virtual void DoClientToScreen( int *x, int *y ) const wxOVERRIDE;
    virtual void DoScreenToClient( int *x, int *y ) const wxOVERRIDE;
    virtual void DoGetPosition( int *x, int *y ) const wxOVERRIDE;
    virtual void DoGetSize( int *width, int *height ) const wxOVERRIDE;
    virtual void DoGetClientSize( int *width, int *height ) const wxOVERRIDE;
    virtual void DoSetSize(int x, int y,
                           int width, int height,
                           int sizeFlags = wxSIZE_AUTO) wxOVERRIDE;
    virtual void DoSetClientSize(int width, int height) wxOVERRIDE;
    virtual wxSize DoGetBorderSize() const wxOVERRIDE;
    virtual void DoMoveWindow(int x, int y, int width, int height) wxOVERRIDE;
    virtual void DoEnable(bool enable) wxOVERRIDE;

#if wxUSE_MENUS_NATIVE
    virtual bool DoPopupMenu( wxMenu *menu, int x, int y ) wxOVERRIDE;
#endif 
    virtual void DoCaptureMouse() wxOVERRIDE;
    virtual void DoReleaseMouse() wxOVERRIDE;

    virtual void DoFreeze() wxOVERRIDE;
    virtual void DoThaw() wxOVERRIDE;

    void GTKConnectFreezeWidget(GtkWidget* widget);
    void GTKFreezeWidget(GtkWidget *w);
    void GTKThawWidget(GtkWidget *w);
    void GTKDisconnect(void* instance);

#if wxUSE_TOOLTIPS
    virtual void DoSetToolTip( wxToolTip *tip ) wxOVERRIDE;
#endif 
                            void GTKCreateScrolledWindowWith(GtkWidget* view);

    virtual void DoMoveInTabOrder(wxWindow *win, WindowOrder move) wxOVERRIDE;
    virtual bool DoNavigateIn(int flags) wxOVERRIDE;


        void RealizeTabOrder();

#ifdef __WXGTK3__
            void ApplyCssStyle(GtkCssProvider* provider, const char* style);
#else                 GtkRcStyle* GTKCreateWidgetStyle();
#endif

    void GTKApplyWidgetStyle(bool forceStyle = false);

            virtual void DoApplyWidgetStyle(GtkRcStyle *style);

    void GTKApplyStyle(GtkWidget* widget, GtkRcStyle* style);

        static void GTKScrolledWindowSetBorder(GtkWidget* w, int style);

                    unsigned long GTKConnectWidget(const char *signal, wxGTKCallback callback);

    void ConstrainSize();

#ifdef __WXGTK3__
    static GdkWindow* GTKFindWindow(GtkWidget* widget);
    static void GTKFindWindow(GtkWidget* widget, wxArrayGdkWindows& windows);
#endif

private:
    void Init();

                virtual bool GTKNeedsParent() const { return !IsTopLevel(); }

    enum ScrollUnit { ScrollUnit_Line, ScrollUnit_Page, ScrollUnit_Max };

                        bool DoScrollByUnits(ScrollDir dir, ScrollUnit unit, int units);
    virtual void AddChildGTK(wxWindowGTK* child);

#ifdef __WXGTK3__
        cairo_t* m_paintContext;
        GtkStyleProvider* m_styleProvider;

public:
    cairo_t* GTKPaintContext() const
    {
        return m_paintContext;
    }
    void GTKSizeRevalidate();
#endif

    wxDECLARE_DYNAMIC_CLASS(wxWindowGTK);
    wxDECLARE_NO_COPY_CLASS(wxWindowGTK);
};

#endif 